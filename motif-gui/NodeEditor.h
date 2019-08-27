#ifndef NODEEDITOR_H
#define NODEEDITOR_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

extern WidgetClass nodeEditorWidgetClass;

typedef struct NodeEditorClassRec  *NodeEditorWidgetClass;
typedef struct NodeEditorRec       *NodeEditorWidget;

#ifndef XmIsNodeEditor
#define XmIsNodeEditor(w)     XtIsSubclass(w, nodeEditorWidgetClass)
#endif

extern Widget XmCreateNodeEditor(
  Widget parent, char *name, Arg *arglist, Cardinal argCount);
extern Widget XmVaCreateNodeEditor(Widget parent, char *name, ...);
extern Widget XmVaCreateManagedNodeEditor(Widget parent, char *name, ...);

extern int NodeEditorAddNode(Widget editor, char *name, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
