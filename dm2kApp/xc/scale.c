#include <stdlib.h>
#include "scale.h"
#include "math.h"
static TickDescription tickDesc[] = { 
  { 51, 1  },
  { 41, 1  },
  { 31, 1  },
  { 21, 1  },
  { 51, 5  },
  { 11, 1  },
  { 41, 5  },
  { 31, 5  },
  { 51, 10 },
  { 5 , 1  },
  { 41, 10 },
  { 21, 5  },
  { 4 , 1  },
  { 31, 10 },
  { 3 , 1  },
  { 5 , 2  },
  { 21, 10 },
  { 11, 5  },
  { 3 , 2  },
  { 2 , 1  },
};

static int tickDescNum = sizeof(tickDesc) / sizeof(tickDesc[0]);


static int GetOffsetPositionByValue(value, lPos, hPos, lVal, hVal)
     double value;
     int    lPos;
     int    hPos;
     double lVal;
     double hVal;
{
  double ratio;

  if (value >= hVal)
    return 0;

  if (value <= lVal)
    return lPos - hPos;

  ratio = (double)(abs(lPos - hPos)) / (hVal - lVal);
  return (int)(ratio * (value - lVal));
}

static int GetOffsetPositionByValueLog(value, lPos, hPos, lVal, hVal)
     double value;
     int    lPos;
     int    hPos;
     double lVal;
     double hVal;
{
  double ratio;

  if (value >= hVal)
    return 0;

  if (value <= lVal)
    return lPos - hPos;

  ratio = (double)(abs(lPos - hPos)) / (log(hVal) - log(lVal));
  return (int)(ratio * (log(value) - log(lVal)));
}

TickDescription * GetTickDescription(linear, fontSize, lPos, hPos, lVal, hVal)
     int    linear;
     int    fontSize;
     int    lPos;
     int    hPos;
     double lVal;
     double hVal;
{
  register int i;
  int (*getOffset)();

  if (fontSize < 1)
    return 0;

  if (linear)
    getOffset = GetOffsetPositionByValue;
  else
    getOffset = GetOffsetPositionByValueLog;

  for (i = 0; i < tickDescNum; i++) {
    double value = lVal + (hVal - lVal)/(double)(tickDesc[i].ticks-1);

    if ((*getOffset)(value, lPos, hPos, lVal, hVal) 
	> fontSize/tickDesc[i].ticksForLabel 
	&& lPos-hPos > tickDesc[i].ticks * 3)
      return &(tickDesc[i]);
  }
  
  return 0;	    
}


