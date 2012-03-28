/*
*****************************************************************
                          COPYRIGHT NOTIFICATION
*****************************************************************

THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
AND IN ALL SOURCE LISTINGS OF THE CODE.

(C)  COPYRIGHT 1993 UNIVERSITY OF CHICAGO

Argonne National Laboratory (ANL), with facilities in the States of
Illinois and Idaho, is owned by the United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

Portions of this material resulted from work developed under a U.S.
Government contract and are subject to the following license:  For
a period of five years from March 30, 1993, the Government is
granted for itself and others acting on its behalf a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, and perform
publicly and display publicly.  With the approval of DOE, this
period may be renewed for two additional five year periods.
Following the expiration of this period or periods, the Government
is granted for itself and others acting on its behalf, a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, distribute copies
to the public, perform publicly and display publicly, and to permit
others to do so.

*****************************************************************
                                DISCLAIMER
*****************************************************************

NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
OWNED RIGHTS.

*****************************************************************
LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (708-252-2000).
*/
/*****************************************************************************
 *
 *     Original Author : Mark Andersion
 *     Current Author  : Frederick Vong
 *
 * Modification Log:
 * -----------------
 * .01  03-01-95        vong    2.0.0 release
 * .02  09-08-95        vong    conform to c++ syntax
 *
 *****************************************************************************
*/

#include "dm2k.h"
#define DEFAULT_CLR 14
#define DEFAULT_BCLR 3


/*
 * monitorAttribute
 */
void monitorAttributeInit(DlMonitor *monitor) 
{
  monitor->rdbk = NULL;
  monitor->clr  = DEFAULT_CLR;
  monitor->bclr = DEFAULT_BCLR;
}

void monitorAttributeDestroy(DlMonitor *monitor) 
{
  DM2KFREE(monitor->rdbk);
}

void monitorAttributeCopy(DlMonitor * to, DlMonitor * from) 
{
  if (from == NULL || to == NULL) 
    return;

  renewString(&to->rdbk, from->rdbk);
  to->clr  = from->clr;
  to->bclr = from->bclr;
}


/*
 * plotcomAttribute
 */
void plotcomAttributeInit(DlPlotcom *plotcom) 
{
  plotcom->title  = NULL;
  plotcom->xlabel = NULL;
  plotcom->ylabel = NULL;

  plotcom->clr  = DEFAULT_CLR;
  plotcom->bclr = DEFAULT_BCLR;
}

void plotcomAttributeCopy(DlPlotcom * to, DlPlotcom * from) 
{
  if (from == NULL || to == NULL) 
    return;

  renewString(&to->title, from->title);
  renewString(&to->xlabel, from->xlabel);
  renewString(&to->ylabel, from->ylabel);

  to->clr  = from->clr;
  to->bclr = from->bclr;
}

void plotcomAttributeDestroy(DlPlotcom *plotcom) 
{
  DM2KFREE(plotcom->title);
  DM2KFREE(plotcom->xlabel);
  DM2KFREE(plotcom->ylabel);
}


/*
 * plotAxisDefinition
 */
void plotAxisDefinitionInit(DlPlotAxisDefinition *axisDefinition) 
{
  axisDefinition->axisStyle  = LINEAR_AXIS;
  axisDefinition->rangeStyle = CHANNEL_RANGE;
  axisDefinition->minRange   = 0.0;
  axisDefinition->maxRange   = 1.0;
  axisDefinition->timeFormat = HHMMSS;
}

void plotAxisDefinitionCopy(DlPlotAxisDefinition * to,
			    DlPlotAxisDefinition * from) 
{
  if (from == NULL || to == NULL) 
    return;

  *to = *from;
}

void plotAxisDefinitionDestroy(DlPlotAxisDefinition *axisDefinition) 
{
  return;
}

/*
 * penAttribute
 */
void penAttributeInit(DlPen *pen)
{
  pen->chan     = NULL;
  pen->utilChan = NULL;
  pen->clr = DEFAULT_CLR;
}

