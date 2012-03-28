/*
 * xgif.c - displays GIF pictures on an X11 display
 *
 *  Author:    John Bradley, University of Pennsylvania
 *                (bradley@cis.upenn.edu)
 *
 *
 *	MDA - editorial comment:
 *		MAJOR MODIFICATIONS to make this sensible.
 *		be careful about running on non-32-bit machines
 *   09-13-95   vong  conform to c++ syntax
 *   02-29-96   vong  support 16 and 32 bits graphics machine
 */



/* include files */
#include "dm2k.h"
#include "xgif.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef __cplusplus
extern "C" int _XGetBitsPerPixel(Display *, int);
#else
extern int _XGetBitsPerPixel(Display *, int);
#endif

#define MAXEXPAND 16

static void AddToPixel(GIFData *gif, Byte Index);
static int ReadCode(void);

/*
 * initialize for GIF processing
 */
Boolean initializeGIF(DisplayInfo *displayInfo, DlImage *dlImage)
{
  GIFData      * gif;
  int            x, y;
  unsigned int   w, h;
  Boolean        success;


  x = dlImage->object.x;
  y = dlImage->object.y;
  w = dlImage->object.width;
  h = dlImage->object.height;

  /* free any existing GIF resources */
  freeGIF(displayInfo,dlImage);

  if (!(gif = (GIFData *) malloc(sizeof(GIFData)))) {
    fprintf(stderr,"\ninitializeGIF: malloc error!");
    return(False);
  }

  /* (MDA) programmatically set nostrip to false - see what happens */
  gif->nostrip = False;
  gif->strip = 0;

  gif->theImage = NULL;
  gif->expImage = NULL;
  gif->fcol = 0;
  gif->bcol = 0;
  gif->mfont= 0;
  gif->mfinfo = NULL;
  gif->theCmap   = cmap;	/* MDA - used to be DefaultColormap() */
  gif->theGC     = DefaultGC(display,screenNum);
  gif->theVisual = DefaultVisual(display,screenNum);
  gif->numcols   = 0;

#if 0
  /* TrueColor visual does not show real count of cells
   */
  if (XMatchVisualInfo(display, screenNum, 
		       DefaultDepth(display,screenNum),
		       TrueColor, &visualInfo))
    gif->dispcells = 1 << DefaultDepth(display,screenNum);
  else
    gif->dispcells = DisplayCells(display, screenNum);
  
  if (gif->dispcells < 255) {
    fprintf(stderr,"initializeGIF: >= 8-plane display required");
    return(False);
  }
#endif

  dlImage->privateDataData = (XtPointer) gif;

  /*
   * open/read the file
   */
  success = loadGIF(displayInfo,dlImage);
  if (!success) {
    return(False);
  }

  gif->iWIDE = gif->theImage->width;
  gif->iHIGH = gif->theImage->height;

  gif->eWIDE = gif->iWIDE;
  gif->eHIGH = gif->iHIGH;

  resizeGIF(displayInfo,dlImage);

  if (gif->expImage) {
/*
    XSetForeground(display,gif->theGC,gif->bcol);
    XFillRectangle(display,XtWindow(displayInfo->drawingArea),
                   gif->theGC,x,y,w,h);
    XSetForeground(display,gif->theGC,gif->fcol);
*/
#if 1
    XPutImage(display,XtWindow(displayInfo->drawingArea),
	      gif->theGC,gif->expImage,
	      0,0,x,y,w,h);
#endif
    XPutImage(display,displayInfo->drawingAreaPixmap,
	      gif->theGC,gif->expImage,
	      0,0,x,y,w,h);

/*
    XSetForeground(display,gif->theGC,gif->fcol);
*/
  }
  return(True);

}

