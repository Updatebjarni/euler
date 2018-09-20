#define _GNU_SOURCE
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include"orgel.h"
#include"orgel-io.h"

#include"keyboard.spec.c"


typedef struct keyboard_module{
  module;
  int left, right, transpose;
  }keyboard_module;

static void tick(module *_m, int elapsed){
  keyboard_module *m=(keyboard_module *)_m;
  int len=0;
  if(INPUT(m).connection){
    struct out_terminal *v=&(INPUT(m).connection->out_terminal);
    for(int i=0; i<v->key_events_value.len; ++i){
      key_event e=v->key_events_value.buf[i];
      if(e.key>=m->left && e.key<=m->right)
        OUTPUT(m).key_events_value.buf[len]=e;
        OUTPUT(m).key_events_value.buf[len].key+=m->transpose;
        ++len;
      }
    }
  OUTPUT(m).key_events_value.len=len;
  }

static void attention(jack *j){
  if(j==(jack *)&INPUT(j->parent_module)){
    if(j->in_terminal.connection){
      OUTPUT(j->parent_module).key_events_value.buf=
        realloc(
          OUTPUT(j->parent_module).key_events_value.buf,
          j->in_terminal.connection->out_terminal.key_events_value.bufsize
          );
      OUTPUT(j->parent_module).key_events_value.bufsize=
        j->in_terminal.connection->out_terminal.key_events_value.bufsize;
      }
    }
  }

class keyboard_class;

static module *create(char **argv){
// *** create keyregion split [note]
//       splits input into two sides, at note. without note, read key from mog.
//       this produces a module with two outputs "left" and "right"
//     create keyregion [range=left,right]
//       creates a module that has one output, with keys in the given range.
//       without range, reads two keys from mog.
  jack *source;
  long n=2;
  char *sourcename, *end;
  int i=0, *points;

  if(*argv && !strcmp(*argv, "split")){
    ++argv;
    if(!*argv)goto no_source;
    asprintf(&sourcename, "mog/%s", *argv);
    if(!(source=find_jack(sourcename, DIR_OUT)))goto no_source;
    ++argv;
    if(*argv && !strcmp(*argv, "into")){
      ++argv;
      if(!*argv)goto syntax_error;
      n=strtol(*argv, &end, 0);
      if(*end)goto syntax_error;
      ++argv;
      }
    else n=2;
    points=malloc((n-1)*sizeof(int));
    if(*argv && !strcmp(*argv, "at")){
      ++argv;
      while(*argv && i<(n-1)){
        if(strtonote(*argv, points+i))goto syntax_error;
        ++i;
        ++argv;
        if(*argv && !strcmp(*argv, ","))++argv;
        }
      }
    for(; i<(n-1); ++i){
      printf("Press key for split %d: ", i+1); fflush(stdout);
      points[i]=mog_grab_key();
      printf("%s\n", notetostr(points[i]));
      }
    return; // ***
    }
  else if(*argv && !strcmp(*argv, "range")){
    }
  else{
    printf("Specify one of \"range\" and \"split\".\n");
    return;
    }

/*
  for(; *argv; ++argv){
    if(!strcmp(*argv, "lower") || !strcmp(*argv, "upper") ||
       !strcmp(*argv, "pedal")){
      keyboard=*argv;
      continue;
      }
    if(**argv=='+' || **argv=='-'){
      transpose=strtol(*argv, &end, 0);
      if(*end)goto syntax_error;
      continue;
      }
    if(!strcmp(*argv, "split")){
      if(range)goto range_or_split;
      split=1;
      continue;
      }
    if(!strcmp(*argv, "range")){
      if(split)goto range_or_split;
      range=1;
      continue;
      }
    if(!strtocv(*argv, &key)){
      if(split){
        
        }
      else if(range){
        }
      else goto range_or_split;
      continue;
      }
*/

  module *m=malloc(sizeof(keyboard_module));
  default_module_init(m, &keyboard_class);
  OUTPUT(m).changed=1;
  return m;

  no_source:
  printf("Specify one of \"upper\", \"lower\", and \"pedal\".\n");
  return;
  syntax_error:
  printf("Syntax error.\n");
  return;
  }

class keyboard_class={
  "keyboard",                           // name
  "Plocka ut en del av ett klaviatur",  // descr
  &input, &output,                      // default_input, default_output
  tick,                                 // default_tick
  0, 0,                                 // default_destroy, default_config
  DYNAMIC_CLASS,                        // is_static
  create,                               // create
  0,                                    // create_counter
  0,                                    // default_debug
  attention                             // default_attention
  };
