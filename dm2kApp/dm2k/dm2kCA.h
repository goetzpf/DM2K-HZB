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
 * .02  09-05-95        vong    2.1.0 release
 *
 *****************************************************************************
*/

/*
 * include file for widget-based display manager
 */

#ifndef __DM2KCA_H__
#define __DM2KCA_H__

#define __USING_TIME_STAMP__

#ifdef ALLOCATE_STORAGE
#define EXTERN
#else
#define EXTERN extern
#endif


#include "cadef.h"
#include "db_access.h"
#include "alarm.h"			/* alarm status, severity */

/* allow ca_build_and_connect to take NULL strings if user specifies them,
   and treat them as " " a whitespace string
 */
#define CA_BUILD_AND_CONNECT(a,b,c,d,e,f,g) \
	( a == NULL ? ca_build_and_connect(" ",b,c,d,e,f,g) : \
	 ( (a)[0] == '\0' ? ca_build_and_connect(" ",b,c,d,e,f,g) : \
	   ca_build_and_connect(a,b,c,d,e,f,g) ) )
	


#define CA_PEND_EVENT_TIME	1e-6			/* formerly 0.0001    */
#define MAX_STATE_STRING_SIZE	(db_state_text_dim+1)	/* from db_access.h   */
				/* also have MAX_STRING_SIZE from db_access.h */

/***
 *** new data types
 ***/

#ifdef __USING_TIME_STAMP__
typedef union {
  struct dbr_time_string s;
  struct dbr_time_enum   e;
  struct dbr_time_char   c;
  struct dbr_time_short  i;
  struct dbr_time_long   l;
  struct dbr_time_float  f;
  struct dbr_time_double d;
} dataBuf;
#else
typedef union {
  struct dbr_sts_string s;
  struct dbr_sts_enum   e;
  struct dbr_sts_char   c;
  struct dbr_sts_short  i;
  struct dbr_sts_long   l;
  struct dbr_sts_float  f;
  struct dbr_sts_double d;
} dataBuf;
#endif

typedef union {
  struct dbr_sts_string  s;
  struct dbr_ctrl_enum   e;
  struct dbr_ctrl_char   c;
  struct dbr_ctrl_short  i;
  struct dbr_ctrl_long   l;
  struct dbr_ctrl_float  f;
  struct dbr_ctrl_double d;
} infoBuf;

#define MAX_EVENT_DATA 16
typedef struct _Record {
  int       caId;
  int       elementCount;
  short     dataType;
  double    value;
  double    hopr;
  double    lopr;
  short     precision;
  short     status;
  short     severity;
  Boolean   connected;
  Boolean   readAccess;
  Boolean   writeAccess;
  char      *stateStrings[16];
  const char *name;
  XtPointer array;
  TS_STAMP  time;

  XtPointer clientData;
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

typedef struct _Channel {
  int       caId;
  dataBuf   *data;
  infoBuf   info;
  chid      chID;
  evid      evID;
  int       size;             /* size of data buffer (number of char) */
  Record    *pr;
  Boolean   previouslyConnected;
} Channel;

void dm2kDestroyRecord(Record *pr);
void dm2kRecordAddUpdateValueCb(Record *pr, void (*updateValueCb)(XtPointer));
void dm2kRecordAddGraphicalInfoCb(Record *pr, void (*updateGraphicalInfoCb)(XtPointer));
#endif  /* __DM2KCA_H__ */
