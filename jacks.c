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
        to->in_terminal.state=DISCONNECTED;
        return 0;
        }
      else{
        to->out_terminal.connections=0;
        to->out_terminal.nconnections=0;
        to->out_terminal.parent_module=m;
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
