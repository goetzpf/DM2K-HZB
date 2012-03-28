#ifndef _DM2K_INIT_TASK_H
#define _DM2K_INIT_TASK_H

#define INIT_SHARED_C      0
#define LAST_INIT_C        INIT_SHARED_C + 1

#ifdef ALLOCATE_STORAGE
  InitTask dm2kInitTask[LAST_INIT_C] = {
    {False, dm2kInitSharedDotC},};
#else
  extern InitTask dm2kInitTask[];
#endif

#endif
