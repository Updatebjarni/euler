#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include"orgel.h"

static char **toklist, *tok;
static int toklist_len, toklist_space, tok_len, tok_space;
static const char notenames[]="AABCCDDEFFGG";


char *notetostr(int note){
  static char buf[5];
  char *to=buf;
  int octave=note/12;
  note%=12;
  *to=notenames[note];
  ++to;
  if(note && notenames[note]==notenames[note-1])
    *(to++)='#';
  sprintf(to, "%d", octave);
  return buf;
  }

int strtonote(char *s, int *to){
  long note, octave;
  char *p;

  if((!*s) || !(p=strchr(notenames, *s)))return -1;
  note=p-notenames; ++s;
  if(*s=='b'){--note; ++s;}
  else if(*s=='#'){++note; ++s;}
  if(note<0)note+=12;
  if(!*s)return -1;
  octave=strtol(s, &p, 0);
  if(*p)return -1;
  *to=octave*12+note;
  return 0;
  }

int strtocv(char *s, long *to){
  int note;
  if(strtonote(s, &note))return -1;
  *to=note*HALFNOTE;
  return 0;
  }

/*
  char *name;
  int number, type;
  union{long intval; char *strval; jack *jackval; module *modval;};
  }paramspec;
*/

int parse_param(char **argv, paramspec *specs){
  char **p=argv;
  if(*p && !strcmp(*p, ","))++p;
  if(!*p)return -1;
  for(int i=0; specs[i].name; ++i)
    if(!strcmp(*p, specs[i].name)){
      ++p;
      if(!*p || strcmp(*p, "="))return -1;
      ++p;
      if(!*p && specs[i].type!=PARAM_FLAG)return -1;
      switch(specs[i].type){
        case PARAM_KEY:
        case PARAM_CV:
        case PARAM_NUMBER:
        case PARAM_FLAG:
        case PARAM_STRING:
        case PARAM_INPUT:
        case PARAM_OUTPUT:
        case PARAM_MODULE:
        default: return -1;
        }
      }
  return -1;
  }

static void new_tok(){
  tok_len=0;
  tok_space=16;
  tok=malloc(tok_space+1);
  }

static void add_to_tok(char c){
  if(tok_len==tok_space){
    tok_space+=16;
    tok=realloc(tok, tok_space+1);
    }

  tok[tok_len]=c;
  tok[tok_len+1]='\0';
  ++tok_len;
  }

static void add_tok(){
  if(!tok_len)return;

  if(toklist_len==toklist_space){
    toklist_space+=16;
    toklist=realloc(toklist, sizeof(*toklist)*(toklist_space+1));
    }

  toklist[toklist_len]=tok;
  toklist[toklist_len+1]=0;
  ++toklist_len;

  new_tok();
  }

void free_toklist(char **toklist, int len){
  int i;
  for(i=0; i<len; i++)
    free(toklist[i]);
  free(toklist);
  }

// Returns number of tokens, or negative in case of error.
// If 0 is returned, *to does not need to be freed.
int cmdlex(char ***to, char *str){
  char c;

  toklist_len=0;
  toklist_space=16;
  toklist=malloc(sizeof(*toklist)*(toklist_space+1));
  toklist[0]=0;

  new_tok();

  start:
  c=*str++;
  if(isspace(c)){
    add_tok();
    goto start;
    }
  if(!c){
  done:
    add_tok();
    if(toklist_len)
      *to=toklist;
    else{
      free(toklist);
      *to=0;
      }
    free(tok);
    return toklist_len;
    }
  if(strchr("()=,", c)){
    add_tok();
    add_to_tok(c);
    add_tok();
    goto start;
    }
  if(c=='"')goto double_quoted;
  if(c=='\'')goto single_quoted;
  if(c==';')goto done;

  if(c=='\\'){
    c=*str++;
    if(!c)goto error;
    }
  add_to_tok(c);
  goto start;

  double_quoted:
  c=*str++;
  if(c=='"')goto start;
  if(c=='\\')c=*str++;
  if(!c)goto error;
  add_to_tok(c);
  goto double_quoted;

  single_quoted:
  c=*str++;
  if(c=='\'')goto start;
  if(c=='\\')c=*str++;
  if(!c)goto error;
  add_to_tok(c);
  goto single_quoted;

  error:
  free(tok);
  free_toklist(toklist, toklist_len);
  return -1;
  }
