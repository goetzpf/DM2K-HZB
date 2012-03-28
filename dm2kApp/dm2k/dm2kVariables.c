static char RcsId[]      =
"@(#)$Id: dm2kVariables.c,v 1.3 2009-01-13 10:35:28 birke Exp $";

/*+*********************************************************************
 *
 * Time-stamp: <08 Jun 00 16:16:46 Thomas Birke>
 *
 * File:       dm2Variables.c
 * Project:    
 *
 * Descr.:     
 *
 * Author(s):  Thomas Birke
 *
 * $Revision: 1.3 $
 * $Date: 2009-01-13 10:35:28 $
 *
 * $Author: birke $
 *
 * $Log: dm2kVariables.c,v $
 * Revision 1.3  2009-01-13 10:35:28  birke
 * latest changes/fixes (numerous small)
 *
 * Revision 1.2  2006/03/28 15:32:06  birke
 * now a 3.14 application...
 *
 * Revision 1.1  2006/01/18 06:19:50  birke
 * changes made over the last 4 years...
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

#include "dm2k.h"
#include "dm2kVariables.h"

#include <stdio.h>

void parseVariables(DisplayInfo *displayInfo) 
{
   char         token[MAX_TOKEN_LENGTH];
   TOKEN        tokenType;
   int          nestingLevel = 0;
   DlVariable  *nVar = NULL;

   do {
      switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	 if (!strcmp(token,"var")) {
	    nVar = DM2KALLOC(DlVariable);
	    if (nVar) {
	       nVar->next = displayInfo->dlVars;
	       if (displayInfo->dlVars)
		  displayInfo->dlVars->prev = nVar;
	       displayInfo->dlVars = nVar;
	    }
	 } else if (!strcmp(token,"name")) {
	    getToken(displayInfo,token);
	    getToken(displayInfo,token);
	    if (displayInfo->dlVars) {
	       DM2KFREE(displayInfo->dlVars->name);
	       displayInfo->dlVars->name = STRDUP(token);
	    }
	 } else if (!strcmp(token,"value")) {
	    getToken(displayInfo,token);
	    getToken(displayInfo,token);
	    if (displayInfo->dlVars) {
	       DM2KFREE(displayInfo->dlVars->value);
	       displayInfo->dlVars->value = STRDUP(token);
	    }
	 }
      case T_EQUAL:
	 break;
      case T_LEFT_BRACE:
	 nestingLevel++; break;
      case T_RIGHT_BRACE:
	 nestingLevel--; break;
      }
   } while ( (nestingLevel > 0) && (tokenType != T_EOF) );
}

void writeVariables(
  FILE *stream,
  DisplayInfo *displayInfo)
{
   DlVariable *tmp = displayInfo->dlVars;
   if (tmp) {
      fprintf(stream,"\nvariables {");
      while (tmp) {
	 fprintf(stream,"\n\tvar {");
	 fprintf(stream,"\n\t\tname=\"%s\"", tmp->name);
	 fprintf(stream,"\n\t\tvalue=\"%s\"", tmp->value);
	 fprintf(stream,"\n\t}");
	 tmp = tmp->next;
      }
      fprintf(stream,"\n}");
   }
}

