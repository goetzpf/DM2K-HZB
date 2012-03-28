/*
 *      Text.c
 *
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Mon Jul 27 14:18:02 1992, patchlevel 2
 *                                      AtTextDestroy() generates a core
 *                                      dump when destroying NULL. Reported
 *                                      and fixed by Gustaf Neumann
 *                                      (neumann@dec4.wu-wien.ac.at)
 *                                      AtTextDraw() and AtTextDrawJustified()
 *                                      changed for drawing to a pixmap
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  Text.c";

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

#include "Text.h"

#include <ctype.h>

#include <X11/Xlibint.h> 

#define New(type) (type *)malloc(sizeof(type))

#define    sym_forall           34
#define    sym_exists           36
#define    sym_ni               39
#define    sym_ast              42
#define    sym_cong             64
#define    sym_therefore        92
#define    sym_perp             94
#define    sym_similar          126
#define    sym_prime            162
#define    sym_lte              163
#define    sym_infty            165
#define    sym_function         166
#define    sym_clubs            167
#define    sym_diamonds         168
#define    sym_hearts           169
#define    sym_spades           170
#define    sym_leftrightarrow   171
#define    sym_leftarrow        172
#define    sym_uparrow          173
#define    sym_rightarrow       174
#define    sym_downarrow        175
#define    sym_degr             176
#define    sym_pm               177
#define    sym_dblprime         178
#define    sym_gte              179
#define    sym_times            180
#define    sym_propto           181
#define    sym_partial          182
#define    sym_bullet           183
#define    sym_div              184
#define    sym_neq              185
#define    sym_eqv              186
#define    sym_approx           187
#define    sym_ldots            188
#define    sym_aleph            192
#define    sym_iset             193
#define    sym_rset             194
#define    sym_weierstrass      195
#define    sym_otimes           196
#define    sym_oplus            197
#define    sym_emptyset         198
#define    sym_inter            199
#define    sym_union            200
#define    sym_prsupset         201
#define    sym_supset           202
#define    sym_notsubset        203
#define    sym_prsubset         204
#define    sym_subset           205
#define    sym_in               206
#define    sym_notin            207
#define    sym_angle            208
#define    sym_nabla            209
#define    sym_registered       210
#define    sym_copyright        211
#define    sym_trademark        212
#define    sym_cdot             215
#define    sym_not              216
#define    sym_and              217
#define    sym_or               218
#define    sym_dbleftrightarrow 219
#define    sym_dbleftarrow      220
#define    sym_dbuparrow        221
#define    sym_dbrightarrow     222
#define    sym_dbdownarrow      223


#define sym_alpha       97
#define sym_beta        98
#define sym_chi 99
#define sym_delta       100
#define sym_epsilon     101
#define sym_phi 102
#define sym_gamma       103
#define sym_eta 104
#define sym_iota        105
#define sym_kappa       107
#define sym_lambda      108
#define sym_mu  109
#define sym_nu  110
#define sym_omicron     111
#define sym_pi  112
#define sym_theta       113
#define sym_rho 114
#define sym_sigma       115
#define sym_tau 116
#define sym_upsilon     117
#define sym_omega       119
#define sym_xi  120
#define sym_psi 121
#define sym_zeta        122

#define sym_Alpha       65
#define sym_Beta        66
#define sym_Chi 67
#define sym_Delta       68
#define sym_Epsilon     69
#define sym_Phi 70
#define sym_Gamma       71
#define sym_Eta 72
#define sym_Iota        73
#define sym_Kappa       75
#define sym_Lambda      76
#define sym_Mu  77
#define sym_Nu  78
#define sym_Omicron     79
#define sym_Pi  80
#define sym_Theta       81
#define sym_Rho 82
#define sym_Sigma       83
#define sym_Tau 84
#define sym_Upsilon     85
#define sym_Omega       87
#define sym_Xi  88
#define sym_Psi 89
#define sym_Zeta        90

#define sym_theta1      74
#define sym_sigma1      86
#define sym_phi1        106
#define sym_omega1      118
#define sym_Upsilon1 161


#define    env_bold             1       /* 'b' */
#define    env_ita              2       /* 'i' */
#define    env_greek            3       /* 'g' */
#define    env_roman            4       /* 'r' - change to roman font */
#define    env_bigger           5
#define    env_smaller          6
#define    env_super            7       /* '+' */
#define    env_sub              8       /* '-' */
/* special, handle later */
#define    env_vec              9       /* vector above variable */
#define    env_over             10      /* over - fractions */
#define    env_sqrt             11      /* sqrt - square root */
#define    env_int              12      /* int - integration */
#define    env_sum              13      /* sum - summation */
#define    env_prod             14      /* prod - pi sign for product */
#define    env_num              15      /* sub-environments */
#define    env_denom            16
#define    env_from             17
#define    env_to               18

