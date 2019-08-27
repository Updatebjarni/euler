#ifndef NODEEDITORP_H
#define NODEEDITORP_H

#include <X11/CompositeP.h>
#include <Xm/XmP.h>
#include "NodeEditor.h"


#ifdef __cplusplus
extern "C" {
#endif

struct connection{
  int nsegments;
  struct{int x, y;} *point;
  };

typedef struct NodeEditorClassPart{
  }NodeEditorClassPart;

typedef struct NodeEditorClassRec{
  CoreClassPart         core_class;
  CompositeClassPart    composite_class;
  NodeEditorClassPart   nodeeditor_class;
  }NodeEditorClassRec;

extern NodeEditorClassRec nodeEditorClassRec;

#define XmInheritSetOverrideCallback  ((XtWidgetProc) _XtInherit)
#define XmInheritResize      ((XtWidgetProc) _XtInherit)
#define XmInheritRealize    ((XtRealizeProc) _XtInherit)

typedef struct NodeEditorPart{
  Position offs_x, offs_y;
  Window clicktrap;
  Widget class_picker;
  XtTranslations previous_trans;
  Position dest_x, dest_y;
  GC line_GC;
  Pixel foreground_pixel;
  int nconnections;
  struct connection *connection;
  }NodeEditorPart;

typedef struct NodeEditorRec{
   CorePart         core;
   CompositePart    composite;
   NodeEditorPart   nodeeditor;
  }NodeEditorRec;

Widget create_class_picker(Widget parent);

void ClassInitialize(void);
void ClassPartInitialize(WidgetClass c);
void Resize(Widget wid);
void Initialize(Widget req, Widget new_w, ArgList args, Cardinal *num_args);
XtGeometryResult QueryGeometry(Widget wid,
                               XtWidgetGeometry *intended,
                               XtWidgetGeometry *reply);
void Destroy(Widget w);
void Redisplay(Widget wid, XEvent *event, Region region);
Boolean SetValues(Widget cw, Widget rw, Widget nw,
                  ArgList args, Cardinal *num_args);
void SetValuesAlmost(Widget cw, Widget nw,
                     XtWidgetGeometry *request, XtWidgetGeometry *reply);

/* Actions */
static void save_coords(Widget, XEvent *, String *, Cardinal *);
static void draw_line(Widget, XEvent *, String *, Cardinal *);
void skapa(Widget, XEvent *, String *, Cardinal *);
void bakklick(Widget, XEvent *, String *, Cardinal *);
void flytta(Widget, XEvent *, String *, Cardinal *);
void popup_class_picker(Widget, XEvent *, String *, Cardinal *);
void cancel_class_picker(Widget, XEvent *, String *, Cardinal *);
void dragen(Widget, XEvent *, String *, Cardinal *);
void klickad(Widget, XEvent *, String *, Cardinal *);
void conbtndown(Widget, XEvent *, String *, Cardinal *);
void conbtnup(Widget, XEvent *, String *, Cardinal *);
void conbtn3down(Widget, XEvent *, String *, Cardinal *);
void class_clicked(Widget, XEvent *, String *, Cardinal *);
void Enter(Widget, XEvent *, String *, Cardinal *);
void Leave(Widget, XEvent *, String *, Cardinal *);

#ifdef __cplusplus
}
#endif

#endif
