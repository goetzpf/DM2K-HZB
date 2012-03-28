#ifndef _DM2KVARIABLES_H_
#define _DM2KVARIABLES_H_

/*+*********************************************************************
 *
 * Time-stamp: <08 Jun 00 14:29:09 Thomas Birke>
 *
 * File:       dm2kVariables.h
 * Project:    
 *
 * Descr.:     
 *
 * Author(s):  Thomas Birke
 *
 * $Revision: 1.2 $
 * $Date: 2006-03-28 15:32:06 $
 *
 * $Author: birke $
 *
 * $Log: dm2kVariables.h,v $
 * Revision 1.2  2006-03-28 15:32:06  birke
 * now a 3.14 application...
 *
 * Revision 1.1  2006/01/18 06:19:51  birke
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

typedef struct _DlVariable {
   char *name;
   char *value;
   struct _DlVariable *next;
   struct _DlVariable *prev;
} DlVariable;

#endif