#define cmd_space       1
#define cmd_nl          2
#define cmd_vector      3
#define cmd_sqrt        4
#define cmd_integral    5
#define cmd_summation   6
#define cmd_product     7
#define cmd_fraction    8


#define ENVIRONMENT 1
#define SYMBOL      2
#define NEWLINE     3
#define END         4
#define STRING      5
#define END_OF_STRING 6
#define START_OF_STRING 7


typedef struct {
    char s[100];
    unsigned short sp;
} char_stack;

static void init_char_stack(cs)
char_stack *cs;
{
    cs->sp = 0;
    cs->s[0] = '\0';
}

static char pop_char_stack(cs)
char_stack *cs;
{
    cs->sp--;
    return cs->s[cs->sp];
}

static void push_char_stack(cs,c)
char_stack *cs;
char c;
{
    cs->s[cs->sp++] = c;
}



static char otherside(opener)
char opener;
{
   switch (opener) {
    case '(':
      return(')');
    case '{':
      return('}');
    case '[':
      return(']');
    case '<':
      return('>');
   }
   /*NOTREACHED*/
   return 0;
}

static int openparen(c)
char c;
{
    return (c=='(')?1:(c=='[')?1:(c=='{')?1:(c=='<')?1:0;
}


/*
   text points to beginning of text string.  return value is
   length of string, up to but not including the passed terminator
   or the default terminators \0 \n @.
   Turns "@@" into "@".
   Turns "@ " into " ".
   Turns "@)" into ")", and similarly for "@]", "@}", and "@>"
   0 is a valid return value.
*/
/*static int text_length(text,terminator)
     char *text;
     char terminator;
{
   int len=0;

   while (1) {
      while (*text!='@' && *text!='\n' && *text!=terminator && *text) {
	 text++;
	 len++;
      }

      if (*text == '@') {
	  if ((*(text+1) == '@') || (*(text+1) == ' ') ||
	      (*(text+1) == ')') || (*(text+1) == ']') ||
	      (*(text+1) == '}') || (*(text+1) == '>')) {
	      *text = *(text+1);
	      bcopy(text+2,text+1,strlen(text+1));
	      text++;
	      len++;
	  }
	  else return(len);
      }
      else return (len);
  }
}*/


/*
 * on entry: *c == '@'
 * read the next token (up to an openparen) and compare
 * it against environment names.  If a match, return the
 * length of the matched string and put the environment
 * type in the env parameter.
 * If no openparen, or bad env name, return 0;
 */
