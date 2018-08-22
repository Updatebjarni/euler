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

enum{UNCHANGED, CHANGED, DISCONNECTED};

typedef enum{
  TYPE_EMPTY, TYPE_BUNDLE, TYPE_ARRAY,
  TYPE_BOOL, TYPE_INT32,
  TYPE_KEY_EVENTS
  }jack_type;

typedef struct key_event{
  unsigned char key, state;
  }key_event;

typedef union{
  long int32;
  int bool;
  struct{int len; key_event *buf;}key_events;
  }jack_value;

typedef struct output_jack output_jack;
struct output_jack{
  char *name;
  jack_type type;
  union{
    struct{int len; output_jack *elements;}bundle;
    struct{int len; output_jack *elements;}array;
    struct{
      struct module *parent_module;
      jack_value value;
      struct input_jack *connections;
      int nconnections;
      }terminal;
    };
  };

typedef struct input_jack input_jack;
struct input_jack{
  char *name;
  jack_type type;
  union{
    struct{int len; input_jack *elements;}bundle;
    struct{int len; input_jack *elements;}array;
    struct{int state; output_jack *connection;}terminal;
    };
  };

typedef struct module module;
struct module{
  struct class *type;
  char *name;
  void (*tick)(module *, int elapsed);
  void (*destroy)(module *);
  input_jack inputs;
  output_jack outputs;
  int last_updated;
  void (*config)(struct module *);  
  };

typedef struct class{
  char *name, *descr;
  int is_static;
  module *(*create)();
  int create_counter;
  }class;

void start_rt(void);
void stop_rt(void);
void run_module(module *);
void stop_module(module *);
module *find_module(char *name);
class *find_class(char *name);
module *create_module(char *class_name);
output_jack *find_output(char *);
input_jack *find_input(char *);
int connect_jacks(output_jack *, input_jack *);

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

int mog_grab_key();

#endif
