#ifndef __IMAGE_H__
#define __IMAGE_H__

/* +-------------------------------------------------------------------+ */
/* | Copyright (C) 1993, David Koblas (koblas@netcom.com)              | */
/* | Copyright 1995, 1996 Torsten Martinsen (bullestock@dk-online.dk)  | */
/* |                                                                   | */
/* | Permission to use, copy, modify, and to distribute this software  | */
/* | and its documentation for any purpose is hereby granted without   | */
/* | fee, provided that the above copyright notice appear in all       | */
/* | copies and that both that copyright notice and this permission    | */
/* | notice appear in supporting documentation.  There is no           | */
/* | representations about the suitability of this software for        | */
/* | any purpose.  this software is provided "as is" without express   | */
/* | or implied warranty.                                              | */
/* |                                                                   | */
/* +-------------------------------------------------------------------+ */

typedef struct {
    int refCount;		/* reference count */

    /*
    **  Special notes:
    **    if the image isBW then there will be a two entry
    **       colormap BLACK == 0, WHITE == 1
    **    if the image isGrey, then the colormap is 256 entries
    **       BLACK == 0 .. WHITE == 255
     */
    int isGrey, isBW; /* simple indicator flags  GreyScale, Black & White */

    /*
    **  bytes per pixel (3 for RGB, 2 for 0..256+ cmap, 1 for 0..256 cmap)
     */
    int scale;

    /*
    **  Colormap entries
    **   rgb rgb rgb [1..size]
     */
    int cmapPacked;		/* Boolean, is the colormap packed 
				   down to just the used colors */
    int cmapSize;     /* number of colors in colormap == 0 if no colormap */
    unsigned char *cmapData;

    /*
    **  Image data
    **   either rgb rgb rgb
    **   or     idx idx idx
    **
    **   if image has colormap, and the size > 256, then
    **     data is pointers to unsigned shorts.
     */
    int width, height;		/* width, height of image */
    unsigned char *data;
    unsigned char *maskData;

    /*
    **  These values are here because the XPM calls are TOO dependant
    **   on X Windows.  They are not, and should not, be used by anything
    **   else.
     */
    unsigned long sourcePixmap;
    unsigned long sourceColormap;
    unsigned long sourceMask;
} Image;

#if 0
#ifdef _XLIB_H_
/* Used to transfer information to routines in iprocess.c */
struct imageprocessinfo {
    int oilArea;
    int noiseDelta;
    int spreadDistance;
    int pixelizeXSize;
    int pixelizeYSize;
    int despeckleMask;
    int smoothMaskSize;
    int tilt;
    int solarizeThreshold;
    int contrastW;
    int contrastB;
    int quantizeColors;
    int tiltX1;
    int tiltY1;
    int tiltX2;
    int tiltY2;
    XColor *background;
};

#endif

#define ImagePixel(image, x, y)						\
	(((image)->cmapSize == 0)					\
	  ? &((image)->data[((y) * (image)->width + (x)) * 3])		\
	  : (((image)->cmapSize > 256)					\
	     ? &((image)->cmapData[((unsigned short *)(image)->data)	\
				[(y) * (image)->width + (x)] * 3])	\
	     : &((image)->cmapData[(image)->data[(y)*(image)->width+(x)] * 3])))

#define ImageSetCmap(image, index, r, g, b) do {		\
			image->cmapData[(index) * 3 + 0] = r;	\
			image->cmapData[(index) * 3 + 1] = g;	\
			image->cmapData[(index) * 3 + 2] = b;	\
		} while (0)

/* image.c */
Image *ImageNew(int width, int height);
Image *ImageNewCmap(int width, int height, int size);
Image *ImageNewBW(int width, int height);
Image *ImageNewGrey(int width, int height);
void ImageMakeMask(Image * image);
void ImageDelete(Image * image);
Image *ImageToRGB(Image * image);

#ifdef _XtIntrinsic_h
Image *PixmapToImage(Widget w, Pixmap pix, Colormap cmap);
void PixmapToImageMask(Widget w, Image * image, Pixmap mask);
Boolean ImageToPixmap(Image * image, Widget w, Pixmap * pix, Colormap * cmap);
Pixmap MaskDataToPixmap(Widget w, int width, int height,
			char *data, XRectangle * rect);
Pixmap ImageMaskToPixmap(Widget w, Image * image);
Boolean ImageToPixmapCmap(Image * image, Widget w, Pixmap * pix, Colormap cmap);
#endif

/* imageComp.c */
Image *ImageCompress(Image * input, int ncolors, int noforce);

/* iprocess.c */
Image *ImageSmooth(Image * input);
Image *ImageSharpen(Image * input);
Image *ImageEdge(Image * input);
Image *ImageEmbose(Image * input);
Image *ImageInvert(Image * input);
Image *ImageOilPaint(Image * input);
Image *ImageAddNoise(Image * input);
Image *ImageSpread(Image * input);
Image *ImageBlend(Image * input);
Image *ImagePixelize(Image * input);
Image *ImageDespeckle(Image * input);
Image *ImageNormContrast(Image * input);
Image *ImageHistogram(Image * input);
Image *ImageSolarize(Image * input);
Image *ImageQuantize(Image * input);
Image *ImageGrey(Image * input);
Image *ImageTilt(Image * input);
Image *ImageDirectionalFilter(Image * input);

/* texture.c */
Image *draw_plasma(int w, int h);
Image *draw_landscape(int w, int h, int clouds);

#endif 
#endif				/* __IMAGE_H__ */