void drawGIF(DisplayInfo * displayInfo,
	     DlImage     * dlImage)
{
  GIFData *gif;
  int x, y;
  unsigned int w, h;

  gif = (GIFData *) dlImage->privateDataData;

  if (gif->expImage != NULL) {
    x = dlImage->object.x;
    y = dlImage->object.y;
    w = dlImage->object.width;
    h = dlImage->object.height;
    /* draw to pixmap, since traversal will copy pixmap to window..*/
#if 1
    XPutImage(display,XtWindow(displayInfo->drawingArea),
	      gif->theGC,gif->expImage,
	      0,0,x,y,w,h);
#endif
    XPutImage(display,displayInfo->drawingAreaPixmap,
	      gif->theGC,gif->expImage,
	      0,0,x,y,w,h);
  }
}


/***********************************/
#ifdef __cplusplus
void resizeGIF(DisplayInfo *,DlImage *dlImage)
#else
void resizeGIF(DisplayInfo *displayInfo,DlImage *dlImage)
#endif
{
  GIFData      * gif;
  unsigned int   w,h;
  unsigned int   screenW, screenH;

  /* warning:  this code'll only run machines where int=32-bits */

  gif = (GIFData *) dlImage->privateDataData;

  screenW = WidthOfScreen(ScreenOfDisplay(display, screenNum));
  screenH = HeightOfScreen(ScreenOfDisplay(display, screenNum));

  if (dlImage->object.width > screenW) dlImage->object.width = screenW;
  if (dlImage->object.height > screenH)  dlImage->object.height = screenH;

  w = dlImage->object.width;
  h = dlImage->object.height;

  /* simply return if no GIF image attached */
  if (gif->expImage == NULL && gif->theImage == NULL) return;


  gif->currentWidth = w;
  gif->currentHeight= h;

  if (w==gif->iWIDE && h==gif->iHIGH) 
    {
      /* very special case 
       */
      gif->expImage = gif->theImage;
      
      gif->eWIDE = gif->iWIDE;  
      gif->eHIGH = gif->iHIGH;
    }
  else
    { 
      /* have to do some work */
      /* if it's a big image, this'll take a while.  mention it */
      if (w*h>(400*400)) {
	/* (MDA) - could change cursor to stopwatch...*/
      }
      
      /* first, kill the old gif->expImage, if one exists */
      if (gif->expImage && gif->expImage != gif->theImage) {
	XDestroyImage((XImage *)(gif->expImage));
	gif->expImage->data = NULL;
      }
      
      /* create gif->expImage of the appropriate size */
      gif->eWIDE = w;  gif->eHIGH = h;
      
      {
	int sx, sy, dx, dy;
	int sh, dh, sw, dw;
	char *ilptr,*ipptr,*elptr,*epptr;
	int bytesPerPixel = gif->theImage->bits_per_pixel/8;

	gif->expImage = XCreateImage(display,gif->theVisual,
				     DefaultDepth(display,screenNum),ZPixmap,
				     0,NULL,
				     gif->eWIDE,gif->eHIGH,32,
				     0);
	
	if (gif->expImage == NULL) {
	  fprintf(stderr,"\nresizeGIF: unable to create a %dx%d image\n",
		  w,h);
	  return;
	}

	gif->expImage->data = 
	  (char *) calloc (h, gif->expImage->bytes_per_line);
	
	if (gif->expImage->data == NULL) {
	  XDestroyImage(gif->expImage);
	  gif->expImage = NULL;
	  fprintf(stderr,"\nresizeGIF: unable to create a %dx%d image\n",
		  w,h);
	  return;
	}
	
	sw = gif->theImage->width;
	sh = gif->theImage->height;
	dw = gif->expImage->width;
	dh = gif->expImage->height;
	
	elptr = epptr = (char *) gif->expImage->data;

	for (dy=0;  dy<dh; dy++) {
	  sy = (sh * dy) / dh;
	  
	  epptr = elptr;
	  ilptr = gif->theImage->data + (sy * gif->theImage->bytes_per_line);
	
	  for (dx=0;  dx <dw;  dx++) 
	    {
	      register int n;

	      sx = (sw * dx) / dw;
	      ipptr = ilptr + sx*bytesPerPixel;
	    
	      for (n = 0; n < bytesPerPixel; n++) 
		*epptr++ = *ipptr++;
	    }
	  elptr += gif->expImage->bytes_per_line;
	}
      }
    }
}

