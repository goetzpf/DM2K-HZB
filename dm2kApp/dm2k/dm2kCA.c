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
 *                              - decouple the graphic object from
 *                                channel access.
 * .03  09-11-95        vong    conform to c++ syntax
 * .04  09-12-95        vong    temperory fix for callbacks after
 *                              ca_clear_event and ca_clear_channel
 * .05  09-28-95        vong    check for channel access return args.dbr as NULL.
 * .06  12-01-95        vong    limit the precision between 0 to 16
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien      Add channel information routine
 *
 *****************************************************************************
*/
#include "dm2k.h"

static void dm2kUpdateGraphicalInfoCb(struct event_handler_args args);
static void dm2kUpdateChannelCb(struct event_handler_args args);
static void dm2kCAFdRegistrationCb( void *dummy, int fd, int condition);
static void dm2kProcessCA(XtPointer, int *, XtInputId *);
Boolean dm2kWorkProc(XtPointer);
#if 0
static void dm2kAddUpdateRequest(Channel *);
static void dm2kRepaintRegion(Channel *);
#endif

#define CA_PAGE_SIZE 100
#define CA_PAGE_COUNT 10

typedef struct _CATask {
  int freeListSize;
  int freeListCount;
  int *freeList;
  Channel **pages;
  int pageCount;
  int pageSize;
  int nextPage;
  int nextFree;
  int channelCount;
  int channelConnected;
  int caEventCount;
} CATask;

static CATask caTask;

void CATaskGetInfo(int *channelCount, int *channelConnected, int *caEventCount) {
  *channelCount = caTask.channelCount;
  *channelConnected = caTask.channelConnected;
  *caEventCount = caTask.caEventCount;
  caTask.caEventCount = 0;
  return;
}

static void dm2kCAExceptionHandlerCb(struct exception_handler_args args) {
  if (args.chid == NULL) {
    dm2kPrintf("message : %s\n",ca_message(args.stat));
  } else {
    dm2kPrintf("channel name : %s\nmessage : %s\n\n",
          ca_name(args.chid),ca_message(args.stat));
  }
  dm2kPostTime();
  return;
}

int CATaskInit() 
{
  caTask.freeListSize = CA_PAGE_SIZE;
  caTask.freeListCount = 0;

  caTask.freeList = (int *) calloc (CA_PAGE_SIZE, sizeof(int));
  if (caTask.freeList == NULL) {
    dm2kPrintf("CATaskInit : memory allocation error!\n");
    return -1;
  }

  caTask.pages = (Channel **) calloc (CA_PAGE_COUNT, sizeof(Channel *));
  if (caTask.pages == NULL) {
    free((char*)caTask.freeList);
    caTask.freeList = NULL;
    dm2kPrintf("CATaskInit : memory allocation error!\n");
    return -1;
  }

  caTask.pageCount = 1;
  caTask.pageSize = CA_PAGE_COUNT;
  /* allocate the page */

  caTask.pages[0] = (Channel *) calloc(CA_PAGE_SIZE, sizeof(Channel));
  if (caTask.pages[0] == NULL) {
    free((char*)caTask.freeList);
    caTask.freeList = NULL;
    free((char*)caTask.pages);
    caTask.pages = NULL;
    dm2kPrintf("CATaskInit : memory allocation error!\n");
    return -1;
  }

  caTask.nextFree = 0;
  caTask.nextPage = 0;
  caTask.channelCount = 0;
  caTask.channelConnected = 0;
  caTask.caEventCount = 0;
  return ECA_NORMAL;
}

void caTaskDelete() {
  if (caTask.freeList) {
    free((char *)caTask.freeList);
    caTask.freeList = NULL;
    caTask.freeListCount = 0;
    caTask.freeListSize = 0;
  }  
  if (caTask.pageCount) {
    int i;
    for (i=0; i < caTask.pageCount; i++) {
      if (caTask.pages[i])
        free((char *)caTask.pages[i]);
    }
    free((char *)caTask.pages);
    caTask.pages = NULL;
    caTask.pageCount = 0;
  } 
}
  
