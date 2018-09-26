#include<math.h>
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

  if(!*s)return -1;
  note=strtol(s, &p, 0);
  if(!*p){*to=note; return 0;}
  if(!(p=strchr(notenames, *s)))return -1;
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
  long n;
  char *end;
  double d;

  n=strtol(s, &end, 0);
  if(!*end){*to=n; return 0;}
  if(!strcmp(end, "H")){*to=n*HALFNOTE; return 0;}
  d=strtod(s, &end);
  if(end!=s){
    if(!strcmp(end, "Hz")){
      *to=log2(d/6.875)*OCTAVE;
      return 0;
      }
    if(!strcmp(end, "V")){
      *to=d*VOLT;
      return 0;
      }
    return -1;
    }
  if(strtonote(s, &note))return -1;
  *to=note*HALFNOTE;
  return 0;
  }

int parse_param(char ***argv, paramspec *specs){
  char **p=*argv, *end;
  int note;
  long cv, n;
  jack *j;
  module *m;

  if(!p)return -1;
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
          if(strtonote(*p, &note))return -1;
          specs[i].intval=note;
          *argv=p+1;
          return specs[i].number;
        case PARAM_CV:
          if(strtocv(*p, &cv))return -1;
          specs[i].intval=cv;
          *argv=p+1;
          return specs[i].number;
        case PARAM_NUMBER:
          n=strtol(*p, &end, 0);
          if(!*end){
            specs[i].intval=n;
            *argv=p+1;
            return specs[i].number;
            }
          return -1;
        case PARAM_FLAG:
          n=strtol(*p, &end, 0);
          if(!*end && (n==0 || n==1)){
            specs[i].intval=n;
            *argv=p+1;
            return specs[i].number;
            }
          if(!strcmp(*p, "true") || !strcmp(*p, "t") ||
             !strcmp(*p, "on") || !strcmp(*p, "yes")){
            specs[i].intval=1;
            *argv=p+1;
            return specs[i].number;
            }
          if(!strcmp(*p, "false") || !strcmp(*p, "nil") ||
             !strcmp(*p, "off") || !strcmp(*p, "no")){
            specs[i].intval=0;
            *argv=p+1;
            return specs[i].number;
            }
          return -1;
        case PARAM_STRING:
          specs[i].strval=*p;
          *argv=p+1;
          return specs[i].number;
        case PARAM_INPUT:
          if(!(j=find_jack(*p, DIR_IN)))return -1;
          specs[i].jackval=j;
          *argv=p+1;
          return specs[i].number;
        case PARAM_OUTPUT:
          if(!(j=find_jack(*p, DIR_OUT)))return -1;
          specs[i].jackval=j;
          *argv=p+1;
          return specs[i].number;
        case PARAM_MODULE:
          if(!(m=find_module(*p)))return -1;
          specs[i].modval=m;
          *argv=p+1;
          return specs[i].number;
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