/*
 *  xgifload.c contents
 */


#define NEXTBYTE (*ptr++)
#define IMAGESEP 0x2c
#define INTERLACEMASK 0x40
#define COLORMAPMASK 0x80

static FILE *fp;

static int BitOffset,			/* Bit Offset of next code */
  XC, YC,			/* Output X and Y coords of current pixel */
  Pass,			/* Used by output routine if interlaced pic */
  OutCount,			/* Decompressor output 'stack count' */
  RWidth, RHeight,		/* screen dimensions */
  Width, Height,		/* image dimensions */
  LeftOfs, TopOfs,		/* image offset */
  BitsPerPixel,		/* Bits per pixel, read from GIF header */
  BytesPerScanline,		/* Bytes per scanline in output raster */
  ColorMapSize,		/* number of colors */
  Background,			/* background color */
  CodeSize,			/* Code size, read from GIF header */
  InitCodeSize,		/* Starting code size, used during Clear */
  Code,			/* Value returned by ReadCode */
  MaxCode,			/* limiting value for current code size */
  ClearCode,			/* GIF clear code */
  EOFCode,			/* GIF end-of-information code */
  CurCode, OldCode, InCode,	/* Decompressor variables */
  FirstFree,			/* First free code, generated per GIF spec */
  FreeCode,			/* Decompressor, next free slot in hash table */
  FinChar,			/* Decompressor variable */
  BitMask,			/* AND mask for data size */
  ReadMask,			/* Code AND mask for current code size */
  BytesOffsetPerPixel,        /* Bytes offset per pixel */   
  ScreenDepth;                /* Bits per Pixel */

static Boolean Interlace, HasColormap;
static Boolean verbose = False;

static Byte *Image;			/* The result array */
static Byte *RawGIF;			/* The heap array to hold it, raw */
static Byte *Raster;			/* The raster data stream, unblocked */

/* The hash table used by the decompressor */
static int Prefix[4096];
static int Suffix[4096];

/* An output array used by the decompressor */
static int OutCode[1025];

/* The color map, read from the GIF header */
static Byte Red[256], Green[256], Blue[256];

static char *id = "GIF87a";



