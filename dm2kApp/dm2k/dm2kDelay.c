static char RcsId[]      =
"@(#)$Id: dm2kDelay.c,v 1.4 2009-01-13 10:35:28 birke Exp $";

/*+*********************************************************************
 *
 * Time-stamp: <07 Jan 00 15:40:29 Thomas Birke>
 *
 * File:       dm2kDelay.c
 * Project:    
 *
 * Descr.:     
 *
 * Author(s):  Thomas Birke
 *
 * $Revision: 1.4 $
 * $Date: 2009-01-13 10:35:28 $
 *
 * $Author: birke $
 *
 * $Log: dm2kDelay.c,v $
 * Revision 1.4  2009-01-13 10:35:28  birke
 * latest changes/fixes (numerous small)
 *
 * Revision 1.3  2006/03/28 15:32:06  birke
 * now a 3.14 application...
 *
 * Revision 1.2  2000/01/11 13:42:38  birke
 * prerelease 2.5.1
 *
 * Revision 1.1  1999/10/22 13:29:25  birke
 * ...2.5.1
 *
 *
 * This software is copyrighted by the BERLINER SPEICHERRING 
 * GESELLSCHAFT FUER SYNCHROTRONSTRAHLUNG M.B.H., BERLIN, GERMANY. 
 * The following terms apply to all files associated with the 
 * software. 
 *
 * BESSY hereby grants permission to use, copy, and modify this 
 * software and its documentation for non-commercial educational or 
 * research purposes, provided that existing copyright notices are 
 * retained in all copies. 
 *
 * The receiver of the software provides BESSY with all enhancements, 
 * including complete translations, made by the receiver.
 *
 * IN NO EVENT SHALL BESSY BE LIABLE TO ANY PARTY FOR DIRECT, 
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING 
 * OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY 
 * DERIVATIVES THEREOF, EVEN IF BESSY HAS BEEN ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 *
 * BESSY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 * A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS 
 * PROVIDED ON AN "AS IS" BASIS, AND BESSY HAS NO OBLIGATION TO 
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR 
 * MODIFICATIONS. 
 *
 * Copyright (c) 1997 by Berliner Elektronenspeicherring-Gesellschaft
 *                            fuer Synchrotronstrahlung m.b.H.,
 *                                    Berlin, Germany
 *
 *********************************************************************-*/

#include <stdlib.h>

#include "dm2k.h"
#include "dm2kDelay.h"

/* bookkeeping of installed and still pending timeouts */
struct DelayTimeOut {
   XtIntervalId id;
   XtPointer key;
   struct DelayTimeOut *next;
   struct DelayTimeOut *prev;
};

struct DelayTimeOut *delayTimeOutHead = 0;

/* just to store CB-relevant data */
struct DelayCB {
   DelayProc *p;
   XtPointer closure;
};

/* create and fill an new DelayCB structure */
XtPointer newCB(DelayProc p, XtPointer closure)
{
   struct DelayCB* cb = calloc(1, sizeof(struct DelayCB));
   cb->p = p;
   cb->closure = closure;
   return (XtPointer)cb;
}

static int tos = 0;

void removeTimeOut( struct DelayTimeOut *l ) 
{
   if (l->prev) l->prev->next = l->next;
   if (l->next) l->next->prev = l->prev;
   if ( l == delayTimeOutHead ) delayTimeOutHead = l->next;
   free(l);
}

void searchNremoveTimeOut( XtIntervalId id ) 
{
   struct DelayTimeOut *l = delayTimeOutHead;
   struct DelayTimeOut *n;
   while (l) {
      n = l->next;
      if (l->id == id) {
	 removeTimeOut(l);
	 return;
      }
      l = n;
   }
}

void insertTimeOut (XtPointer key, XtIntervalId id ) 
{
   struct DelayTimeOut *l = calloc( 1, sizeof(struct DelayTimeOut));
   l->id = id;
   l->key = key;
   l->next = delayTimeOutHead;
   l->prev = 0;
   if (delayTimeOutHead) {
      delayTimeOutHead->prev = l;
   }
   delayTimeOutHead = l;
}


/* the central callback function */
void delayCB ( XtPointer closure, XtIntervalId *idp )
{
   struct DelayCB* cb = (struct DelayCB*)closure;
   cb->p(cb->closure);
   searchNremoveTimeOut(*idp);
   free(cb);
}

void delayExec( int ms, XtPointer key, DelayProc p, XtPointer closure ) 
{
   insertTimeOut(key, XtAppAddTimeOut( appContext, ms, 
				       (XtTimerCallbackProc)delayCB,
				       newCB(p, closure) ) );
}

void cancelDelay( XtPointer key ) 
{
   struct DelayTimeOut *l = delayTimeOutHead;
   struct DelayTimeOut *n;
   while (l) {
      n = l->next;
      if (l->key == key) {
	 XtRemoveTimeOut(l->id);
	 removeTimeOut(l);
      }
      l = n;
   }
}

