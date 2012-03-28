#ifndef _XcScale_h_
#define _XcScale_h_

#include "Xc.h"

typedef struct _TickDescription {
  int ticks;
  int ticksForLabel;
} TickDescription;

extern TickDescription * GetTickDescription Xc_PROTO((int    linear,
						      int    fonfSize,
						      int    lPos,
						      int    hPos,
						      double lVal,
						      double hVal));

#endif /* _XcScale_h_ */
