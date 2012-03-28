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
 *      Simple Copy of EPICS/base/src/cvtFast.c to avoid to compile
 *      medm with libCom
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab. High Performance Computing Group
 *
 * Revision History:
 *   $Log: dm2kCdevUtils.c,v $
 *   Revision 1.3  2009-01-13 10:35:28  birke
 *   latest changes/fixes (numerous small)
 *
 *   Revision 1.2  2006/03/28 15:32:06  birke
 *   now a 3.14 application...
 *
 *   Revision 1.1  2000/02/23 16:17:12  birke
 *   += cdev-support
 *
 *
 */
#ifdef DM2K_CDEV

#include <stdio.h>
#include <stdlib.h>
#include <dm2k.h>

/*
 * This routine converts numbers less than 10,000,000. It defers to f_to_str for
 * numbers requiring more than 8 places of precision. There are only eight decimal
 */
static long	frac_multiplier[] =
	{1,10,100,1000,10000,100000,1000000,10000000,100000000};

int  dm2kCvtFloatToString(
	float flt_value,
	char  *pstr_value,
	unsigned short precision)
{
        unsigned short	got_one,i;
	long		whole,iplace,number,fraction,fplace;
	float		ftemp;
	char		*startAddr;

	/* can this routine handle this conversion */
	if (precision > 8 || flt_value > 10000000.0 || flt_value < -10000000.0) {
		sprintf(pstr_value,"%12.5e",(double)flt_value);
		return((int)strlen(pstr_value));
	}
	startAddr = pstr_value;

	/* determine the sign */
        if (flt_value < 0){
                *pstr_value++ = '-';
                flt_value = -flt_value;
        };

 	/* remove the whole number portion */
	whole = (long)flt_value;
	ftemp = flt_value - whole;

	/* multiplier to convert fractional portion to integer */
	fplace = frac_multiplier[precision];
	fraction = (long)(ftemp * fplace * 10);	
	fraction = (fraction + 5) / 10;		/* round up */

	/* determine rounding into the whole number portion */
	if ((fraction / fplace) >= 1){
		whole++;
		fraction -= fplace;
	}

	/* whole numbers */
        got_one = 0;
        for (iplace = 10000000; iplace >= 1; iplace /= 10){
                if (whole >= iplace){
                        got_one = 1;
                        number = whole / iplace;
                        whole = whole - (number * iplace);
                        *pstr_value = number + '0';
                        pstr_value++;
                }else if (got_one){
                        *pstr_value = '0';
                        pstr_value++;
                }
        }
        if (!got_one){
                *pstr_value = '0';
                pstr_value++;
        }

        /* fraction */
        if (precision > 0){
		/* convert fractional portional to ASCII */
                *pstr_value = '.';
                pstr_value++;
                for (fplace /= 10, i = precision; i > 0; fplace /= 10,i--){
                        number = fraction / fplace;
                        fraction -= number * fplace;
                        *pstr_value = number + '0';
                        pstr_value++;
                }
        }
        *pstr_value = 0;

        return((int)(pstr_value - startAddr));
}

int  dm2kCvtDoubleToString(
	double flt_value,
	char  *pstr_value,
	unsigned short precision)
{
        unsigned short	got_one,i;
	long		whole,iplace,number,fraction,fplace;
	double		ftemp;
	char		*startAddr;

	/* can this routine handle this conversion */
	if (precision > 8 || flt_value > 10000000.0 || flt_value < -10000000.0) {
		if (precision > 8 || flt_value > 1e16 || flt_value < -1e16) {
		    if(precision>17) precision=17;
		    sprintf(pstr_value,"%*.*e",precision+7,precision,
			flt_value);
		} else {
		    sprintf(pstr_value,"%.0f",flt_value);
		}
		return((int)strlen(pstr_value));
	}
	startAddr = pstr_value;

	/* determine the sign */
        if (flt_value < 0){
                *pstr_value++ = '-';
                flt_value = -flt_value;
        };

 	/* remove the whole number portion */
	whole = (long)flt_value;
	ftemp = flt_value - whole;

	/* multiplier to convert fractional portion to integer */
	fplace = frac_multiplier[precision];
	fraction = (long)(ftemp * fplace * 10);	
	fraction = (fraction + 5) / 10;		/* round up */

	/* determine rounding into the whole number portion */
	if ((fraction / fplace) >= 1){
		whole++;
		fraction -= fplace;
	}

	/* whole numbers */
        got_one = 0;
        for (iplace = 10000000; iplace >= 1; iplace /= 10){
                if (whole >= iplace){
                        got_one = 1;
                        number = whole / iplace;
                        whole = whole - (number * iplace);
                        *pstr_value = number + '0';
                        pstr_value++;
                }else if (got_one){
                        *pstr_value = '0';
                        pstr_value++;
                }
        }
        if (!got_one){
                *pstr_value = '0';
                pstr_value++;
        }

        /* fraction */
        if (precision > 0){
		/* convert fractional portional to ASCII */
                *pstr_value = '.';
                pstr_value++;
                for (fplace /= 10, i = precision; i > 0; fplace /= 10,i--){
                        number = fraction / fplace;
                        fraction -= number * fplace;
                        *pstr_value = number + '0';
                        pstr_value++;
                }
        }
        *pstr_value = 0;

        return((int)(pstr_value - startAddr));
}

/*
 * dm2kCvtFloatToExpString
 *
 * converts floating point numbers to E-format NULL terminated strings
 */
