#include<stdio.h>
#include<X11/IntrinsicP.h>
#include<Xm/XmP.h>
#include<Xm/Form.h>
#include<Xm/Label.h>
#include<Xm/Separator.h>
#include<Xm/RowColumn.h>
#include<Xm/Frame.h>
#include<Xm/ToggleB.h>
#include"MovableP.h"
#include"../orgel.h"



const char EuNmodulePtr[]="modulePtr";
const char EuCModulePtr[]="ModulePtr";

static XtTranslations parsed_handle_trans;

static char *handle_trans=
  "<Btn1Motion>: drag_handle()\n"
  "<Btn1Down>: click_handle()\n";

static XtActionsRec ActionsList[]={
  {"drag_handle", drag_handle},
  {"click_handle", click_handle}
  };

#define offset(field) XtOffsetOf(MovableRec, field)
static XtResource resources[]={
  {XmNtitle, XmCTitle, XmRString,
   sizeof(String), offset(movable.title), XtRImmediate, NULL},
  {EuNmodulePtr, EuCModulePtr, XmRPointer,
   sizeof(XtPointer), offset(movable.module_ptr), XtRImmediate, NULL}
  };

/* Definition for resources that need special processing in get values. */

static XmSyntheticResource syn_resources[]={
  };

MovableClassRec movableClassRec =
{
   {                    /* core_class fields    */
      (WidgetClass) &xmFormClassRec,   /* superclass   */
      "Movable",                /* class_name           */
      sizeof(MovableRec),       /* widget_size          */
      ClassInitialize,          /* class_initialize */
      ClassPartInitialize,      /* class init part proc */
      False,                    /* class_inited         */
      Initialize,               /* initialize           */
      (XtArgsProc)NULL,         /* initialize_notify    */
      XtInheritRealize,         /* realize              */
      ActionsList,              /* actions              */
      XtNumber(ActionsList),    /* num_actions          */
      resources,                /* resources            */
      XtNumber(resources),      /* num_resources        */
      NULLQUARK,                /* xrm_class            */
      True,                     /* compress_motion  */
      XtExposeCompressMaximal,  /* compress_exposure    */
      False,                    /* compress_enterleave  */
      False,                    /* visible_interest     */
      (XtWidgetProc)NULL,       /* destroy              */
      XtInheritResize,          /* resize               */
      XtInheritExpose,          /* expose               */
      InheritSetValues,         /* set_values           */
      (XtArgsFunc)NULL,         /* set_values_hook      */
      XtInheritSetValuesAlmost, /* set_values_almost    */
      (XtArgsProc)NULL,         /* get_values_hook      */
      (XtAcceptFocusProc)NULL,  /* accept_focus         */
      XtVersion,                /* version      */
      NULL,                     /* callback_private */
      XtInheritTranslations,    /* tm_table             */
      XtInheritQueryGeometry,   /* Query Geometry proc  */
      (XtStringProc)NULL,       /* disp accelerator     */
      NULL,                     /* extension            */    
   },

   {                    /* composite_class fields */
      XtInheritGeometryManager, /* geometry_manager       */
      XtInheritChangeManaged,   /* change_managed         */
      XtInheritInsertChild,     /* insert_child           */
      XtInheritDeleteChild,     /* delete_child           */
      NULL,                     /* extension              */
   },

   {                    /* constraint_class fields */
      NULL,                         /* constraint resource     */
      0,                            /* number of constraints   */
      sizeof(MovableConstraintRec), /* size of constraint      */
      NULL,                         /* initialization          */
      NULL,                         /* destroy proc            */
      NULL,                         /* set_values proc         */
      NULL,                         /* extension               */
   },

   {                        /* manager_class fields   */
      XtInheritTranslations,                /* translations           */
      NULL,                                 /* syn_resources          */
      0,                                    /* num_syn_resources      */
      NULL,                                 /* syn_cont_resources     */
      0,                                    /* num_syn_cont_resources */
      XmInheritParentProcess,               /* parent_process         */
      NULL,                                 /* extension              */
   },

   {                        /* bulletin_board_class fields */
      FALSE,                                /* always_install_accelerators */
      (XmGeoCreateProc)NULL,                /* geo_matrix_create  */
      XmInheritFocusMovedProc,              /* focus_moved_proc   */
      NULL,                                 /* extension          */
   },

   {                        /* form_class fields  */
      (XtPointer) NULL,                     /* extension          */
   },

   {                        /* movable_class fields */
   
   }
};

extern XtAppContext app;

static void click_handle(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  XButtonEvent *e=(XButtonEvent *)event;
  MovableWidget m=(MovableWidget)XtParent(widget);

  XRaiseWindow(XtDisplay(widget), XtWindow(XtParent(widget)));

  m->movable.offs_x=e->x;
  m->movable.offs_y=e->y;
  }

static void drag_handle(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  XMotionEvent *e=(XMotionEvent *)event;
  MovableWidget m=(MovableWidget)XtParent(widget);
  Position x, y;
  XtVaGetValues(XtParent(widget), XtNx, &x, XtNy, &y, NULL);

  XtMoveWidget(XtParent(widget), x+e->x-m->movable.offs_x,
                       y+e->y-m->movable.offs_y);

  }

WidgetClass movableWidgetClass=(WidgetClass)&movableClassRec;

static XtWidgetProc composite_insert_child;

