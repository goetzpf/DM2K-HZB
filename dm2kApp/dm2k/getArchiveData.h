#ifndef __getArchiveData__h
#define __getArchiveData__h


#define REQUEST_MODE_ONE_SHOT    1
#define REQUEST_MODE_CONTINUE    2

extern int getArchiveData(
      char         * name,              /* channal name */
      long           fromTime,          /* asking time from */
      long           toTime,            /* asking time up to */
      long           desiredCount,      /* asking count of data */
      long           requestMode,       /* short request or continues */
      long         * returnedFromTime,  /* real time from */
      long         * returnedToTime,    /* real time up to */
      float       ** returnedData,      /* data */
      long         * returnedCount);    /* real count of data */

#endif /*__getArchiveData__h*/

