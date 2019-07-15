#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdint.h>
#include<stdarg.h>
#include"orgel.h"


static char *infile;
static char *p, *this_line;
static int line, column, column_before;
static char **path;
static int pathlen;


void path_push(char *name){
  path=realloc(path, sizeof(path[0])*pathlen+1);
  path[pathlen]=name;
  ++pathlen;
  }

void path_pop(){
  --pathlen;
  }

char *dir(){
  return strcmp(path[0], "input")?"DIR_OUT":"DIR_IN";
  }

int apsprintf(char **to, char *fmt, ...){
  char *str;
  int ret;
  va_list ap;

  va_start(ap, fmt);
  ret=vasprintf(&str, fmt, ap);
  va_end(ap);
  if(*to){
    char *tmp=str;
    asprintf(&str, "%s%s", *to, tmp);
    free(tmp);
    free(*to);
    }
  *to=str;
  return ret;
  }

char *pathstr(char *extra){
  if(extra)path_push(extra);
  char *str=strdup(path[0]);
  for(int i=1; i<pathlen; ++i)
    apsprintf(&str, "_%s", path[i]);
  if(extra)path_pop();
  return str;
  }

void skip_ws(){
  while(1){
    switch(*p){
      case ' ': ++column; break;
      case '\t': column+=8; break;
      case '\n': ++line; column=0; this_line=p+1; break;
      default: return;
      }
    ++p;
    }
  }

char *lex_identifier(){
  int len=0;
  char *id;

  skip_ws();
  column_before=column;
  sscanf(p, "%*[A-Za-z]%n", &len); if(!len)return 0;
  sscanf(p, "%m[A-Za-z0-9_]%n", &id, &len);
  p+=len;
  column+=len;

  return id;
  }

char *lex_label(){
  skip_ws();
  column_before=column;
  char *psave=p, *id=lex_identifier();
  if(!id)return 0;
  if(*p!=':'){
    free(id);
    p=psave;
    column=column_before;
    return 0;
    }
  ++p;
  ++column;
  return id;
  }

int lex_keyword(char *keyword){
  skip_ws();
  column_before=column;
  char *psave=p, *id=lex_identifier();
  if(!id)return 0;
  if(!strcmp(id, keyword)){
    free(id);
    return 1;
    }
  free(id);
  p=psave;
  column=column_before;
  return 0;
  }

int lex_int32_value(int *to){
  skip_ws();
  int n=0;
  int matched=sscanf(p, "%i%n", to, &n);
  if(matched<1)return 0;  
  column+=n;
  p+=n;
  return 1;
  }

int lex_bool_value(int *to){
  skip_ws();
  column_before=column;
  if(lex_keyword("true") || lex_keyword("yes") || 
     lex_keyword("on") || lex_keyword("enable")){
    *to=1;
    return 1;
    }
  if(lex_keyword("false") || lex_keyword("no") ||
     lex_keyword("off") || lex_keyword("disable")){
    *to=0;
    return 1;
    }
  char *psave=p;
  if(lex_int32_value(to)){
    if(*to==0 || *to==1)return 1;
    p=psave;
    column=column_before;
    }
  return 0;
  }

int lex_ws_char(char c){
  skip_ws();
  if(*p!=c)return 0;
  ++p;
  ++column;
  return 1;
  }

char *typename(jack *j){
  if(j->type==TYPE_BUNDLE){
    char *str, *ps;
    ps=pathstr(0);
    asprintf(&str, "struct %s", ps);
    free(ps);
    return str;
    }
  char *suf;
  switch(j->type){
    case TYPE_EMPTY:      suf=""; break;
    case TYPE_LOGIC:      suf="_logic"; break;
    case TYPE_VIRTUAL_CV: suf="_virtual_cv"; break;
    case TYPE_NUMBER:     suf="_number"; break;
    case TYPE_KEY_EVENTS: suf="_key_events"; break;
    case TYPE_ARRAY:      suf=""; break;
    default: return 0;
    }
  char *str;
  asprintf(&str, "jack%s", suf);
  return str;
  }

char *typespec(jack *j){
  if(j->type==TYPE_BUNDLE){
    char *str=0, *ps;
    ps=pathstr(0);
    apsprintf(&str, "struct %s{\n  jack _;\n", ps);
    free(ps);
    for(int i=1; i<=j->len; ++i){
      char *type;
      if(j[i].type==TYPE_ARRAY){
        path_push(j[i].name);
//        type=pathstr(0);
        apsprintf(&str, "  union{%s *%s; jack _%s;};\n",
                        typename(j[i].array), j[i].name, j[i].name);
        path_pop();
        }
      else{
        type=typespec(j+i);
        apsprintf(&str, "  %s %s;\n", type, j[i].name);
        free(type);
        }
      }
    apsprintf(&str, "  }");
    return str;
    }
  return typename(j);
  }

