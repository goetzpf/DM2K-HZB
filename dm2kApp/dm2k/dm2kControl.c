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
 * .02  09-08-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 * .                    Fabien  Add writeDlSensitive
 *
 *****************************************************************************
*/

#include "dm2k.h"

#if 0
#ifdef __cplusplus
static void createDlControl(
  DisplayInfo *,
  DlControl *control)
#else
static void createDlControl(
  DisplayInfo *displayInfo,
  DlControl *control)
#endif
{
  if (control != NULL) {
    renewString(&control->ctrl,globalResourceBundle.ctrl);
    control->clr = globalResourceBundle.clr;
    control->bclr = globalResourceBundle.bclr;
  }
}
#endif

void controlAttributeInit(DlControl *control) 
{
  if (control != NULL) {
    control->ctrl = NULL;
    control->clr  = 14;
    control->bclr = 0;
  }
}

void controlAttributeDestroy(DlControl *control) 
{
  if (control != NULL) {
    DM2KFREE(control->ctrl);
    control->ctrl = NULL;
  }
}

void controlAttributeCopy(DlControl * to, DlControl * from) 
{
  if (from != NULL && to != NULL) {
    to->clr  = from->clr;
    to->bclr = from->bclr;
    renewString(&to->ctrl, from->ctrl);
  }
}

void parseControl(
  DisplayInfo *displayInfo,
  DlControl *control)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"ctrl") || STREQL(token,"chan")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&control->ctrl,token);
      } 
      else if (STREQL(token,"clr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	control->clr = atoi(token) % DL_MAX_COLORS;
      } 
      else if (STREQL(token,"bclr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	control->bclr = atoi(token) % DL_MAX_COLORS;
      }
      break;
    case T_LEFT_BRACE:
      nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
      break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );
}

void writeDlControl(
  FILE *stream,
  DlControl *dlControl,
  int level)
{
  char indent[256]; level=MIN(level,256-2);

  level = MIN(level, 256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%scontrol {",indent);
    if ((dlControl->ctrl != NULL) && dlControl->ctrl[0])
      fprintf(stream,"\n%s\tchan=\"%s\"",indent,dlControl->ctrl);
    fprintf(stream,"\n%s\tclr=%d",indent,dlControl->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlControl->bclr);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%scontrol {",indent);
    fprintf(stream,"\n%s\tctrl=\"%s\"",indent,
	    dlControl->ctrl ? dlControl->ctrl : "");
    fprintf(stream,"\n%s\tclr=%d",indent,dlControl->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlControl->bclr);
    fprintf(stream,"\n%s}",indent);
	}
#endif
}


/* Routines for Sensitivity of a controller */
/* ---------------------------------------- */

void sensitveAttributeInit (
  DlSensitive *sensitive)
{
  sensitive->chan = NULL;
  sensitive->mode = IF_NOT_ZERO;
}

void sensitveAttributeDestroy (
  DlSensitive *sensitive)
{
  DM2KFREE(sensitive->chan);
  sensitive->chan = NULL;
}

void sensitveAttributeCopy (
  DlSensitive * to,
  DlSensitive * from)
{
  renewString(&to->chan, from->chan);
  to->mode = from->mode;
}

void parseSensitive (
  DisplayInfo *displayInfo,
  DlSensitive *sensitive)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"ctrl") || STREQL(token,"chan")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&sensitive->chan,token);
      } 
      else if (STREQL(token,"sensitive_mode")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"if not zero"))
	  sensitive->mode = IF_NOT_ZERO;
	else if (STREQL(token,"if zero"))
	  sensitive->mode = IF_ZERO;
	else
	  printf("unknown token in file``%s''\n",token);
      }
      break;
    case T_LEFT_BRACE:
      nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
       break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
                && (tokenType != T_EOF) );
}


