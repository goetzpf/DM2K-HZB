/*
 * routine to create product description shell for all Motif-based EPICS tools
 *
 *  input parameters are the product name, and product description
 */
#include <stdio.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>

#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/MwmUtil.h>


static Widget okButton;

static void closeProductDescriptionCallback(
  Widget w,
  XtPointer client_data,
  XtPointer call_data)
{
  Widget shell = (Widget)client_data;
  XtPopdown(shell);
}

static void popdownProductDescriptionShell(XtPointer xtPointer,XtIntervalId*interval )
{
  Arg args[3];
  Widget widget;
  Atom WM_DELETE_WINDOW;

  widget = (Widget) xtPointer;
  XtPopdown(widget);

  XtSetArg(args[0],XmNmwmDecorations,MWM_DECOR_ALL);
  XtSetArg(args[1],XmNdeleteResponse,XmDO_NOTHING);
  XtSetValues(widget,args,2);

  WM_DELETE_WINDOW = XmInternAtom(XtDisplay(widget),
	"WM_DELETE_WINDOW",False);
  XmAddWMProtocolCallback(widget,WM_DELETE_WINDOW,
	closeProductDescriptionCallback,(XtPointer)widget);

  XtManageChild(okButton);
}


/*
 * function to create, set and popup an EPICS product description shell
 *  widget hierarchy:
 *
 * productDescriptionShell
 *    form
 *	nameLabel
 *	separator
 *	descriptionLabel
 *	versionInfoLabel
 *	developedAtLabel
 *	okButton
 *
 */
Widget createAndPopupProductDescriptionShell(
  XtAppContext appContext,	/* application context		*/
  Widget topLevelShell,		/* application's topLevel shell	*/
  char *name,			/* product/program name		*/
  XmFontList nameFontList,	/*   and font list (or NULL)	*/
  Pixmap namePixmap,		/* name Pixmap (or NULL)	*/
  char *description,		/* product description		*/
  XmFontList descriptionFontList,/*   and font list (or NULL)	*/
  char *versionInfo,		/* product version number	*/
  char *developedAt,		/* at and by...			*/
  XmFontList otherFontList,	/*   and font list (or NULL)	*/
  int background,		/* background color (or -1)	*/
  int foreground,		/* foreground color (or -1)	*/
  int seconds)			/* seconds to leave posted	*/
{
  Widget productDescriptionShell, form;
  Arg args[15];
  Widget children[6], nameLabel, descriptionLabel, versionInfoLabel,
	separator, developedAtLabel;
  XmString nameXmString = (XmString)NULL, descriptionXmString = (XmString)NULL,
	versionInfoXmString = (XmString)NULL,
	developedAtXmString = (XmString)NULL, okXmString = (XmString)NULL;
  Dimension formHeight, nameHeight;
  int n, offset;


/* create the shell */
  n = 0;
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  XtSetArg(args[n],XmNmwmDecorations, MWM_DECOR_ALL|
	MWM_DECOR_BORDER|MWM_DECOR_RESIZEH|MWM_DECOR_TITLE|MWM_DECOR_MENU|
	MWM_DECOR_MINIMIZE|MWM_DECOR_MAXIMIZE); n++;
  XtSetArg(args[n],XmNtitle,"Version"); n++;
  productDescriptionShell = XtCreatePopupShell("productDescriptionShell",
		topLevelShellWidgetClass,topLevelShell,args, n);

  n = 0;
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  XtSetArg(args[n],XmNnoResize,True); n++;
  XtSetArg(args[n],XmNshadowThickness,2); n++;
  XtSetArg(args[n],XmNshadowType,XmSHADOW_OUT); n++;
  XtSetArg(args[n],XmNautoUnmanage,False); n++;
  form = XmCreateForm(productDescriptionShell,"form",args,n);

/* generate XmStrings */
  if (name != NULL) nameXmString = XmStringCreateLtoR(name,
		XmFONTLIST_DEFAULT_TAG);
  if (description != NULL) descriptionXmString =
	XmStringCreateLtoR(description,XmFONTLIST_DEFAULT_TAG);
  if (versionInfo != NULL) versionInfoXmString =
	XmStringCreateLtoR(versionInfo,XmFONTLIST_DEFAULT_TAG);
  if (developedAt != NULL) developedAtXmString =
	XmStringCreateLtoR(developedAt,XmFONTLIST_DEFAULT_TAG);


/* 
 * now create the label children:
 */

/* name */
  n = 0;
  if (namePixmap == (Pixmap) NULL) {
    XtSetArg(args[n],XmNlabelString,nameXmString); n++;
    if (nameFontList != NULL) {
	XtSetArg(args[n],XmNfontList,nameFontList); n++;
    }
  } else {
    XtSetArg(args[n],XmNlabelType,XmPIXMAP); n++;
    XtSetArg(args[n],XmNlabelPixmap,namePixmap); n++;
  }
  XtSetArg(args[n],XmNalignment,XmALIGNMENT_BEGINNING); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNleftPosition,1); n++;
  XtSetArg(args[n],XmNresizable,False); n++;
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  nameLabel = XmCreateLabel(form,"nameLabel",args,n);