int dm2kCAInitialize()
{
  int status;
  /*
   * add CA's fd to X
   */
  status = ca_add_fd_registration(dm2kCAFdRegistrationCb,NULL);
  if (status != ECA_NORMAL) return status;

  status = ca_add_exception_event(dm2kCAExceptionHandlerCb, NULL);
  if (status != ECA_NORMAL) return status;

  status = CATaskInit();
  return status;
}


static void caPendEventV (char *caller, float howlong)
{
  /*
   * flush CA send buffer and continue, since connection event handler used
   *	also flush X buffer
   */
  XFlush(display);
#ifdef __MONITOR_CA_PEND_EVENT__
  {
    double t;
    t = dm2kTime();
    ca_pend_event(howlong);
    t = dm2kTime() - t;
    if (t > 0.5) {
      printf("%s : time used by ca_pend_event = %8.1f\n", caller, t);
    }
  }
#else
  ca_pend_event(howlong);
#endif
}


void caCheckEvent (char *caller)
{
  caPendEventV (caller, 0.000001);
}

void caPendEvent (char *caller)
{
  caPendEventV (caller, CA_PEND_EVENT_TIME);
}

void dm2kCATerminate()
{
  caTaskDelete();
  /* cancel registration of the CA file descriptors */
  SEVCHK(ca_add_fd_registration(dm2kCAFdRegistrationCb,NULL),
                "\ndmTerminateCA:  error removing CA's fd from X");
  /* and close channel access */
  caPendEventV ("dm2kCATerminate", 20.0*CA_PEND_EVENT_TIME); /* don't allow early returns */

  SEVCHK(ca_task_exit(),"\ndmTerminateCA: error exiting CA");
}

#ifdef __cplusplus
static void dm2kCAFdRegistrationCb( void *, int fd, int condition)
#else
static void dm2kCAFdRegistrationCb( void *dummy, int fd, int condition)
#endif
{
  int currentNumInps;

#define NUM_INITIAL_FDS 100
typedef struct {
        XtInputId inputId;
        int fd;
} InputIdAndFd;
 static InputIdAndFd *inp = NULL;
 static int maxInps = 0, numInps = 0;
 int i, j, k;



 if (inp == NULL && maxInps == 0) {
/* first time through */
   inp = (InputIdAndFd *) calloc(1,NUM_INITIAL_FDS*sizeof(InputIdAndFd));
   maxInps = NUM_INITIAL_FDS;
   numInps = 0;
 }

 if (condition) {
/*
 * add new fd
 */
   if (numInps < maxInps-1) {

        inp[numInps].fd = fd;
        inp[numInps].inputId  = XtAppAddInput(appContext,fd,
                        (XtPointer)XtInputReadMask,
                        dm2kProcessCA,(XtPointer)NULL);
        numInps++;

   } else {

fprintf(stderr,"\ndmRegisterCA: info: realloc-ing input fd's array");

        maxInps = 2*maxInps;
#if defined(__cplusplus) && defined(SUNOS4) && !defined(__GNUG__)
        inp = (InputIdAndFd *) realloc ((malloc_t)inp,maxInps*sizeof(InputIdAndFd));
#else
        inp = (InputIdAndFd *) realloc (inp,maxInps*sizeof(InputIdAndFd));
#endif
        inp[numInps].fd = fd;
        inp[numInps].inputId  = XtAppAddInput(appContext,fd,
                        (XtPointer)XtInputReadMask,
                        dm2kProcessCA,(XtPointer)NULL);
        numInps++;
   }

 } else {

  currentNumInps = numInps;

/*
 * remove old fd/inputId
 */
   for (i = 0; i < numInps; i++) {
        if (inp[i].fd == fd) {
           XtRemoveInput(inp[i].inputId);
           inp[i].inputId = (XtInputId)NULL;
           inp[i].fd = (int)NULL;
           currentNumInps--;
        }
   }

/* now remove holes in the array */
   i = 0;
   while (i < numInps) {
        if (inp[i].inputId == (XtInputId)NULL) {
           j = i+1;
           k = 0;
           while(inp[j].inputId != (XtInputId)NULL) {
              inp[i+k].inputId = inp[j].inputId;
              inp[i+k].fd = inp[j].fd;
              j++;
              k++;
           }
           i = j-1;
        }
        i++;
   }
   numInps = currentNumInps;

 }

#ifdef DEBUG
fprintf(stderr,"\ndmRegisterCA: numInps = %d\n\t",numInps);
for (i = 0; i < maxInps; i++)
    fprintf(stderr,"%d ",inp[i].fd);
fprintf(stderr,"\n");
#endif

}