void penAttributeCopy(DlPen * to, DlPen * from)
{
  if (from == NULL || to == NULL) 
    return;

  renewString(&to->chan, from->chan);
  renewString(&to->utilChan, from->utilChan);
  to->clr = from->clr;
}

void penAttributeDestroy(DlPen *pen)
{
  DM2KFREE(pen->chan);
  DM2KFREE(pen->utilChan);
  pen->clr = DEFAULT_CLR;
}


/*
 * traceAttribute
 */
void traceAttributeInit(DlTrace *trace) 
{
  trace->xdata    = NULL;
  trace->ydata    = NULL;
  trace->data_clr = DEFAULT_CLR;
}

void traceAttributeCopy(DlTrace * to, DlTrace * from)
{
  if (from == NULL || to == NULL) 
    return;

  renewString(&to->xdata, from->xdata);
  renewString(&to->ydata, from->ydata);
  to->data_clr = from->data_clr;
}

void traceAttributeDestroy(DlTrace *trace) 
{
  DM2KFREE(trace->xdata);
  DM2KFREE(trace->ydata);
}



void parseMonitor(
  DisplayInfo *displayInfo,
  DlMonitor *monitor)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"rdbk") || STREQL(token,"chan")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&(monitor->rdbk),token);
      }
      else if (STREQL(token,"clr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	monitor->clr = atoi(token) % DL_MAX_COLORS;
      }
      else if (STREQL(token,"bclr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	monitor->bclr = atoi(token) % DL_MAX_COLORS;
      }
      break;
    case T_LEFT_BRACE:
      nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
       break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );
}

void parsePlotcom(
  DisplayInfo *displayInfo,
  DlPlotcom *plotcom)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"title")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&plotcom->title,token);
      }
      else if (STREQL(token,"xlabel")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&plotcom->xlabel,token);
      } 
      else if (STREQL(token,"ylabel")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&plotcom->ylabel,token);
      } 
      else if (STREQL(token,"package")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&plotcom->package,token);
      } 
      else if (STREQL(token,"clr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	plotcom->clr = atoi(token) % DL_MAX_COLORS;
      }
      else if (STREQL(token,"bclr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	plotcom->bclr = atoi(token) % DL_MAX_COLORS;
      }
      break;
    case T_LEFT_BRACE:
      nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
       break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );
}


/***
 *** PlotAxisDefinition element in each plot type object
 ***/


void parsePlotAxisDefinition(
  DisplayInfo *displayInfo,
  DlPlotAxisDefinition *dlPlotAxisDefinition)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"axisStyle")) {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        if (STREQL(token,"linear")) 
          dlPlotAxisDefinition->axisStyle = LINEAR_AXIS;
        else
	  if (STREQL(token,"log10")) 
	    dlPlotAxisDefinition->axisStyle = LOG10_AXIS;
	  else
	    if (STREQL(token,"time"))
	      dlPlotAxisDefinition->axisStyle = TIME_AXIS;
      } 
      else if (STREQL(token,"rangeStyle")) {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        if (STREQL(token,"from channel")) 
          dlPlotAxisDefinition->rangeStyle = CHANNEL_RANGE;
        else if (STREQL(token,"user-specified")) 
          dlPlotAxisDefinition->rangeStyle = USER_SPECIFIED_RANGE;
        else if (STREQL(token,"auto-scale")) 
          dlPlotAxisDefinition->rangeStyle = AUTO_SCALE_RANGE;
      }
      else if (STREQL(token,"minRange")) {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        dlPlotAxisDefinition->minRange = atof(token);
      } 
      else if (STREQL(token,"maxRange")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlPlotAxisDefinition->maxRange = atof(token);
      } 
      else if (STREQL(token,"timeFormat")) {
        int i;
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        for (i=FIRST_CP_TIME_FORMAT;
             i<FIRST_CP_TIME_FORMAT+NUM_CP_TIME_FORMATS;i++) {
          if (STREQL(token,stringValueTable[i])) {
            dlPlotAxisDefinition->timeFormat = (CartesianPlotTimeFormat_t)i;
            break;
          }
        }
      }
      break;
    case T_LEFT_BRACE:
      nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
       break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
		&& (tokenType != T_EOF) );
}