char *initialiser(jack *j){
  char *str, *ps;
  switch(j->type){
    case TYPE_EMPTY:      str=strdup("{.type=TYPE_EMPTY"); break;
    case TYPE_LOGIC:      str=strdup("{.type=TYPE_LOGIC"); break;
    case TYPE_VIRTUAL_CV: str=strdup("{.type=TYPE_VIRTUAL_CV"); break;
    case TYPE_NUMBER:     str=strdup("{.type=TYPE_NUMBER"); break;
    case TYPE_KEY_EVENTS:
      asprintf(&str, "{.type=TYPE_KEY_EVENTS, ._value.key_events.len=%d",
                     j->_value.key_events.len);
      break;
    case TYPE_ARRAY:
      ps=pathstr(j->name);
      asprintf(&str, "{.type=TYPE_ARRAY, .len=%d,\n"
                     "    .array=(jack *)&%s_element",
                     j->len, ps);
      free(ps);
      break;
    case TYPE_BUNDLE:
      asprintf(&str, "{\n  ._={.type=TYPE_BUNDLE, .len=%d, .dir=%s}",
                     j->len, dir());
      for(int i=1; i<=j->len; ++i){
        char *s=initialiser(j+i);
        apsprintf(&str, ",\n  .%s%s=%s",
                        j[i].type==TYPE_ARRAY?"_":"", j[i].name, s);
        free(s);
        }
      apsprintf(&str, "\n  }");
      return str;
    }
  if(j->name)apsprintf(&str, ", .name=\"%s\"", j->name);
//apsprintf(&str, ", .id=%s", pathstr(j->name)); *** NUMERICAL ID?
  apsprintf(&str, ", .dir=%s", dir());
  apsprintf(&str, "}");
  return str;
  }

void define(jack *j, char *suf){
  char *type=typespec(j);
  char *name=pathstr(suf);
  char *init=initialiser(j);
  printf("static %s %s=%s;\n\n", type, name, init);
  free(init);
  free(name);
  free(type);
  }

jack *jack_stub(int type){
  jack *j=malloc(sizeof(jack));
  j->name=0;
  j->type=type;
  return j;
  }

jack *parse_jack(){
  skip_ws();
  if(lex_keyword("virtual_cv"))
    return jack_stub(TYPE_VIRTUAL_CV);
  else if(lex_keyword("logic"))
    return jack_stub(TYPE_LOGIC);
  else if(lex_keyword("number"))
    return jack_stub(TYPE_NUMBER);
  else if(lex_keyword("empty"))
    return jack_stub(TYPE_EMPTY);
  else if(lex_keyword("key_events")){
    jack *j=jack_stub(TYPE_KEY_EVENTS);
    if(lex_ws_char('(')){
      int n;
      if(!(lex_int32_value(&n) && lex_ws_char(')'))){
        free(j);
        return 0;
        }
      j->_value.key_events.len=n;
      }
    else
      j->_value.key_events.len=0;
    return j;
    }
  else if(lex_ws_char('{')){
    jack *j=jack_stub(TYPE_BUNDLE);
    int n=0;
    while(!lex_ws_char('}')){
      skip_ws();
      char *name=lex_label();
      if(!name){free(j); return 0;}
      path_push(name);
      jack *element=parse_jack();
      path_pop();
      if(!element){
        free(name);
        free(j);
        return 0;
        }  
      element->name=name;

      ++n;
      j=realloc(j, sizeof(jack[n+1]));
      j[n]=*element;
      free(element);
      }

    j->len=n;

    return j;
    }
  else if(lex_keyword("array")){
    jack *j=jack_stub(TYPE_ARRAY);
    int n;
    if(!(lex_ws_char('(') && lex_int32_value(&n) && lex_ws_char(')'))){
      free(j);
      return 0;
      }
    lex_keyword("of");
    skip_ws();
    jack *element=parse_jack();
//    path_push("element");
    if(element)define(element, "element");
//    path_pop();
    if(!element){
      free(j);
      return 0;
      }
    j->len=n;
    j->array=element;
    return j;
    }
  return 0;
  }

void error(){
  if(!*p)
    fprintf(stderr, "*** %s: premature end of file\n", infile);
  else{
    fprintf(stderr, "*** %s: line %d, column %d:\n", infile, line, column);
    fprintf(stderr, "    %.*s\n", (int)strcspn(this_line, "\n"), this_line);
    fprintf(stderr, "    %*s^\n", column, "");
    }
  exit(1);
  }

char temp_file_name[]="/tmp/jackspec-XXXXXX";

void remove_temp_file(){
  unlink(temp_file_name);
  }

int main(int n, char *arg[]){
  struct stat st;
  int ifd, ofd;
  char *buf;
  jack *j;
  char *outfile, *cp_cmd;

  if(n<3){
    fprintf(stderr, "*** Usage: %s <infile> <outfile>\n", arg[0]);
    return 1;
    }

  infile=arg[1];
  outfile=arg[2];
  ifd=open(infile, O_RDONLY);
  fstat(ifd, &st);
  buf=malloc(st.st_size+1);
  read(ifd, buf, st.st_size);
  buf[st.st_size]='\0';

  atexit(remove_temp_file);
  ofd=mkstemp(temp_file_name);
  close(1);
  dup2(ofd, 1);
  close(ofd);

  p=this_line=buf;
  line=1; column=0;

  for(int n=0; n<2; ++n){
    if(lex_keyword("input"))path_push("input");
    else if(lex_keyword("output"))path_push("output");
    else error();
    if(!(j=parse_jack()))error();
    define(j, 0);
    char *tn=typename(j), *ps=pathstr(0);
    printf("typedef %s %s_t;\n", tn, ps);
    free(ps); free(tn);
    path_pop();
    }

  fflush(stdout);

  asprintf(&cp_cmd, "cp '%s' '%s'", temp_file_name, outfile);
  system(cp_cmd);

  return 0;
  }