#ifdef __cplusplus
static void dm2kProcessCA(XtPointer, int *, XtInputId *)
#else
static void dm2kProcessCA(XtPointer dummy1, int *dummy2, XtInputId *dummy3)
#endif
{
  caPendEvent ("dm2kProcessCA");
}

static void dm2kReplaceAccessRightsEventCb(struct access_rights_handler_args args)
{
  Channel *pCh = (Channel *) ca_puser(args.chid);

  caTask.caEventCount++;
  if (globalDisplayListTraversalMode != DL_EXECUTE) return;
#if 1
  if ((!pCh) || (!pCh->chID)) return;
#endif

  pCh->pr->readAccess = ca_read_access(pCh->chID);
  pCh->pr->writeAccess = ca_write_access(pCh->chID);
  if (pCh->pr->updateValueCb) 
    pCh->pr->updateValueCb((XtPointer)pCh->pr); 
}

void dm2kConnectEventCb(struct connection_handler_args args) {
  int status;
  Channel *pCh = (Channel *) ca_puser(args.chid);

  caTask.caEventCount++;
  if (globalDisplayListTraversalMode != DL_EXECUTE) return;
#if 1
  if ((pCh == NULL) || (pCh->chID == NULL)) return;
#endif

  if ((args.op == CA_OP_CONN_UP) && (ca_read_access(pCh->chID))) {
    /* get the graphical information every time a channel is connected
       or reconnected. */
    status = ca_array_get_callback(dbf_type_to_DBR_CTRL(ca_field_type(args.chid)),
                      1, args.chid, dm2kUpdateGraphicalInfoCb, NULL);
    if (status != ECA_NORMAL) {
      dm2kPrintf("Error : connectionEventCb : ca_get_callback : %s\n",
           ca_message(status));
      dm2kPostTime();
    }
  }
  if ((args.op == CA_OP_CONN_UP) && (pCh->previouslyConnected == False)) {
    status = ca_replace_access_rights_event(pCh->chID,dm2kReplaceAccessRightsEventCb);
    if (status != ECA_NORMAL) {
      dm2kPrintf("Error : connectionEventCb : ca_replace_access_rights_event : %s\n",
		   ca_message(status));
      dm2kPostTime();
    }
#ifdef __USING_TIME_STAMP__
    status = ca_add_array_event(
                 dbf_type_to_DBR_TIME(ca_field_type(pCh->chID)),
		 ca_element_count(pCh->chID),pCh->chID,
		 dm2kUpdateChannelCb, pCh, 0.0,0.0,0.0, &(pCh->evID));
#else
    status = ca_add_array_event(
                 dbf_type_to_DBR_STS(ca_field_type(pCh->chID)),
		 ca_element_count(pCh->chID),pCh->chID,
		 dm2kUpdateChannelCb, pCh, 0.0,0.0,0.0, &(pCh->evID));
#endif
    if (status != ECA_NORMAL) {
      dm2kPrintf("Error : connectionEventCb : ca_add_event : %s\n",
		   ca_message(status));
      dm2kPostTime();
    }
    pCh->previouslyConnected = True;
    pCh->pr->elementCount = ca_element_count(pCh->chID);
    pCh->pr->dataType = ca_field_type(args.chid);
    pCh->pr->connected = True;
    caTask.channelConnected++;
  } else {
    if (args.op == CA_OP_CONN_UP) {
      pCh->pr->connected = True;
      caTask.channelConnected++;
    } else {
      pCh->pr->connected = False;
      caTask.channelConnected--;
    }   
    if (pCh->pr->updateValueCb)
      pCh->pr->updateValueCb((XtPointer)pCh->pr); 
  }
}


