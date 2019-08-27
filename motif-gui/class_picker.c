#include<stdlib.h>
#include<X11/IntrinsicP.h>
#include<Xm/XmP.h>
#include<Xm/Form.h>
#include<Xm/Label.h>
#include<Xm/RowColumn.h>
#include<Xm/Frame.h>
#include"NodeEditorP.h"
#include"../orgel.h"


static Widget editor;

void class_clicked(Widget w, XEvent *e, String *argv, Cardinal *argc){
  Cardinal n=1;
  String s=XtName(w);
  cancel_class_picker(editor, e, &s, &n);
  }

Widget create_class_picker(Widget parent){
  Widget frame, title, grid;
  XmString str;
  NodeEditorWidget nep=(NodeEditorWidget)parent;

  editor=parent;

  Dimension width, height;
  XtVaGetValues(parent, XtNwidth, &width, XtNheight, &height, NULL);

  frame=XtVaCreateManagedWidget(
         "frame", xmFormWidgetClass, parent,
         XmNwidth, width-height/5, XmNheight, height*4/5,
         XmNverticalSpacing, 2,
         XmNhorizontalSpacing, 2,
         XmNx, width, XmNy, height,
         XmNtraversalOn, FALSE,
         NULL);

  str=XmStringCreateLocalized("MODULES");
  title=XtVaCreateManagedWidget(
          "title", xmLabelWidgetClass, frame,
          XmNforeground, 0x000000,
          XmNbackground, 0x00FF00,
          XmNlabelString, str,
          XmNtopAttachment, XmATTACH_FORM,
          XmNrightAttachment, XmATTACH_FORM,
          XmNleftAttachment, XmATTACH_FORM,
          NULL);

  grid=XtVaCreateManagedWidget(
         "grid", xmRowColumnWidgetClass, frame,
         XmNentryBorder, 5, 
         XmNtraversalOn, FALSE,
         XmNorientation, XmHORIZONTAL,
         XmNpacking, XmPACK_COLUMN,
         XmNnumColumns, 1,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, title,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNisAligned, True,
         NULL);

  int n;
  for(n=0; all_classes[n]; ++n){
    str=XmStringCreateLocalized(all_classes[n]->name);
    XtVaCreateManagedWidget(
      all_classes[n]->name, xmLabelWidgetClass, grid,
      XmNforeground, 0x000000,
      XmNbackground, all_classes[n]->is_static?0xFFFF00:0x00FF00,
      XmNlabelString, str,
      XmNtranslations, XtParseTranslationTable("<Btn1Down>: class_clicked()"),
      NULL);
    }

  Dimension picker_width, picker_height;
  for(int rows=1, bestdiff=9999; rows<n; ++rows){
    XtVaSetValues(grid, XmNnumColumns, rows, NULL);
    XtVaGetValues(frame, XmNwidth, &picker_width,
                         XmNheight, &picker_height, NULL);
    if(picker_width<width && picker_height<height){
      int diff=abs((width-picker_width)-(height-picker_height));
      if(diff<bestdiff)
        bestdiff=diff;
      else{
        XtVaSetValues(grid, XmNnumColumns, --rows, NULL);
        break;
        }
      }
    }
  XtVaGetValues(frame, XmNwidth, &picker_width,
                       XmNheight, &picker_height, NULL);
  XtVaSetValues(frame, XmNx, (width-picker_width)/2,
                       XmNy, (height-picker_height)/2, NULL);

  return frame;
  }
