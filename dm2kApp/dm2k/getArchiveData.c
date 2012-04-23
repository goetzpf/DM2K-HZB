/*
 *	getArchiveData.c		
 *     
 *     getArchiveData () routine prepares some arrays of value and time
 *      existed in archive record.
 *
 *      getRawDataFromRecord() 
 *          1. get IDs of 'record_name.NVAL'
 *                        'record_name.TIM'
 *                        'record_name.VAL'
 *          2. get arrays of time and data
 *      
 *      orderArrays()
 *          sort the arrays of time and data according to time
 *      
 *      getOnlineVal()
 *          1. appoximation 
 *          2. get returned from_time, returned to_time,
 *             returned data array and returned element 
 *
 */
#include <stdio.h>

#if defined(SYSV) || defined(SVR4) || defined(SOLARIS)
#include <string.h>
#else
#include <strings.h>
#endif

#include <cadef.h>
#include <caerr.h>
#include <db_access.h>
#include <tsDefs.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <memory.h>

#include "dm2k.h"
#include "dm2kWidget.h"
#include "getArchiveData.h"

#define ERROR	1
#define OK	0

#define READ_VALUE_DELAY        0.7
#define NAME_SIZE               64

/*----------------------------------------------------------*/ 
#define CA_PEND(messg)                                       \
  {                                                          \
    int my_status = ca_pend_io(READ_VALUE_DELAY);            \
    if (my_status == ECA_TIMEOUT) {                          \
       if (ca_pend_io(READ_VALUE_DELAY) != ECA_NORMAL) {     \
         SEVCHK (my_status,messg)                            \
         RETURN_ERROR;                                       \
       }                                                     \
    }                                                        \
    else if (my_status != ECA_NORMAL) {                      \
      RETURN_ERROR;                                          \
    }                                                        \
  }                                                          \
/*----------------------------------------------------------*/

/*-------------------------------------------------------*/ 
#define _CA_PEND(messg)                                    \
  {                                                       \
    int my_status = ca_pend_io(READ_VALUE_DELAY);         \
    if (my_status != ECA_NORMAL) {                        \
       SEVCHK (my_status,messg)                           \
       RETURN_ERROR;                                      \
    }                                                     \
  }                                                       \
/*-------------------------------------------------------*/

#define ARCHIVE_RECORD_TAG__NUMBER_OF_ELEMENTS ".NVAL"
#define ARCHIVE_RECORD_TAG__TIME_ARRAY         ".TIM"
#define ARCHIVE_RECORD_TAG__VAL_DATA           ".VAL"

#define PRINTF(messg) fprintf(stderr,"%s:%d\n%s\n",__FILE__, __LINE__, messg)
 
typedef struct _channelIds {
    struct _channelIds *next;
    char               *name;
    chid               nvalId;
    chid               timId;
    chid               valId;   
}ChannelIds;

static ChannelIds * listChannelIds;

/* forward declaration
 */

static int getChannelIds(
      char        *name,       /* record name */  
      long        requestMode,
      ChannelIds  **IDs);      /* record IDs */

static int getRawDataFromRecord (
      ChannelIds  *IDs,    
      long        requestMode,
      long        **timeData, /* returned time data pointer */
      float       **valData,  /* returned val data pointer */
      long        *count);    /* how much elements are in array */

static int orderArrays (
      long  *timeData,
      float *valData,
      long  count);

static int getOnlineVal(
      long  *timeData,
      float *valData,
      long  count,
      long  fromTime,
      long  toTime,
      long  numElement,
      float **returnedData,
      long  *returnedFromTime,
      long  *returnedToTime,
      long  *returnedCount);

static float makeApproximation (
      long  * timeData,
      float * valData,
      long    elements,
      int   * index,
      int     time);


/*
 *********************************************************************
 * let's start hard coding routines in C..
 *********************************************************************
 */


static void destroyChannelId (ChannelIds  *IDs)
{
  if (IDs) {
    free (IDs->name);
    ca_clear_channel(IDs->nvalId);
    ca_clear_channel(IDs->timId);
    ca_clear_channel(IDs->valId);
    free((char*)IDs);
  }    
}