/***
 *** pen element in each strip chart
 ***/


void parsePen(
  DisplayInfo *displayInfo,
  DlPen *pen)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"chan")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&pen->chan,token);
      } 
      else if (STREQL(token,"utilChan")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&pen->utilChan,token);
      } 
      else if (STREQL(token,"clr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	pen->clr = atoi(token) % DL_MAX_COLORS;
      }
      break;
    case T_LEFT_BRACE:
      nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
       break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );
}



/***
 *** trace element in each cartesian plot
 ***/


void parseTrace(
  DisplayInfo *displayInfo,
  DlTrace *trace)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"xdata")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&trace->xdata,token);
      }
      else if (STREQL(token,"ydata")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&trace->ydata,token);
      }
      else if (STREQL(token,"data_clr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	trace->data_clr = atoi(token) % DL_MAX_COLORS;
      }
      break;
    case T_LEFT_BRACE:
      nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
       break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );
}

void writeDlMonitor(
		    FILE *stream,
  DlMonitor *dlMonitor,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);


  for (i = 0;  i < MIN(level,256-2); i++) 
    indent[i] = '\t';
  indent[i] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%smonitor {",indent);
    if ((dlMonitor->rdbk != NULL) && dlMonitor->rdbk[0])
      fprintf(stream,"\n%s\tchan=\"%s\"",indent,dlMonitor->rdbk);
    
    fprintf(stream,"\n%s\tclr=%d",indent,dlMonitor->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlMonitor->bclr);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%smonitor {",indent);
    fprintf(stream,"\n%s\trdbk=\"%s\"",
	    indent,dlMonitor->rdbk ? dlMonitor->rdbk : "");
    fprintf(stream,"\n%s\tclr=%d",indent,dlMonitor->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlMonitor->bclr);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

void writeDlPlotcom(
  FILE *stream,
  DlPlotcom *dlPlotcom,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

  for (i = 0;  i < MIN(level,256-2); i++) 
    indent[i] = '\t';
  indent[i] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%splotcom {",indent);
    if (dlPlotcom->title != NULL)
      fprintf(stream,"\n%s\ttitle=\"%s\"",indent,dlPlotcom->title);

    if (dlPlotcom->xlabel != NULL)
      fprintf(stream,"\n%s\txlabel=\"%s\"",indent,dlPlotcom->xlabel);

    if (dlPlotcom->ylabel != NULL)
      fprintf(stream,"\n%s\tylabel=\"%s\"",indent,dlPlotcom->ylabel);

    fprintf(stream,"\n%s\tclr=%d",indent,dlPlotcom->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlPlotcom->bclr);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%splotcom {",indent);
    fprintf(stream,"\n%s\ttitle=\"%s\"",indent,CARE_PRINT(dlPlotcom->title));
    fprintf(stream,"\n%s\txlabel=\"%s\"",indent,CARE_PRINT(dlPlotcom->xlabel));
    fprintf(stream,"\n%s\tylabel=\"%s\"",indent,CARE_PRINT(dlPlotcom->ylabel));
    fprintf(stream,"\n%s\tclr=%d",indent,dlPlotcom->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlPlotcom->bclr);
    fprintf(stream,"\n%s\tpackage=\"%s\"",indent,dlPlotcom->package);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}


void writeDlPen(
  FILE *stream,
  DlPen *dlPen,
  int index,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    if ( dlPen->chan == NULL || dlPen->chan[0] == 0 ) return;
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } 
#endif

  for (i = 0;  i < MIN(level,256-2); i++)
    indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%spen[%d] {",indent,index);
  fprintf(stream,"\n%s\tchan=\"%s\"",indent,dlPen->chan);
  fprintf(stream,"\n%s\tutilChan=\"%s\"",indent,dlPen->utilChan);
  fprintf(stream,"\n%s\tclr=%d",indent,dlPen->clr);
  fprintf(stream,"\n%s}",indent);
}


void writeDlTrace(
  FILE *stream,
  DlTrace *dlTrace,
  int index,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  if ((dlTrace->xdata == NULL) && (dlTrace->ydata == NULL)) return;
#ifdef SUPPORT_0201XX_FILE_FORMAT
  }
#endif

  for (i = 0;  i < MIN(level, 256-2); i++) 
    indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%strace[%d] {",indent,index);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  if (dlTrace->xdata != NULL)
    fprintf(stream,"\n%s\txdata=\"%s\"",indent,dlTrace->xdata);
  if (dlTrace->ydata != NULL)
    fprintf(stream,"\n%s\tydata=\"%s\"",indent,dlTrace->ydata);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\txdata=\"%s\"",indent,CARE_PRINT(dlTrace->xdata));
    fprintf(stream,"\n%s\tydata=\"%s\"",indent,CARE_PRINT(dlTrace->ydata));
  }
#endif
  fprintf(stream,"\n%s\tdata_clr=%d",indent,dlTrace->data_clr);
  fprintf(stream,"\n%s}",indent);
}



void writeDlPlotAxisDefinition(
  FILE *stream,
  DlPlotAxisDefinition *dlPlotAxisDefinition,
  int axisNumber,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  if ((dlPlotAxisDefinition->axisStyle == LINEAR_AXIS) &&
      (dlPlotAxisDefinition->rangeStyle == CHANNEL_RANGE) &&
      (dlPlotAxisDefinition->minRange == 0.0) &&
      (dlPlotAxisDefinition->maxRange == 1.0)) return;
#ifdef SUPPORT_0201XX_FILE_FORMAT
  }
#endif

  for (i = 0;  i < MIN(level, 256-2); i++) 
    indent[i] = '\t';
  indent[i] = '\0';

  switch (axisNumber) {
    case X_AXIS_ELEMENT: fprintf(stream,"\n%sx_axis {",indent); break;
    case Y1_AXIS_ELEMENT: fprintf(stream,"\n%sy1_axis {",indent); break;
    case Y2_AXIS_ELEMENT: fprintf(stream,"\n%sy2_axis {",indent); break;
  }
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  if (dlPlotAxisDefinition->axisStyle != LINEAR_AXIS)
    fprintf(stream,"\n%s\taxisStyle=\"%s\"",indent,
			stringValueTable[dlPlotAxisDefinition->axisStyle]);

  if (dlPlotAxisDefinition->rangeStyle != CHANNEL_RANGE)
    fprintf(stream,"\n%s\trangeStyle=\"%s\"",indent,
			stringValueTable[dlPlotAxisDefinition->rangeStyle]);
  if (dlPlotAxisDefinition->minRange != 0.0)
    fprintf(stream,"\n%s\tminRange=%f",indent,dlPlotAxisDefinition->minRange);

  if (dlPlotAxisDefinition->maxRange != 1.0)
    fprintf(stream,"\n%s\tmaxRange=%f",indent,dlPlotAxisDefinition->maxRange);

  if (dlPlotAxisDefinition->timeFormat != HHMMSS) {
    fprintf(stream,"\n%s\ttimeFormat=\"%s\"",indent,
            stringValueTable[dlPlotAxisDefinition->timeFormat]);

  }
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\taxisStyle=\"%s\"",indent,
  		stringValueTable[dlPlotAxisDefinition->axisStyle]);
    fprintf(stream,"\n%s\trangeStyle=\"%s\"",indent,
  		stringValueTable[dlPlotAxisDefinition->rangeStyle]);
    fprintf(stream,"\n%s\tminRange=%f",indent,dlPlotAxisDefinition->minRange);
    fprintf(stream,"\n%s\tmaxRange=%f",indent,dlPlotAxisDefinition->maxRange);
  }
#endif
  fprintf(stream,"\n%s}",indent);
}

