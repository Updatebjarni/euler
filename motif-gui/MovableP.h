#ifndef MOVABLEP_H
#define MOVABLEP_H

#include <Xm/FormP.h>
#include <Xm/XmP.h>
#include "Movable.h"


#ifdef __cplusplus
extern "C" {
#endif

#define InheritGeometryManager  ((XtGeometryHandler) _XtInherit)
#define InheritChangeManaged    ((XtWidgetProc) _XtInherit)
#define InheritDeleteChild      ((XtWidgetProc) _XtInherit)
#define InheritSetValues        ((XtSetValuesFunc) _XtInherit)

typedef struct MovableClassPart{
  }MovableClassPart;

typedef struct MovableClassRec{
  CoreClassPart         core_class;
  CompositeClassPart    composite_class;
  ConstraintClassPart   constraint_class;
  XmManagerClassPart    manager_class;
  XmBulletinBoardClassPart  bulletin_board_class;
  XmFormClassPart       form_class;
  MovableClassPart      movable_class;
  }MovableClassRec;

extern MovableClassRec movableClassRec;

typedef struct MovablePart{
  Position offs_x, offs_y;
  String title;
  Widget title_widget;
  XtPointer module_ptr;
  }MovablePart;

typedef struct MovableRec{
  CorePart	  core;
  CompositePart  composite;
  ConstraintPart constraint;
  XmManagerPart  manager;
  XmBulletinBoardPart  bulletin_board;
  XmFormPart     form;
  MovablePart    movable;
 }MovableRec;

typedef struct{
  int           dummy;
  }MovableConstraintPart;

typedef struct{
  XmManagerConstraintPart manager;
  /* No bulletin board constraint part */
  XmFormConstraintPart    form;
  MovableConstraintPart   movable;
  }MovableConstraintRec;

static void ClassInitialize(void);
static void ClassPartInitialize(WidgetClass c);
static void Resize(Widget wid);
static void Initialize(Widget req, Widget new_w, ArgList args, Cardinal *num_args);
static XtGeometryResult QueryGeometry(Widget wid,
                               XtWidgetGeometry *intended,
                               XtWidgetGeometry *reply);
static void Destroy(Widget w);
static void Redisplay(Widget wid, XEvent *event, Region region);
static Boolean SetValues(Widget cw, Widget rw, Widget nw,
                  ArgList args, Cardinal *num_args);
static void SetValuesAlmost(Widget cw, Widget nw,
                     XtWidgetGeometry *request, XtWidgetGeometry *reply);

/* Actions */
static void drag_handle(Widget, XEvent *, String *, Cardinal *);
static void click_handle(Widget, XEvent *, String *, Cardinal *);

#ifdef __cplusplus
}
#endif

#endif
