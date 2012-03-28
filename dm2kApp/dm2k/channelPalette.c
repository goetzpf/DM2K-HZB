/*
*****************************************************************
                          COPYRIGHT NOTIFICATION
*****************************************************************

THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
AND IN ALL SOURCE LISTINGS OF THE CODE.

(C)  COPYRIGHT 1993 UNIVERSITY OF CHICAGO

Argonne National Laboratory (ANL), with facilities in the States of
Illinois and Idaho, is owned by the United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

Portions of this material resulted from work developed under a U.S.
Government contract and are subject to the following license:  For
a period of five years from March 30, 1993, the Government is
granted for itself and others acting on its behalf a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, and perform
publicly and display publicly.  With the approval of DOE, this
period may be renewed for two additional five year periods.
Following the expiration of this period or periods, the Government
is granted for itself and others acting on its behalf, a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, distribute copies
to the public, perform publicly and display publicly, and to permit
others to do so.

*****************************************************************
                                DISCLAIMER
*****************************************************************

NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
OWNED RIGHTS.

*****************************************************************
LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (708-252-2000).
*/
/*****************************************************************************
 *
 *     Original Author : Mark Andersion
 *     Current Author  : Frederick Vong
 *
 * Modification Log:
 * -----------------
 * .01  03-01-95        vong    2.0.0 release
 * .02  09-07-95        vong    conform to c++ syntax
 *
 *****************************************************************************
*/

#include "dm2k.h"

static Widget channelFilePDM, openFSD;


#define N_MAX_MENU_ELES 5
#define N_MAIN_MENU_ELES 2


/*
 * create the file pulldown menu pane
 */
#define N_FILE_MENU_ELES 5
#define FILE_BTN_POSN 0
#undef FILE_OPEN_BTN
#define FILE_OPEN_BTN	 0
#undef FILE_SAVE_BTN
#define FILE_SAVE_BTN	 1
#undef FILE_SAVE_AS_BTN
#define FILE_SAVE_AS_BTN 2
#undef FILE_CLOSE_BTN
#define FILE_CLOSE_BTN	 3

/*
 * create the help pulldown menu pane
 */
#define N_HELP_MENU_ELES 1
#define HELP_BTN_POSN 1



/********************************************
 **************** Callbacks *****************
 ********************************************/
#ifdef __cplusplus
static void fileOpenCallback(Widget w, XtPointer, XtPointer cbs)
#else
static void fileOpenCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  switch(((XmAnyCallbackStruct *) cbs)->reason){
	case XmCR_CANCEL:
		XtUnmanageChild(w);
		break;
	case XmCR_OK:
		XtUnmanageChild(w);
		break;
  }
}

#ifdef __cplusplus
static void fileMenuSimpleCallback(Widget, XtPointer cd, XtPointer)
#else
static void fileMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonNumber = (int) cd;
  XmString label;
  int n;
  Arg args[10];
  Widget textField;

    switch(buttonNumber) {
	    case FILE_OPEN_BTN:
		if (openFSD == NULL) {
		    n = 0;
		    label = XmStringCreateSimple(CHANNEL_DIALOG_MASK);
		    XtSetArg(args[n],XmNdirMask,label); n++;
		    XtSetArg(args[n],XmNdialogStyle,
				XmDIALOG_PRIMARY_APPLICATION_MODAL); n++;
		    openFSD = XmCreateFileSelectionDialog(channelFilePDM,
				"openFSD",args,n);
/* make Filter text field insensitive to prevent user hand-editing dirMask */
		    textField = XmFileSelectionBoxGetChild(openFSD,
				XmDIALOG_FILTER_TEXT);
		    XtSetSensitive(textField,FALSE);
		    XtAddCallback(openFSD,XmNokCallback,
				(XtCallbackProc)fileOpenCallback,
				FILE_OPEN_BTN);
		    XtAddCallback(openFSD,XmNcancelCallback,
				(XtCallbackProc)fileOpenCallback,FILE_OPEN_BTN);
		    XmStringFree(label);
		    XtManageChild(openFSD);
		} else {
		    XtManageChild(openFSD);
		}
		break;
	    case FILE_SAVE_BTN:
		break;
	    case FILE_SAVE_AS_BTN:
		break;
	    case FILE_CLOSE_BTN:
		XtPopdown(channelS);
		break;
    }
}





/*
 * directly invoked routines...
 */


