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

static Widget variableFilePDM, openFSD;

#define VARIABLE_DIALOG_MASK ".var"

#define v_N_MAX_MENU_ELES 5

enum 
{
   v_FILE_BTN_POSN,
   v_HELP_BTN_POSN,
   v_N_MAIN_MENU_ELES
};


/*
 * create the file pulldown menu pane
 */
enum 
{
   v_FILE_OPEN_BTN,
   v_FILE_SAVE_BTN,
   v_FILE_SAVE_AS_BTN,
   v_FILE_CLOSE_BTN,
   v_N_FILE_MENU_ELES
};

/*
 * create the help pulldown menu pane
 */
enum 
{
   v_HELP_BTN,
   v_N_HELP_MENU_ELES
};



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
	    case v_FILE_OPEN_BTN:
		if (openFSD == NULL) {
		    n = 0;
		    label = XmStringCreateSimple(VARIABLE_DIALOG_MASK);
		    XtSetArg(args[n],XmNdirMask,label); n++;
		    XtSetArg(args[n],XmNdialogStyle,
				XmDIALOG_PRIMARY_APPLICATION_MODAL); n++;
		    openFSD = XmCreateFileSelectionDialog(variableFilePDM,
				"openFSD",args,n);
/* make Filter text field insensitive to prevent user hand-editing dirMask */
		    textField = XmFileSelectionBoxGetChild(openFSD,
				XmDIALOG_FILTER_TEXT);
		    XtSetSensitive(textField,FALSE);
		    XtAddCallback(openFSD,XmNokCallback,
				(XtCallbackProc)fileOpenCallback,
				v_FILE_OPEN_BTN);
		    XtAddCallback(openFSD,XmNcancelCallback,
				(XtCallbackProc)fileOpenCallback,v_FILE_OPEN_BTN);
		    XmStringFree(label);
		    XtManageChild(openFSD);
		} else {
		    XtManageChild(openFSD);
		}
		break;
	    case v_FILE_SAVE_BTN:
		break;
	    case v_FILE_SAVE_AS_BTN:
		break;
	    case v_FILE_CLOSE_BTN:
		XtPopdown(variableS);
		break;
    }
}





/*
 * directly invoked routines...
 */


void createVariable()
{
  Widget paletteSW;

  XmString buttons[v_N_MAX_MENU_ELES];
  KeySym keySyms[v_N_MAX_MENU_ELES];
  XmButtonType buttonType[v_N_MAX_MENU_ELES];
  Widget variableMB;
  Widget variableHelpPDM;
  Widget menuHelpWidget;


 int i, n;

 Arg args[10];



 openFSD = NULL;

/*
 * create a main window in a shell
 */
 n = 0;
 XtSetArg(args[n],XmNiconName,"Variables"); n++;
 XtSetArg(args[n],XmNtitle,"Variable Palette"); n++;
 XtSetArg(args[n],XtNallowShellResize,TRUE); n++;
 XtSetArg(args[n],XmNkeyboardFocusPolicy,XmEXPLICIT); n++;
/* map window manager menu Close function to application close... */
 XtSetArg(args[n],XmNdeleteResponse,XmDO_NOTHING); n++;
 variableS = XtCreatePopupShell("variableS",topLevelShellWidgetClass,
		mainShell,args,n);
 XmAddWMProtocolCallback(variableS,WM_DELETE_WINDOW,
		(XtCallbackProc)wmCloseCallback, (XtPointer)OTHER_SHELL);

 variableMW = XmCreateMainWindow(variableS,"variableMW",NULL,0);


/*
 * create the menu bar
 */
  buttons[0] = XmStringCreateSimple("File");
  buttons[1] = XmStringCreateSimple("Help");
  keySyms[0] = 'F';
  keySyms[1] = 'H';
  n = 0;
  XtSetArg(args[n],XmNbuttonCount,v_N_MAIN_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonMnemonics,keySyms); n++;
  variableMB = XmCreateSimpleMenuBar(variableMW, "variableMB",args,n);

  /* set the Help cascade button in the menu bar */
  menuHelpWidget = XtNameToWidget(variableMB,"*button_1");
  XtVaSetValues(variableMB,XmNmenuHelpWidget,menuHelpWidget,
		NULL);
  for (i = 0; i < v_N_MAIN_MENU_ELES; i++) XmStringFree(buttons[i]);


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
  XtSetArg(args[n],XmNbuttonCount,v_N_FILE_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonType,buttonType); n++;
  XtSetArg(args[n],XmNbuttonMnemonics,keySyms); n++;
  XtSetArg(args[n],XmNpostFromButton,v_FILE_BTN_POSN); n++;
  XtSetArg(args[n],XmNsimpleCallback,(XtCallbackProc)fileMenuSimpleCallback);
	n++;
  variableFilePDM = XmCreateSimplePulldownMenu(variableMB,"variableFilePDM",
	args,n);
  for (i = 0; i < v_N_FILE_MENU_ELES; i++) XmStringFree(buttons[i]);



/*
 * create the help pulldown menu pane
 */
  buttons[0] = XmStringCreateSimple("On Variable Palette...");
  keySyms[0] = 'C';
  buttonType[0] = XmPUSHBUTTON;
  n = 0;
  XtSetArg(args[n],XmNbuttonCount,v_N_HELP_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonType,buttonType); n++;
  XtSetArg(args[n],XmNbuttonMnemonics,keySyms); n++;
  XtSetArg(args[n],XmNpostFromButton,v_HELP_BTN_POSN); n++;
  variableHelpPDM = XmCreateSimplePulldownMenu(variableMB,
		"variableHelpPDM",args,n);
  XmStringFree(buttons[0]);


  
/*
 * Add the Palette Radio Box for the drawing variable toggle buttons
 *
 */
  paletteSW = XmCreateScrolledWindow(variableMW,"paletteSW",NULL,0);

 XmMainWindowSetAreas(variableMW,variableMB,NULL,NULL,NULL,paletteSW);

 
/*
 * manage the composites
 */
  XtManageChild(variableMB);
  XtManageChild(paletteSW);
  XtManageChild(variableMW);

}