static void dm2kUpdateGraphicalInfoCb(struct event_handler_args args) {
  int nBytes;
  int i;
  Channel *pCh = (Channel *) ca_puser(args.chid);
  Record *pr = pCh->pr;

  if (!(args.status & CA_M_SUCCESS)) { return; }

  if (globalDisplayListTraversalMode != DL_EXECUTE) return;
#if 1
  if ((!pCh) || (!pCh->chID)) return;
  if (!args.dbr) return;
#endif

  caTask.caEventCount++;
  nBytes = dbr_size_n(args.type, args.count);
  memcpy((void *)&(pCh->info),args.dbr,nBytes);
  switch (ca_field_type(args.chid)) {
  case DBF_STRING :
    pr->value = 0.0;
    pr->hopr = 0.0;
    pr->lopr = 0.0;
    pr->precision = 0;
    pr->lowerAlarmLimit = 0.0;
    pr->upperAlarmLimit = 0.0;
    break;
  case DBF_ENUM :
    pr->value = (double) pCh->info.e.value;
    pr->hopr = (double) pCh->info.e.no_str - 1.0;
    pr->lopr = 0.0;
    pr->lowerAlarmLimit = 0.0;
    pr->upperAlarmLimit = pr->hopr;
    pr->lowerWarningLimit = 0.0;
    pr->upperWarningLimit = pr->hopr;
    pr->precision = 0;
    for (i = 0; i < pCh->info.e.no_str; i++) { 
     pr->stateStrings[i] = pCh->info.e.strs[i];
    }
    break;
  case DBF_CHAR :
    pr->value = (double) pCh->info.c.value;
    pr->hopr = (double) pCh->info.c.upper_disp_limit;
    pr->lopr = (double) pCh->info.c.lower_disp_limit;
    pr->precision = 0;
    pr->lowerAlarmLimit = (double) pCh->info.c.lower_alarm_limit;
    pr->upperAlarmLimit = (double) pCh->info.c.upper_alarm_limit;
    pr->lowerWarningLimit = (double) pCh->info.c.lower_warning_limit;
    pr->upperWarningLimit = (double) pCh->info.c.upper_warning_limit;
    break;
  case DBF_INT :
    pr->value = (double) pCh->info.i.value;
    pr->hopr = (double) pCh->info.i.upper_disp_limit;
    pr->lopr = (double) pCh->info.i.lower_disp_limit;
    pr->precision = 0;
    pr->lowerAlarmLimit = (double) pCh->info.i.lower_alarm_limit;
    pr->upperAlarmLimit = (double) pCh->info.i.upper_alarm_limit;
    pr->lowerWarningLimit = (double) pCh->info.i.lower_warning_limit;
    pr->upperWarningLimit = (double) pCh->info.i.upper_warning_limit;
    break;
  case DBF_LONG :
    pr->value = (double) pCh->info.l.value;
    pr->hopr = (double) pCh->info.l.upper_disp_limit;
    pr->lopr = (double) pCh->info.l.lower_disp_limit;
    pr->precision = 0;
    pr->lowerAlarmLimit = (double) pCh->info.l.lower_alarm_limit;
    pr->upperAlarmLimit = (double) pCh->info.l.upper_alarm_limit;
    pr->lowerWarningLimit = (double) pCh->info.l.lower_warning_limit;
    pr->upperWarningLimit = (double) pCh->info.l.upper_warning_limit;
    break;
  case DBF_FLOAT :
    pr->value = (double) pCh->info.f.value;
    pr->hopr = (double) pCh->info.f.upper_disp_limit;
    pr->lopr = (double) pCh->info.f.lower_disp_limit;
    pr->precision = pCh->info.f.precision;
    pr->lowerAlarmLimit = (double) pCh->info.f.lower_alarm_limit;
    pr->upperAlarmLimit = (double) pCh->info.f.upper_alarm_limit;
    pr->lowerWarningLimit = (double) pCh->info.f.lower_warning_limit;
    pr->upperWarningLimit = (double) pCh->info.f.upper_warning_limit;
    break;
  case DBF_DOUBLE :
    pr->value = (double) pCh->info.d.value;
    pr->hopr = (double) pCh->info.d.upper_disp_limit;
    pr->lopr = (double) pCh->info.d.lower_disp_limit;
    pr->precision = pCh->info.f.precision;
    pr->lowerAlarmLimit = (double) pCh->info.d.lower_alarm_limit;
    pr->upperAlarmLimit = (double) pCh->info.d.upper_alarm_limit;
    pr->lowerWarningLimit = (double) pCh->info.d.lower_warning_limit;
    pr->upperWarningLimit = (double) pCh->info.d.upper_warning_limit;
    break;
  default :
    dm2kPostMsg("dm2kUpdateGraphicalInfoCb : unknown data type\n");
    return;
  }

  if (pr->precision < 0) {
    dm2kPrintf("dm2kUpdateGraphicalInfoCb : pv \"%s\" precision = %d\n",
                ca_name(pCh->chID), pr->precision);
    pr->precision = 0;
  }
  else if (pr->precision > 16) {
    dm2kPrintf("dm2kUpdateGraphicalInfoCb : pv \"%s\" precision = %d\n",
                ca_name(pCh->chID), pr->precision);
    pr->precision = 16;
  }

  if (pr->updateGraphicalInfoCb) {
    pr->updateGraphicalInfoCb((XtPointer)pr);
  } 
  else if (pr->updateValueCb) {
    pr->updateValueCb((XtPointer)pr);
  }
}

