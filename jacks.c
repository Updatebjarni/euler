#define _GNU_SOURCE
#include<string.h>
#include<stdlib.h>
#include"orgel.h"


int create_jack(jack *to, jack *template, int dir, module *m){
  int n;

  *to=*template;
  switch(template->type){
    case TYPE_ARRAY:
      to->array.elements=malloc(sizeof(jack[template->array.len]));
      for(int i=0; i<template->array.len; ++i)
        create_jack(to->array.elements+i, template->array.elements, dir, m);
      return 0;
    case TYPE_BUNDLE:
      to->bundle.elements=malloc(sizeof(named_jack[template->bundle.len]));
      for(int i=0; i<template->bundle.len; ++i){
        to->bundle.elements[i].name=template->bundle.elements[i].name;
        create_jack(&(to->bundle.elements[i].element),
                    &(template->bundle.elements[i].element), dir, m);
        }
      return 0;
    default:
      if(dir==DIR_IN){
        to->in_terminal.connection=0;
        return 0;
        }
      else{
        to->out_terminal.connections=0;
        to->out_terminal.nconnections=0;
        to->out_terminal.parent_module=m;
        to->out_terminal.changed=0;
        switch(template->type){
          case TYPE_EMPTY:
          case TYPE_BOOL:
          case TYPE_INT32:
            return 0;
          case TYPE_KEY_EVENTS:
            n=template->out_terminal.value.key_events.len;
            to->out_terminal.value.key_events.buf=malloc(sizeof(key_event[n]));
            return 0;
          default:  // Shouldn't happen
            return 1;
          }
        }
    }
  }

void show_jack(jack *j, int dir, int indent){
  switch(j->type){
    case TYPE_EMPTY:
      printf("empty\n");
      break;
    case TYPE_BOOL:
      printf("bool\n");
      break;
    case TYPE_INT32:
      printf("int32\n");
      break;
    case TYPE_KEY_EVENTS:
      printf("key_events\n");
      break;
    case TYPE_ARRAY:
      printf("array(%d) of ", j->array.len);
      show_jack(j->array.elements, dir, indent);
      break;
    case TYPE_BUNDLE:
      printf("{\n");
      for(int i=0; i<j->bundle.len; ++i){
        printf("%*s%s: ", indent+2, "", j->bundle.elements[i].name);
        show_jack(&(j->bundle.elements[i].element), dir, indent+2);
        }
      printf("%*s}\n", indent+2, "");
      break;
    default:  // Shouldn't happen
      printf("%*s???\n", indent, "");
    }
  }

static jack *lookup(jack *j, char *name){
  named_jack *elements=j->bundle.elements;
  for(int i=0; i<j->bundle.len; ++i)
    if(!strcmp(name, elements[i].name))return &elements[i].element;
  return 0;
  }

jack *find_jack(char *_pathstr, int dir){
  char *pathstr=strdupa(_pathstr);
  char **path=tokenise(pathstr, "/");
  module *m=find_module(path[0]);
  if(!m)return 0;
  jack *j=(dir==DIR_IN)?(&(m->input)):(&(m->output));
  for(int i=1; path[i]; ++i){
    if(j->type!=TYPE_BUNDLE)return 0;
    int l=0, n, array_syntax;
    array_syntax=(sscanf(path[i], "%*[^[]%n[%d]", &l, &n)==1);
    path[i][l]=0;
    j=lookup(j, path[i]);
    if(!j)return 0;
    if(array_syntax){
      if(j->type!=TYPE_ARRAY || j->array.len<=n)return 0;
      j=j->array.elements+n;
      }
    }
  return j;
  }

int is_terminal(jack *j){
  if(j->type==TYPE_BUNDLE || j->type==TYPE_ARRAY)return 0;
  return 1;
  }

/*** Must be run only with modules locked ***/
int disconnect_input(jack *input){
  out_terminal *output;
  if(!is_terminal(input))return 1;
  output=(out_terminal *)input->in_terminal.connection;
  input->in_terminal.connection=0;
  int i;
  for(i=output->nconnections; output->connections[i]!=input; ++i);
  output->connections[i]=output->connections[output->nconnections-1];
  --(output->nconnections);
  return 0;
  }

/*** Must be run only with modules locked ***/
int disconnect_output(jack *_output){
  if(!is_terminal(_output))return 1;
  struct out_terminal *output=(struct out_terminal *)_output;
  for(int i=0; i<output->nconnections; ++i)
    ((struct in_terminal *)output->connections[i])->connection=0;
  free(output->connections);
  output->connections=0;
  output->nconnections=0;
  return 0;
  }

/*** Must be run only with modules locked ***/
int disconnect_tree(jack *tree){
  switch(tree->type){
    case TYPE_ARRAY:
      for(int i=0; i<tree->array.len; ++i)
        if(is_terminal(tree->array.elements+i))
          disconnect_output(tree->array.elements+i);
        else
          disconnect_tree(tree->array.elements+i);
      break;
    case TYPE_BUNDLE:
      for(int i=0; i<tree->bundle.len; ++i)
        if(is_terminal((jack *)(tree->bundle.elements+i)))
          disconnect_output((jack *)(tree->bundle.elements+i));
        else
          disconnect_tree((jack *)(tree->bundle.elements+i));
      break;
    default:
      disconnect_output(tree);
    }
  return 0;
  }

/*** Must be run only with modules locked ***/
int connect_jacks(jack *output, jack *input){
  if(!is_terminal(output) || !is_terminal(input))return 1;
  if(input->in_terminal.connection){
    disconnect_input(input);
    printf("(removed previous connection)\n");
    }
  if(output->type != input->type)return 1;
  output->out_terminal.connections=realloc(
    output->out_terminal.connections,
    sizeof(jack *)*(output->out_terminal.nconnections+1)
    );
  output->out_terminal.connections[output->out_terminal.nconnections]=input;
  ++(output->out_terminal.nconnections);
  input->in_terminal.connection=output;
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
  LOCK_MODULES();
  if(connect_jacks(out, in))
    printf("Connection failed.\n");
  UNLOCK_MODULES();
  }
