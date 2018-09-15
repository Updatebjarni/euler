#include<math.h>
#include<stdint.h>
#include"orgel.h"

#include"na.spec.c"


static void tick(module *m, int elapsed){
  if(INPUT(m)->signal.connection && INPUT(m)->control.connection){
    int64_t result=(int64_t)(INPUT(m)->signal.connection->value);
    result+=INPUT(m)->control.connection->value;
    if(result>INT32_MAX)result=INT32_MAX;
    if(result<INT32_MIN)result=INT32_MIN;
    OUTPUT(m).int32_value=result;
    }
  }

/*
static int init(module *m, char **argv){

  }
*/

class add_class={
  "add",                     // char *name
  "Add two signals",         // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  0,                         // int (*init)(module *, char **)
  0                          // int create_counter
  };