/*****************************/
#ifdef __cplusplus
Boolean loadGIF(DisplayInfo *, DlImage *dlImage)
#else
Boolean loadGIF(DisplayInfo *displayInfo, DlImage *dlImage)
#endif
{
  GIFData *gif;
  char *fname;
  Boolean success;
  int            filesize;
  register Byte  ch, ch1;
  register Byte *ptr, *ptr1;
  register int   i,j;
  static Byte    lmasks[8] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};
  Byte           lmask;

  char fullPathName[2*MAX_TOKEN_LENGTH], dirName[2*MAX_TOKEN_LENGTH];
  char *dir;
  int startPos;

  gif = (GIFData *) dlImage->privateDataData;
  /* T. Straumann: added paranoia check */
  fname = dlImage->imageName ? dlImage->imageName : "";

  /* initiliaze some globals */
  BitOffset = 0;
  XC = 0;
  YC = 0;
  Pass = 0;
  OutCount = 0;
  ScreenDepth = DefaultDepth(display,screenNum);

  fp = fopen(fname,"r");

  /* try to get a valid GIF file somewhere 
   * if not in current directory, look in EPICS_DISPLAY_PATH directory 
   */
  if (fp == NULL) {
    dir = getenv(DISPLAY_LIST_ENV);
    if (dir != NULL) {
      startPos = 0;
      while (fp == NULL &&
	     extractStringBetweenColons(dir,dirName,startPos,&startPos)) {
	strcpy(fullPathName,dirName);
	strcat(fullPathName,"/");
	strcat(fullPathName,fname);
	fp = fopen(fullPathName,"r");
      }
    }
  }
  
  if (fp == NULL) {
    fprintf(stderr,"\nloadGIF: file <%s>  not found",fname);
    return(False);
  }

  /* find the size of the file 
   */
  fseek(fp, 0L, SEEK_END);
  filesize = (int) ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  success = True;

  if (!(ptr = RawGIF = (Byte *) malloc(filesize))) {
    fprintf(stderr,"loadGIF: not enough memory to read gif file");
    success = False;
  }

  if (!(Raster = (Byte *) malloc(filesize))) {
    fprintf(stderr,"loadGIF: not enough memory to read gif file");
    success = False;
  }

  if (fread((char *)ptr, filesize, 1, fp) != 1) {
    fprintf(stderr,"loadGIF: GIF data read failed");
    success = False;
  }

  if (strncmp((char *)ptr, (char *)id, 6) 
      &&strncmp((char *)ptr, "GIF89",5)) {
    fprintf(stderr,"loadGIF: not a GIF file");
    success = False;
  }

  if (!success) return(False);

  ptr += 6;

  /* Get variables from the GIF screen descriptor */

  ch = NEXTBYTE;
  RWidth = ch + 0x100 * NEXTBYTE; /* screen dimensions... not used. */
  ch = NEXTBYTE;
  RHeight = ch + 0x100 * NEXTBYTE;

  if (verbose)
    fprintf(stderr, "loadGIF: screen dims: %dx%d.\n", RWidth, RHeight);

  ch = NEXTBYTE;
  HasColormap = ((ch & COLORMAPMASK) ? True : False);

  BitsPerPixel = (ch & 7) + 1;
  gif->numcols = ColorMapSize = 1 << BitsPerPixel;
  BitMask = ColorMapSize - 1;

  Background = NEXTBYTE;	/* background color... not used. */

  if (NEXTBYTE) {		/* supposed to be NULL */
    fprintf(stderr,"loadGIF: corrupt GIF file (bad screen descriptor)");
  }


  /* Read in global colormap. */

  if (HasColormap) {
    if (verbose) {
      fprintf(stderr,
	      "loadGIF: %s is %dx%d, %d bits per pixel, (%d colors).\n",
	      fname, Width,Height,BitsPerPixel, ColorMapSize);
    }

    for (i = 0; i < ColorMapSize; i++) {
      Red[i] = NEXTBYTE;
      Green[i] = NEXTBYTE;
      Blue[i] = NEXTBYTE;
    }

    /* Allocate the X colors for this picture */

    if (gif->nostrip)  {	/* nostrip was set.  try REAL hard to do it */
      j = 0;
  
      lmask = lmasks[gif->strip];

      for (i=0; i<gif->numcols; i++) {
	gif->defs[i].red   = (Red[i]  &lmask)<<8;
	gif->defs[i].green = (Green[i]&lmask)<<8;
	gif->defs[i].blue  = (Blue[i] &lmask)<<8;
	gif->defs[i].flags = DoRed | DoGreen | DoBlue;

	if (!XAllocColor(display,gif->theCmap,&gif->defs[i])) { 
	  j++;  gif->defs[i].pixel = 0xffff;
	}
	gif->cols[i] = gif->defs[i].pixel;
      }

      if (j) {			/* failed to pull it off */
	XColor ctab[256];

	fprintf(stderr,"%s%d%s%d%s","loadGIF: failed to allocate ",
		j," out of",gif->numcols,
		" colors.  Trying extra hard.\n");
	
	/* read in the color table */
	for (i = 0; i < gif->numcols; i++) 
	  ctab[i].pixel = i;

	XQueryColors(display,gif->theCmap,ctab,gif->numcols);
	
	for (i = 0; i < gif->numcols; i++)
	  if (gif->cols[i] == 0xffff) {	/* an unallocated pixel */
	    int d, mdist, close;
	    unsigned long r,g,b;

	    mdist = 100000;   
	    close = -1;

	    r =  Red[i];
	    g =  Green[i];
	    b =  Blue[i];

	    for (j = 0; j < gif->numcols; j++) {
	      d = abs((int)(r - (ctab[j].red>>8))) +
		  abs((int)(g - (ctab[j].green>>8))) +
		  abs((int)(b - (ctab[j].blue>>8)));

	      if (d<mdist) {
		mdist=d; 
		  close=j; 
	      }
	    }
	    
	    if (close<0) {
	      fprintf(stderr,
		      "loadGIF: simply can't do it.  Sorry.");
	    }
	    memcpy( (void*)(&gif->defs[i]),
		    (void*)(&gif->defs[close]),
		    sizeof(XColor));
	    gif->cols[i] = ctab[close].pixel;
	  }
      }
    }
    else {			/* strip wasn't set, do the best auto-strip */
      j = 0;

      while (gif->strip<8) {
	lmask = lmasks[gif->strip];
	for (i=0; i<gif->numcols; i++) {
	  gif->defs[i].red   = (Red[i]  &lmask)<<8;
	  gif->defs[i].green = (Green[i]&lmask)<<8;
	  gif->defs[i].blue  = (Blue[i] &lmask)<<8;
	  gif->defs[i].flags = DoRed | DoGreen | DoBlue;
	  if (!XAllocColor(display,gif->theCmap,&gif->defs[i])) break;
	  gif->cols[i] = gif->defs[i].pixel;
	}

	if (i<gif->numcols) {	/* failed */
	  gif->strip++; 
	  j++;
	  XFreeColors(display,gif->theCmap,gif->cols,i,0L);
	}
	else 
	  break;
      }

      if (j && gif->strip<8)
	if (verbose) 
	  fprintf(stderr,
		  "loadGIF:  %s stripped %d bits\n",fname,gif->strip);

      if (gif->strip==8) {
	fprintf(stderr,
		"loadGIF: failed to allocate the desired colors.\n");
	for (i=0; i<gif->numcols; i++) 
	  gif->cols[i]=i;
      }
    }
  }
  else {			/* no colormap in GIF file */
    fprintf(stderr,
	    "loadGIF:  warning!  no colortable in this file.  Winging it.\n");
    if (!gif->numcols) 
      gif->numcols=256;

    for (i=0; i<gif->numcols; i++) 
      gif->cols[i] = (unsigned long) i;
  }

  /* Check for image seperator */

  if (NEXTBYTE != IMAGESEP) {
    fprintf(stderr,"loadGIF: corrupt GIF file (no image separator)");
  }

  /* Now read in values from the image descriptor */

  ch = NEXTBYTE;
  LeftOfs = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  TopOfs = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  Width = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  Height = ch + 0x100 * NEXTBYTE;
  Interlace = ((NEXTBYTE & INTERLACEMASK) ? True : False);

  if (verbose) {
    fprintf(stderr, "loadGIF: Reading a %d by %d %sinterlaced image...\n",
	    Width, Height, (Interlace) ? "" : "non-");
    fprintf(stderr,"loadGIF:  %s is %dx%d, %d colors, %sinterlaced\n",
	    fname, Width,Height,ColorMapSize,(Interlace) ? "" : "non-");
  }
  

  /* Note that I ignore the possible existence of a local color map.
   * I'm told there aren't many files around that use them, and the spec
   * says it's defined for future use.  This could lead to an error
   * reading some files. 
   */

  /* Start reading the raster data. First we get the intial code size
   * and compute decompressor constant values, based on this code size.
   */
  CodeSize = NEXTBYTE;
  ClearCode = (1 << CodeSize);
  EOFCode = ClearCode + 1;
  FreeCode = FirstFree = ClearCode + 2;

  /* The GIF spec has it that the code size is the code size used to
   * compute the above values is the code size given in the file, but the
   * code size used in compression/decompression is the code size given in
   * the file plus one. (thus the ++).
   */
  CodeSize++;
  InitCodeSize = CodeSize;
  MaxCode = (1 << CodeSize);
  ReadMask = MaxCode - 1;

  /* Read the raster data.  Here we just transpose it from the GIF array
   * to the Raster array, turning it from a series of blocks into one long
   * data stream, which makes life much easier for ReadCode().
   */
  ptr1 = Raster;
  do {
    ch = ch1 = NEXTBYTE;
    while (ch--) *ptr1++ = NEXTBYTE;
    if ((ptr1 - Raster) > filesize){
      fprintf(stderr,"loadGIF: corrupt GIF file (unblock)\n");
    }
  } while(ch1);

  free((char *)RawGIF);		/* We're done with the raw data now... */

  if (verbose)
    fprintf(stderr, "loadGIF: done.\n Decompressing...\n");


  /* Allocate the X Image 
   */
  BytesOffsetPerPixel = (_XGetBitsPerPixel(display, ScreenDepth)+7)/8;

  gif->theImage = XCreateImage(display,gif->theVisual,
			       ScreenDepth,ZPixmap,0,
			       NULL,Width,Height,32,0);

  if (gif->theImage == NULL) {
    fprintf(stderr,"loadGIF: unable to create XImage");
    return(False);
  }

  if (verbose)
    fprintf(stderr, "loadGIF: Allocating %d bytes of memory for the image\n  BytesOffsetPerPixel = %d\n  Height = %d\n  bytes_per_line = %d\n",
	    Height * gif->theImage->bytes_per_line,
	    BytesOffsetPerPixel, Height, gif->theImage->bytes_per_line);
  gif->theImage->data = (char *) calloc (Height,gif->theImage->bytes_per_line);
  
  if (gif->theImage->data == NULL) {
    XDestroyImage(gif->theImage);
    gif->theImage = NULL;
    Image = NULL;
    fprintf(stderr,"loadGIF: not enough memory for XImage");
    return(False);
  }
  
  Image = (Byte *) gif->theImage->data;
  BytesPerScanline = gif->theImage->bytes_per_line;

  /* Decompress the file, continuing until you see the GIF EOF code.
   * One obvious enhancement is to add checking for corrupt files here.
   */
  Code = ReadCode();
  while (Code != EOFCode) {

    /* Clear code sets everything back to its initial value, then reads the
     * immediately subsequent code as uncompressed data.
     */
    if (Code == ClearCode) {
      CodeSize = InitCodeSize;
      MaxCode = (1 << CodeSize);
      ReadMask = MaxCode - 1;
      FreeCode = FirstFree;
      CurCode = OldCode = Code = ReadCode();
      FinChar = CurCode & BitMask;
      AddToPixel(gif,FinChar);
    }
    else {

      /* If not a clear code, then must be data:
       * save same as CurCode and InCode 
       */
      CurCode = InCode = Code;

      /* If greater or equal to FreeCode, not in the hash table yet;
       * repeat the last character decoded
       */
      if (CurCode >= FreeCode) {
	CurCode = OldCode;
	OutCode[OutCount++] = FinChar;
      }

      /* Unless this code is raw data, pursue the chain pointed to by CurCode
       * through the hash table to its end; each code in the chain puts its
       * associated output code on the output queue.
       */
      while (CurCode > BitMask) {
	if (OutCount > 1024){
	  fprintf(stderr,"corrupt GIF file (OutCount)");
	  return False;
	}
	OutCode[OutCount++] = Suffix[CurCode];
	CurCode = Prefix[CurCode];
      }

      /* The last code in the chain is treated as raw data. 
       */
      FinChar = CurCode & BitMask;
      OutCode[OutCount++] = FinChar;

      /* Now we put the data out to the Output routine.
       * It's been stacked LIFO, so deal with it that way...
       */
      for (i = OutCount - 1; i >= 0; i--)
	AddToPixel(gif,OutCode[i]);
      OutCount = 0;

      /* Build the hash table on-the-fly. No table is stored in the file. 
       */
      Prefix[FreeCode] = OldCode;
      Suffix[FreeCode] = FinChar;
      OldCode = InCode;

      /* Point to the next slot in the table.  If we exceed the current
       * MaxCode value, increment the code size unless it's already 12.  If it
       * is, do nothing: the next code decompressed better be CLEAR
       */
      FreeCode++;
      if (FreeCode >= MaxCode) {
	if (CodeSize < 12) {
	  CodeSize++;
	  MaxCode *= 2;
	  ReadMask = (1 << CodeSize) - 1;
	}
      }
    }
    Code = ReadCode();
  }

  free((char *)Raster);

  if (verbose)
    fprintf(stderr, "loadGIF: done.\n");

  fclose(fp);

  return(True);
}


