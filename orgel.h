#ifndef ORGEL_H
#define ORGEL_H

#include<stdio.h>

#define STATIC_CLASS  1
#define DYNAMIC_CLASS 0

/* int32: pitch, cutoff, amplitude, etc
   pitch/cutoff: 1V/octave, 440Hz @ 6V, 1V = 2^27 = 134217728
 */

#define A440     805306368L
#define OCTAVE   134217728L
#define HALFNOTE 11184811L

enum{CONNECTED, DISCONNECTED};

typedef enum{
  TYPE_EMPTY, TYPE_BUNDLE, TYPE_ARRAY,
  TYPE_BOOL, TYPE_INT32,
  TYPE_KEY_EVENTS
  }jack_type;

#define DIR_IN  0
#define DIR_OUT 1

typedef struct module module;

enum{KEY_DOWN, KEY_UP};

typedef struct key_event{
  unsigned char key, state;
  }key_event;

typedef union{
  long int32_value;
  int bool_value;
  struct{int len; key_event *buf; int bufsize;}key_events_value;
  }jack_value;

typedef struct named_jack named_jack;
typedef struct jack jack;

struct out_terminal{
  jack_value;
  jack **connections;
  int nconnections;
  int changed;
  };//out_terminal;

typedef struct in_terminal{jack *connection;} in_terminal;

struct jack{
  union{
    named_jack *bundle;
    jack *array;
    struct in_terminal in_terminal;
    struct out_terminal out_terminal;
    };
  int len;
  jack_type type;
  module *parent_module;
  void (*attention)(jack *);
  };

struct named_jack{
  jack element;
  char *name;
  };

struct module{
  struct class *type;
  char *name;
  void (*tick)(module *, int elapsed);
  void (*destroy)(module *);
  void (*config)(module *, char **);
  jack input;
  jack output;
  int last_updated;
  void (*debug)(module *);
  void (*attention)(jack *);
  void (*init)(module *);
  };

typedef struct class{
  char *name, *descr;
  jack *default_input, *default_output;
  void (*default_tick)(module *, int elapsed);
  void (*default_destroy)(module *);
  void (*default_config)(module *, char **);
  int is_static;
  module *(*create)(char **);
  int create_counter;
  void (*default_debug)(module *);
  void (*default_attention)(jack *);
  void (*default_init)(module *);
  }class;

enum{PARAM_KEY=1, PARAM_CV, PARAM_NUMBER, PARAM_FLAG, PARAM_STRING,
     PARAM_INPUT, PARAM_OUTPUT, PARAM_MODULE};

typedef struct{
  char *name;
  int number, type;
  union{long intval; char *strval; jack *jackval; module *modval;};
  }paramspec;

int parse_param(char **argv, paramspec *specs);

void run_cmdline(char *line);
char **tokenise(char *str, char *sep);
void start_rt(void);
void stop_rt(void);
void run_module(module *);
void stop_module(module *);
module *find_module(char *name);
class *find_class(char *name);
void default_module_init(module *m, class *c);
module *create_module(char *class_name, char **argv);
jack *find_jack(char *path, int dir);
int is_terminal(jack *);
int connect_jacks(jack *out, jack *in);
int disconnect_input(jack *input);
int disconnect_output(jack *_output);
int disconnect_tree(jack *tree);
int create_jack(jack *to, jack *template, int dir, module *m);
void show_jack(jack *j, int dir, int indent);
char *notetostr(int note);
int strtonote(char *s, int *to);
int strtocv(char *s, long *to);
void free_toklist(char **toklist, int len);
int cmdlex(char ***to, char *str);

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

extern int modules_lock;
#define TRY_LOCK_MODULES() \
        __atomic_exchange_n(&modules_lock, 1, __ATOMIC_SEQ_CST)
#define LOCK_MODULES() while(TRY_LOCK_MODULES());
#define UNLOCK_MODULES() __atomic_store_n(&modules_lock, 0, __ATOMIC_SEQ_CST)

extern int hardware_lock;
#define TRY_LOCK_HARDWARE() \
        __atomic_exchange_n(&hardware_lock, 1, __ATOMIC_SEQ_CST)
#define LOCK_HARDWARE() while(TRY_LOCK_HARDWARE());
#define UNLOCK_HARDWARE() __atomic_store_n(&hardware_lock, 0, __ATOMIC_SEQ_CST)

int mog_grab_key();

#endif