static void dm2kUpdateChannelCb(struct event_handler_args args) {
  int nBytes;
  Channel *pCh = (Channel *) ca_puser(args.chid);
  Boolean severityChanged = False;
  Boolean zeroAndNoneZeroTransition = False;
  double value;
  Record *pr = pCh->pr;

  if (!(args.status & CA_M_SUCCESS)) { return; }

  if (globalDisplayListTraversalMode != DL_EXECUTE) return;
  if ((!pCh) || (!pCh->chID)) return;
  if (!args.dbr) return;
  caTask.caEventCount++;
  if (ca_read_access(args.chid)) {
    /* if we have the read access */
    nBytes = dbr_size_n(args.type, args.count);
    if (!pCh->data) {
      pCh->data = (dataBuf *) malloc(nBytes);
      pCh->size = nBytes;
      if (!pCh->data) {
        dm2kPrintf("dm2kUpdateChannelCb : memory allocation error!\n");
        return;
      }
    } else 
    if (pCh->size < nBytes) {
      free((char *)pCh->data);
      pCh->data = (dataBuf *) malloc(nBytes);
      pCh->size = nBytes;
      if (pCh->data == NULL) {
        dm2kPrintf("dm2kUpdateChannelCb : memory allocation error!\n");
        return;
      }
    }
    pr->time = ((dataBuf *) args.dbr)->s.stamp;
    switch (ca_field_type(args.chid)) {
    case DBF_STRING :
      pr->array = (XtPointer) pCh->data->s.value;
      break;
    case DBF_ENUM :
      pr->array = (XtPointer) &(pCh->data->e.value);
      break;
    case DBF_CHAR :
      pr->array = (XtPointer) &(pCh->data->c.value);
      break;
    case DBF_INT :
      pr->array = (XtPointer) &(pCh->data->i.value);
      break;
    case DBF_LONG :
      pr->array = (XtPointer) &(pCh->data->l.value);
      break;
    case DBF_FLOAT :
      pr->array = (XtPointer) &(pCh->data->f.value);
      break;
    case DBF_DOUBLE :
      pr->array = (XtPointer) &(pCh->data->d.value);
      break;
    default :
      break;
    }

    if (ca_field_type(args.chid) == DBF_STRING || ca_element_count(args.chid) > 1) {
      memcpy((void *)pCh->data,args.dbr,nBytes);
    }
    switch (ca_field_type(args.chid)) {
    case DBF_STRING :
      value = 0.0;
      break;
    case DBF_ENUM :
      value = (double) ((dataBuf *) (args.dbr))->e.value;
      break;
    case DBF_CHAR :
      value = (double) ((dataBuf *) (args.dbr))->c.value;
      break;
    case DBF_INT :
      value = (double) ((dataBuf *) (args.dbr))->i.value;
      break;
    case DBF_LONG :
      value = (double) ((dataBuf *) (args.dbr))->l.value;
      break;
    case DBF_FLOAT :
      value = (double) ((dataBuf *) (args.dbr))->f.value;
      break;
    case DBF_DOUBLE :
      value = ((dataBuf *) (args.dbr))->d.value;
      break;
    default :
      value = 0.0;
      break;
    }

    if (((value == 0.0) && (pr->value != 0.0)) || ((value != 0.0) && (pr->value == 0.0)))
      zeroAndNoneZeroTransition = True;
    pr->value = value;

    if (pr->severity != ((dataBuf *) (args.dbr))->d.severity) {
      pr->severity = ((dataBuf *) (args.dbr))->d.severity;
      severityChanged = True;
    }
    if (pr->monitorValueChanged && pCh->pr->updateValueCb) {
        pr->updateValueCb((XtPointer)pr);
    } else
    if (pr->monitorSeverityChanged && severityChanged && pr->updateValueCb) {
        pr->updateValueCb((XtPointer)pr);
    } else
    if (pr->monitorZeroAndNoneZeroTransition 
        && zeroAndNoneZeroTransition && pr->updateValueCb) {
        pr->updateValueCb((XtPointer)pr);
    }
  }
}