static int getenvironmentname(c,terminator,env)
char *c;
char terminator;
int *env;
{
    int len;
    char save;

    *env = -1;
    c++;

    /*
     * scan forward until the next open delimiter
     */
    for(len=0;
	(c[len]!='\0')&&(c[len]!='\n')&&
	(c[len]!=terminator)&&!openparen(c[len]);
	len++);
    if ((c[len] == '\0')  || (c[len] == '\n') || (c[len] == terminator))
	return 0;

    save = c[len];
    c[len] = '\0';

#define COMPARE(str,sym) if (!(strcasecmp(c,str))) {*env=(int)sym; break;}

    switch(*c) {
    case 'a': case 'A':
	break;
    case 'b': case 'B':
	COMPARE("bigger",env_bigger);
	COMPARE("b", env_bold);
	break;
    case 'c': case 'C':
/*      COMPARE("c", env_c);
	COMPARE("center", env_center); */
	break;
    case 'd': case 'D':
	COMPARE("denom", env_denom);
	break;
    case 'e': case 'E':
	break;
    case 'f': case 'F':
/*      COMPARE("f", env_fix); */
	COMPARE("from", env_from);
	break;
    case 'g': case 'G':
	COMPARE("g", env_greek);
	break;
    case 'h': case 'H':
	break;
    case 'i': case 'I':
	COMPARE("i", env_ita);
	COMPARE("int", env_int);
	break;
    case 'l': case 'L':
	break;
    case 'm': case 'M':
	break;
    case 'n': case 'N':
	COMPARE("num", env_num);
	break;
    case 'o': case 'O':
	COMPARE("over", env_over);
	break;
    case 'p': case 'P':
	COMPARE("prod", env_prod);
	break;
    case 'r': case 'R':
	COMPARE("r", env_roman);
	break;
    case 's': case 'S':
	COMPARE("smaller",env_smaller);
	COMPARE("sqrt", env_sqrt);
	COMPARE("sum", env_sum);
	break;
    case 't': case 'T':
	COMPARE("to", env_to);
	break;
    case 'u': case 'U':
/*      COMPARE("u", env_u); */
	break;
    case 'v': case 'V':
	COMPARE("vec", env_vec);
	break;
    case '^':
	break;
    case '>':
/*      COMPARE(">", env_right);*/
	break;
    case '+':
	COMPARE("+", env_super);
	break;
    case '-':
	COMPARE("-", env_sub);
	break;
    }

#undef COMPARE

    c[len] = save;
    if (*env == -1) return 0;      /* if no match found */
    else return len;
}

/*
 * looks for a symbol name.
 * if it finds a valid one, returns its length, otherwise returns 0.
 * if it finds a valid name, return symbol in sym.
 * on entry: *c == '@';
 */
