#ifndef _Jpt_PLOT_TEXT2_P_H_
#define _Jpt_PLOT_TEXT2_P_H_

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#include "Text2.h"

typedef struct {
  PlotTextDesc td;
  GCValues    gcv;
  GC          gc;
  Pixmap      oldp, newp;
} __TextHandle;


#endif
