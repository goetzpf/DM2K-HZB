/******************************************************************************
 *  
 * Author: Vladimir T. Romanovski (romsky@x4u2.desy.de)
 *
 * Organization: KRYK/@DESY
 *
 * 10-Apr-97 
 *
 *****************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/MwmUtil.h>

#include "dm2k.h"

typedef struct _TimeOutClientData{
  XtAppContext   app;
  int          * flag;
  Widget         shell;
}TimeOutClientData;


/*ARGSUSED*/
static void TimerProc(XtPointer      xtPointer,
		      XtIntervalId * id)
{
  TimeOutClientData * data = (TimeOutClientData *) xtPointer;

  if (*data->flag) {
    XtAppAddTimeOut(data->app, 250L, 
		    (XtTimerCallbackProc)TimerProc, xtPointer);
  } else {
    XtDestroyWidget(data->shell);

    free((char*)xtPointer);
  }
}

/*ARGSUSED*/
static void PopupProductDescriptionShellCallback(Widget w, 
						 XtPointer cd, 
						 XtPointer cbs)
{
  unsigned int display_width;
  unsigned int display_height;
  Dimension    shell_width;
  Dimension    shell_height;

  display_width  = DisplayWidth(display, DefaultScreen(display));
  display_height = DisplayHeight(display, DefaultScreen(display));

  XtVaGetValues(w, 
		XmNwidth, &shell_width,
		XmNheight, &shell_height,
		NULL);

  if (shell_width == 0)
    shell_width = 310;

  if (shell_height == 0)
    shell_height = 600;

  XtVaSetValues(w,
		XmNx, (Position)((display_width - shell_width) / 2),
		XmNy, (Position)((display_height - shell_height) / 2),
		NULL);

}


/*
 * function to create, set and popup an EPICS product description shell
 *  widget hierarchy:
 *
 * productDescriptionShell
 *    form
 *	nameLabel
 *	separator
 *	developedAtLabel
 *	separator
 *
 */