int caAdd(char *name, Record *pr) {
  Channel *pCh;
  int status;
  if ((caTask.freeListCount < 1) && (caTask.nextFree >= CA_PAGE_SIZE)) {
    /* if not enought pages, increase number of pages */
    if (caTask.pageCount >= caTask.pageSize) {
      caTask.pageSize += CA_PAGE_COUNT;
#if defined(__cplusplus) && defined(SUNOS4) && !defined(__GNUG__)
      caTask.pages = (Channel **) realloc((malloc_t)caTask.pages,sizeof(Channel *)*caTask.pageSize);
#else
      caTask.pages = (Channel **) realloc(caTask.pages,sizeof(Channel *)*caTask.pageSize);
#endif
      if (caTask.pages == NULL) {
        dm2kPrintf("\ncaAdd : memory allocation error\n");
        return -1;
      }
    }
    /* add one more page */
    caTask.pages[caTask.pageCount] = (Channel *) calloc(CA_PAGE_SIZE, sizeof(Channel));
    if (caTask.pages[caTask.pageCount] == NULL) {
      dm2kPrintf("\ncaAdd : memory allocation error\n");
      return -1;
    }
    caTask.pageCount++;
    caTask.nextPage++;
    caTask.nextFree=0;
  }
  if (caTask.nextFree < CA_PAGE_SIZE) {
    pCh = &((caTask.pages[caTask.nextPage])[caTask.nextFree]);
    pCh->caId = caTask.nextPage * CA_PAGE_SIZE + caTask.nextFree;
    caTask.nextFree++;
  } else {
    int index;
    caTask.freeListCount--;
    index = caTask.freeList[caTask.freeListCount];
    pCh = &((caTask.pages[index/CA_PAGE_SIZE])[index % CA_PAGE_SIZE]);
    pCh->caId = index;
  }

  pCh->data = NULL;
  pCh->chID = NULL;
  pCh->evID = NULL;
  pCh->size = 0;
  pCh->pr = pr;
  pCh->previouslyConnected = False;
  if (STRLEN(name) > (size_t)0) {
    status = ca_build_and_connect(name,TYPENOTCONN,0,
           &(pCh->chID),NULL,dm2kConnectEventCb,pCh);
  } else {
    status = ca_build_and_connect(" ",TYPENOTCONN,0,
           &(pCh->chID),NULL,dm2kConnectEventCb,pCh);
  }
  if (status != ECA_NORMAL) {
    SEVCHK(status,"caAdd : ca_build_and_connect failed\n");
  } else {
    pCh->pr->name = ca_name(pCh->chID);
  }
  caTask.channelCount++;
  return pCh->caId;
}