/* separator */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftWidget,nameLabel); n++;
  XtSetArg(args[n],XmNorientation,XmVERTICAL); n++;
  XtSetArg(args[n],XmNshadowThickness,2); n++;
  XtSetArg(args[n],XmNseparatorType,XmSHADOW_ETCHED_IN); n++;
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  separator = XmCreateSeparator(form,"separator",args,n);

/* description */
  n = 0;
  XtSetArg(args[n],XmNalignment,XmALIGNMENT_BEGINNING); n++;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNtopPosition,5); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftWidget,separator); n++;
  XtSetArg(args[n],XmNlabelString,descriptionXmString); n++;
  if (descriptionFontList != NULL) {
    XtSetArg(args[n],XmNfontList,descriptionFontList); n++;
  }
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  descriptionLabel = XmCreateLabel(form,"descriptionLabel",args,n);

/* version info */
  n = 0;
  XtSetArg(args[n],XmNalignment,XmALIGNMENT_BEGINNING); n++;
  XtSetArg(args[n],XmNlabelString,versionInfoXmString); n++;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,descriptionLabel); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET); n++;
  XtSetArg(args[n],XmNleftWidget,descriptionLabel); n++;
  if (otherFontList != NULL) {
    XtSetArg(args[n],XmNfontList,otherFontList); n++;
  }
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  versionInfoLabel = XmCreateLabel(form,"versionInfoLabel",args,n);

/* developed at/by... */
  n = 0;
  XtSetArg(args[n],XmNalignment,XmALIGNMENT_BEGINNING); n++;
  XtSetArg(args[n],XmNlabelString,developedAtXmString); n++;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,versionInfoLabel); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET); n++;
  XtSetArg(args[n],XmNleftWidget,versionInfoLabel); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNrightPosition,95); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNbottomPosition,95); n++;

  if (otherFontList != NULL) {
    XtSetArg(args[n],XmNfontList,otherFontList); n++;
  }
  if (background >= 0) {
    XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
    XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  developedAtLabel = XmCreateLabel(form,"developedAtLabel",args,n);


/* ok button */
  okXmString = XmStringCreateSimple("ok");
  n = 0;
  XtSetArg(args[n],XmNlabelString,okXmString); n++;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopOffset,8); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightOffset,8); n++;
  if (otherFontList != NULL) {
    XtSetArg(args[n],XmNfontList,otherFontList); n++;
  }
  if (background >= 0) {
      XtSetArg(args[n],XmNbackground,(unsigned long)background); n++; }
  if (foreground >= 0) {
      XtSetArg(args[n],XmNforeground,(unsigned long)foreground); n++; }
  okButton = XmCreatePushButton(form,"okButton",args,n);
  XtAddCallback(okButton,XmNactivateCallback,
	closeProductDescriptionCallback,
	(XtPointer)productDescriptionShell);n++;
  XmStringFree(okXmString);


  children[0] = nameLabel; children[1] = descriptionLabel;
  children[2] = versionInfoLabel; children[3] = developedAtLabel;
  children[4] = separator;
  XtManageChildren(children,5);
  XtManageChild(form);

  XtPopup(productDescriptionShell,XtGrabNone);

/* now center nameLabel vertically in form space */
  XtSetArg(args[0],XmNheight,&nameHeight);
  XtGetValues(nameLabel,args,1);
  XtSetArg(args[0],XmNheight,&formHeight);
  XtGetValues(form,args,1);
  offset = (formHeight - nameHeight);
  offset = offset/2;
  XtSetArg(args[0],XmNtopOffset,offset);
  XtSetValues(nameLabel,args,1);


  if (nameXmString != (XmString)NULL) XmStringFree(nameXmString);
  if (descriptionXmString != (XmString)NULL) XmStringFree(descriptionXmString);
  if (versionInfoXmString != (XmString)NULL) XmStringFree(versionInfoXmString);
  if (developedAtXmString != (XmString)NULL) XmStringFree(developedAtXmString);

/* register timeout procedure to make the dialog go away after N seconds */
  XtAppAddTimeOut(appContext,(unsigned long)(1000*seconds),
		popdownProductDescriptionShell,
		(XtPointer)productDescriptionShell);

  return(productDescriptionShell);
}


#ifdef TEST_PRODUCT_DESCRIPTION_SHELL
/*************************************************************************/

main(int argc, char **argv)
{
  Widget topLevel, shell;
  XtAppContext appContext;

  topLevel = XtAppInitialize(&appContext, "TEST", NULL, 0, &argc, argv,
                fallbackResources, NULL, 0);
  XmRegisterConverters();
  shell = createAndPopupProductDescriptionShell(appContext,topLevel,
  	"DM2K", NULL,(Pixmap)NULL,
  	"Motif-based Editor & Display Manager", NULL,
  	"Version 1.0",
  	"developed at Argonne National Laboratory, by Mark Anderson", NULL,
  	-1, -1, 3);


  XtRealizeWidget(topLevel);
  XtAppMainLoop(appContext);

}
#endif /* TEST */
