#define _GNU_SOURCE
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include"orgel.h"
#include"orgel-io.h"

#include"keyboard.spec.c"


typedef struct keyboard_module{
  module;
  int map[256], n;
  int *transpose;
  }keyboard_module;

static void tick(module *_m, int elapsed){
  keyboard_module *m=(keyboard_module *)_m;
  for(int i=0; i<m->n; ++i)
    OUTPUT(m)->range.INDEX(i).key_events_value.len=0;
  if(INPUT(m).connection){
    struct out_terminal *v=&(INPUT(m).connection->out_terminal);
    for(int i=0; i<v->key_events_value.len; ++i){
      key_event e=v->key_events_value.buf[i];
      int range=m->map[e.key];
      e.key+=m->transpose[range];
      int pos=OUTPUT(m)->range.INDEX(range).key_events_value.len++;
      OUTPUT(m)->range.INDEX(range).key_events_value.buf[pos]=e;
      }
    }
  OUTPUT(m)->range.INDEX(0).changed=1;
  }

static void attention(jack *j){
  keyboard_module *m=(keyboard_module *)j->parent_module;

  if(j==(jack *)&INPUT(m)){
    if(INPUT(m).connection){
      for(int i=0; i<m->n; ++i)
        resize_key_events(
          &(OUTPUT(m)->range.INDEX(i)),
          INPUT(m).connection->out_terminal.key_events_value.bufsize
          );
      }
    }
  }

class keyboard_class;

static int intcmp(const void *a, const void *b){
  return (*(int *)a)-(*(int *)b);
  }

static module *create(char **argv){
  jack *source=0;
  long n=2;
  char *sourcename, *end;
  int i=0, *points;
  struct{unsigned char left, right;} *ranges;

  if(*argv){
    asprintf(&sourcename, "mog/%s", *argv);
    if((source=find_jack(*argv, DIR_OUT)) ||
       (source=find_jack(sourcename, DIR_OUT)))
      ++argv;
    free(sourcename);
    }

  if(*argv && !strcmp(*argv, "split")){
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
    qsort(points, n-1, sizeof(int), intcmp);
    ranges=malloc(sizeof(ranges[0])*n);
    ranges[0].left=0;
    ranges[n-1].right=255;
    for(int i=0; i<(n-1); ++i){
      ranges[i].right=points[i]-1;
      ranges[i+1].left=points[i];
      }
    free(points);
    }
  else if(*argv && !strcmp(*argv, "range")){
// ***
printf("range is unimplemented :(\n"); return 0;
    }
  else{
    printf("Specify one of \"range\" and \"split\".\n");
    return 0;
    }

  keyboard_module *m=malloc(sizeof(keyboard_module));
  default_module_init((module *)m, &keyboard_class);

  for(int key=0, range=0; key<256; ++key){
    if(key>ranges[range].right)++range;
    m->map[key]=range;
    }
  free(ranges);

  jack *rj=malloc(sizeof(jack[n]));
  for(int i=0; i<n; ++i)
    create_jack(rj+i, &output_range_template, DIR_OUT, m);
  jack *a=(jack *)&(OUTPUT(m)->range);
  a->array=rj;
  a->len=n;
  m->n=n;

  m->transpose=malloc(sizeof(int)*n);
  for(int i=0; i<n; ++i)
    m->transpose[i]=0;

  connect_jacks(source, &(m->input));

  return m;

  syntax_error:
  printf("Syntax error.\n");
  return 0;
  }

static void config(module *_m, char **argv){
  keyboard_module *m=(keyboard_module *)_m;
  int range, offset;
  char *end;

  if(argv && argv[0] && !strcmp(argv[0], "transpose") && argv[1]){
    ++argv;
    while(argv[0]){
      if(!strcmp(argv[0], ","))++argv;
      if(argv[0] && (range=strtol(argv[0], &end, 0), !*end) &&
         argv[1] && (offset=strtol(argv[1], &end, 0), !*end)){
        if(range<0 || range>=m->n){
          printf("Range must be between 0 and %d for module %s.\n",
                 m->n, m->name);
          return;
          }
        m->transpose[range]=offset;
        argv+=2;
        }
      else{
        printf("Syntax error.\n");
        return;
        }
      }
    }
  else printf("Use 'config keyboard(transpose <range> <offset>)' to "
              "transpose a range.\n");
  }

class keyboard_class={
  "keyboard",                           // name
  "Plocka ut en del av ett klaviatur",  // descr
  &input, &output,                      // default_input, default_output
  tick,                                 // default_tick
  0,                                    // default_destroy
  config,                               // default_config
  DYNAMIC_CLASS,                        // is_static
  create,                               // create
  0,                                    // create_counter
  0,                                    // default_debug
  attention                             // default_attention
  };
