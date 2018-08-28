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

typedef struct module module;

typedef struct key_event{
  unsigned char key, state;
  }key_event;

typedef union{
  long int32;
  int bool;
  struct{int len; key_event *buf;}key_events;
  }jack_value;

typedef struct named_jack named_jack;
typedef struct jack jack;

struct out_terminal{
  jack *connections;
  int nconnections;
  jack_value value;
  module *parent_module;
  };

struct in_terminal{jack *connection; int state;};

struct jack{
  union{
    struct{named_jack *elements; int len;}bundle;
    struct{jack *elements; int len;}array;
    struct in_terminal in_terminal;
    struct out_terminal out_terminal;
    };
  jack_type type;
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
  jack input;
  jack output;
  int last_updated;
  void (*config)(module *);
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
jack *find_output(char *);
jack *find_input(char *);
int connect_jacks(jack *out, jack *in);

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