int getArchiveData(
      char   * name,
      long     fromTime,
      long     toTime,
      long     desiredCount,
      long     requestMode,
      long   * returnedFromTime,
      long   * returnedToTime,
      float ** returnedData,
      long   * returnedCount)
{
  long          count;
  long        * timeData;
  float       * valData;
  ChannelIds  * IDs;
  char        * archName;
  int           len;

  if (name == NULL) {
    PRINTF("invalid parameters");
    return ERROR;
  }

  archName = name;

  /* let's check whether cnannel name has ``.VAL'' like sufix
   */
  for (len = STRLEN(name); len >=0; len--)
    if (name[len] == '.')
      break;

  if (len <= 0) {
    /* if not, than check for ``_h'' sufix 
     */
    len = STRLEN(name);
    
    if (name[len-2] != '_' || name[len-1] != 'h')
      {
	archName = (char*) malloc (sizeof(char) *(len + 3));
	if (archName == NULL) {
	  PRINTF("cannot allocate memory");
	  return ERROR;
	}
	
	strncpy(archName, name,len);
	archName[len++] = '_';
	archName[len++] = 'h';
	archName[len] = '\0';
      }
  } 
  else {
    /* if ``.VAL'' sufix exists than do not take it in to acount
     */
    if (name[len-2] == '_' && name[len-1] == 'h')
      len -= 2;
    
    archName = (char*) malloc (sizeof(char) * (len + 3));
    if (archName == NULL) {
      PRINTF("cannot allocate memory");
      return ERROR;
    }

    strncpy(archName, name,len);
    archName[len++] = '_';
    archName[len++] = 'h';
    archName[len]   = '\0';
  }

  if (getChannelIds(archName, requestMode, &IDs)) {
    if (archName != name) free(archName);
    return ERROR;
  }

  if (getRawDataFromRecord (IDs, requestMode, &timeData, &valData, &count)) {
    if (requestMode == REQUEST_MODE_ONE_SHOT)
      destroyChannelId (IDs);
    if (archName != name) free(archName);
    return ERROR;
  } 


  if (orderArrays (timeData, valData, count)) {
    if (requestMode == REQUEST_MODE_ONE_SHOT)
      destroyChannelId (IDs);
    if (archName != name) free(archName);
    return ERROR;
  }


  if (fromTime < 0) {
    /* ``fromTime'' is indeed time interval 
     */
    if (toTime == 0) {
      fromTime = (toTime = timeData[count-1]) + fromTime;
    }
    else
      fromTime = toTime + fromTime;
  }

  if (getOnlineVal(timeData,
		   valData,
		   count,
		   fromTime,
		   toTime,
		   desiredCount,
		   returnedData,
		   returnedFromTime,
		   returnedToTime,
		   returnedCount)) 
    {
      if (requestMode == REQUEST_MODE_ONE_SHOT)
	destroyChannelId (IDs);
      if (archName != name) free(archName);
      return ERROR;
    }


  if (archName != name) free(archName);

  return OK;
}


