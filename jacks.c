#define _GNU_SOURCE
#include<string.h>
#include<stdlib.h>
#include<stdarg.h>
#include"orgel.h"


int create_jack(jack *to, jack *template, module *m){
  int n, size;

  *to=*template;
  to->parent_module=m;
  switch(template->type){
    case TYPE_ARRAY:
      size=template->array->len+1;
      to->array=malloc(template->len*size*sizeof(jack));
      to->array_template=template->array;
      for(int i=0; i<template->len; ++i){
        create_jack(to->array+(i*size), template->array, m);
        to->array[i*size].parent_jack=to;
        }
      return 0;
    case TYPE_BUNDLE:
      for(int i=1; i<=template->len; ++i){
        create_jack(to+i, template+i, m);
        to[i].parent_jack=to;
        }
      return 0;
    default:
      if(template->dir==DIR_IN){
        to->value=to->default_value;
        return 0;
        }
      else{
        switch(template->type){
          case TYPE_EMPTY:
          case TYPE_LOGIC:
          case TYPE_VIRTUAL_CV:
            return 0;
          case TYPE_KEY_EVENTS:
            n=template->value.key_events.len;
            to->value.key_events.buf=malloc(sizeof(key_event[n]));
            to->value.key_events.len=0;
            to->value.key_events.bufsize=n;
            return 0;
          default:  // Shouldn't happen
            return 1;
          }
        }
    }
  }

void jack_vfortree(jack *j, void (*func)(jack *, va_list), va_list ap){
  if(j->type==TYPE_ARRAY){
    int size=j->array->len+1;
    for(int i=1; i<=j->len; ++i)
      jack_vfortree(j->array+(i*size), func, ap);
    }
  else if(j->type==TYPE_BUNDLE){
    for(int i=1; i<=j->len; ++i)
      jack_vfortree(j+i, func, ap);
    }
  va_list copy;
  va_copy(copy, ap);
  func(j, copy);
  va_end(copy);
  }

void jack_fortree(jack *j, void (*func)(jack *, va_list), ...){
  va_list ap;
  va_start(ap, func);
  jack_vfortree(j, func, ap);
  va_end(ap);
  }

int jack_foreach(jack *j, void (*func)(jack *, va_list), ...){
  if(j->type!=TYPE_ARRAY)return -1;
  int size=j->array->len+1;
  for(int i=1; i<=j->len; ++i){
    va_list ap;
    va_start(ap, func);
    func(j->array+(i*size), ap);
    va_end(ap);
    }
  return 0;
  }

void destroy_jack(jack *j){
  if(j->type==TYPE_ARRAY){
    int size=j->array->len+1;
    for(int i=0; i<j->len; ++i)
      destroy_jack(j->array+i*size);
    }
  else if(j->type==TYPE_BUNDLE){
    for(int i=0; i<j->len; ++i)
      destroy_jack(j+i);
    }
  else if(j->type==TYPE_KEY_EVENTS){
    free(j->value.key_events.buf);
    }
  free(j->name);
  free(j);
  }

int resize_jack(jack *j, int len){
  if(j->type!=TYPE_ARRAY)return -1;
  if(j->len==len)return 0;

  int size=j->array_template->len+1;

  if(j->len<len){  // Extend the array
    j->array=realloc(j->array, len*size*sizeof(jack));
    for(int i=0; i<(j->len*size); ++i)
      if(j->array[i].type!=TYPE_BUNDLE)
        for(int con=0; con<j->array[i].nconnections; ++con)
          j->array[i].connections[con]->connection=j->array+i;
    for(int i=j->len; i<len; ++i)
      create_jack(j->array+(i*size), j->array_template, j->parent_module);
    j->len=len;
    }
  else{            // Shorten the array
    for(int i=len; i<j->len; ++i)
      destroy_jack(j+(i*size));
    j->len=len;
    if(!len){
      free(j->array);
      j->array=0;
      }
    }

  return 0;
  }

char *jack_name(jack *j); /* *** */
int jack_id(int *to, jack *j); /* *** */

void show_jack(jack *j, int dir, int indent){
  switch(j->type){
    case TYPE_EMPTY:
      printf("empty\n");
      break;
    case TYPE_LOGIC:
      printf("logic\n");
      break;
    case TYPE_VIRTUAL_CV:
      printf("virtual_cv\n");
      break;
    case TYPE_NUMBER:
      printf("number\n");
      break;
    case TYPE_KEY_EVENTS:
      printf("key_events\n");
      break;
    case TYPE_ARRAY:
      printf("array(%d) of ", j->len);
      show_jack(j->array, dir, indent);
      break;
    case TYPE_BUNDLE:
      printf("{\n");
      for(int i=1; i<=j->len; ++i){
        printf("%*s%s: ", indent+2, "", j[i].name);
        show_jack(j+i, dir, indent+2);
        }
      printf("%*s}\n", indent+2, "");
      break;
    default:  // Shouldn't happen
      printf("%*s???\n", indent, "");
    }
  }

