#include<stdio.h>
#include<X11/IntrinsicP.h>
#include<Xm/XmP.h>
#include<Xm/Form.h>
#include<Xm/Label.h>
#include<Xm/Separator.h>
#include<Xm/RowColumn.h>
#include<Xm/Frame.h>
#include"NodeEditorP.h"
#include"Movable.h"
#include"../orgel.h"



static char trans[]="<Btn1Down>: save_coords()\n"
                    "<Btn1Up>: draw_line()\n"
                    "<Btn3Down>: bakklick()\n"
                    "<Motion>: flytta()\n"
                    "<Enter>: Enter()\n"
                    "<Leave>: Leave()\n"
                    "<Btn2Down>: popup_class_picker()\n";

static char *clicktrap_trans="<BtnDown>: cancel_class_picker()";
static XtTranslations parsed_clicktrap_trans;

static XtActionsRec ActionsList[]={
  {"save_coords", save_coords},
  {"draw_line", draw_line},
  {"bakklick", bakklick},
  {"skapa", skapa},
  {"flytta", flytta},
  { "Enter",    Enter},
  { "Leave",    Leave},
  {"popup_class_picker", popup_class_picker},
  {"cancel_class_picker", cancel_class_picker},
  {"conbtndown", conbtndown},
  {"conbtnup", conbtnup},
  {"conbtn3down", conbtn3down},
  {"class_clicked", class_clicked}
  };

static void save_coords(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  XButtonEvent *e=(XButtonEvent *)event;
  NodeEditorWidget ne=(NodeEditorWidget)widget;

  ne->nodeeditor.offs_x=e->x;
  ne->nodeeditor.offs_y=e->y;
  }

static void draw_line(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  XButtonEvent *e=(XButtonEvent *)event;
  NodeEditorWidget ne=(NodeEditorWidget)widget;

//  XDrawLine(XtDisplay(widget), XtWindow(widget), ne->nodeeditor.line_GC,
//    ne->nodeeditor.offs_x, ne->nodeeditor.offs_y, e->x, e->y);
  }

void cancel_class_picker(
  Widget widget, XEvent *event, String *args, Cardinal *num_args
  ){
  NodeEditorWidget nep=(NodeEditorWidget)widget;
  XDestroyWindow(XtDisplay(widget), nep->nodeeditor.clicktrap);
  XtVaSetValues(widget, XtNtranslations, nep->nodeeditor.previous_trans, NULL);
  XtUnmanageChild(nep->nodeeditor.class_picker);
  if(*num_args)
  NodeEditorAddNode(widget, args[0],
                    nep->nodeeditor.dest_x, nep->nodeeditor.dest_y);
  }

void popup_class_picker(
  Widget widget, XEvent *event, String *args, Cardinal *num_args
  ){
  NodeEditorWidget nep=(NodeEditorWidget)widget;
  XButtonEvent *e=(XButtonEvent *)event;

  nep->nodeeditor.dest_x=e->x;
  nep->nodeeditor.dest_y=e->y;

  Dimension width, height;
  XtVaGetValues(widget, XtNwidth, &width, XtNheight, &height, NULL);

  XtVaGetValues(widget,
                XtNtranslations, &nep->nodeeditor.previous_trans,
                NULL);

  XtVaSetValues(widget, XtNtranslations, parsed_clicktrap_trans, NULL);

  nep->nodeeditor.clicktrap=XCreateWindow(
    XtDisplay(widget), XtWindow(widget),
    0, 0, width, height,
    0, CopyFromParent, InputOnly,
    0, 0, 0);
  XMapWindow(XtDisplay(widget), nep->nodeeditor.clicktrap);

  nep->nodeeditor.class_picker=create_class_picker(widget);
  }

void bakklick(Widget widget, XEvent *event, String *args, Cardinal *num_args){
XButtonEvent *e=(XButtonEvent *)event;
if(e->subwindow){
  printf("%p\n", XtClass(XtWindowToWidget(e->display, e->subwindow)));
  printf("%s\n", (XtClass(XtWindowToWidget(e->display, e->subwindow))==xmPrimitiveWidgetClass)?"primitive":"not primitive");
  }
XtUngrabPointer(widget, CurrentTime);
  }

