/******************************************************************************
 *  
 * Author: Vladimir T. Romanovski  (romsky@x4u2.desy.de)
 *
 * Organization: KRYK/@DESY 1996
 *

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>         /* system() */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include "dm2k.h"
#include "dm2kWidget.h"
  
#ifdef __cplusplus
extern "C" void help_protocol (Widget);
#endif

/******************************************************************************
 *
 * callback: dm2k_help_callback
 *         
 * It is intended to read DM2K_HELP system environment
 * and to make system call(via system()) of given script. 
 * The parameter for the script will be a title of shell widget.
 *
*****************************************************************************/
static void dm2k_help_callback (Widget shell, 
				caddr_t closure,
				caddr_t call_data)
{
  char * title = NULL;
  char * env = getenv(DM2K_HELP_ENV);

  if (env != NULL) 
  {
    char * command;
    
    XtVaGetValues (shell, XmNtitle, &title, NULL);

    command = (char*) malloc (STRLEN(env) + STRLEN(title) + 5);
    sprintf (command, "%s %s &", env, title);

    (void) system (command);
    free (command);
  } 
  else 
  {
    fprintf (stderr, "Sorry, environment DM2K_HELP is not set..\n");
  }
}


/*****************************************************************************
*
 * function: help_protocol(Widget shell)
 *
 * It is intented for installing customized Motif window manager protocol for
 * shell widget.
 *

*****************************************************************************/
void help_protocol (Widget shell)
{
  Atom message, protocol;
  char buf[80];

  if (getenv(DM2K_HELP_ENV)) {
     message = XmInternAtom (XtDisplay(shell), "_MOTIF_WM_MESSAGES", FALSE);
     sprintf( buf, "_%s_HELP", MAIN_NAME );
     protocol = XmInternAtom (XtDisplay(shell), buf, FALSE);
     
     XmAddProtocols (shell, message, &protocol, 1);
     XmAddProtocolCallback (shell, message, protocol, 
			    (XtCallbackProc)dm2k_help_callback, NULL);

     sprintf (buf, "%s_Help _l Ctrl<Key>l f.send_msg %ld", MAIN_Name, protocol);
     XtVaSetValues (shell, XmNmwmMenu, buf, NULL);
  }
}
