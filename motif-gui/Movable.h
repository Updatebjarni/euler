#ifndef MOVABLE_H
#define MOVABLE_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

extern WidgetClass movableWidgetClass;

typedef struct MovableClassRec  *MovableWidgetClass;
typedef struct MovableRec       *MovableWidget;

#ifndef XmIsMovable
#define XmIsMovable(w)     XtIsSubclass(w, movableWidgetClass)
#endif

extern Widget XmCreateMovable(
  Widget parent, char *name, Arg *arglist, Cardinal argCount);
extern Widget XmVaCreateMovable(Widget parent, char *name, ...);
extern Widget XmVaCreateManagedMovable(Widget parent, char *name, ...);

extern const char EuNmodulePtr[];
extern const char EuCModulePtr[];

#ifdef __cplusplus
}
#endif

#endif