static XtWidgetProc insert_child(Widget w){
  unsigned char top;
  XtVaGetValues(w, XmNtopAttachment, &top, NULL);
  if(top==XmATTACH_FORM){
    MovableWidget m=(MovableWidget)XtParent(w);
    if(m->movable.title_widget){
      XtVaSetValues(w, XmNtopAttachment, XmATTACH_WIDGET,
                       XmNtopWidget, m->movable.title_widget, NULL);
      }
    }
  composite_insert_child(w);
  }

static void ClassInitialize(void){
  parsed_handle_trans=XtParseTranslationTable(handle_trans);
  }

static void ClassPartInitialize(WidgetClass c){
  MovableWidgetClass wc = (MovableWidgetClass) c;
  composite_insert_child=wc->composite_class.insert_child;
  wc->composite_class.insert_child=insert_child;
  }

static Widget new_jackbox(jack *j, char *name, Widget previous){
  XmString str;
  Widget jackbox=XtVaCreateManagedWidget(
         "jackbox", xmFormWidgetClass, XtParent(previous),
//         XmNwidth, width-height/5, XmNheight, height*4/5,
         XmNverticalSpacing, 2,
         XmNhorizontalSpacing, 2,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, previous,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtraversalOn, FALSE,
         NULL);
  str=XmStringCreateLocalized(name);
  Widget checkbox=XtVaCreateManagedWidget(
          "checkbox", xmToggleButtonWidgetClass, jackbox,
          XmNtopAttachment, XmATTACH_FORM,
          XmNleftAttachment, XmATTACH_FORM,
          XmNbottomAttachment, XmATTACH_FORM,
          XmNlabelString, str,
          XmNforeground, 0x000000,
          XmNbackground, 0x00FF00,
          NULL);
  XmStringFree(str);
  return jackbox;
  }

static void Initialize(
  Widget req, Widget new_w, ArgList args, Cardinal *num_args
  ){
  MovableWidget mp=(MovableWidget)new_w;
  mp->movable.title_widget=0;
  XmString str=XmStringCreateLocalized(mp->movable.title);
  Widget title=XtVaCreateManagedWidget(
          "title", xmLabelWidgetClass, new_w,
          XmNtopAttachment, XmATTACH_FORM,
          XmNrightAttachment, XmATTACH_FORM,
          XmNleftAttachment, XmATTACH_FORM,
          XmNforeground, 0x000000,
          XmNbackground, 0x00FF00,
          XmNlabelString, str,
          NULL);
  mp->movable.title_widget=title;
  XmStringFree(str);
  XtOverrideTranslations(title , parsed_handle_trans);

  module *mod=mp->movable.module_ptr;
  
  Widget previous=title;

jack *j=mod->input_ptr;
char *s;
switch(j->type){
  case TYPE_EMPTY: new_jackbox(j, "empty", previous); break;
  case TYPE_VIRTUAL_CV: new_jackbox(j, "virtual cv", previous); break;
  case TYPE_LOGIC: new_jackbox(j, "logic", previous); break;
  case TYPE_NUMBER: new_jackbox(j, "number", previous); break;
  case TYPE_KEY_EVENTS: new_jackbox(j, "key events", previous); break;
  case TYPE_ARRAY:
    asprintf(&s, "array(%d) of ...", j->len);
//    dump_jack(j->array_template,
//              (j->array_template->type==TYPE_BUNDLE)?depth:0);
    new_jackbox(j, s, previous);
    free(s);
    break;
  case TYPE_BUNDLE:
    for(int i=1; i<=j->len; ++i){
      previous=new_jackbox(j, j[i].name, previous);
      }
    break;
  }

  }

static void Destroy(Widget w){
//  MovableWidget mp = (MovableWidget)w;
  }

static Boolean SetValues(
  Widget cw, Widget rw, Widget nw, ArgList args, Cardinal *num_args
  ){
//  NodeEditorWidget current = (NodeEditorWidget) cw;
//  NodeEditorWidget req = (NodeEditorWidget) rw;
//  NodeEditorWidget new_w = (NodeEditorWidget) nw;
//  NodeEditorPart        *newlp, *curlp, *reqlp;
  
  /* Get pointers to the label parts  */
//  newlp = &(new_w->nodeeditor);
//  curlp = &(current->nodeeditor);
//  reqlp = &(req->nodeeditor);
  
  return FALSE;
  }

Widget XmCreateMovable(
  Widget parent, char *name, Arg *arglist, Cardinal argCount
  ){
  return XtCreateWidget(name, movableWidgetClass, parent, arglist, argCount);
  }

Widget XmVaCreateMovable(Widget parent, char *name, ...){
  Widget w;
  va_list var;
  int count;
    
  va_start(var, name);
  count = XmeCountVaListSimple(var);
  va_end(var);

  va_start(var, name);
  w = XmeVLCreateWidget(
    name, movableWidgetClass, parent, False, var, count);
  va_end(var);   
  return w;
  }

Widget XmVaCreateManagedMovable(Widget parent, char *name, ...){
  Widget w = NULL;
  va_list var;
  int count;
    
  va_start(var, name);
  count = XmeCountVaListSimple(var);
  va_end(var);
    
  va_start(var, name);
  w = XmeVLCreateWidget(
    name, movableWidgetClass, parent, True, var, count);
  va_end(var);   
  return w;
  }
