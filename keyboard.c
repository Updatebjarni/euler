#define _GNU_SOURCE
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include"orgel.h"
#include"orgel-io.h"

#include"keyboard.spec.c"


typedef struct keyboard_module{
  MODULE_BASE
  int map[256], n;
  int *transpose;
  }keyboard_module;

static void plugstatus(module *_m, jack *j){
  keyboard_module *m=(keyboard_module *)_m;
  if(j!=&m->input)return;
  for(int i=0; i<m->n; ++i){
    resize_key_events(
      (jack *)&(m->output.range[i]),
      m->input.connection->value.bufsize
      );
    }
  }

static void tick(module *_m, int elapsed){
  keyboard_module *m=(keyboard_module *)_m;
  for(int i=0; i<m->n; ++i)
    m->output.range[i].value.len=0;
  for(int i=0; i<m->input.value.len; ++i){
    key_event e=m->input.value.buf[i];
    int range=m->map[e.key];
    e.key+=m->transpose[range];
    int pos=m->output.range[range].value.len++;
    m->output.range[range].value.buf[pos]=e;
    }
  set_output(&m->output.range[0]);
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
  char *dummy=0;
  if(!argv)argv=&dummy;
  char *kbdname=(*argv)?(*argv):"lower";

  asprintf(&sourcename, "mog/%s", kbdname);
  if((source=find_jack(kbdname, DIR_OUT)) ||
     (source=find_jack(sourcename, DIR_OUT)))
    ++argv;

  if(!source){
    fprintf(stderr, "Source \"%s\" not found.\n", sourcename);
    free(sourcename);
    return 0;
    }

  free(sourcename);

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
    n=1;
    ranges=malloc(sizeof(ranges[0])*1);
    ranges[0].left=0;
    ranges[0].right=255;
    }

fprintf(stderr, "hej\n");

  keyboard_module *m=malloc(sizeof(keyboard_module));
  base_module_init(m, &keyboard_class);

fprintf(stderr, "hej 2\n");
  for(int key=0, range=0; key<256; ++key){
    if(key>ranges[range].right)++range;
    m->map[key]=range;
    }
  free(ranges);

fprintf(stderr, "hej 3\n");
  resize_jack(&m->output._range, n);
  m->n=n;

fprintf(stderr, "hej 4\n");
  m->transpose=malloc(sizeof(int)*n);
  for(int i=0; i<n; ++i)
    m->transpose[i]=0;

fprintf(stderr, "hej 5: %p, %p\n", source, &(m->input));
  connect_jacks(source, &(m->input));

fprintf(stderr, "hej 6\n");

  return (module *)m;

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
  .name="keyboard",
  .descr="Plocka ut en del av ett klaviatur",
  .tick=tick,
  .destroy=0,
  .config=config,
  .is_static=DYNAMIC_CLASS,
  .create=create,
  .create_counter=0,
  .debug=0,
  .reset=0,
  .plugstatus=plugstatus
  };