void writeDlSensitive (
  FILE *stream,
  DlSensitive *sensitive,
  int level)
{
  char indent[256]; level=MIN(level,256-2);

  if (sensitive->chan == NULL) 
    return;

  level = MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%ssensitive {",indent);
    if ((sensitive->chan != NULL) && sensitive->chan[0] )
      fprintf(stream,"\n%s\tchan=\"%s\"",indent,sensitive->chan);
    fprintf(stream,"\n%s\tsensitive_mode=\"%s\"",
	    indent,stringValueTable[sensitive->mode]);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%ssensitive {",indent);
    fprintf(stream,"\n%s\tctrl=\"%s\"",indent,
	    sensitive->chan ? sensitive->chan : "");
    fprintf(stream,"\n%s\tsensitive_mode=\"%s\"",
	    indent,stringValueTable[sensitive->mode]);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}


/* Create the Sensitive PV record */

Record *sensitiveCreateRecord (
  DlSensitive *dlSensitive,
  void (*updateValueCb)(XtPointer),
  void (*updateGraphicalInfoCb)(XtPointer),
  XtPointer pmb,            /* the object dynamic descriptor */
  Boolean *sensitive_flg)
{
  Boolean sensitive;
  Record *record;

  sensitive = ( dlSensitive->chan != NULL ) && dlSensitive->chan[0];

  if ( sensitive_flg ) 
    *sensitive_flg = sensitive;

  if ( ! sensitive ) 
    return NULL;

  record = dm2kAllocateRecord (dlSensitive->chan, updateValueCb,
			       updateGraphicalInfoCb, pmb);
  return (record);
}


/* Destroy the Sensitive PV record */

void sensitiveDestroyRecord (
  Record **record_pt,
  Boolean *sensitive_flg)
{
  if ( sensitive_flg ) *sensitive_flg = False;
  if ( ! *record_pt ) return;
  dm2kDestroyRecord (*record_pt);
  *record_pt = NULL;
}


/* Set / Reset the widget sensitivity state */

void sensitiveSetWidget (
  Widget widget,
  DlSensitive *dlSensitive,
  Record *record,
  UpdateTask *updateTask)
{
  if ( !widget ) return;

  if ( globalDm2kReadOnly ) {
    XtSetSensitive (widget, False);
    if (XtIsRealized(widget))
      XDefineCursor(display, XtWindow(widget), readOnlyCursor);
    return;
  }

  if ( !record ) return;

  if (! record->connected) {
    /* white rectangle cannot be used :
     * it can result in manage / unmanage oscillations !
     */
    XtSetSensitive (widget, False);
    if (XtIsRealized(widget))
      XDefineCursor(display, XtWindow(widget), readOnlyCursor);
    return;
  }

  if (record->readAccess) {
    int sensv;
    sensv = (((int)record->value) != 0);
    if (dlSensitive->mode != IF_NOT_ZERO)
      sensv = !sensv;

    XtSetSensitive (widget, sensv);
    if (XtIsRealized(widget)) {
      if (sensv)
	XDefineCursor(display, XtWindow(widget), None);
      else
	XDefineCursor(display, XtWindow(widget), readOnlyCursor);
    }
  }
}



/* Build info for the controller sensitivity */

void sensitiveInfo (
  char *msg,
  DlSensitive *dlSensitive,
  Record *record)
{
  char strg[512];

  if ( ! record ) return;

  strcat (msg, "\n\n  Sensitivity PV : Sensitive if PV is ");
  if (dlSensitive->mode != IF_NOT_ZERO)
    strcat (msg, "Zero\n");
  else
    strcat (msg, "Not Zero\n");

  dm2kChanelInfo (strg, record, dlSensitive->chan);
  strcat (msg, strg);
}


/* generic information for a sensitive controller */

void sensitiveControllerInfoSimple (char *msg,
		      DlControl *control,
		      DlSensitive *sensitive)
{
  strcat (msg, " ");
  if ( sensitive ) {
    if  (sensitive->chan != NULL)
      strcat (msg, "(sensitive) ");
  }

  strcat (msg, control->ctrl ? control->ctrl : "");
}

void sensitiveControllerInfo (char *msg,
		      DlControl *dlControl,
		      Record *ctrlRecord,
		      DlSensitive *dlSensitive,
		      Record *sensitiveRecord)
{
  char strg[512];

  if ( sensitiveRecord ) {
    strcat (msg, "\n    Sensitivity controlled");
  }

  if ( dlControl ) {
    dm2kChanelInfo (strg, ctrlRecord, dlControl->ctrl);
    sprintf (&msg[STRLEN(msg)], "\n\n  Control PV :\n%s", strg);
  }
  sensitiveInfo (msg, dlSensitive, sensitiveRecord);
}


/* Event handlers for controller objects in EDIT mode */

void editObjectHandler (
      DisplayInfo *displayInfo,
      DlElement   *dlElement) 
{
  /* add button press handlers */
  XtAddEventHandler (dlElement->widget, ButtonPressMask, False,
		     handleButtonPress, (XtPointer)displayInfo);

  /* add enter / leave window handlers */
  XtAddEventHandler (dlElement->widget, EnterWindowMask, False,
		     handleEnterObject, (XtPointer)dlElement);
  XtAddEventHandler (dlElement->widget, LeaveWindowMask, False,
		     handleLeaveObject, (XtPointer)dlElement);
}


void controlHandler (
      DisplayInfo *displayInfo,
      DlElement   *dlElement) 
{
  /* remove all translations if in edit mode */
  XtUninstallTranslations (dlElement->widget);

  editObjectHandler (displayInfo, dlElement);
}
