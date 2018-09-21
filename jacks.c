#define _GNU_SOURCE
#include<string.h>
#include<stdlib.h>
#include"orgel.h"


int create_jack(jack *to, jack *template, int dir, module *m){
  int n;

  *to=*template;
  to->attention=m->attention;
  to->parent_module=m;
  switch(template->type){
    case TYPE_ARRAY:
      to->array=malloc(sizeof(jack[template->len]));
      for(int i=0; i<template->len; ++i)
        create_jack(to->array+i, template->array, dir, m);
      return 0;
    case TYPE_BUNDLE:
      to->bundle=malloc(sizeof(named_jack[template->len]));
      for(int i=0; i<template->len; ++i){
        to->bundle[i].name=template->bundle[i].name;
        create_jack(&(to->bundle[i].element),
                    &(template->bundle[i].element), dir, m);
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
        to->out_terminal.changed=0;
        switch(template->type){
          case TYPE_EMPTY:
          case TYPE_BOOL:
          case TYPE_INT32:
            return 0;
          case TYPE_KEY_EVENTS:
            n=template->out_terminal.key_events_value.len;
            to->out_terminal.key_events_value.buf=malloc(sizeof(key_event[n]));
            to->out_terminal.key_events_value.len=0;
            to->out_terminal.key_events_value.bufsize=n;
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
      printf("array(%d) of ", j->len);
      show_jack(j->array, dir, indent);
      break;
    case TYPE_BUNDLE:
      printf("{\n");
      for(int i=0; i<j->len; ++i){
        printf("%*s%s: ", indent+2, "", j->bundle[i].name);
        show_jack(&(j->bundle[i].element), dir, indent+2);
        }
      printf("%*s}\n", indent+2, "");
      break;
    default:  // Shouldn't happen
      printf("%*s???\n", indent, "");
    }
  }

static jack *lookup(jack *j, char *name){
  named_jack *elements=j->bundle;
  for(int i=0; i<j->len; ++i)
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
      if(j->type!=TYPE_ARRAY || j->len<=n)return 0;
      j=j->array+n;
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
  struct out_terminal *outterm;
  if(!is_terminal(input))return 1;
  jack *output=input->in_terminal.connection;
  outterm=&(output->out_terminal);
  input->in_terminal.connection=0;
  if(input->attention)input->attention(input);
  int i;
  for(i=outterm->nconnections; outterm->connections[i]!=input; ++i);
  outterm->connections[i]=outterm->connections[outterm->nconnections-1];
  --(outterm->nconnections);
  if(!outterm->nconnections){
    free(outterm->connections);
    outterm->connections=0;
    if(!output->parent_module){
      free(output);
      return 0;
      }
    }
  if(output->parent_module && output->attention)
    output->attention(output);
  return 0;
  }

/*** Must be run only with modules locked ***/
int disconnect_output(jack *output){
  if(!is_terminal(output))return 1;
  for(int i=0; i<output->out_terminal.nconnections; ++i){
    jack *input=output->out_terminal.connections[i];
    input->in_terminal.connection=0;
    if(input->attention)input->attention(input);
    }
  free(output->out_terminal.connections);
  output->out_terminal.connections=0;
  output->out_terminal.nconnections=0;
  if(output->attention)output->attention(output);
  return 0;
  }

/*** Must be run only with modules locked ***/
int disconnect_tree(jack *tree){
  switch(tree->type){
    case TYPE_ARRAY:
      for(int i=0; i<tree->len; ++i)
        if(is_terminal(tree->array+i))
          disconnect_output(tree->array+i);
        else
          disconnect_tree(tree->array+i);
      break;
    case TYPE_BUNDLE:
      for(int i=0; i<tree->len; ++i)
        if(is_terminal((jack *)(tree->bundle+i)))
          disconnect_output((jack *)(tree->bundle+i));
        else
          disconnect_tree((jack *)(tree->bundle+i));
      break;
    default:
      disconnect_output(tree);
    }
  return 0;
  }

/*** Must be run only with modules locked ***/
int connect_jacks(jack *output, jack *input){
  if(!is_terminal(output) || !is_terminal(input))return 1;
  if(output->type != input->type)return 1;
  if(input->in_terminal.connection){
    disconnect_input(input);
    printf("(removed previous connection)\n");
    }
  output->out_terminal.connections=realloc(
    output->out_terminal.connections,
    sizeof(jack *)*(output->out_terminal.nconnections+1)
    );
  output->out_terminal.connections[output->out_terminal.nconnections]=input;
  ++(output->out_terminal.nconnections);
  input->in_terminal.connection=output;
  if(output->parent_module && output->attention)output->attention(output);
  if(input->attention)input->attention(input);
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