void ProductDescriptionPopupShell(
  XtAppContext   appContext,	        /* application context		*/
  Widget         parent,		/* parent widget for the shell	*/
  const char   * name,			/* product/program name		*/
  Pixmap         namePixmap,		/* name Pixmap (or NULL)	*/
  const char   * description,		/* product description		*/
  const char   * versionInfo,		/* product version number	*/
  const char   * developedAt,		/* at and by...			*/
  int            background,		/* background color (or -1)	*/
  int            foreground,		/* foreground color (or -1)	*/
  int          * popdownFlag)		/* flag to pop down shell       */
{
  Widget              productDescriptionShell;
  Widget              form;
  Widget              nameLabel;
  Widget              descriptionLabel;
  Widget              separator;
  XmString            nameXmString;
  XmString            descriptionXmString;
  XmString            versionInfoXmString;
  XmString            developedAtXmString;
  XmString            tmpXmString;
  XmString            tmp1XmString;
  Arg                 args[15];
  int                 n;
  TimeOutClientData * data;


  /* generate XmStrings 
   */
  if (name == NULL)        name        = "\n";
  if (description == NULL) description = "\n";
  if (versionInfo == NULL) versionInfo = "\n";
  if (developedAt == NULL) developedAt = "\n";

  nameXmString        =  XmStringCreateLtoR((char*)name,	"PRODUCT_NAME_TAG");
  descriptionXmString =  XmStringCreateLtoR((char*)description, "DESCRIPTION_TAG");
  versionInfoXmString =  XmStringCreateLtoR((char*)versionInfo, "VERSION_INFO_TAG");
  developedAtXmString =  XmStringCreateLtoR((char*)developedAt, "DEVELOPED_AT_TAG");

  tmp1XmString = XmStringConcat (descriptionXmString, versionInfoXmString);
  tmpXmString  = XmStringConcat (tmp1XmString, developedAtXmString);


  /* create the Override shell 
   */
  n = 0;
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  XtSetArg(args[n],XmNtitle,               "Version"); n++;
  XtSetArg(args[n],XtNborderWidth,         0); n++;
  productDescriptionShell = XtCreatePopupShell("productDescriptionShell",
					       overrideShellWidgetClass,
					       parent,args, n);

  XtAddCallback(productDescriptionShell, XtNpopupCallback, 
		PopupProductDescriptionShellCallback, NULL);

  /* container
   */
  n = 0;
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }
  
  XtSetArg(args[n],XtNborderWidth,         0); n++;
  XtSetArg(args[n],XmNmarginHeight,        2); n++;
  XtSetArg(args[n],XmNmarginWidth,         2); n++;
  XtSetArg(args[n],XmNshadowThickness,     2); n++;
  XtSetArg(args[n],XmNshadowType,          XmSHADOW_OUT); n++;

  form = XmCreateForm(productDescriptionShell, "form", args, n);

  /* 
   * now create the children:
   */
  
  /* product name 
   */
  n = 0;
  if (   namePixmap == (Pixmap) NULL 
      || namePixmap == XtUnspecifiedPixmap
      || namePixmap == None) 
    {
      XtSetArg(args[n],XmNlabelType,     XmSTRING); n++;
      XtSetArg(args[n],XmNlabelString,   nameXmString); n++;
    } 
  else 
    {
      XtSetArg(args[n],XmNlabelType,     XmPIXMAP); n++;
      XtSetArg(args[n],XmNlabelPixmap,   namePixmap); n++;
    }
  
  XtSetArg(args[n],XmNalignment,       XmALIGNMENT_CENTER); n++;
  XtSetArg(args[n],XmNtopAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNleftAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment, XmATTACH_FORM); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  nameLabel = XtCreateManagedWidget ("nameLabel",
				     xmLabelWidgetClass, form,
				     args, n);

  /* separator 
   */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,    XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopWidget,        nameLabel); n++;
  XtSetArg(args[n],XmNorientation,      XmHORIZONTAL); n++;
  XtSetArg(args[n],XmNshadowThickness,  2); n++;
  XtSetArg(args[n],XmNseparatorType,    XmSHADOW_ETCHED_IN); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  separator = XtCreateManagedWidget ("separator", xmSeparatorWidgetClass, form,
				     args, n);

  /* description label
   */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,    XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopWidget,        separator); n++;
  XtSetArg(args[n],XmNlabelType,        XmSTRING); n++;
  XtSetArg(args[n],XmNlabelString,      tmpXmString); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  descriptionLabel = XtCreateManagedWidget ("descriptionLabel",
					    xmLabelWidgetClass, form,
					    args, n);
  
  XtManageChild(form);

  XtPopup(productDescriptionShell,XtGrabNone);

  XmStringFree(tmpXmString);
  XmStringFree(tmp1XmString);
  XmStringFree(developedAtXmString);
  XmStringFree(versionInfoXmString);
  XmStringFree(descriptionXmString);
  XmStringFree(nameXmString);

  /* register timeout procedure to make the dialog go away after N seconds 
   */
  data = DM2KALLOC(TimeOutClientData);
  if (data == NULL) {
    XtDestroyWidget(productDescriptionShell);
    return;
  }

  data->app   = appContext;
  data->flag  = popdownFlag;
  data->shell = productDescriptionShell;

  XtAppAddTimeOut(appContext, 250L, (XtTimerCallbackProc)TimerProc,
		  (XtPointer)data);
}