/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit Bytes, so we have to
 * maintain our location in the Raster array as a BIT Offset.  We compute
 * the Byte Offset into the raster array by dividing this by 8, pick up
 * three Bytes, compute the bit Offset into our 24-bit chunk, shift to
 * bring the desired code to the bottom, then mask it off and return it. 
 */
static int ReadCode()
{
  int RawCode, ByteOffset;

  ByteOffset = BitOffset / 8;
  RawCode = Raster[ByteOffset] + (0x100 * Raster[ByteOffset + 1]);
  if (CodeSize + BitOffset % 8 > 15)
    RawCode += (0x10000 * Raster[ByteOffset + 2]);
  RawCode >>= (BitOffset % 8);
  BitOffset += CodeSize;
  return(RawCode & ReadMask);
}

void dump(GIFData *gif) {
  int i;
  for (i=0; i<32; i++) {
    if (i && ((i>>4)<<4) == i) {
      printf("\n");
    }
    printf("%02x ",(unsigned char) gif->theImage->data[i]);
  }
  printf("\n");
}

static void AddToPixel(GIFData *gif, Byte Index)
{
  Pixel           clr = gif->cols[Index&(gif->numcols-1)];

  if ( YC >= gif->theImage->height ) {
     fprintf( stderr, "AddToPixel: coordinates out of range (%d, %d) # (%d, %d)\n",
	      XC, YC, gif->theImage->width, gif->theImage->height );
     return;
  }

  XPutPixel( gif->theImage, XC, YC, clr );
  XC++;
  if ( XC >= gif->theImage->width ) {
     XC = 0;
     YC++;
  }
  return;
}



/*
 * free the X images and color cells in the default colormap (in anticipation
 *   of a new image)
 */
#ifdef __cplusplus
void freeGIF(DisplayInfo *, DlImage *dlImage)
#else
     void freeGIF(DisplayInfo *displayInfo, DlImage *dlImage)
#endif
{
  GIFData *gif;

  gif = (GIFData *) dlImage->privateDataData;

  if (gif != NULL) {
    /* kill the old images 
     */
    if (gif->expImage) {
      if( gif->expImage != gif->theImage)
	XDestroyImage((XImage *)(gif->expImage));
      else
	gif->expImage = NULL;
    }

    if (gif->theImage) {
      XDestroyImage((XImage *)(gif->theImage));
      gif->theImage = NULL;
    }

    if (gif->numcols > 0) {
      XFreeColors(display, gif->theCmap, gif->cols, gif->numcols, 0);
      gif->numcols = 0;
    }
    /* free existing privateData data */
    free( (char *) dlImage->privateDataData);
    dlImage->privateDataData = NULL;
  }
}