void flytta(Widget w, XEvent *event, String *args, Cardinal *num){
  XMotionEvent *e=(XMotionEvent *)event;
//  printf("weee %d,%d\n", (int)e->x, (int)e->y);
  }

void conbtndown(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  printf("btndown widget: %p\n", widget);
XtGrabPointer(XtParent(XtParent(widget)), True, PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
  }
void conbtnup(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  printf("btnup widget: %p\n", widget);
  }
void conbtn3down(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  printf("btn3, ungrab\n");
XtUngrabPointer(XtParent(XtParent(widget)), CurrentTime);
  }

static char *contrans="<Btn1Down>: conbtndown()\n"
                      "<Btn1Up>: conbtnup()\n"
                      "<Btn3Down>: conbtn3down()";

_XmForegroundColorDefault(
        Widget widget,
        int offset,
        XrmValue *value )
{
printf("hehu\n");
//   XmeGetDefaultPixel (widget, XmFOREGROUND, offset, value);
char *p=(char *)widget;
p+=offset;
Pixel *px=(Pixel *)p;
*p=0x00FF00;
}

static XtResource resources[]={
  {XmNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
     XtOffsetOf(NodeEditorRec, nodeeditor.foreground_pixel),
//XmRCallProc, (XtPointer) _XmForegroundColorDefault
XtRString, "#00FF00"
}
  };

/* Definition for resources that need special processing in get values. */

static XmSyntheticResource syn_resources[]={
  };

NodeEditorClassRec nodeEditorClassRec={
  {
    (WidgetClass) &compositeClassRec,    /* superclass         */
    "NodeEditor",                        /* class_name         */
    sizeof(NodeEditorRec),               /* widget_size         */
    ClassInitialize,                     /* class_initialize    */
    ClassPartInitialize,                 /* chained class init  */
    FALSE,                               /* class_inited         */
    Initialize,                          /* initialize         */
    NULL,                                /* initialize hook     */
    XtInheritRealize,                    /* realize         */
    ActionsList,                         /* actions         */
    XtNumber(ActionsList),               /* num_actions         */
    resources,                           /* resources         */
    XtNumber(resources),                 /* num_resources       */
    NULLQUARK,                           /* xrm_class         */
    TRUE,                                /* compress_motion     */
    XtExposeCompressMaximal,             /* compress_exposure   */
    TRUE,                                /* compress enter/exit */
    FALSE,                               /* visible_interest    */
    Destroy,                             /* destroy         */
    Resize,                              /* resize         */
    Redisplay,                           /* expose         */
    SetValues,                           /* set_values         */
    NULL,                                /* set values hook     */
    XtInheritSetValuesAlmost,            /* set values almost   */
    NULL,                                /* get values hook     */
    NULL,                                /* accept_focus         */
    XtVersion,                           /* version         */
    NULL,                                /* callback offsetlst  */
    trans,                                /* default trans       */
    QueryGeometry,                       /* query geo proc      */
    NULL,                                /* display accelerator */
    NULL                                 /* extension record    */
  },

  { /* CompositeClassPart */
    NULL,                                /* geometry_manager */
    NULL,                                /* change_managed */
    _XtInherit,                          /* insert_child */
    _XtInherit,                          /* delete_child */
    NULL                                 /* extension */
  },

  { /* NodeEditorClassPart */
  }
};

extern XtAppContext app;

void skapa(Widget widget, XEvent *event, String *args, Cardinal *num_args){
  static int i;
  static char name[20];

  XButtonEvent *e=(XButtonEvent *)event;

  sprintf(name, "foo %d", i++);
  NodeEditorAddNode(widget, name, e->x, e->y);
  }

WidgetClass nodeEditorWidgetClass=(WidgetClass)&nodeEditorClassRec;

void ClassInitialize(void){
  parsed_clicktrap_trans=XtParseTranslationTable(clicktrap_trans);
  }

/*
char* _XmCBNameActivate(){
  return XmNactivateCallback;
  }

char* _XmCBNameValueChanged(){
  return XmNvalueChangedCallback;
  }
*/

XtWidgetProc composite_insert_child;

XtWidgetProc insert_child(Widget w){
  printf("adding widget\n");
  composite_insert_child(w);
  }

void ClassPartInitialize(WidgetClass c){
  NodeEditorWidgetClass wc = (NodeEditorWidgetClass) c;
/*
  NodeEditorWidgetClass super =
    (NodeEditorWidgetClass)wc->core_class.superclass;
  
  if (wc->nodeeditor_class.setOverrideCallback == XmInheritSetOverrideCallback)
    wc->nodeeditor_class.setOverrideCallback = 
      super->nodeeditor_class.setOverrideCallback;
  
  if (wc->nodeeditor_class.translations == XtInheritTranslations)
    wc->nodeeditor_class.translations = super->nodeeditor_class.translations;
*/
  
//  _XmFastSubclassInit (c, XmLABEL_BIT); ???
  composite_insert_child=wc->composite_class.insert_child;
  wc->composite_class.insert_child=insert_child;
  }

void Resize(Widget wid){
//  NodeEditorWidget newlw = (NodeEditorWidget) wid;
//  NodeEditorPart *lp = &(newlw->nodeeditor);
  }

void Initialize(
  Widget req, Widget new_w, ArgList args, Cardinal *num_args
  ){
  NodeEditorWidget nep=(NodeEditorWidget)new_w;

  XGCValues values;
  XtGCMask  valueMask;

  valueMask = GCForeground | GCBackground | GCLineWidth | GCFunction;

  values.line_width=3;
  values.function=GXxor;

  values.foreground=nep->nodeeditor.foreground_pixel;
  values.background=nep->core.background_pixel;

  nep->nodeeditor.nconnections=0;
  nep->nodeeditor.connection=0;

  Pixel p;
  XtVaGetValues(new_w, XmNforeground, &p, NULL);
  nep->nodeeditor.line_GC=XtGetGC((Widget)nep, valueMask, &values);
  }

XtGeometryResult QueryGeometry(
  Widget widget, XtWidgetGeometry *intended, XtWidgetGeometry *desired
  ){
//  NodeEditorWidget lw = (NodeEditorWidget) widget;
  
  desired->width = XtWidth(widget);
  desired->height = XtHeight(widget);
  
  return XmeReplyToQueryGeometry(widget, intended, desired);
  }

void Destroy(Widget w){
//  NodeEditorWidget lw = (NodeEditorWidget) w;
  }

void Redisplay(Widget wid, XEvent *event, Region region
  ){
  NodeEditorWidget ne=(NodeEditorWidget)wid;
  printf("draw!\n");
  }

void Enter(
  Widget wid, XEvent *event, String *params, Cardinal *num_params
  ){
//  NodeEditorWidget w = (NodeEditorWidget) wid;
  }

void Leave(
  Widget wid, XEvent *event, String *params, Cardinal *num_params
  ){
//  NodeEditorWidget w = (NodeEditorWidget) wid;
  }

Boolean SetValues(
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

Widget XmCreateNodeEditor(
  Widget parent, char *name, Arg *arglist, Cardinal argCount
  ){
  return XtCreateWidget(name,nodeEditorWidgetClass,parent,arglist,argCount);
  }

Widget XmVaCreateNodeEditor(Widget parent, char *name, ...){
  register Widget w;
  va_list var;
  int count;
    
  va_start(var,name);
  count = XmeCountVaListSimple(var);
  va_end(var);

  va_start(var, name);
  w = XmeVLCreateWidget(
    name, nodeEditorWidgetClass, parent, False, var, count);
  va_end(var);   
  return w;
  }

Widget XmVaCreateManagedNodeEditor(Widget parent, char *name, ...){
  Widget w = NULL;
  va_list var;
  int count;
    
  va_start(var, name);
  count = XmeCountVaListSimple(var);
  va_end(var);
    
  va_start(var, name);
  w = XmeVLCreateWidget(
    name, nodeEditorWidgetClass, parent, True, var, count);
  va_end(var);   
  return w;
  }

#define IN 1
#define OUT 0

#define CONSIZE 6

void put_connector_on(Widget label, int dir){
  Position x, y;
  Dimension w, h;
  XtVaGetValues(label, XtNx, &x, XtNy, &y, XtNwidth, &w, XtNheight, &h, NULL);
  if(dir==OUT)x=(x+w-CONSIZE);
  Widget con=XtVaCreateManagedWidget(
    "con", xmPrimitiveWidgetClass, XtParent(label),
    XmNwidth, (Dimension)CONSIZE, XmNheight, (Dimension)CONSIZE,
XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, label,
XmNtopOffset, h/2-CONSIZE/2,
(dir==IN)?XmNleftAttachment:XmNrightAttachment, XmATTACH_FORM,
(dir==IN)?XmNleftWidget:XmNrightWidget, label,
(dir==IN)?XmNleftOffset:XmNrightOffset, 0,
XmNborderWidth, 1, XmNborderColor, 0x00FF00,
    NULL);

  }

int NodeEditorAddNode(Widget editor, char *name, int x, int y){
  Widget node, label;
Widget in1, in2, out1;
  XmString str;

module *mod=create_module(name, 0);
if(!mod)return 0;

  node=XtVaCreateWidget(
         mod->name, movableWidgetClass, editor,
//         XmNwidth, 100, XmNheight, 100,
         XmNverticalSpacing, 2, 
         XmNhorizontalSpacing, 2, 
//XmNtopShadowColor, 0x008000,
//XmNbottomShadowColor, 0x008000,
         XmNx, x, XmNy, y,
XmNallowOverlap, TRUE,
XmNtraversalOn, FALSE,
XmNtitle, mod->name,
EuNmodulePtr, mod,
         NULL);

/*
  str=XmStringCreateLocalized("Signal in");
  in1=XtVaCreateManagedWidget(
          "in1", xmLabelWidgetClass, node,
          XmNtopAttachment, XmATTACH_FORM,
          XmNrightAttachment, XmATTACH_FORM,
          XmNleftAttachment, XmATTACH_FORM,
          XmNforeground, 0x000000,
          XmNbackground, 0x00B000,
XmNmarginLeft, 10,
XmNalignment, XmALIGNMENT_BEGINNING,
          XmNlabelString, str,
          NULL);

  str=XmStringCreateLocalized("Modulator");
  in2=XtVaCreateManagedWidget(
          "in2", xmLabelWidgetClass, node,
          XmNtopAttachment, XmATTACH_WIDGET,
          XmNtopWidget, in1,
          XmNrightAttachment, XmATTACH_FORM,
          XmNleftAttachment, XmATTACH_FORM,
          XmNforeground, 0x000000,
          XmNbackground, 0x00B000,
XmNmarginLeft, 10,
XmNalignment, XmALIGNMENT_BEGINNING,
          XmNlabelString, str,
          NULL);

  str=XmStringCreateLocalized("Signal out");
  out1=XtVaCreateManagedWidget(
          "out1", xmLabelWidgetClass, node,
          XmNtopAttachment, XmATTACH_WIDGET,
          XmNtopWidget, in2,
          XmNrightAttachment, XmATTACH_FORM,
          XmNleftAttachment, XmATTACH_FORM,
          XmNbottomAttachment, XmATTACH_FORM,
          XmNforeground, 0x000000,
          XmNbackground, 0x00B000,
XmNmarginRight, 10,
XmNalignment, XmALIGNMENT_END,
          XmNlabelString, str,
          NULL);
*/
  XtManageChild(node);
//XtOverrideTranslations(out1, XtParseTranslationTable(contrans));

//put_connector_on(in1, IN);
//put_connector_on(in2, IN);
//put_connector_on(out1, OUT);


  }