Widget ProductDescriptionCreatePopupDialogShell(
  Widget         parent,		/* parent widget for the shell	*/
  const char   * name,			/* product/program name		*/
  Pixmap         namePixmap,		/* name Pixmap (or NULL)	*/
  const char   * description,		/* product description		*/
  const char   * versionInfo,		/* product version number	*/
  const char   * developedAt,		/* at and by...			*/
  int            background,		/* background color (or -1)	*/
  int            foreground)		/* foreground color (or -1)	*/
{
  Widget              form;
  Widget              nameLabel;
  Widget              descriptionLabel;
  Widget              separator;
  Widget              widget;
  XmString            nameXmString;
  XmString            descriptionXmString;
  XmString            versionInfoXmString;
  XmString            developedAtXmString;
  XmString            tmpXmString;
  XmString            tmp1XmString;
  XmString            okXmString;
  Arg                 args[15];
  int                 n;


  /* generate XmStrings 
   */
  if (name == NULL)        name        = "\n";
  if (description == NULL) description = "\n";
  if (versionInfo == NULL) versionInfo = "\n";
  if (developedAt == NULL) developedAt = "\n";

  nameXmString        =  XmStringCreateLtoR((char*)name,        "PRODUCT_NAME_TAG");
  descriptionXmString =  XmStringCreateLtoR((char*)description, "DESCRIPTION_TAG");
  versionInfoXmString =  XmStringCreateLtoR((char*)versionInfo, "VERSION_INFO_TAG");
  developedAtXmString =  XmStringCreateLtoR((char*)developedAt, "DEVELOPED_AT_TAG");
  okXmString          =  XmStringCreateLocalized("Ok");

  tmp1XmString = XmStringConcat (descriptionXmString, versionInfoXmString);
  tmpXmString  = XmStringConcat (tmp1XmString, developedAtXmString);


  /* container
   */
  n = 0;
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }
  
  XtSetArg(args[n],XtNborderWidth,         0); n++;
  XtSetArg(args[n],XmNmarginHeight,        2); n++;
  XtSetArg(args[n],XmNmarginWidth,         2); n++;
  XtSetArg(args[n],XmNshadowThickness,     2); n++;
  XtSetArg(args[n],XmNfractionBase,        7); n++;
  XtSetArg(args[n],XmNshadowType,          XmSHADOW_OUT); n++;
  XtSetArg(args[n],XmNautoUnmanage,        True); n++;

  form = XmCreateFormDialog(parent, "productDescriptionShell", args, n);


  XtVaSetValues(XtParent(form), XmNtitle,  "Version", NULL);

  XtAddCallback(XtParent(form), XtNpopupCallback, 
		PopupProductDescriptionShellCallback, NULL);

  /* 
   * now create the children:
   */
  
  /* product name 
   */
  n = 0;
  if (namePixmap == (Pixmap) NULL || namePixmap == XtUnspecifiedPixmap) 
  {
    XtSetArg(args[n],XmNlabelType,     XmSTRING); n++;
    XtSetArg(args[n],XmNlabelString,   nameXmString); n++;
  } 
  else {
    XtSetArg(args[n],XmNlabelType,     XmPIXMAP); n++;
    XtSetArg(args[n],XmNlabelPixmap,   namePixmap); n++;
  }

  XtSetArg(args[n],XmNalignment,       XmALIGNMENT_CENTER); n++;
  XtSetArg(args[n],XmNtopAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNleftAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment, XmATTACH_FORM); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  nameLabel = XtCreateManagedWidget ("nameLabel",
				     xmLabelWidgetClass, form,
				     args, n);

  /* separator 
   */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,    XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopWidget,        nameLabel); n++;
  XtSetArg(args[n],XmNorientation,      XmHORIZONTAL); n++;
  XtSetArg(args[n],XmNshadowThickness,  2); n++;
  XtSetArg(args[n],XmNseparatorType,    XmSHADOW_ETCHED_IN); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  separator = XtCreateManagedWidget ("separator", xmSeparatorWidgetClass, form,
				     args, n);

  /* description label
   */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,    XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopWidget,        separator); n++;
  XtSetArg(args[n],XmNlabelType,        XmSTRING); n++;
  XtSetArg(args[n],XmNlabelString,      tmpXmString); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  descriptionLabel = XtCreateManagedWidget ("descriptionLabel",
					    xmLabelWidgetClass, form,
					    args, n);
  
  /* separator 
   */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,    XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopWidget,        descriptionLabel); n++;
  XtSetArg(args[n],XmNorientation,      XmHORIZONTAL); n++;
  XtSetArg(args[n],XmNshadowThickness,  2); n++;
  XtSetArg(args[n],XmNseparatorType,    XmSHADOW_ETCHED_IN); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  separator = XtCreateManagedWidget ("separator", xmSeparatorWidgetClass, form,
				     args, n);

  /* ok button
   */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,    XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftAttachment,   XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNrightAttachment,  XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopWidget,        separator); n++;
  XtSetArg(args[n],XmNleftPosition,     3); n++;
  XtSetArg(args[n],XmNrightPosition,    4); n++;
  XtSetArg(args[n],XmNtopOffset,        8); n++;
  XtSetArg(args[n],XmNbottomOffset,     8); n++;
  XtSetArg(args[n],XmNlabelType,        XmSTRING); n++;
  XtSetArg(args[n],XmNlabelString,      okXmString); n++;

  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; 
  }

  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; 
  }

  widget = XtCreateManagedWidget ("ok", xmPushButtonWidgetClass, form, 
				  args, n);

  XtVaSetValues(form, XmNdefaultButton, widget, NULL);
  
  XmStringFree(tmpXmString);
  XmStringFree(tmp1XmString);
  XmStringFree(developedAtXmString);
  XmStringFree(versionInfoXmString);
  XmStringFree(descriptionXmString);
  XmStringFree(nameXmString);
  XmStringFree(okXmString);

  return form;
}