static int getsymbolname(c,sym)
char *c;
int *sym;
{
    int len;
    char save;

    c++;              /* move past the '@' */
    *sym = -1;

    /* all the other symbols require non-alphanumerics after their names to
     * delimit them.  So figure out how long the name is, then compare
     * it with all the symbol names.
     */
    for(len=0; isalnum(c[len]); len++);

    if (len == 0) return 0;

    save = c[len];
    c[len] = '\0';
#define COMPARE(str,symbol) if (!(strcasecmp(c, str))) \
			     {*sym = (int)symbol; break;}
#define UCOMPARE(str,symbol) if (!(strcmp(c, str))) \
			     {*sym = (int)symbol; break;}

    switch(*c) {
    case 'a': case 'A':
	COMPARE("aleph", sym_aleph);
	UCOMPARE("alpha", sym_alpha);
	UCOMPARE("Alpha", sym_Alpha);
	COMPARE("and", sym_and);
	COMPARE("angle", sym_angle);
	COMPARE("approx", sym_approx);
	COMPARE("ast", sym_ast);
	break;
    case 'b': case 'B':
	UCOMPARE("beta", sym_beta);
	UCOMPARE("Beta", sym_Beta);
	COMPARE("bullet", sym_bullet);
	break;
    case 'c': case 'C':
	COMPARE("cdot", sym_cdot);
	UCOMPARE("chi",sym_chi);
	UCOMPARE("Chi",sym_Chi);
	COMPARE("clubs",sym_clubs);
	COMPARE("cong", sym_cong);
	COMPARE("copyright",sym_copyright);
	break;
    case 'd': case 'D':
	COMPARE("dbdownarrow", sym_dbdownarrow);
	COMPARE("dbrightarrow", sym_dbrightarrow);
	COMPARE("dbleftarrow", sym_dbleftarrow);
	COMPARE("dbleftrightarrow", sym_dbleftrightarrow);
	COMPARE("dblprime", sym_dblprime);
	COMPARE("dbuparrow", sym_dbuparrow);
	COMPARE("degr", sym_degr);
	UCOMPARE("delta", sym_delta);
	UCOMPARE("Delta", sym_Delta);
	COMPARE("diamonds", sym_diamonds);
	COMPARE("div", sym_div);
	COMPARE("downarrow", sym_downarrow);
	break;
    case 'e': case 'E':
	COMPARE("emptyset", sym_emptyset);
	UCOMPARE("epsilon", sym_epsilon);
	UCOMPARE("Epsilon", sym_Epsilon);
	COMPARE("eqv", sym_eqv);
	UCOMPARE("eta", sym_eta);
	UCOMPARE("Eta", sym_Eta);
	COMPARE("exists", sym_exists);
	break;
    case 'f': case 'F':
	COMPARE("forall", sym_forall);
	COMPARE("function", sym_function);
	break;
    case 'g': case 'G':
	UCOMPARE("gamma", sym_gamma);
	UCOMPARE("Gamma", sym_Gamma);
	COMPARE("gte", sym_gte);
	break;
    case 'h': case 'H':
	COMPARE("hearts", sym_hearts);
	break;
    case 'i': case 'I':
	COMPARE("in", sym_in);
	COMPARE("infty", sym_infty);
	COMPARE("inter", sym_inter);
	UCOMPARE("iota", sym_iota);
	UCOMPARE("Iota", sym_Iota);
	COMPARE("iset", sym_iset);
	break;
    case 'j': case 'J':
	break;
    case 'k': case 'K':
	UCOMPARE("kappa", sym_kappa);
	UCOMPARE("Kappa", sym_Kappa);
	break;
    case 'l': case 'L':
	UCOMPARE("lambda", sym_lambda);
	UCOMPARE("Lambda", sym_Lambda);
	COMPARE("ldots", sym_ldots);
	COMPARE("leftarrow", sym_leftarrow);
	COMPARE("leftrightarrow", sym_leftrightarrow);
	COMPARE("lte", sym_lte);
	break;
    case 'm': case 'M':
	UCOMPARE("mu", sym_mu);
	UCOMPARE("Mu", sym_Mu);
	break;
    case 'n': case 'N':
	COMPARE("nabla", sym_nabla);
	COMPARE("neq", sym_neq);
	COMPARE("ni", sym_ni);
	COMPARE("not",sym_not);
	COMPARE("notin", sym_notin);
	COMPARE("notsubset", sym_notsubset);
	UCOMPARE("nu", sym_nu);
	UCOMPARE("Nu", sym_Nu);
	break;
    case 'o': case 'O':
	UCOMPARE("omega",sym_omega);
	UCOMPARE("Omega",sym_Omega);
	UCOMPARE("omega1",sym_omega1);
	UCOMPARE("omicron",sym_omicron);
	UCOMPARE("Omicron",sym_Omicron);
	COMPARE("oplus", sym_oplus);
	COMPARE("or", sym_or);
	COMPARE("otimes", sym_otimes);
	break;
    case 'p': case 'P':
	COMPARE("partial", sym_partial);
	COMPARE("perp", sym_perp);
	UCOMPARE("phi", sym_phi);
	UCOMPARE("Phi", sym_Phi);
	UCOMPARE("phi1", sym_phi1);
	UCOMPARE("pi", sym_pi);
	UCOMPARE("Pi", sym_Pi);
	COMPARE("pm", sym_pm);
	COMPARE("prime", sym_prime);
	COMPARE("propto", sym_propto);
	COMPARE("prsubset", sym_prsubset);
	COMPARE("prsupset", sym_prsupset);
	UCOMPARE("psi", sym_psi);
	UCOMPARE("Psi", sym_Psi);
	break;
    case 'r': case 'R':
	COMPARE("registered", sym_registered);
	UCOMPARE("rho", sym_rho);
	UCOMPARE("Rho", sym_Rho);
	COMPARE("rightarrow", sym_rightarrow);
	COMPARE("rset", sym_rset);
	break;
    case 's': case 'S':
	UCOMPARE("sigma", sym_sigma);
	UCOMPARE("Sigma", sym_Sigma);
	UCOMPARE("sigma1", sym_sigma1);
	COMPARE("similar", sym_similar);
	COMPARE("spades", sym_spades);
	COMPARE("subset", sym_subset);
	COMPARE("supset", sym_supset);
	break;
    case 't': case 'T':
	UCOMPARE("tau", sym_tau);
	UCOMPARE("Tau", sym_Tau);
	COMPARE("therefore", sym_therefore);
	UCOMPARE("theta", sym_theta);
	UCOMPARE("Theta", sym_Theta);
	UCOMPARE("theta1", sym_theta1);
	COMPARE("times", sym_times);
	COMPARE("trademark", sym_trademark);
	break;
    case 'u': case 'U':
	COMPARE("union", sym_union);
	COMPARE("uparrow", sym_uparrow);
	UCOMPARE("upsilon", sym_upsilon)
	UCOMPARE("Upsilon", sym_Upsilon)
	COMPARE("upsilon1", sym_Upsilon1)
	break;
    case 'v': case 'V':
	break;
    case 'w': case 'W':
	COMPARE("weierstrass", sym_weierstrass);
	break;
    case 'x': case 'X':
	UCOMPARE("xi", sym_xi);
	UCOMPARE("Xi", sym_Xi);
	break;
    case 'y': case 'Y':
	break;
    case 'z': case 'Z':
	UCOMPARE("zeta", sym_zeta);
	UCOMPARE("Zeta", sym_Zeta);
	break;
    }

#undef COMPARE
#undef UCOMPARE

    c[len] = save;
    if (*sym == -1) return 0;
    else return len;
}


