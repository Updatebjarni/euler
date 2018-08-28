#include"orgel.h"


static output_jack mog_outputs[]={
  {"lower-right", TYPE_KEY_EVENTS,
   .terminal={&module_object, {.key_events={0, mog_out}}, 0, 0}
   },
  {"lower-left", TYPE_KEY_EVENTS,
   .terminal={&module_object, {.key_events={0, mog_out}}, 0, 0}
   }
  };


static void tick(module *m, int elapsed){
  }

class mog_class;

static module module_object={
  &prio_class,   // struct class *type;
  0,             // char *name;
  tick,          // void (*tick)(module *, int elapsed);
  destroy,       // void (*destroy)(module *);
  {0, TYPE_KEY_EVENTS},  // inputs
  {0, TYPE_ARRAY, .array={2, prio_outputs}},  // outputs
  0,             // last_updated
  0              // config
  };

static output_jack out_bundle[]={
  {"monophone", TYPE_ARRAY, .array={1, elements}},
  {"multitrig", TYPE_BOOL, .terminal={parent, .bool=0, connections, 
  };

static module *mog_create(){
  module *m=malloc(sizeof(module));
  m->type=&prio_class;
  m->name=0;
  m->tick=tick;
  m->destroy=destroy;
  m->inputs={"events", TYPE_KEY_EVENTS, .terminal={STATE_DISCONNECTED, 0}};
  output_jack *monophone_array=malloc(sizeof(output_jack[1]));
  output_jack *monophone=malloc(sizeof(output_jack));
  monophone->name=strdup("monophone");
  monophone->type=TYPE_ARRAY;
  monophone->array={1, monophone_array};
  output_jack *multitrig=malloc(sizeof(output_jack));
  multitrig->name=strdup("multitrig");
  multitrig->type=TYPE_BOOL;
  multitrig->terminal={m, .bool=0, 0, 0};
  output_jack *out_bundle=malloc(sizeof(output_jack[2]));
  out_bundle[0]=monophone;
  out_bundle[1]=multitrig;
  m->outputs={0, TYPE_BUNDLE, .bundle={2, out_bundle}};
  m->last_updated=0;
  m->config=config;
  return m;
  }

class prio_class={
  "prio", "Note priority",
  DYNAMIC_CLASS,
  prio_create,
  0
  };