static int getChannelIds(
      char        * name,
      long          requestMode,
      ChannelIds ** IDs)
{
  register ChannelIds * ids = NULL;
  char                  string[NAME_SIZE];
  int                   status;

  /* T. Straumann: protect against null pointer arg */
  if ( ! name) {
	PRINTF("getChannelIds: 'name' == NULL");
	return ERROR;
  }

  if (STRLEN(name) > NAME_SIZE -6) {
    PRINTF("name is very long");
    return ERROR;
  }

  /* check static list of preexeisted ChannelIds for the name
   */
  if (requestMode == REQUEST_MODE_ONE_SHOT && listChannelIds) 
  {
    /* remove node from static list
     */
   if (strcmp(name, listChannelIds->name) == 0) 
   {
     *IDs = listChannelIds;
     listChannelIds = listChannelIds->next;
     return OK;
   } 
   else 
   {
     register ChannelIds *p, *n;
   
     for (p = listChannelIds, n = p->next; n != NULL; p = n, n = n->next) {
       if (strcmp(name, n->name) == 0) {
	 p->next = n->next;
	 *IDs = n;
	 return OK;
       }
     }
   }
  }

  /* if the node is exists in the list, return it.
   */
  for (ids = listChannelIds; ids != NULL; ids = ids->next) {
    if (strcmp(name, ids->name) == 0){
      *IDs = ids;
      return OK;
    }
  }

  /* make a new ChannelIds
   */
  ids = (ChannelIds *) malloc (sizeof (ChannelIds));
  
  if (ids == NULL) {
    PRINTF("cannot allocate memory");
    return ERROR;
  }
  
  /* copy name
   */
  ids->name = (char*) malloc (sizeof(char) * (STRLEN(name)+1));
  if (ids->name == NULL) {
    free ((char*)ids);
    PRINTF("cannot allocate memory");
    return ERROR;
  }
  
  strcpy (ids->name, name);
  
  /* get number of element in time&data arrays
   */
  strcpy(string, name);
  strcat(string, ARCHIVE_RECORD_TAG__NUMBER_OF_ELEMENTS); 

  status = ca_search(string, &ids->nvalId);
  if (status != ECA_NORMAL) {
    if (ids->name) free((char*)ids->name); 
    if (ids) free((char*)ids);             
    return ERROR;
  }

#define RETURN_ERROR                     \
 ca_clear_channel(ids->nvalId);          \
 if (ids->name) free((char*)ids->name);  \
 if (ids) free((char*)ids);              \
 return ERROR

  CA_PEND("ca_pend_io failure");

#undef RETURN_ERROR

  /* array of time
   */
  strcpy(string, name);
  strcat(string, ARCHIVE_RECORD_TAG__TIME_ARRAY);

  status = ca_search(string, &ids->timId);
  if (status != ECA_NORMAL) {
    ca_clear_channel(ids->nvalId);
    if (ids->name) free((char*)ids->name); 
    if (ids) free((char*)ids);             
    return ERROR;
  }

#define RETURN_ERROR                     \
 ca_clear_channel(ids->nvalId);          \
 ca_clear_channel(ids->timId);           \
 if (ids->name) free((char*)ids->name);  \
 if (ids) free((char*)ids);              \
 return ERROR

  CA_PEND("ca_pend_io failure");

#undef RETURN_ERROR

  /* array of data
   */
  strcpy(string, name);
  strcat(string, ARCHIVE_RECORD_TAG__VAL_DATA);

  status = ca_search(string, &ids->valId);
  if (status != ECA_NORMAL) {
    ca_clear_channel(ids->nvalId);
    ca_clear_channel(ids->timId);
    if (ids->name) free((char*)ids->name); 
    if (ids) free((char*)ids);             
    return ERROR;
  }

#define RETURN_ERROR                     \
 ca_clear_channel(ids->nvalId);          \
 ca_clear_channel(ids->timId);           \
 ca_clear_channel(ids->valId);	         \
 if (ids->name) free((char*)ids->name);  \
 if (ids) free((char*)ids);              \
 return ERROR

  CA_PEND("ca_pend_io failure");

#undef RETURN_ERROR  

  if (requestMode == REQUEST_MODE_CONTINUE) {
    ids->next = listChannelIds;
    listChannelIds = ids;
  }

  if (IDs) *IDs = ids;

  return OK;
}


static int getRawDataFromRecord (
      ChannelIds   * IDs,        /* records' names */
      long           requestMode,
      long        ** timeData,   /* returned time data pointer */
      float       ** valData,    /* returned val data pointer */
      long         * count)      /* how much elements are in array */
{
  int         status;

#define RETURN_ERROR return ERROR

  if (timeData == NULL || valData == NULL || count == NULL) {
    INFORM_INTERNAL_ERROR();
    return ERROR;
  }

  *timeData = NULL;
  *valData  = NULL;

  status = ca_get (DBF_LONG, IDs->nvalId, count);
  if (status != ECA_NORMAL) {
    RETURN_ERROR;
  }
  CA_PEND("ca_pend_io failure");
 
  /* if number is not proper
   */
  if (*count < 1) {
    *count = 0;
    ca_clear_channel(IDs->nvalId);
    PRINTF("archive record has 0 size arraries");
    return ERROR;
  }
 
  /* allocate memory for arrays from record
   */
  *timeData = (long*) calloc (*count, sizeof (long));
  *valData = (float*) calloc (*count, sizeof (float));
 
  /* if there is no more memory
   */
  if (*valData == NULL || *timeData == NULL) 
    {
      ca_clear_channel(IDs->nvalId);
 
      if (*valData)  free ((char*)valData);
      if (*timeData) free ((char*)timeData);
 
      *timeData = NULL;
      *valData  = NULL;

      PRINTF("cannot allocate memory");

      return ERROR;
    }

  /* read the arrays
   */
  ca_array_get(DBF_LONG, (unsigned long)*count, IDs->timId, *timeData);
  ca_array_get(DBF_FLOAT,(unsigned long)*count, IDs->valId, *valData);


  CA_PEND("ca_pend_io failure");


  return OK;
#undef RETURN_ERROR
}