int Draw2(d, w, gc, t, x, y)
Display *d;
Drawable w;
GC gc;
AtText *t;
short x,y;
{
    /*static GC localgc = None; */

    if (!t || !d || !w) return(-1);
    /*if (localgc == None)
	localgc = XCreateGC(d,w,0L,NULL);

    XCopyGC(d, gc, GCFunction | GCPlaneMask | GCForeground | GCBackground |
	    GCFont | GCSubwindowMode | GCClipXOrigin | GCClipYOrigin |
	    GCClipMask | GCFillStyle | GCTile, localgc);

    if (t->fid) XSetFont(d, localgc, t->fid);
    */
    if (t->str) 
      XDrawString(d, w, gc, x, y, t->str, strlen(t->str));
} 


void PlotTextRotateRefresh(dpy, win, d, gc, t)
Display *dpy;
Window win;
Drawable d;
GC gc;
AtText *t;
{
    Pixmap pixmap;
    XImage *image;
    XWindowAttributes wa;
    GC localgc;
    char *data;
    int i,j;
    unsigned long foreground;
    unsigned long background;
    int w,h;  /* width, height */

    /*
     * the height and width of the rotated text.
     * when actually rotating the text, we'll have to reverse these sometimes.
     */
    w = t->width;
    h = t->ascent + t->descent ;

    if ((w==0) || (h==0)) return; /* if there's nothing there, don't bother */

    XGetWindowAttributes(dpy, win, &wa); /* we need the depth and the visual */
    /* this only works for a window, not a drawable.  Fix it. */

    localgc = XCreateGC(dpy, d, 0L, NULL);
    XCopyGC(dpy, gc, GCFunction | GCPlaneMask | GCForeground | GCBackground |
	    GCSubwindowMode | GCFillStyle | GCFont | GCTile,  localgc);
     
    pixmap = XCreatePixmap(dpy, d, w, h, wa.depth);
    foreground = gc->values.foreground;
    background = gc->values.background;
    XSetForeground(dpy, localgc, background);
    XFillRectangle(dpy, pixmap, localgc, 0, 0, w, h);
    XSetForeground(dpy, localgc, foreground);
    Draw2(dpy, pixmap, localgc, t, 0, t->ascent-1);
    image = XGetImage(dpy, pixmap, 0, 0, w, h, AllPlanes, XYPixmap);
     
    data = (char *) malloc(wa.depth * w * (h+7)/8);
    XDestroyImage(t->rotated_image);
    t->rotated_image = XCreateImage(dpy, wa.visual, wa.depth, XYPixmap, 0, data,
				    h, w, 8, (h+7)/8);

    for(i=0; i < w; i++)
	for(j=0; j < h; j++)
	    XPutPixel(t->rotated_image, j, w-i-1, XGetPixel(image, i, j));

    t->rotated_depth = wa.depth;

    XDestroyImage(image);
    XFreePixmap(dpy, pixmap);
    XFreeGC(dpy, localgc);
} /* end of PlotTextRotateRefresh */