void createChannel()
{
  Widget paletteSW;

  XmString buttons[N_MAX_MENU_ELES];
  KeySym keySyms[N_MAX_MENU_ELES];
  XmButtonType buttonType[N_MAX_MENU_ELES];
  Widget channelMB;
  Widget channelHelpPDM;
  Widget menuHelpWidget;


 int i, n;

 Arg args[10];



 openFSD = NULL;

/*
 * create a main window in a shell
 */
 n = 0;
 XtSetArg(args[n],XmNiconName,"Channels"); n++;
 XtSetArg(args[n],XmNtitle,"Channel Palette"); n++;
 XtSetArg(args[n],XtNallowShellResize,TRUE); n++;
 XtSetArg(args[n],XmNkeyboardFocusPolicy,XmEXPLICIT); n++;
/* map window manager menu Close function to application close... */
 XtSetArg(args[n],XmNdeleteResponse,XmDO_NOTHING); n++;
 channelS = XtCreatePopupShell("channelS",topLevelShellWidgetClass,
		mainShell,args,n);
 XmAddWMProtocolCallback(channelS,WM_DELETE_WINDOW,
		(XtCallbackProc)wmCloseCallback, (XtPointer)OTHER_SHELL);

 channelMW = XmCreateMainWindow(channelS,"channelMW",NULL,0);


/*
 * create the menu bar
 */
  buttons[0] = XmStringCreateSimple("File");
  buttons[1] = XmStringCreateSimple("Help");
  keySyms[0] = 'F';
  keySyms[1] = 'H';
  n = 0;
  XtSetArg(args[n],XmNbuttonCount,N_MAIN_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonMnemonics,keySyms); n++;
  channelMB = XmCreateSimpleMenuBar(channelMW, "channelMB",args,n);

  /* set the Help cascade button in the menu bar */
  menuHelpWidget = XtNameToWidget(channelMB,"*button_1");
  XtVaSetValues(channelMB,XmNmenuHelpWidget,menuHelpWidget,
		NULL);
  for (i = 0; i < N_MAIN_MENU_ELES; i++) XmStringFree(buttons[i]);


/*
 * create the file pulldown menu pane
 */
  buttons[0] = XmStringCreateSimple("Open...");
  buttons[1] = XmStringCreateSimple("Save");
  buttons[2] = XmStringCreateSimple("Save As...");
  buttons[3] = XmStringCreateSimple("Separator");
  buttons[4] = XmStringCreateSimple("Close");
  keySyms[0] = 'O';
  keySyms[1] = 'S';
  keySyms[2] = 'A';
  keySyms[3] = ' ';
  keySyms[4] = 'C';
  buttonType[0] = XmPUSHBUTTON;
  buttonType[1] = XmPUSHBUTTON;
  buttonType[2] = XmPUSHBUTTON;
  buttonType[3] = XmSEPARATOR;
  buttonType[4] = XmPUSHBUTTON;
  n = 0;
  XtSetArg(args[n],XmNbuttonCount,N_FILE_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonType,buttonType); n++;
  XtSetArg(args[n],XmNbuttonMnemonics,keySyms); n++;
  XtSetArg(args[n],XmNpostFromButton,FILE_BTN_POSN); n++;
  XtSetArg(args[n],XmNsimpleCallback,(XtCallbackProc)fileMenuSimpleCallback);
	n++;
  channelFilePDM = XmCreateSimplePulldownMenu(channelMB,"channelFilePDM",
	args,n);
  for (i = 0; i < N_FILE_MENU_ELES; i++) XmStringFree(buttons[i]);



/*
 * create the help pulldown menu pane
 */
  buttons[0] = XmStringCreateSimple("On Channel Palette...");
  keySyms[0] = 'C';
  buttonType[0] = XmPUSHBUTTON;
  n = 0;
  XtSetArg(args[n],XmNbuttonCount,N_HELP_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonType,buttonType); n++;
  XtSetArg(args[n],XmNbuttonMnemonics,keySyms); n++;
  XtSetArg(args[n],XmNpostFromButton,HELP_BTN_POSN); n++;
  channelHelpPDM = XmCreateSimplePulldownMenu(channelMB,
		"channelHelpPDM",args,n);
  XmStringFree(buttons[0]);



/*
 * Add the Palette Radio Box for the drawing channel toggle buttons
 *
 */
 paletteSW = XmCreateScrolledWindow(channelMW,"paletteSW",NULL,0);

  XmMainWindowSetAreas(channelMW,channelMB,NULL,NULL,NULL,paletteSW);


/*
 * manage the composites
 */
  XtManageChild(channelMB);
  XtManageChild(paletteSW);
  XtManageChild(channelMW);

}


