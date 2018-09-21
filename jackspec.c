#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include"orgel.h"


static char *infile;
static char *p, *this_line;
static int line, column, column_before;
static int dir;
static char **path;
static int pathlen;

char *jack_value_spec(int type){
  switch(type){
    case TYPE_BOOL: return "int";
    case TYPE_INT32: return "long";
    case TYPE_KEY_EVENTS: return "struct{int len; key_event *buf;}";
    default: break;
    }
  return 0;
  }

int is_terminal(jack *j){
  if(j->type==TYPE_BUNDLE || j->type==TYPE_ARRAY)return 0;
  return 1;
  }

char *pathstr(char *extra){
  if(extra){
    path=realloc(path, pathlen+1);
    path[pathlen]=extra;
    ++pathlen;
    }
  char *str=strdup((dir==DIR_IN)?"input":"output"), *tmp;
  for(int i=0; i<pathlen; ++i){
    asprintf(&tmp, "%s_%s", str, path[i]);
    free(str);
    str=tmp;
    }
  if(extra)--pathlen;
  return str;
  }

char *jackstr(jack *j, char *name){
  char *str;
  switch(j->type){
    case TYPE_EMPTY: return strdup("{.type=TYPE_EMPTY}");
    case TYPE_BOOL: return strdup("{.type=TYPE_BOOL}");
    case TYPE_INT32: return strdup("{.type=TYPE_INT32}");
    case TYPE_KEY_EVENTS:
      if(dir==DIR_IN)return strdup("{.type=TYPE_KEY_EVENTS}");
      asprintf(&str, "{.type=TYPE_KEY_EVENTS, "
                      ".out_terminal.key_events_value.len=%d}",
                     j->out_terminal.key_events_value.len);
      return str;
    case TYPE_ARRAY:
      asprintf(&str, "{.type=TYPE_ARRAY, .len=%d,"
                     " .array=(jack *)&%s_template}",
               j->len, pathstr(name));
      return str;
    case TYPE_BUNDLE:
      asprintf(&str, "{.type=TYPE_BUNDLE, .len=%d,"
                     " .bundle=(named_jack *)&%s_bundle}",
               j->len, pathstr(name));
      return str;
    }
  return 0;  // Shouldn't happen
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

named_jack *parse_named();

jack *parse_jack(){
  jack *j=malloc(sizeof(jack));
  skip_ws();
  if(lex_keyword("int32")){
    j->type=TYPE_INT32;
    return j;
    }
  if(lex_keyword("bool")){
    j->type=TYPE_BOOL;
    return j;
    }
  if(lex_keyword("empty")){
    j->type=TYPE_EMPTY;
    return j;
    }
  if(lex_keyword("key_events")){
    j->type=TYPE_KEY_EVENTS;
    if(dir==DIR_IN)return j;
    int n;
    if(!(lex_ws_char('(') && lex_int32_value(&n) && lex_ws_char(')'))){
      free(j);
      return 0;
      }
    j->out_terminal.key_events_value.len=n;
    return j;
    }
  if(lex_ws_char('{')){
    int n=0;
    named_jack *list=0;
    while(!lex_ws_char('}')){
      named_jack *named=parse_named();
      if(!named){free(j); return 0;}
      list=realloc(list, sizeof(named_jack[n+1]));
      list[n]=*named;
      free(named);
      ++n;
      }
    j->type=TYPE_BUNDLE;
    j->len=n;
    j->bundle=list;

    char *s=pathstr(0);
    printf("static struct %s_bundle{\n", s);
    for(int i=0; i<n; ++i){
      if(list[i].element.type==TYPE_BUNDLE)
        printf("  union{ named_jack _%s; struct %s_bundle *%s; };\n",
               list[i].name, pathstr(list[i].name), list[i].name);
      else if(list[i].element.type==TYPE_ARRAY){
        if(is_terminal(list[i].element.array))
          if(dir==DIR_IN)
            printf("  union{  struct{ struct{ union{"
                   "struct {%s value;} *connection; struct jack;}ptr;"
                   " } *ptr; } %s; named_jack _%s; };\n",  // Oh my.
                   jack_value_spec(list[i].element.array->type),
                   list[i].name, list[i].name);
          else
            printf("  union{  struct{ struct{ "
                   "union{struct out_terminal; struct jack;}ptr;"
                   " } *ptr; } %s; named_jack _%s; };\n",
                   list[i].name, list[i].name);
        else
          printf("  union{ named_jack; struct %s_template *ptr; } %s;\n",
                 pathstr(list[i].name), list[i].name);
        }
      else
        if(dir==DIR_IN)
          printf("  union{ struct{struct{%s value;} *connection;} %s; "
                 "named_jack _%s;};\n",
                 jack_value_spec(list[i].element.type),
                 list[i].name, list[i].name);
      else
        printf("  union{struct out_terminal %s; named_jack _%s;};\n",
               list[i].name, list[i].name);
      }
    printf("  }%s_bundle={\n", s);
    for(int i=0; i<n; ++i){
      printf("%s  ", i?",\n":"");
      if(((list[i].element.type==TYPE_ARRAY) &&
         (list[i].element.array->type==TYPE_BUNDLE)))
        printf(".%s={.name=\"%s\", .element=", list[i].name, list[i].name);
      else
        printf("._%s={.name=\"%s\", .element=", list[i].name, list[i].name);
      char *js=jackstr(&list[i].element, list[i].name);
      printf(js);
      free(js);
      printf("}");
      }
    printf("\n  };\n");
    free(s);
    return j;
    }
  if(lex_keyword("array")){
    int n;
    if(!(lex_ws_char('(') && lex_int32_value(&n) && lex_ws_char(')'))){
      free(j);
      return 0;
      }
    lex_keyword("of");
    skip_ws();
    jack *e=parse_jack();
    if(!e){
      free(j);
      return 0;
      }
    char *ps=pathstr(0), *js=jackstr(e, 0);
    if(e->type==TYPE_BUNDLE)
      printf("static struct %s_template{\n"
             "  union{struct %s_bundle *ptr; jack j;};\n"
             "  }%s_template={\n"
             "  .j=%s\n"
             "  };\n", ps, ps, ps, js);
    else
      if(dir==DIR_IN)
        printf("static union{ struct{%s value;} *connection; struct jack; } "
               "%s_template=%s;\n", jack_value_spec(e->type), ps, js);
      else
        printf("static union{ struct out_terminal; struct jack; } "
               "%s_template=%s;\n", ps, js);
    free(ps); free(js);
    j->type=TYPE_ARRAY;
    j->len=n;
    j->array=e;
    return j;
    }

  free(j);
  return 0;
  }

named_jack *parse_named(){
  skip_ws();
  char *name=lex_label();
  if(!name)return 0;
  path=realloc(path, sizeof(path[0])*pathlen+1);
  path[pathlen]=name;
  ++pathlen;
  jack *element=parse_jack();
  --pathlen;
  if(!element){
    free(name);
    return 0;
    }  
  named_jack *named=malloc(sizeof(named_jack));
  named->name=name;
  named->element=*element;
  free(element);
  return named;
  }

void error(){
  if(!*p)
    fprintf(stderr, "*** %s: premature end of file\n", infile);
  else{
    fprintf(stderr, "*** %s: line %d, column %d:\n", infile, line, column);
    fprintf(stderr, "    %.*s\n", strcspn(this_line, "\n"), this_line);
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
  buf=malloc(st.st_size);
  read(ifd, buf, st.st_size);

  atexit(remove_temp_file);
  ofd=mkstemp(temp_file_name);
  close(1);
  dup2(ofd, 1);
  close(ofd);

  p=this_line=buf;
  line=1; column=0;
  dir=DIR_IN;

  for(int n=0; n<2; ++n){
    if(lex_keyword("in"))dir=DIR_IN;
    else if(lex_keyword("out"))dir=DIR_OUT;
    else error();
    if(!(j=parse_jack()))error();
    printf("static jack %s=%s;\n",
           (dir==DIR_IN)?"input":"output", jackstr(j, 0));
    printf("#define %s(m) ", (dir==DIR_IN)?"INPUT":"OUTPUT");
    char *name=(dir==DIR_IN)?"in":"out";
    if(j->type==TYPE_ARRAY)
      printf("((struct %sput_template *)m->%sput.array)\n", name, name);
    else if(j->type==TYPE_BUNDLE)
      printf("((struct %sput_bundle *)m->%sput.bundle)\n", name, name);
    else
      printf("(*(struct %s_terminal *)&(m->%sput))\n", name, name);
    }
  printf("#define INDEX(i) ptr[i].ptr\n");
  fflush(stdout);

  asprintf(&cp_cmd, "cp '%s' '%s'", temp_file_name, outfile);
  system(cp_cmd);

  return 0;
  }