static jack *lookup(jack *j, char *name){
  for(int i=1; i<=j->len; ++i)
    if(!strcmp(name, j[i].name))return j+i;
  return 0;
  }

jack *find_jack(char *_pathstr, int dir){
  jack *j=0;
  char *pathstr=strdup(_pathstr);
  char **path=tokenise(pathstr, "/");
  module *m=find_module(path[0]);
  if(!m)goto done;
  j=(dir==DIR_IN)?m->input_ptr:m->output_ptr;
  for(int i=1; path[i]; ++i){
    if(j->type!=TYPE_BUNDLE){j=0; goto done;}
    int l=0, n, array_syntax;
    array_syntax=(sscanf(path[i], "%*[^[]%n[%d]", &l, &n)==1);
    path[i][l]=0;
    j=lookup(j, path[i]);
    if(!j)goto done;
    if(array_syntax){
      if(j->type!=TYPE_ARRAY || j->len<=n){j=0; goto done;}
      j=j->array+(n*(j->array->len+1));
      }
    }
done:
  free(path);
  free(pathstr);
  return j;
  }

int is_terminal(jack *j){
  if(j->type==TYPE_BUNDLE || j->type==TYPE_ARRAY)return 0;
  return 1;
  }

static void set_valchanged(jack *j){
  do{
    j->valchanged=1;
    j=j->parent_jack;
    }while(j);
  }

static void plugstatus(jack *j){
  if(j->plugstatus)
    j->plugstatus(j->parent_module, j);
  else if(j->parent_module->plugstatus)
    j->parent_module->plugstatus(j->parent_module, j);
  }

int disconnect_jack(jack *j){
  int size;
  switch(j->type){
    case TYPE_ARRAY:
      size=j->array->len+1;
      for(int i=0; i<j->len; ++i)
        disconnect_jack(j->array+i*size);
      break;
    case TYPE_BUNDLE:
      for(int i=0; i<j->len; ++i)
        disconnect_jack(j+i);
      break;
    default:
      if(j->dir==DIR_OUT){
        while(j->nconnections)
          disconnect_jack(j->connections[0]);
        }
      else{
        jack *output=j->connection;
        LOCK_NEST();
        j->connection=0;
        j->value=j->default_value;
        if(!output){set_valchanged(j); LOCK_UNNEST(); return 0;}
        plugstatus(j);
        int i;
        for(i=0; output->connections[i]!=j; ++i);
        output->connections[i]=output->connections[output->nconnections-1];
        --(output->nconnections);
        if(!output->nconnections){
          free(output->connections);
          output->connections=0;
          // *** remove if "set" command gets changed
          if(!output->parent_module){
            free(output);
            LOCK_UNNEST();
            return 0;
            }
          }
        plugstatus(output);
        LOCK_UNNEST();
      }
    }
  return 0;
  }

int connect_jacks(jack *output, jack *input){
  if(!is_terminal(output) || !is_terminal(input))return 1;
  if(output->type != input->type)return 1;
  if(input->connection){
    disconnect_jack(input);
    printf("(removed previous connection)\n");
    }
  LOCK_NEST();
  output->connections=realloc(
    output->connections, sizeof(jack *)*(output->nconnections+1)
    );
  output->connections[output->nconnections]=input;
  ++(output->nconnections);
  input->connection=output;
  input->value=output->value;
  plugstatus(input);
  plugstatus(output);
  LOCK_UNNEST();
  return 0;
  }

void set_output(jack *j){
  for(int i=0; i<j->nconnections; ++i){
    j->connections[i]->value=j->value;
    set_valchanged(j->connections[i]);
    }
  }

int resize_key_events(jack *j, int bufsize){
  LOCK_NEST();
  key_event *buf=realloc(j->value.key_events.buf, bufsize*sizeof(key_event));
  if(!buf){LOCK_UNNEST(); return 1;}
  j->value.key_events.buf=buf;
  j->value.key_events.bufsize=bufsize;
  for(int i=0; i<j->nconnections; ++i){
    j->connections[i]->value=j->value;
    plugstatus(j->connections[i]);
    }
  LOCK_UNNEST();
  return 0;
  }

const char help_connect[]="Connect an output jack to an input jack.\n"
                          "Usage: connect <output> <input>\n";

void cmd_connect(char **argv){
  if(!argv[1] || !argv[2]){
    printf("Too few parameters.\n");
    return;
    }
  jack *out=find_jack(argv[1], DIR_OUT);
  if(!out){
    printf("Output jack \"%s\" not found.\n", argv[1]);
    return;
    }
  jack *in=find_jack(argv[2], DIR_IN);
  if(!in){
    printf("Input jack \"%s\" not found.\n", argv[2]);
printf("(to be connected to %s)\n", argv[1]);
    return;
    }
  LOCK_NEST();
  if(connect_jacks(out, in))
    printf("Connection failed.\n");
  LOCK_UNNEST();
  }