int  dm2kCvtFloatToExpString(
  float			f_value,
  char			*pstr_value,
  unsigned short	f_precision)
{
    /*sunos uses char*sprint as function prototype*/
    sprintf(pstr_value,"%.*e",(int)f_precision,(double)f_value);
    return((int)strlen(pstr_value));
}

/*
 * dm2kCvtFloatToCompactString
 *
 * Converts floating point numbers to %g format NULL terminated strings,
 * resulting in the most "compact" expression of the value
 * ("f" notation if 10-4 < |value| < 10+4, otherwise "e" notation)
 */
int  dm2kCvtFloatToCompactString(
  float			f_value,
  char			*pstr_value,
  unsigned short	f_precision )
{
  if ((f_value < 1.e4 && f_value > 1.e-4) ||
		(f_value > -1.e4 && f_value < -1.e-4) || f_value == 0.0) {
    return(dm2kCvtFloatToString(f_value,pstr_value,f_precision));
  } else {
    return(dm2kCvtFloatToExpString(f_value,pstr_value,f_precision));
  }
}



/*
 * dm2kCvtDoubleToExpString
 *
 * converts double precision floating point numbers to E-format NULL
 *	terminated strings
 */

int  dm2kCvtDoubleToExpString(
  double		f_value,
  char			*pstr_value,
  unsigned short	f_precision )
{
    sprintf(pstr_value,"%.*e",(int)f_precision,f_value);
    return((int)strlen(pstr_value));
}


/*
 * dm2kCvtDoubleToCompactString
 *
 * Converts double precision floating point numbers to %g format NULL
 *	terminated strings, resulting in the most "compact" expression
 *	of the value ("f" notation if 10-4 < |value| < 10+4, otherwise
 *	"e" notation)
 */
int  dm2kCvtDoubleToCompactString(
  double		f_value,
  char			*pstr_value,
  unsigned short	f_precision )
{
  if ((f_value < 1.e4 && f_value > 1.e-4) ||
		(f_value > -1.e4 && f_value < -1.e-4) || f_value == 0.0) {
    return(dm2kCvtDoubleToString(f_value,pstr_value,f_precision));
  } else {
    return(dm2kCvtDoubleToExpString(f_value,pstr_value,f_precision));
  }
}

/* Convert various integer types to ascii */

static char digit_to_ascii[10]={'0','1','2','3','4','5','6','7','8','9'};

int  dm2kCvtCharToString(
	char source,
	char *pdest)
{
    unsigned char val,temp;
    char	  digit[3];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if(source == CHAR_MIN) {
	    sprintf(pdest,"%d",CHAR_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


int  dm2kCvtUcharToString(
    unsigned char source,
    char	  *pdest)
{
    unsigned char val,temp;
    char	  digit[3];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


int  dm2kCvtShortToString(
    short source,
    char  *pdest)
{
    short val,temp;
    char  digit[6];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if(source == SHRT_MIN) {
	    sprintf(pdest,"%d",SHRT_MIN);
	    return((int)(strlen(pdest)));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


int  dm2kCvtUshortToString(
    unsigned short source,
    char	  *pdest)
{
    unsigned short val,temp;
    char	  digit[5];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


int  dm2kCvtLongToString(
    long source,
    char  *pdest)
{
    long  val,temp;
    char  digit[11];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if((unsigned long)source == (unsigned long)LONG_MIN) {
	    sprintf(pdest,"%lu",(unsigned long)LONG_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


int  dm2kCvtUlongToString(
    unsigned long source,
    char	  *pdest)
{
    unsigned long val,temp;
    char	  digit[10];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


/* Convert hex digits to ascii */

static char hex_digit_to_ascii[16]={'0','1','2','3','4','5','6','7','8','9',
		'a','b','c','d','e','f'};


int  dm2kCvtLongToHexString(
    long source,
    char  *pdest)
{
    long  val,temp;
    char  digit[10];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if((unsigned long)source == (unsigned long)LONG_MIN) {
	    sprintf(pdest,"%x",(unsigned long)LONG_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/16;
	digit[i] = hex_digit_to_ascii[val - temp*16];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


int  dm2kCvtLongToOctalString(
    long source,
    char  *pdest)
{
    long  val,temp;
    char  digit[16];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if((unsigned long)source == (unsigned long)LONG_MIN) {
	    sprintf(pdest,"%o",(unsigned long)LONG_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/8;
	/* reuse digit_to_ascii since octal is a subset of decimal */
	digit[i] = digit_to_ascii[val - temp*8];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}

/*
 *
 * dm2kCvtBitsToUlong()
 *
 * extract a bit field from the source unsigend long
 */
unsigned long  dm2kCvtBitsToUlong(
unsigned long   src,
unsigned        bitFieldOffset,
unsigned        bitFieldLength)
{
        unsigned long   mask;

        src = src >> bitFieldOffset;

        mask = (1<<bitFieldLength)-1;

        src = src & mask;

        return src;
}


/*
 *
 * dm2kCvtUlongToBits()
 *
 * insert a bit field from the source unsigend long
 * into the destination unsigned long
 */
unsigned long  dm2kCvtUlongToBits(
unsigned long   src,
unsigned long   dest,
unsigned        bitFieldOffset,
unsigned        bitFieldLength)
{
        unsigned long   mask;

        mask = (1<<bitFieldLength)-1;
        mask = mask << bitFieldOffset;
        src = src << bitFieldOffset;
        dest = (dest & ~mask) | (src & mask);

        return dest;
}

#endif     /* #ifdef DM2_CDEV */