void caDelete(Record *pr) {
  int status;
   Channel *pCh = NULL;
   
  if (!pr)
    return;
  pCh = &((caTask.pages[pr->caId/CA_PAGE_SIZE])[pr->caId % CA_PAGE_SIZE]);
  if (pCh->chID && ca_state(pCh->chID) == cs_conn)
    caTask.channelConnected--;
#if 0
  ca_change_connection_event(pCh->chID,NULL);
  ca_replace_access_rights_event(pCh->chID,NULL);
#endif
  if (pCh->evID) {
    status = ca_clear_event(pCh->evID);
    SEVCHK(status,"caDelete : ca_clear_event() failed!");
    if (status != ECA_NORMAL) return;
  }
  pCh->evID = NULL;
  if (pCh->chID) {
    status = ca_clear_channel(pCh->chID);
    SEVCHK(status,"vCA::vCA() : ca_add_exception_event failed!");
    if (status != ECA_NORMAL) return;
  }
  pCh->chID = NULL;
  if (pCh->data) {
    free((char *)pCh->data);
    pCh->data = NULL;
  }
  if (caTask.freeListCount >= caTask.freeListSize) {
    caTask.freeListSize += CA_PAGE_SIZE;
#if defined(__cplusplus) && defined(SUNOS4) && !defined(__GNUG__)
    caTask.freeList = (int *) realloc((malloc_t)caTask.freeList,sizeof(int)*caTask.freeListSize);
#else
    caTask.freeList = (int *) realloc(caTask.freeList,sizeof(int)*caTask.freeListSize);
#endif
    if (caTask.freeList == NULL) {
      dm2kPrintf("\ncaDelete : memory allocation error\n");
      return;
    }
  }
  caTask.freeList[caTask.freeListCount] = pCh->caId;
  caTask.freeListCount++;
  caTask.channelCount--;
}

static Record nullRecord = {-1,-1,-1,0.0,0.0,0.0,-1,
                            NO_ALARM,NO_ALARM,False,False,False,
                            {NULL,NULL,NULL,NULL,
                             NULL,NULL,NULL,NULL,
                             NULL,NULL,NULL,NULL,
                             NULL,NULL,NULL,NULL},
                            NULL,NULL,
                            {0,0},
                            NULL,NULL,NULL,
                            True,True,True,
			    0.0,0.0,0.0
};

Record *dm2kAllocateRecord(char *name,
                           void (*updateValueCb)(XtPointer),
                           void (*updateGraphicalInfoCb)(XtPointer),
                           XtPointer clientData) {
  Record *record = NULL;
  if (!name) return record;
  record = (Record *) malloc(sizeof(Record));
  if (record) {
    *record = nullRecord;
    record->caId = caAdd(name,record);
    record->updateValueCb = updateValueCb;
    record->updateGraphicalInfoCb = updateGraphicalInfoCb;
    record->clientData = clientData;
  }
  return record;
}

void dm2kDestroyRecord(Record *pr) {
  if (!pr)
    return;
  caDelete(pr);
  *pr = nullRecord;
  /* FIXME: exchange statements above and below ?! */
  free((char *)pr);
}

void dm2kSendDouble(Record *pr, double data) {
  Channel *pCh = &((caTask.pages[pr->caId/CA_PAGE_SIZE])[pr->caId % CA_PAGE_SIZE]);
  SEVCHK(ca_put(DBR_DOUBLE,pCh->chID,&data),"dm2kSendDouble: error in ca_put");
  ca_flush_io();
}  