static void Rotate(dpy, win, d, gc, t)
Display *dpy;
Window win;
Drawable d;
GC gc;
AtText *t;
{
    Pixmap pixmap;
    XImage *image;
    XWindowAttributes wa;
    GC localgc;
    char *data;
    int i,j;
    unsigned long foreground;
    unsigned long background;
    int w,h;  /* width, height */

    /*
     * the height and width of the rotated text.
     * when actually rotating the text, we'll have to reverse these sometimes.
     */
    w = t->width;
    h = t->ascent + t->descent ;

    if ((w==0) || (h==0)) return; /* if there's nothing there, don't bother */

    XGetWindowAttributes(dpy, win, &wa); /* we need the depth and the visual */
    /* this only works for a window, not a drawable.  Fix it. */

    localgc = XCreateGC(dpy, d, 0L, NULL);
    XCopyGC(dpy, gc, GCFunction | GCPlaneMask | GCForeground | GCBackground |
	    GCSubwindowMode | GCFillStyle | GCFont | GCTile,  localgc);
     
    /*
     * create a pixmap the size of the unrotated text
     * clear the pixmap to the background color
     * draw the text into it unrotated
     * get the text as an unrotated image
     * Since everything is unrotated, use w and h backwards
     */
    pixmap = XCreatePixmap(dpy, d, w, h, wa.depth);
    foreground = gc->values.foreground;
    background = gc->values.background;
    XSetForeground(dpy, localgc, background);
    XFillRectangle(dpy, pixmap, localgc, 0, 0, w, h);
    XSetForeground(dpy, localgc, foreground);
    Draw2(dpy, pixmap, localgc, t, 0, t->ascent-1);
    image = XGetImage(dpy, pixmap, 0, 0, w, h, AllPlanes, XYPixmap);
    /*
     * allocate enough memory for the rotated image
     * create the image.
     */
    data = (char *) malloc(wa.depth * w * (h+7)/8);
    t->rotated_image = XCreateImage(dpy, wa.visual, wa.depth, XYPixmap, 0, data,
				    h, w, 8, (h+7)/8);

    /*
     * rotate the image
     */
    for(i=0; i < w; i++)
	for(j=0; j < h; j++)
	    XPutPixel(t->rotated_image, j, w-i-1, XGetPixel(image, i, j));

    t->rotated_depth = wa.depth;

    XDestroyImage(image);
    XFreePixmap(dpy, pixmap);
    XFreeGC(dpy, localgc);
}


int AtTextRenewFont(Widget w, AtText *text, Font fid)
{
  if (!text || !w || !fid) return(-1);
    text->fid = fid;
    text->fs = XQueryFont(XtDisplay(w), fid); 
    if (text->fs == NULL) {
      printf("Can not load font for fid %d\n", fid);
      text->width = text->str ? strlen(text->str) * 8 : 0;
      text->ascent = 15;
      text->descent = 5;
      }
    else {
      text->width = text->str ? 
	 XTextWidth(text->fs, text->str, strlen(text->str)) : 0;
      text->ascent = text->fs->ascent;
      text->descent = text->fs->descent;
      }
}


int At2TextRenew(AtText *text, char *str, XFontStruct *fs)
{
  if (!text) return(-1);
  if ((fs) && (str)) text->width = XTextWidth(fs, str, strlen(str));
  else text->width = 30;
}


