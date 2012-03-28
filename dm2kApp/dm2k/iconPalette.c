#include <ctype.h>
#include "dm2k.h"
#include <Xm/MwmUtil.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <cvtFast.h>
#ifdef __cplusplus
}
#endif

void createIconWindowDialog (Display *display,
			     Widget   parent,
			     Widget  *icon_shell_return,
			     Widget  *icon_widget_return)
{
  Widget iconFm, iconSW, iconRC;
  int    n;
  Arg    args[20];
  

  iconFm = XmCreateFormDialog(parent, "iconMainForm", NULL, 0);
  XtManageChild(iconFm);

  n = 0;
  XtSetArg(args[n],XtNtitle,          "DM2K Icon Window"); n++;
  XtSetArg(args[n],XtNiconName,       "DM2K Icon Window"); n++;
  XtSetArg(args[n],XmNdeleteResponse, XmDO_NOTHING); n++;
  XtSetValues (XtParent(iconFm), args, n);


  n = 0;
  XtSetArg(args[n],XmNscrollingPolicy,        XmAUTOMATIC); n++;
  XtSetArg(args[n],XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
  iconSW = XmCreateScrolledWindow(iconFm, "iconWindowSW", args, n);
  XtManageChild(iconSW);

  n = 0;
  XtSetArg(args[n],XmNnumColumns, 1); n++;
  XtSetArg(args[n],XmNorientation,XmVERTICAL); n++;
  XtSetArg(args[n],XmNpacking,    XmPACK_COLUMN); n++;
  XtSetArg(args[n],XmNwidth,      100); n++;
  XtSetArg(args[n],XmNheight,     100); n++;
  iconRC = XmCreateRowColumn(iconSW, "iconWindowRC", args, n);
  XtManageChild(iconRC);

  if (icon_shell_return)
    *icon_shell_return = XtParent(iconFm);

  if (icon_widget_return)
    *icon_widget_return = iconRC;
}
		       
Window getIconWindow (Widget iconWindow, Pixmap icon)
{
  Widget button;
  
  button = XtVaCreateManagedWidget("iconButton",
				   xmPushButtonWidgetClass,iconWindow ,
				   XmNlabelType,      XmPIXMAP,
				   XmNmarginTop,      0,
				   XmNmarginBottom,   0,
				   XmNmarginLeft,     0,
				   XmNmarginRight,    0,
				   XmNmarginWidth,    0,
				   XmNmarginHeight,   0,
				   XmNwidth,          32,
				   XmNheight,         32,
				   XmNpushButtonEnabled, True,
				   XmNhighlightThickness, 0,
				   XmNalignment, XmALIGNMENT_CENTER,
				   XmNlabelPixmap,     icon,
				   XmNindicatorOn, False,
				   XmNrecomputeSize, False,
				   NULL);
  return XtWindow(button);
  
}



