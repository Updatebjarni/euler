#ifndef ORGEL_H
#define ORGEL_H

#include<stdint.h>
#include<stdio.h>
#include<string.h>

#define STATIC_CLASS  1
#define DYNAMIC_CLASS 0

/* virtual cv: pitch, cutoff, amplitude, etc
   pitch/cutoff: 1V/octave, 440Hz @ 6V, 1V = 2^27 = 134217728
 */

#define VOLT     134217728L
#define OCTAVE   VOLT
#define A440     6*OCTAVE
#define HALFNOTE 11184811L
#define CVMAX    VOLT*10

enum{CONNECTED, DISCONNECTED};

typedef enum{
  TYPE_EMPTY, TYPE_BUNDLE, TYPE_ARRAY,
  TYPE_LOGIC, TYPE_VIRTUAL_CV, TYPE_NUMBER,
  TYPE_KEY_EVENTS
  }jack_type;

#define DIR_IN  0
#define DIR_OUT 1

typedef struct module module;
typedef struct jack jack;

enum{KEY_DOWN, KEY_UP};

typedef struct key_event{
  unsigned char key, state;
  }key_event;

typedef struct key_events{int len; key_event *buf; int bufsize;}key_events;
typedef union{
  int32_t virtual_cv;
  int32_t number;
  int logic;
  struct{int len; key_event *buf; int bufsize;}key_events;
  }jack_value;

#define JACK_TYPE(NAME, TYPE) \
typedef struct NAME{ \
  union{ \
    struct NAME *connection; \
    struct NAME **connections; \
    jack *array; \
    }; \
  uint32_t type:20, dir:1, valchanged:1; \
  char *name; \
  jack *parent_jack; \
  module *parent_module; \
  void (*plugstatus)(module *m, jack *j); \
  union{ \
    int len;  /* Array eller bundle */ \
    int nconnections;  /* Output terminal */ \
    }; \
  union{ \
    TYPE value; \
    jack_value _value; \
    }; \
  union{ \
    TYPE default_value; \
    jack_value _default_value; \
    jack *array_template; \
    }; \
  } NAME;

JACK_TYPE(jack, jack_value)
JACK_TYPE(jack_logic, int)
JACK_TYPE(jack_virtual_cv, int32_t)
JACK_TYPE(jack_key_events, key_events)
JACK_TYPE(jack_number, int32_t)

#define _MODULE_BASE \
  struct class *type; \
  char *name; \
  void (*tick)(module *, int elapsed); \
  void (*destroy)(module *); \
  void (*config)(module *, char **); \
  int last_updated; \
  void (*debug)(module *); \
  void (*reset)(module *); \
  void (*plugstatus)(module *m, jack *j); \
  jack *input_ptr, *output_ptr;

struct module{
  _MODULE_BASE
  };

#define MODULE_BASE \
  _MODULE_BASE \
  input_t input; \
  output_t output;

typedef struct class{
  char *name;                           // Software name of the class
  char *descr;                          // Human-meaningful short description
  void (*tick)(module *, int elapsed);  // Main function, real-time
  void (*destroy)(module *);            // Destructor for the class
  void (*config)(module *, char **);    // Called to reconfigure an instance
  int is_static;                        // Says if multiple instances allowed
  module *(*create)(char **);           // Constructor for the class
  int create_counter;                   // Number of instances existing
  void (*debug)(module *);              // Called to print debug output
  void (*reset)(module *);              // Called to put module in passive state
  void (*plugstatus)(module *m, jack *j);  // Called when a jack is (un)plugged
  }class;

enum{PARAM_KEY, PARAM_CV, PARAM_NUMBER, PARAM_FLAG, PARAM_STRING,
     PARAM_INPUT, PARAM_OUTPUT, PARAM_MODULE};

typedef struct{
  char *name;
  int number, type;
  union{long intval; char *strval; jack *jackval; module *modval;};
  }paramspec;

int parse_param(char ***argv, paramspec *specs);
int parse_parens(char ***argv, char ***paren);
void run_cmdline(char *line);
char **tokenise(char *str, char *sep);
void start_rt(void);
void stop_rt(void);
void run_module(module *);
void stop_module(module *);
void clear_all(void);
module *find_module(char *name);
class *find_class(char *name);
void base_module_init(module *m, class *c,
                      jack *input, jack *input_template,
                      jack *output, jack *output_template);
module *create_module(char *class_name, char **argv);
void destroy_module(module *m);
jack *find_jack(char *path, int dir);
int is_terminal(jack *);
int connect_jacks(jack *out, jack *in);
int disconnect_jack(jack *tree);
int create_jack(jack *to, jack *template, module *m);
void destroy_jack(jack *j);
int resize_jack(jack *j, int len);
void show_jack(jack *j, int dir, int indent);
void set_output(jack *j);
int resize_key_events(jack *j, int bufsize);
char *notetostr(int note);
int strtonote(char *s, int *to);
int strtocv(char *s, long *to);
int strtologic(char *s);
void free_toklist(char **toklist, int len);
int cmdlex(char ***to, char *str);
int gui_main(int argc, char *argv[]);

int e_min(int a, int b);
int e_max(int a, int b);

#ifdef COMPILING_MODULE
#define base_module_init(M, C) \
  base_module_init((module *)M, C, \
                   (jack *)&(M->input), (jack *)&input, \
                   (jack *)&(M->output),(jack *)&output)
#define is_terminal(j) is_terminal((jack *)(j))
#define connect_jacks(out, in) connect_jacks((jack *)(out), (jack *)(in))
#define disconnect_jack(j) disconnect_jack((jack *)(j))
#define create_jack(to, template, m) \
        create_jack((jack *)(to), (jack *)(template), (module *)(m))
#define resize_jack(j, len) resize_jack((jack *)(j), len)
#define show_jack(j, dir, indent) show_jack((jack *)(j), dir, indent)
#define set_output(j) set_output((jack *)(j))
#define resize_key_events(j, bufsize) resize_key_events((jack *)(j), bufsize)
#endif


extern class *all_classes[];
extern module **loaded_modules;

struct command{
  char *name;
  void (*function)(char **cmdline);
  const char *help;
  };
extern struct command commands[];
extern int ncommands;
void (*find_command(char *name))(char **);

FILE *columns(void);

extern int rt_lock, rt_lock_count;

#define TRY_LOCK_EXCL() __sync_val_compare_and_swap(&rt_lock, 0, -1)
#define UNLOCK() __atomic_store_n(&rt_lock, 0, __ATOMIC_SEQ_CST)

#define LOCK_NEST() {while(__sync_val_compare_and_swap(&rt_lock, 0, 1)==-1); \
                     ++rt_lock_count;}
#define LOCK_UNNEST() if(!--rt_lock_count)UNLOCK()

int mog_grab_key();

#endif
