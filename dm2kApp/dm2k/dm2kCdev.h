/*-----------------------------------------------------------------------------
 * Copyright (c) 1994,1995 Southeastern Universities Research Association,
 *                         Continuous Electron Beam Accelerator Facility
 *
 * This software was developed under a United States Government license
 * described in the NOTICE file included as part of this distribution.
 *
 *-----------------------------------------------------------------------------
 *
 * Description:
 *      DM2k CDEV Interface Header file
 *      
 *      Device Name And Attribute Name Cannot Exceed Length 127
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab. High Performance Computing Group
 *
 * Revision History:
 *   $Log: dm2kCdev.h,v $
 *   Revision 1.2  2006-03-28 15:32:06  birke
 *   now a 3.14 application...
 *
 *   Revision 1.1  2000/02/23 16:17:12  birke
 *   += cdev-support
 *
 *
 */

#ifndef _DM2K_CDEV_H
#define _DM2K_CDEV_H

#ifdef DM2K_CDEV

#define BYTE dm2k_cdev_byte

#include <stdarg.h>
#include <cdev.h>
#include <cdevTypes.h>

#define DBF_STRING CDEV_STR
#define DBF_INT    CDEV_INT_16
#define DBF_SHORT  CDEV_INT_16
#define DBF_LONG   CDEV_INT_32
#define DBF_FLOAT  CDEV_FLT
#define DBF_CHAR   CDEV_BYTE_
#define DBF_DOUBLE CDEV_DBL
/* Special data type */
#define DBF_ENUM   100

#undef BYTE

#if defined (__cplusplus)
extern "C" {
#endif

extern int  dm2kCDEVInitialize (void);
extern void dm2kCDEVTerminate  (void);
extern void cdevCheckEvent     (char *);
extern void cdevPendEvent      (char *);

typedef struct _Record {
  long               dev;
  int                elementCount;
  short              dataType;
  double             value;
  double             hopr;
  double             lopr;
  short              precision;
  short              status;
  short              severity;
  Boolean            connected;
  Boolean            readAccess;
  Boolean            writeAccess;
  Boolean            useMsgWhenWrite[2];
  char               **stateStrings;
  char               *name;
  char               *attr; 
  char               *verb;
  char               *fullname;  /* device attr */
  XtPointer          array;
  cdev_TS_STAMP      time;
  int                autoscale;

  XtPointer          clientData;
  void (*updateValueCb)(XtPointer); 
  void (*updateGraphicalInfoCb)(XtPointer); 
  Boolean  monitorSeverityChanged;
  Boolean  monitorValueChanged;
  Boolean  monitorZeroAndNoneZeroTransition;

  double   lowerAlarmLimit;
  double   upperAlarmLimit;
  double   lowerWarningLimit;
  double   upperWarningLimit;
} Record;

extern void dm2kDestroyRecord            (Record *pr);
extern void dm2kRecordAddUpdateValueCb   (Record *pr, 
					  void (*updateValueCb)(XtPointer));
extern void dm2kRecordAddGraphicalInfoCb (Record *pr, 
					  void (*updateGraphicalInfoCb)(XtPointer));

#define CA_PEND_EVENT_TIME 1e-6			/* formerly 0.0001 */
#define caCheckEvent cdevCheckEvent
#define caPendEvent  cdevPendEvent

#define NO_ALARM		0x0
#define	MINOR_ALARM		0x1
#define	MAJOR_ALARM		0x2
#define INVALID_ALARM		0x3
#define ALARM_NSEV		INVALID_ALARM+1

typedef long                    dbr_long_t;
typedef cdev_TS_STAMP           TS_STAMP;

#define MAX_ENUM_STATES		16
#define db_state_dim		MAX_ENUM_STATES	
#define MAX_STRING_SIZE         40

typedef struct _pv_type_name_
{
  char *name;
  int   type;
}dm2kPvTypeName;

struct dbr_time_string{
  short  	status;	 		/* status of value */
  short	        severity;		/* severity of alarm */
  cdev_TS_STAMP	stamp;			/* time stamp */
  char* 	value;			/* current value */
};


/* Misc utility routines */
extern char* dm2kPvName                 (Record* pr);

extern char* dm2kPvType                 (Record* pr);

extern int   dm2kPvDataType             (Record* pr);

extern int   dm2kPvCount                (Record* pr);

extern int   dm2kPvGetValue             (Record* pr, struct dbr_time_string *value);

extern void  dm2kSendMsg                (Record* pr, char* msg);

enum tsTextType{
    TS_TEXT_MONDDYYYY,
    TS_TEXT_MMDDYY
};

char * dm2k_tsStampToText (cdev_TS_STAMP* ts, int type, char* buffer);

#define tsStampToText dm2k_tsStampToText


#define  PVNAME_STRINGSZ 29	/* includes NULL terminator for PVNAME_SZ */
#define	 PVNAME_SZ  (PVNAME_STRINGSZ - 1) /*Process Variable Name Size	*/
#define	 FLDNAME_SZ   4	                  /*Field Name Size		*/

#if defined (__cplusplus)
};
#endif

#endif     /* #ifdef DM2K_CDEV */

#endif     /* #ifndef _DM2K_CDEV_H */