static int orderArrays (
      long  * timeData,
      float * valData,
      long    count)
{
  register int  i;
  int           start;

  /* search for element less then next one, that will be last added one
   */
  for (i = 0; i < count-1; i++) {
    if (timeData[i] > timeData[i+1])
      break;
  }

  /* element next for last one added whould be start element(IMHO)
   */
  start = i < count-1 ? i+1 : 0;


  /* if start element is not the first in array, reordering arrays
   */
  if (start) 
  {
    /* allocate tmp arrays 
     */
    long  * tmpTime = (long*) calloc (count, sizeof(long));
    float * tmpVal = (float*) calloc (count, sizeof(float));
    
    if (tmpTime == NULL || tmpVal == NULL) 
    {
      if (tmpTime) free ((char*)tmpTime);
      if (tmpVal)  free ((char*)tmpVal);

      PRINTF("cannot allocate memory");
      return ERROR;
    }

    /* copy parts from original to tmp
     */
    memcpy ((char*)tmpTime, (char*)&timeData[start], 
	    (count - start) * sizeof (long));
    memcpy ((char*)tmpVal,  (char*)&valData[start], 
	    (count - start) * sizeof (float));
    
    memcpy ((char*)&(tmpTime[count - start]), (char*)timeData, 
	    start * sizeof (long)); 
    memcpy ((char*)&(tmpVal[count - start]),  (char*)valData, 
	    start * sizeof (float)); 
    
    /* copy back
     */
    memcpy((char*)timeData, (char*)tmpTime, count * sizeof (long));
    memcpy((char*)valData, (char*)tmpVal, count * sizeof (float));

    /* free and bye
     */
    free ((char*) tmpVal);
    free ((char*) tmpTime);
  }

  return OK;
}


static int getOnlineVal (
  long   * timeData,
  float  * valData,
  long     count,
  long     fromTime,
  long     toTime,
  long     numElement,
  float ** returnedData,
  long   * returnedFromTime,
  long   * returnedToTime,
  long   * returnedCount)
{
  float   delta;
  int     i;
  long    startTime = -1; /* let's init'ed as negative */
  long    lastTime  = -1; /* let's init'ed as negative */
  int     next;
  float * result;
  int     index = 0;

  if (numElement < 0 || toTime <= fromTime || returnedData == NULL) {
    PRINTF("invalid parameters");
    return ERROR;
  }

  delta = (float)(toTime - fromTime) / (float)numElement;

  result = (float*) calloc (numElement, sizeof(float));
  if (result == NULL) {
    PRINTF("cannot allocate memory");
    return ERROR;
  }

  next = 0;
  for (i = 0; i < numElement; i++) 
  {
    register long time = fromTime + (long)((float)i * delta); 

    if (time >= timeData[0] && time <= timeData[count-1]) 
    {
      if (startTime < 0) 
	startTime = time;

      result[next++] = makeApproximation (timeData, valData, count, 
					  &index, time); 
      lastTime = time;
    }
    else if (time > timeData[count-1])
      break;
  }

  if (numElement != next) 
  {
    *returnedData = (float*) realloc ((char*)result, next * sizeof(float));

    if (*returnedData == NULL) {
      PRINTF("cannot allocate memory");
      next = 0;
      return ERROR;
    }
  } 
  else
    *returnedData = result;

  if (returnedFromTime) *returnedFromTime = startTime;
  if (returnedToTime)   *returnedToTime   = lastTime;
  if (returnedCount)    *returnedCount    = next;

  if (next == 0){
    *returnedFromTime = 0;
    *returnedToTime   = 0;
    *returnedCount    = 0;
  }

  return OK;
}

static float makeApproximation (
     long         * timeData,
     float        * valData,
     long           elements,
     register int * index,
     int            time)
{
  float ratio;
  
  if (timeData[*index] == time)
    return valData[*index];

  for (; *index < elements-1; (*index)++) {
    if (time <= timeData[(*index) + 1]) 
      break;
  }
  
  if (*index == elements -1)
     return valData[elements-1];

  if (timeData[(*index)+1] == time) {
    (*index)++;
    return valData[*index];
  }

  ratio = (valData[(*index) + 1] - valData[*index]) /
    (float)(timeData[(*index) + 1] - timeData[*index]);
  
  return  (valData[*index] + ratio * (float)(time - timeData[*index]));
}
