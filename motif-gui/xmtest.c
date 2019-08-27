#include<stdio.h>
#include<X11/IntrinsicP.h>
#include<X11/Composite.h>
#include<Xm/Xm.h>
#include<Xm/Label.h>
#include<Xm/Form.h>
#include<Xm/Frame.h>
#include<Xm/DrawingA.h>
#include<cairo.h>
#include<cairo-xlib-xrender.h>
#include"NodeEditor.h"

static cairo_surface_t *cairo;
static cairo_t *cr;

static void pushed_fn(Widget w, XtPointer client_data, void *_cbs) {   
//XmPushButtonCallbackStruct *cbs=_cbs;
double xc = 120.0;
double yc = 120.0;
double radius = 100.0;
double angle1 = 45.0  * (3.14/180.0);
double angle2 = 180.0 * (3.14/180.0);

// cairo_scale(cr, 400, 300);
cairo_set_source_rgb (cr, 0.6, 0.2, 0.6);
cairo_set_line_width (cr, 6.0);
cairo_arc (cr, xc, yc, radius, angle1, angle2);
cairo_fill (cr);

cairo_arc (cr, xc, yc, radius, angle1, angle1);
cairo_line_to (cr, xc, yc);
cairo_arc (cr, xc, yc, radius, angle2, angle2);
cairo_line_to (cr, xc, yc);
cairo_stroke (cr);
cairo_surface_flush(cairo);
     printf("Don't Push Me!!\n");
  }

static String resources[] = {
       "*background: black",
       "*foreground: green",
       "*topShadowColor: green",
       "*bottomShadowColor: green",
       "*shadowThickness: 1",
       "XmTest.width: 800",
       "XmTest.height: 600",
       NULL
       }; 

XtAppContext app;

int gui_main(int argc, char **argv){
  Widget top_wid, ne;
Widget mod1, mod1_label;
XmString str;
  top_wid = XtVaAppInitialize(
              &app, 
              "XmTest", /* class name */
              NULL, 0, /* NO command line options table */
              &argc, argv,  /* command line arguments */
              resources, /* fallback_resources list */
              NULL); 
  ne=XtVaCreateManagedWidget(
    "nodeeditor", nodeEditorWidgetClass, top_wid,
    XmNwidth, 800, XmNheight, 600,
    NULL);
/*
  drawing = XtVaCreateManagedWidget(
              "drawing",
              xmDrawingAreaWidgetClass, parent,
              XmNwidth, 400, XmNheight, 300,
              XmNbackground, 0xFFFFFF,
              XmNtopAttachment, XmATTACH_FORM,
              XmNrightAttachment, XmATTACH_FORM,
              XmNbottomAttachment, XmATTACH_FORM,
              XmNleftAttachment, XmATTACH_FORM,
              NULL);
  draw_win=XtWindow(drawing);
  cairo=cairo_xlib_surface_create_with_xrender_format(
    XtDisplay(drawing),
    draw_win,
    XtScreen(drawing),
    XRenderFindStandardFormat(XtDisplay(drawing), PictStandardRGB24),
    400, 300);
  cairo_xlib_surface_set_size(cairo, 400, 300);
  cr=cairo_create(cairo);
*/

  XtRealizeWidget(top_wid);


  XtAppMainLoop(app); /* enter processing loop */ 
  }
