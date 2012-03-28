/*
 *      Text.h
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:19:00 1992, patchlevel 2
 *                                      Draw() changed for drawing
 *                                      to a pixmap instead of a window
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  Text.h"
 */

/*

Copyright 1992 by University of Paderborn
Copyright 1990,1991 by the Massachusetts Institute of Technology

All rights reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the firms, institutes
or employers of the authors not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

THE AUTHORS AND THEIR FIRMS, INSTITUTES OR EMPLOYERS DISCLAIM ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE AUTHORS AND THEIR FIRMS,
INSTITUTES OR EMPLOYERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

#ifndef _Jpt_AtText_h
#define _Jpt_AtText_h

#include "At.h"


typedef struct _AtText {
    char  *str;
    short width;                   /* bounding box info */
    short ascent;
    short descent;
    Boolean rotated;
    XImage *rotated_image;
    unsigned int rotated_depth;
    Font fid;
    XFontStruct *fs;
} AtText;


#define AtTextJUSTIFY_LEFT      0
#define AtTextJUSTIFY_TOP       0
#define AtTextJUSTIFY_CENTER    1
#define AtTextJUSTIFY_RIGHT     2
#define AtTextJUSTIFY_BOTTOM    2

extern void PlotTextRotateRefresh P((Display *, Window, Drawable, GC, AtText *));

extern void AtTextRotate P((AtText *));

extern void AtTextDrawJustified P((Display *, Window, Drawable, GC, AtText *,
				   int, int, int, int, int, int));

#define AtTextWidth(t) ((t)->width)
#define AtTextAscent(t) ((t)->ascent)
#define AtTextDescent(t) ((t)->descent)
#define AtTextHeight(t) ((t)->ascent + (t)->descent)

extern int AtTextPSWidth P((AtText *));
extern int AtTextPSAscent P((AtText *));
extern int AtTextPSDescent P((AtText *));
extern void AtTextPSDraw P((FILE *, AtText *, int, int));
extern void AtTextWritePostscriptProlog P((FILE *));

extern AtText* At2TextCreate P((Widget, char *, Font));
extern void AtTextDestroy P((AtText *));
extern At2TextRenew P((AtText *, char *, XFontStruct *));
extern int AtTextRenewFont P((Widget, AtText *, Font));
#endif /* _AtText_h */