AtText* At2TextCreate(Widget w, char *str, Font fid)
{
    AtText *text;

    text = New(AtText);
    text->str = XtNewString(str);
    text->fid = fid;
    text->fs = XQueryFont(XtDisplay(w), fid); 
    if (text->fs == NULL) {
      printf("Can not load font for fid %d\n", fid);
      text->width = str ? strlen(str) * 8 : 0;
      text->ascent = 15;
      text->descent = 5;
      }
    else {
      text->width = str ? XTextWidth(text->fs, str, strlen(str)) : 0;
      text->ascent = text->fs->ascent;
      text->descent = text->fs->descent;
      }
    text->rotated = 0;
    text->rotated_image = NULL;
    text->rotated_depth = 0;
    return(text);
}

void At2TextDestroy(AtText *t)
{
  if (t)
  free(t);
}

void AtTextDestroy(AtText *t)
{

    if( !t)
	return;

    XtFree(t->str);

    if (t->fs) XFreeFontInfo(NULL, t->fs, 1); 
    if (t->rotated && (t->rotated_image != NULL))
	XDestroyImage(t->rotated_image);

    free(t);
    *SCCSid = *SCCSid;        /* Keep gcc quiet */
} 


void AtTextRotate(t)
AtText *t;
{
    short tmp;

    if (t->rotated == FALSE) {
	t->rotated = TRUE;
	tmp = t->width;
	t->width = t->ascent + t->descent;
	t->ascent = tmp;
	t->descent = 0;
    }
}


void At2TextDraw(dpy, win, d, gc, t, x, y)
Display *dpy;
Window win;
Drawable d;
GC gc;
AtText *t;
int x,y;
{
    if (t->rotated) {
	Window junkw;
	int junki;
	unsigned int junk;
	unsigned int depth;

	if (t->rotated_image == NULL)
	    Rotate(dpy, win, d, gc, t);
	else {
	    XGetGeometry(dpy, d, &junkw, &junki, &junki, &junk, &junk, &junk, &depth);
	    if (depth != t->rotated_depth) {
		fprintf(stderr, "Warning: attempt to draw rotated text in a window\n         of a different depth than than on which it was rotated.\n");
		return;
	    }
	}

	/*XPutImage(dpy, d, gc, t->rotated_image, 0, 0, x, y-t->ascent, t->width, t->ascent); */
	XPutImage(dpy, d, gc, t->rotated_image, 0, 0, x, y, 
	    t->ascent + t->descent, t->width);
    }
    else
	Draw2(dpy, d, gc, t, x, y);
}


void AtTextDrawJustified(dpy, win, d, gc, t, hjust, vjust, x, y, w, h)
Display *dpy;
Window win;
Drawable d;
GC gc;
AtText *t;
int hjust, vjust;
int x,y,w,h;
{
  int tmp_w = t->rotated ? t->ascent + t->descent : t->width;
  int tmp_h = t->rotated ? t->width : t->ascent + t->descent;

    switch (hjust) {
    case AtTextJUSTIFY_LEFT:
	break;
    case AtTextJUSTIFY_CENTER:
	x += (w - tmp_w)/2;
	break;
    case AtTextJUSTIFY_RIGHT:
	x += (w - tmp_w);
	break;
    }

    switch (vjust) {
    case AtTextJUSTIFY_TOP:
	y += t->rotated ? 0 : t->ascent-1;
	break;
    case AtTextJUSTIFY_CENTER:
	y += t->rotated ? (h - tmp_h)/2 : t->ascent-1 + (h - tmp_h)/2;
	break;
    case AtTextJUSTIFY_BOTTOM:
	y += t->rotated ? (h - tmp_h) : h - t->descent;
	break;
    }
    /* for vertical text, the (x, y) is the left and top location of the text
    area; for horizental text, the x is the left, y is the ascent to draw text
    */
    At2TextDraw(dpy, win, d, gc, t, x, y);
}
