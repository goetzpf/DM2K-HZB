#ifndef _faceplateBase_h_
#define _faceplateBase_h_ 1

typedef struct _Faceplate {
  char         * position;
  int            x;
  int            y;
  unsigned int   w;
  unsigned int   h;
  char         * adl;
  char         * macro;
}Faceplate;

typedef struct _FaceplateGroup {
  char            * name;
  char            * title;
  char            * notes;
  char            * comments;
  int               fractionBase;  /*0 means that facep. coor. are in pixel*/ 
  int               x;
  int               y;
  unsigned int      w;
  unsigned int      h;
  Faceplate      ** entries;
  int               entriesNum;
}FaceplateGroup;

#ifdef __cplusplus
extern "C" {
#else
#define const
#endif

extern void destroyFaceplateGroup(FaceplateGroup * fpg);
extern FaceplateGroup * createFaceplateGroup(char * fName);
extern void writeFaceplateGroupToStream(FaceplateGroup * fpg, FILE * stream);
extern Boolean appendNewFaceplate( /* True if it failed */
   FaceplateGroup * fpg, 
   const char     * position,
   int              x,
   int              y,
   unsigned int     w,
   unsigned int     h,
   const char     * adl,
   const char     * macro);

extern void destroyFaceplate(FaceplateGroup *, int);
extern void freeFaceplate(Faceplate *);

#ifdef __cplusplus
}
#endif

#endif