void dm2kSendCharacterArray(Record *pr, char *data, unsigned long size) {
  Channel *pCh = &((caTask.pages[pr->caId/CA_PAGE_SIZE])[pr->caId % CA_PAGE_SIZE]);
  SEVCHK(ca_array_put(DBR_CHAR,size,pCh->chID,data),
    "dm2kSendCharacterArray: error in ca_put");
  ca_flush_io();
}

void dm2kSendString(Record *pr, char *data) {
  Channel *pCh = &((caTask.pages[pr->caId/CA_PAGE_SIZE])[pr->caId % CA_PAGE_SIZE]);
  SEVCHK(ca_put(DBR_STRING,pCh->chID,data),"dm2kSendString: error in ca_put");
  ca_flush_io();
}

void dm2kRecordAddUpdateValueCb(Record *pr, void (*updateValueCb)(XtPointer)) {
  pr->updateValueCb = updateValueCb;
}

void dm2kRecordAddGraphicalInfoCb(Record *pr, void (*updateGraphicalInfoCb)(XtPointer)) {
  pr->updateGraphicalInfoCb = updateGraphicalInfoCb;
}


chid dm2kChid (Record *pr) {

  Channel *pCh = &((caTask.pages[pr->caId/CA_PAGE_SIZE])[pr->caId % CA_PAGE_SIZE]);
  return pCh->chID;
}


void dm2kChanelInfo (char *strg, Record *pr, char *defname) {

  chid cid;
  char *type;
  char *name;

  if ( ! pr ) return;

  cid = dm2kChid (pr);

  if ( ! pr->connected ) {
    name = ca_name(cid);
    if ( ((name[0] == '\0') || (strcmp (name, " ") == 0))&& defname ) name = defname;
    sprintf (strg, "    PV name = %s\n       host = %s",
	   name, ca_host_name (cid));
    strcat (strg, "\n     status = NOT connected");
    return;
  }

  sprintf (strg, "    PV name = %s\n       host = %s",
	 ca_name (cid), ca_host_name (cid));

  /* status seems not to be updated during CA access */
  /* -----------------------------------------------
  if ( pr->status != ECA_NORMAL )
    sprintf (&strg[STRLEN(strg)], "\n      last status = %d %s",
	   pr->status, ca_message (pr->status));
  */

  if ( pr->readAccess || pr->writeAccess ) {
    strcat (strg, "\n    ");
    if ( pr->readAccess ) strcat (strg, " Read");
    if ( pr->writeAccess ) strcat (strg, " Write");
    strcat (strg, " access");
  }
  else strcat (strg, "\n     No access");

  type = NULL;  /* numeric type */
  switch (pr->dataType) {

    case DBF_STRING :
      sprintf (&strg[STRLEN(strg)], "\n     String data type\n     last value = '%s'",
	     (char*)pr->array);
      break;

    case DBF_ENUM :
      sprintf (&strg[STRLEN(strg)], "\n     Enumeration data type\n     last value = '%s'",
	     pr->stateStrings[(int) pr->value]);
      break;

    case DBF_CHAR :
      type = "Byte (8 bits)";
      break;

    case DBF_INT :
      type = "Short (16 bits)";
      break;

    case DBF_LONG :
      type = "Long Integer (32 bits)";
      break;

    case DBF_FLOAT :
    case DBF_DOUBLE :
      sprintf (&strg[STRLEN(strg)], "\n     last value = %f\n     precision = %d",
	     pr->value, pr->precision);
      break;

    default :
      ;
    }
    if ( type ) {
      long val = (long) pr->value;
      sprintf (&strg[STRLEN(strg)], "\n     %s data type\n     last value = %ld ( = %lx hexa )",
	     type, val, val);
    }

  if ( pr->severity == MINOR_ALARM )
    strcat (strg, "\n      MINOR ALARM state");
  else if ( pr->severity == MAJOR_ALARM )
    strcat (strg, "\n      MAJOR ALARM state");

}
