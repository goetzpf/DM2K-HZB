/*-----------------------------------------------------------------------------
 * Copyright (c) 1994,1995 Southeastern Universities Research Association,
 *                         Continuous Electron Beam Accelerator Facility
 *
 * This software was developed under a United States Government license
 * described in the NOTICE file included as part of this distribution.
 *
 *-----------------------------------------------------------------------------
 *
 * Description:
 *      Header file for all conversion routines
 *
 *      Simple copy of epics header to avoid to include the epics path
 *      when to build medm using cdev
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab. High Performance Computing Group
 *
 * Revision History:
 *   $Log: cvtFast.h,v $
 *   Revision 1.2  2006-03-28 15:32:05  birke
 *   now a 3.14 application...
 *
 *   Revision 1.1  2000/02/23 16:17:08  birke
 *   += cdev-support
 *
 *
 *
 */
#ifndef _DM2K_CDEV_UTILS_H
#define _DM2K_CDEV_UTILS_H

#ifdef DM2K_CDEV

#if defined (__cplusplus)
extern "C" {
#endif

#define dm2kCvtFloatToString cvtFloatToString
#define dm2kCvtDoubleToString cvtDoubleToString
#define dm2kCvtFloatToExpString cvtFloatToExpString
#define dm2kCvtFloatToExpString cvtFloatToExpString
#define dm2kCvtDoubleToExpString cvtDoubleToExpString
#define dm2kCvtFloatToCompactString cvtFloatToCompactString
#define dm2kCvtDoubleToCompactString cvtDoubleToCompactString
#define dm2kCvtCharToString cvtCharToString
#define dm2kCvtUcharToString cvtUcharToString
#define dm2kCvtShortToString cvtShortToString
#define dm2kCvtUshortToString cvtUshortToString
#define dm2kCvtLongToString cvtLongToString
#define dm2kCvtUlongToString cvtUlongToString
#define dm2kCvtLongToHexString cvtLongToHexString
#define dm2kCvtLongToOctalString cvtLongToOctalString
#define dm2kCvtBitsToUlong cvtBitsToUlong
#define dm2kCvtUlongToBits cvtUlongToBits

/* all these convertion routines */
extern int dm2kCvtFloatToString  (float value, char *pstring, 
				  unsigned short precision);
extern int dm2kCvtDoubleToString  (double value, char *pstring, 
				   unsigned short precision);
extern int dm2kCvtFloatToExpString (float value, char *pstring, 
				    unsigned short precision);
extern int dm2kCvtDoubleToExpString (double value, char *pstring, 
				     unsigned short precision);
extern int dm2kCvtFloatToCompactString(float value, char *pstring, 
				       unsigned short precision);
extern int dm2kCvtDoubleToCompactString(double value, char *pstring, 
					unsigned short precision);
extern int dm2kCvtCharToString (char value, char *pstring);
extern int dm2kCvtUcharToString (unsigned char value, char *pstring);
extern int dm2kCvtShortToString (short value, char *pstring);
extern int dm2kCvtUshortToString (unsigned short value, char *pstring);
extern int dm2kCvtLongToString (long value, char *pstring);
extern int dm2kCvtUlongToString (unsigned long value, char *pstring);
extern int dm2kCvtLongToHexString (long value, char *pstring);
extern int dm2kCvtLongToOctalString (long value, char *pstring);
extern unsigned long dm2kCvtBitsToUlong (
					 unsigned long  src,
					 unsigned bitFieldOffset,
					 unsigned  bitFieldLength);
extern  unsigned long  dm2kCvtUlongToBits (
					   unsigned long src,
					   unsigned long dest,
					   unsigned      bitFieldOffset,
					   unsigned      bitFieldLength);

#if defined (__cplusplus)
};
#endif

#endif

#endif
