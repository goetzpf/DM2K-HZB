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
 * .02  09-05-95        vong    2.1.0 release
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien      Add drawingAreaDefineCursor in display.c
 *                              and other routines for sensitivity
 *
 *****************************************************************************
*/

#ifndef __PROTO_H__
#define __PROTO_H__

#include "faceplateBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/* actions.c */
void StartDrag(Widget w, XEvent *event);

/* callbacks.c */
void cleanObjectID (void);
void dmDisplayListOk(Widget, XtPointer, XtPointer);
void executePopupMenuCallback(Widget, XtPointer, XtPointer);
void dmCreateRelatedDisplay(Widget, XtPointer, XtPointer);
void dmExecuteShellCommand(Widget w,
			   DlShellCommandEntry *commandEntry,
			   XmPushButtonCallbackStruct *call_data);
void drawingAreaCallback(Widget w, DisplayInfo *displayInfo,
			 XmDrawingAreaCallbackStruct *call_data);
void relatedDisplayMenuButtonDestroy(Widget, XtPointer, XtPointer);
void warnCallback(Widget, XtPointer, XtPointer);
void exitCallback(Widget, XtPointer, XtPointer);
void simpleRadioBoxCallback(Widget w, int buttonNumber,
			    XmToggleButtonCallbackStruct *call_data);
void valuatorValueChanged(Widget, XtPointer, XtPointer);
void createNewDisplay(DisplayInfo * displayInfo,
		      char        * adlFilename,
		      char        * argsString,
		      int           replaceCurrentDisplay);

void associatedMenuCB(Widget , XtPointer , XtPointer);
void genericObjectInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);
void freeUserDataCB(Widget, XtPointer , XtPointer);
Boolean objectInfoImplemented (DlElementType);

/* channelPalette.c */
void createChannel(void);

/* colorPalette.c */
void createColor(void);
void setCurrentDisplayColorsInColorPalette(int rcType, int index);

/* createControllers.c */
DlElement *createDlChoiceButton(DlElement *);
DlElement *createDlMenu(DlElement *);
DlElement *createDlMessageButton(DlElement *);
DlElement *createDlTextEntry(DlElement *);
DlElement *createDlValuator(DlElement *);

/* dm2kControl.c */
void sensitiveControllerInfo (char *, DlControl *, Record *,
			      DlSensitive *, Record *);
void controlAttributeInit(DlControl *control);
void sensitveAttributeInit ( DlSensitive *);
void parseSensitive ( DisplayInfo *, DlSensitive *);
void writeDlSensitive ( FILE *, DlSensitive *, int);
Record *sensitiveCreateRecord ( DlSensitive *, void (*)(XtPointer),
				void (*)(XtPointer), XtPointer, Boolean *);
void sensitiveDestroyRecord ( Record **, Boolean *);
void sensitiveSetWidget ( Widget, DlSensitive *, Record *, UpdateTask *);
void sensitiveInfo ( char *, DlSensitive *, Record *);
void controlHandler ( DisplayInfo *, DlElement *);
void sensitiveControllerInfo (char *, DlControl *, Record *, DlSensitive *, Record *);
void sensitiveControllerInfoSimple (char *, DlControl *, DlSensitive *);
void editObjectHandler ( DisplayInfo *, DlElement *);
void controlAttributeDestroy(DlControl *control);
void sensitveAttributeDestroy (DlSensitive *sensitive);
void controlAttributeCopy (DlControl * to, DlControl * from);
void sensitveAttributeCopy ( DlSensitive * to, DlSensitive * from);

/* createExtensions.c */
DlElement *createDlImage(DlElement *);
DlElement *createDlComposite(DlElement *);
DlElement *handleImageCreate();
DlElement *createDlPolyline(DlElement *);
DlElement *createDlPolygon(DlElement *);
DlElement *handlePolylineCreate(int x0, int y0, Boolean simpleLine);
DlElement *handlePolygonCreate(int x0, int y0);

/* createMonitors.c */
DlElement *createDlMeter(DlElement *);
DlElement *createDlBar(DlElement *);
DlElement *createDlByte(DlElement *);
DlElement *createDlIndicator(DlElement *);
DlElement *createDlTextUpdate(DlElement *);
DlElement *createDlStripChart(DlElement *);
DlElement *createDlCartesianPlot(DlElement *);
void monitorAttributeInit(DlMonitor *monitor);
void traceAttributeInit(DlTrace *trace);
void plotAxisDefinitionInit(DlPlotAxisDefinition *axisDefinition);

/* createStatics.c */
DlElement* createDlElement(DlElementType, XtPointer, DlDispatchTable *);
DlFile *createDlFile(DisplayInfo *displayInfo);
DlElement *createDlDisplay(DlElement *);
DlColormap *createDlColormap(DisplayInfo *displayInfo);
DlElement *createDlRectangle(DlElement *);
DlElement *createDlOval(DlElement *);
DlElement *createDlArc(DlElement *);
DlElement *createDlText(DlElement *);
DlElement *createDlRelatedDisplay(DlElement *);
DlElement *createDlShellCommand(DlElement *);
void createDlObject(DisplayInfo *displayInfo, DlObject *object);
DlElement *handleTextCreate(int x0, int y0);
void objectAttributeInit(DlObject *object);
void basicAttributeInit(DlBasicAttribute *attr);
void dynamicAttributeInit(DlDynamicAttribute *dynAttr);

/* display.c */
void parseDisplayType (DisplayInfo *, DlDisplay *, char *);
char * displayTypeMsg (DisplayType);
void handleShellStructureNotify (Widget widget, XtPointer,  
				 XEvent *,  Boolean *);
void getXYDisplay (DisplayInfo *, Position *, Position *);
void mouseButtonHelpCB (Widget , XtPointer , XtPointer);
void dumpDisplayObjects (DisplayInfo *, char *);
void dumpDisplayInfoList ();
void drawingAreaDefineCursor (DisplayInfo *displayInfo);
DisplayInfo *createDisplay(void);
void positionDisplayRead (DisplayInfo *);
void rebuildDisplayShell (DisplayInfo *, DisplayType );
void displayObjectInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);
void positionDisplay (DisplayInfo *, Boolean);
void positionDisplayRead (DisplayInfo *);
void destroyRebuildDisplayShell (DisplayInfo *, DisplayType);

/* dmInit.c */
DisplayInfo *allocateDisplayInfo(void);
DisplayInfo *createInitializedDisplayInfo(void);
void createDisplayShell (DisplayInfo *);
void dmDisplayListParse(DisplayInfo *, FILE *, char *, char *, char*, Boolean);
DisplayInfo *dmDisplayListParse2(DisplayInfo *,
				 DisplayInfo *, FILE *, char *, char *);
TOKEN parseAndAppendDisplayList(DisplayInfo *, DlList *);

/* eventHandlers.c */
char ** mouseButtonUsageMsg (DlTraversalMode, ActionType, char *, int);
void createPopupDisplayMenu (DlTraversalMode, DisplayInfo *, Widget);
void motionHandler (Widget, XtPointer, XEvent*, Boolean*);
int initEventHandlers(void);
void popup (Widget , XEvent *);
void popupMenu(Widget, XtPointer, XEvent *, Boolean *);
void popdownMenu(Widget, XtPointer, XEvent *, Boolean *);
void handleEnterWindow(Widget, XtPointer, XEvent *, Boolean *);
void handleButtonPress(Widget, XtPointer, XEvent *, Boolean *);
void handleRuntimeButtonPress(Widget, XtPointer, XEvent *, Boolean *);
void highlightSelectedElements(void);
void unhighlightSelectedElements(void);
void unselectSelectedElements(void);
void highlightAndAppendSelectedElements(DlList *);
Boolean unhighlightAndUnselectElement(DlElement *element, int *numSelected);
void moveCompositeChildren(DisplayInfo *cdi, DlElement *element,
	int xOffset, int yOffset, Boolean moveWidgets);
void updateDraggedElements(Position x0, Position y0, Position x1, Position y1);
void updateResizedElements(Position x0, Position y0, Position x1, Position y1);
void handleEnterObject (Widget, XtPointer, XEvent *, Boolean *);
void handleLeaveObject (Widget, XtPointer, XEvent *, Boolean *);

/* executeControllers.c */
int textFieldFontListIndex(int height);
int messageButtonFontListIndex(int height);
int menuFontListIndex(int height);
int valuatorFontListIndex(DlValuator *dlValuator);
void executeDlChoiceButton(DisplayInfo *, DlElement *);
void executeDlMessageButton(DisplayInfo *, DlElement *);
void executeDlValuator(DisplayInfo *, DlElement *);
void executeDlTextEntry(DisplayInfo *, DlElement *);
void executeDlMenu(DisplayInfo *, DlElement *);

/* executeExtensions.c */
void executeDlImage(DisplayInfo *, DlElement *);
void executeDlPolyline(DisplayInfo *, DlElement *);
void executeDlPolygon(DisplayInfo *, DlElement *);

/* executeMonitors.c */
void executeDlMeter(DisplayInfo *, DlElement *);
void executeDlBar(DisplayInfo *, DlElement *);
void executeDlByte(DisplayInfo *, DlElement *);
void executeDlIndicator(DisplayInfo *, DlElement *);
void executeDlTextUpdate(DisplayInfo *, DlElement *);
void executeDlStripChart(DisplayInfo *, DlElement *);
void executeDlCartesianPlot(DisplayInfo *, DlElement *);
void executeDlSurfacePlot(DisplayInfo *, DlElement *);

/* help.c */
void errMsgDlgCreateDlg(int);
void globalHelpCallback(Widget, XtPointer, XtPointer);
void dm2kPostMsg(char *);
void dm2kPostTime();
void dm2kPrintf(char*,...);
void dm2kCreateCAStudyDlg();
void dm2kStartUpdateCAStudyDlg();

/* dm2k.c */
void openFaceplate( const char *fname );
void printerSetup(Widget);
void enableCtrlObjMenu (Boolean, DlElementType);
Widget createDisplayMenu(Widget widget);
Widget buildMenu(Widget,int,char*,char,Boolean,menuEntry_t*);
void dm2kExit();
Boolean dm2kSaveDisplay(DisplayInfo *, char *, Boolean);
void enableEditFunctions();
void disableEditFunctions();
int getUserChoiseViaPopupQuestionDialog (Widget , ...);
void cleanDisplayModeEdit ();
void mouseButtonHelp ();


/* dm2kCA.c */
void displayObjectInfo (char *, Widget, DisplayInfo *, 
			DlElement *, XtPointer);
int  dm2kCAInitialize(void);
void dm2kChanelInfo (char *, Record *, char *);
void dm2kCATerminate(void);
void updateListCreate(Channel *);
void updateListDestroy(Channel *);
void dm2kConnectEventCb(struct connection_handler_args);
void dm2kDisconnectChannel(Channel *pCh);
Record *dm2kAllocateRecord(char*,void(*)(XtPointer),void(*)(XtPointer),XtPointer);
void dm2kDestoryRecord(Record *);
void dm2kSendDouble(Record *, double);
void dm2kSendString(Record *, char *);
void dm2kSendCharacterArray(Record *, char *, unsigned long);
void CATaskGetInfo(int *, int *, int *);
void caPendEvent (char *);

/* dm2kPixmap.c */
void dm2kInitializeImageCache(void);
void dm2kClearImageCache(void);

/* dm2kValuator.c */
void popupValuatorKeyboardEntry(Widget, DisplayInfo *, XEvent *);

/* objectPalette.c */
void invalidObjectWarning (DisplayInfo *, DlElementType );
Boolean checkActiveObject (DlElement *);
void objectPaletteEnterWindow (Widget, XEvent* , String*, Cardinal*);
void objectPaletteLeaveWindow (Widget, XEvent* , String*, Cardinal*);
void enableControllerMenu (DisplayInfo *);
void setResourcePaletteEntriesIfVisible (DlElement *);
void updateLabelObjectINF (DlElement *, char *);
void createObject(void);
void clearResourcePaletteEntries(void);
void objectMenuCallback(Widget,XtPointer,XtPointer);
void objectPaletteSetSensitivity(Boolean);
void setActionToSelect(void);
void setResourcePaletteEntries(void);
void updateGlobalResourceBundleFromElement(DlElement *element);
void updateGlobalResourceBundleAndResourcePalette(Boolean objectDataOnly);
void updateElementFromGlobalResourceBundle(DlElement *elementPtr);
void updateElementObjectAttribute(ResourceBundle *, DlObject *);
void updateElementControlAttribute(ResourceBundle *, DlControl *);
void updateElementSensitiveAttribute(ResourceBundle *, DlSensitive *);
void updateGlobalResourceBundleDisplayPosition (void);
Boolean checkControllerObjectType (DisplayInfo *, DlElementType, 
				   MessageButtonType);
void resetPaletteButton (void);
void invalidObjectWarning (DisplayInfo *, DlElementType );
void displayLeaveWindow (Widget);
Boolean checkControllerObject (DisplayInfo *, DlElement *);

/* static (local functions) ----
void resetGlobalResourceBundleAndResourcePalette(void);
void updateElementBasicAttribute(ResourceBundle *, DlBasicAttribute *);
void updateElementDynamicAttribute(ResourceBundle *, DlDynamicAttribute *);
void updateElementMonitorAttribute(ResourceBundle *, DlMonitor *);
---- */
void enableControllerRC (DisplayInfo *);

/* parseControllers.c */
DlElement *parseChoiceButton(DisplayInfo *); 
DlElement *parseMessageButton(DisplayInfo *);
DlElement *parseValuator(DisplayInfo *);
DlElement *parseTextEntry(DisplayInfo *);
DlElement *parseMenu(DisplayInfo *);
void parseControl(DisplayInfo *, DlControl *);
void parseSensitive( DisplayInfo *, DlSensitive *);

/* parseExtensions.c */
DlElement *parseImage(DisplayInfo *);
DlElement *parseComposite(DisplayInfo *);
DlElement *parsePolyline(DisplayInfo *);
DlElement *parsePolygon(DisplayInfo *);

/* parseMonitors.c */
DlElement *parseMeter(DisplayInfo *);
DlElement *parseBar(DisplayInfo *);
DlElement *parseByte(DisplayInfo *);
DlElement *parseIndicator(DisplayInfo *);
DlElement *parseTextUpdate(DisplayInfo *);
DlElement *parseStripChart(DisplayInfo *);
DlElement *parseCartesianPlot(DisplayInfo *);
void parseMonitor(DisplayInfo *displayInfo, DlMonitor *monitor);
void parsePlotcom(DisplayInfo *displayInfo, DlPlotcom *plotcom);
void parsePen(DisplayInfo *displayInfo, DlPen *pen);
void parseTrace(DisplayInfo *displayInfo, DlTrace *trace);
void parsePlotAxisDefinition(DisplayInfo *displayInfo,
	DlPlotAxisDefinition *axisDefinition);

/* parseStatics.c */
DlFile *parseFile(DisplayInfo *displayInfo);
DlElement *parseDisplay(DisplayInfo *displayInfo);
DlColormap *parseColormap(DisplayInfo *displayInfo, FILE *filePtr);
void parseBasicAttribute(DisplayInfo *, DlBasicAttribute *);
void parseDynamicAttribute(DisplayInfo *, DlDynamicAttribute *);
void parseOldBasicAttribute(DisplayInfo *, DlBasicAttribute *);
void parseOldDynamicAttribute(DisplayInfo *, DlDynamicAttribute *);
DlElement *parseRectangle(DisplayInfo *);
DlElement *parseOval(DisplayInfo *);
DlElement *parseArc(DisplayInfo *);
DlElement *parseText(DisplayInfo *);
DlElement *parseRisingLine(DisplayInfo *);
DlElement *parseFallingLine(DisplayInfo *);
DlElement *parseRelatedDisplay(DisplayInfo *);
DlElement * parseShellCommand(DisplayInfo *);
void parseDlColor(DisplayInfo *displayInfo, FILE *filePtr,
	DlColormapEntry *dlColor);
void parseObject(DisplayInfo *displayInfo, DlObject *object);
void parseRelatedDisplayEntry(DisplayInfo *displayInfo,
	DlRelatedDisplayEntry *relatedDisplay);
void parseShellCommandEntry(DisplayInfo *displayInfo,
	DlShellCommandEntry *shellCommand);
DlColormap *parseAndExtractExternalColormap(DisplayInfo *displayInfo,
	char *filename);
TOKEN getToken(DisplayInfo *displayInfo, char *word);

/* resourcePalette.c */
void initializeXmStringValueTables(void);
void initializeXmStringValueTables (void);
void initializeGlobalResourceBundle(void);
void createResource(void);
void textFieldNumericVerifyCallback(Widget, XtPointer, XtPointer);
void textFieldFloatVerifyCallback(Widget, XtPointer, XtPointer);
void textFieldActivateCallback(Widget w, XtPointer, XtPointer);
Widget createRelatedDisplayDataDialog(Widget parent);
void updateRelatedDisplayDataDialog(void);
Widget createShellCommandDataDialog(Widget parent);
void updateShellCommandDataDialog(void);
void cpEnterCellCallback(Widget w, XtPointer, XtPointer);
void cpUpdateMatrixColors(void);
Widget createCartesianPlotDataDialog(Widget parent);
void updateCartesianPlotDataDialog(void);
Widget createCartesianPlotAxisDialog(Widget parent);
void updateCartesianPlotAxisDialog(void);
void updateCartesianPlotAxisDialogFromWidget(Widget cp);
void scEnterCellCallback(Widget w, XtPointer, XtPointer);
void scUpdateMatrixColors(void);
Widget createStripChartDataDialog(Widget parent);
void updateStripChartDataDialog(void);
void dm2kGetValues(ResourceBundle *pRB, ...);

/* shared.c */
void updateTaskRepaintRectangle (DisplayInfo *displayInfo, 
				 int         x0, 
				 int         y0, 
				 int         x1, 
				 int         y1);

void wmCloseCallback(Widget, XtPointer, XtPointer);
XtCallbackProc wmTakeFocusCallback(Widget w, ShellType shellType,
				   XmAnyCallbackStruct *call_data);
void updateStatusFields(void);
void optionMenuSet(Widget menu, int buttonId);
double dm2kTime();
void updateTaskInit(DisplayInfo *displayInfo);
UpdateTask *updateTaskAddTask(DisplayInfo *, DlObject *, void (*)(XtPointer), XtPointer);
void updateTaskDeleteTask(UpdateTask *);
void updateTaskDeleteAllTask(UpdateTask *);
void updateTaskSetScanRate(UpdateTask *, double);
void updateTaskAddExecuteCb(UpdateTask *, void (*)(XtPointer));
void updateTaskAddDestroyCb(UpdateTask *, void (*)(XtPointer));
void updateTaskMarkUpdate(UpdateTask *pt);
void updateTaskRepaintRegion(DisplayInfo *, Region *);
Boolean dm2kInitSharedDotC(void);
void updateTaskStatusGetInfo(int *taskCount,
                             int *periodicTaskCount,
                             int *updateRequestCount,
                             int *updateDiscardCount,
                             int *periodicUpdateRequestCount,
                             int *periodicUpdateDiscardCount,
                             int *updateRequestQueued,
                             int *updateExecuted,
                             double *timeInterval); 
void updateTaskAddNameCb(UpdateTask *, void (*)(XtPointer, char **, short *, int *));
void optionMenuSensitive (Widget , int , Boolean );



/* updateMonitors.c */
void localCvtDoubleToString( double, char *, unsigned short);
void localCvtDoubleToExpNotationString(double, char *, unsigned short);

void traverseMonitorList(Boolean forcedTraversal, DisplayInfo *displayInfo,
	int regionX, int regionY, unsigned int regionWidth,
	unsigned int regionHeight);
void updateTextUpdate(UpdateTask *);
void draw3DQuestionMark(UpdateTask *);
void draw3DQuestionMark2(Display *, GC, Pixmap, Widget, 
			 int, int, int, int, Pixel);
void draw3DPane(UpdateTask *, Pixel);
void drawReadOnlySymbol(UpdateTask *);
void drawWhiteRectangle(UpdateTask *);

/* utils.c */
DisplayInfo * lookupIdenticalDisplayInfo2 (DisplayInfo *, DisplayInfo *);
int getAllMacros(const char * fileName, char *** result);
char * dm2kMalloc(size_t size);
void invalidObjectMessage (char *, DisplayInfo *, DlElementType );
Boolean displayAlreadySelected (DlElement *);
void freeDisplayXResources (DisplayInfo *);
void deallocateAllDlElements(DlList *l);
int localCvtLongToHexString(long source, char *pdest);
FILE *dmOpenUseableFile(const char *filename);
Boolean extractStringBetweenColons(char *input, char *output, int startPos,
	int  *endPos);
void dmRemoveDisplayList(DisplayInfo *displayInfo);
void dmCleanupDisplayInfo(DisplayInfo *displayInfo, Boolean cleanupDisplayList);
void dmRemoveDisplayInfo(DisplayInfo *displayInfo);
void dmRemoveAllDisplayInfo(void);
void dmTraverseDisplayList(DisplayInfo *displayInfo);
void dmTraverseAllDisplayLists(void);
void dmTraverseNonWidgetsInDisplayList(DisplayInfo *displayInfo);
int dmGetBestFontWithInfo(XFontStruct **fontTable, int nFonts, char *text,
	int h, int w, int *usedH, int *usedW, Boolean textWidthFlag);
void dmSetAndPopupWarningDialog(DisplayInfo *displayInfo,
                                 char        *message,
                                 char        *okBtnLabel,
                                 char        *cancelBtnLabel,
                                 char        *helpBtnLabel);
void dmSetAndPopupQuestionDialog(DisplayInfo *displayInfo,
                                 char        *message,
                                 char        *okBtnLabel,
                                 char        *cancelBtnLabel,
                                 char        *helpBtnLabel);
XtErrorHandler trapExtraneousWarningsHandler(String message);
DisplayInfo *dmGetDisplayInfoFromWidget(Widget widget);
void dmWriteDisplayList(DisplayInfo *displayInfo, FILE *stream);
void dmSetDisplayFileName(DisplayInfo *displayInfo, char *filename);
DlElement *lookupElement(DlList *, Position, Position, Boolean);
DlElement *lookupCompositeChild(DlElement *, Position, Position);
void selectedElementsLookup(DlList *, Position, Position,
			    Position, Position, DlList *, Boolean);
DlElement *lookupCompositeElement(DlElement *elementPtr);
DlElement *lookupDynamicAttributeElement(DlElement *elementPtr);
DlElement *lookupBasicAttributeElement(DlElement *elementPtr);
Boolean dmResizeDisplayList(DisplayInfo *displayInfo, Dimension newWidth,
	Dimension newHeight);
Boolean dmResizeSelectedElements(DisplayInfo *displayInfo, Dimension newWidth,
	Dimension newHeight);
void initializeRubberbanding(void);
void doRubberbanding(Window window, Position *initialX, Position *initialY,
	Position *finalX, Position *finalY);
Boolean doDragging(Window window, Dimension daWidth, Dimension daHeight,
	Position initialX,Position initialY,Position *finalX,Position *finalY);
DisplayInfo *doPasting(Position *displayX, Position *displayY, int *offsetX,
	int *offsetY);
Boolean alreadySelected(DlElement *element);
Boolean doResizing(Window window, Position initialX, Position initialY, 
	Position *finalX, Position *finalY);
Widget lookupElementWidget(DisplayInfo *displayInfo, DlObject *object);
void destroyElementWidget(DisplayInfo *displayInfo, Widget widget);
void clearClipboard(void);
void copySelectedElementsIntoClipboard(void);
DlStructurePtr createCopyOfElementType(DlElementType type, DlStructurePtr ptr);
void copyElementsIntoDisplay(void);
void deleteElementsInDisplay(void);
void unselectElementsInDisplay(void);
void selectAllElementsInDisplay(void);
void lowerSelectedElements(DisplayInfo *);
void ungroupSelectedElements(void);
void raiseSelectedElements(DisplayInfo *);
void alignSelectedElements(int alignment);
void moveElementAfter(DlElement *dst, DlElement *src, DlElement **tail);
void moveSelectedElementsAfterElement(DisplayInfo *displayInfo,
	DlElement *afterThisElement);
void deleteAndFreeElementAndStructure(DisplayInfo *displayInfo, DlElement *ele);
UpdateTask *getUpdateTaskFromWidget(Widget sourceWidget);
UpdateTask *getUpdateTaskFromPosition(DisplayInfo *displayInfo, int x, int y);
NameValueTable *generateNameValueTable(const char *argsString, int *numNameValues);
char *lookupNameValue(NameValueTable *nameValueTable, int numEntries,
	char *name);
void freeNameValueTable(NameValueTable *nameValueTable, int numEntries);
void performMacroSubstitutions(DisplayInfo *displayInfo,
        char *inputString, char *outputString, int sizeOfOutputString);
void colorMenuBar(Widget widget, Pixel fg, Pixel bg);
void dm2kSetDisplayTitle(DisplayInfo *displayInfo);
void dm2kMarkDisplayBeingEdited(DisplayInfo *displayInfo);
void closeDisplay(Widget);
void destroyDlDisplayList(DlList *);
DisplayInfo * choiseDisplayInfo (Cursor );

Pixel extractColor(DisplayInfo * displayInfo, 
		   double        value, 
		   ColorRule   * colorRule,
		   int           defaultColor);

void resizeDlElementList(
  DlList *dlElementList,
  int x,
  int y,
  float scaleX,
  float scaleY);

void resizeDlElementReferenceList(
  DlList *dlElementList,
  int x,
  int y,
  float scaleX,
  float scaleY);

void shiftDlElementList(DlList * dlElementList, int x, int y);
DisplayInfo * lookupIdenticalDisplayInfo (DisplayInfo * displayInfo);
char * dm2kStrdup(const char * a);
void renewString(char ** to, char * from);
size_t dm2kStrlen(const char * a);


/* dm2kWidget.c */
void dm2kInit(char *displayFontName);
void dmTerminateX(void);
unsigned long getPixelFromColormapByString(Display *display, int screen,
			Colormap cmap, char *colorString);
int initDm2kWidget();


/* writeControllers.c */
void writeDlChoiceButton(FILE *, DlElement *, int);
void writeDlMessageButton(FILE *, DlElement *, int);
void writeDlValuator(FILE *, DlElement *, int);
void writeDlTextEntry(FILE *, DlElement *, int);
void writeDlMenu(FILE *, DlElement *, int);
void writeDlControl(FILE *, DlControl *, int);
void writeDlSensitive(FILE *, DlSensitive *, int);

/* writeExtensions.c */
void writeDlImage(FILE *, DlElement *, int);
void writeDlComposite(FILE *, DlElement *, int);
void writeDlPolyline(FILE *, DlElement *, int);
void writeDlPolygon(FILE *, DlElement *, int);

/* writeMonitors.c */
void writeDlMeter(FILE *, DlElement *, int);
void writeDlBar(FILE *, DlElement *, int);
void writeDlByte(FILE *, DlElement *, int);
void writeDlIndicator(FILE *, DlElement *, int);
void writeDlTextUpdate(FILE *, DlElement *, int);
void writeDlStripChart(FILE *, DlElement *, int);
void writeDlCartesianPlot(FILE *, DlElement *, int);
void writeDlSurfacePlot(FILE *, DlElement *, int);
void writeDlMonitor(FILE *, DlMonitor *, int);
void writeDlPlotcom(FILE *, DlPlotcom *, int);
void writeDlPen(FILE *, DlPen *, int, int);
void writeDlTrace(FILE *, DlTrace *, int, int);
void writeDlPlotAxisDefinition(FILE *, DlPlotAxisDefinition *, int, int);

/* writeStatics.c */
void writeDlComposite(FILE *, DlElement *, int);
void writeDlFile(FILE *, DlFile *, int);
void writeDlDisplay(FILE *, DlElement *, int);
void writeDlColormap(FILE *, DlColormap *, int);
void writeDlBasicAttribute(FILE *, DlBasicAttribute *, int);
void writeDlDynamicAttribute(FILE *, DlDynamicAttribute *, unsigned long, int);
void writeDlRectangle(FILE *, DlElement *, int);
void writeDlOval(FILE *, DlElement *, int);
void writeDlArc(FILE *, DlElement *, int);
void writeDlText(FILE *, DlElement *, int);
void writeDlRelatedDisplay(FILE *, DlElement *, int);
void writeDlShellCommand(FILE *, DlElement *, int);
void writeDlColormapEntry(FILE *, DlElement *, int);
void writeDlObject(FILE *, DlObject *, int);
void writeDlRelatedDisplayEntry(FILE *, DlRelatedDisplayEntry *, int, int);
void writeDlShellCommandEntry(FILE *, DlShellCommandEntry *, int, int);

/* xgif.c */
Boolean initializeGIF(DisplayInfo *displayInfo, DlImage *dlImage);
void drawGIF(DisplayInfo *displayInfo, DlImage *dlImage);
void resizeGIF(DisplayInfo *displayInfo, DlImage *dlImage);
Boolean loadGIF(DisplayInfo *displayInfo, DlImage *dlImage);
void freeGIF(DisplayInfo *displayInfo, DlImage *dlImage);

/* dm2kComposite.c */
DlElement *groupObjects(DisplayInfo *);

/* dm2kCommon.c */
void freeDlElement(DlElement *dlElement);
ColorRule * getColorRuleByName(char * name);
GraphicRule * getGraphicRuleByName(char * name);
Boolean getBooleanByName(char * name);
void readColorRulesFromFile(char * filename);
void readGraphicRulesFromFile(char * filename);
int initDm2kCommon(void);
void objectAttributeSet(DlObject *object, int x, int y, unsigned int width,
        unsigned int height);
void objectAttributeCopy(DlObject * to, DlObject * from) ;
void objectAttributeDestroy(DlObject * object) ;
void dynamicAttributeDestroy(DlDynamicAttribute *dynAttr) ;
void dynamicAttributeCopy(DlDynamicAttribute * to,
			  DlDynamicAttribute * from) ;

void executeDlColormap(DisplayInfo *displayInfo, DlColormap *dlColormap);
void executeDlBasicAttribute(DisplayInfo *displayInfo, DlBasicAttribute *attr);

void basicAttributeCopy(DlBasicAttribute * to, DlBasicAttribute * from );
void basicAttributeDestroy(DlBasicAttribute * attr);

void appendDlElement(DlList *tail, DlElement *p);
DlList *createDlList();
void emptyDlList(DlList *);
void appendDlList(DlList *, DlList *);
void insertDlElement(DlList *,DlElement *);
void insertAfter(DlList *l, DlElement *p1, DlElement *p2);
void insertDlListAfter(DlList *l1, DlElement *p, DlList *l2);
void removeDlElement(DlList *,DlElement *);
void dumpDlElementList(DlList *l);
void genericMove(DlElement *, int, int);
void genericScale(DlElement *, int, int);

void destroyObjectAttribute(DlObject * object);
void destroyBasicAttributeInit(DlBasicAttribute * attr);
void destroyDynamicAttribute(DlDynamicAttribute * dynAttr);

Widget fillFileSelBox(Widget openFSD) ;
void fileMenuDialogCallback( Widget w, XtPointer clientData, XtPointer callbackStruct );

/* dm2kRelatedDisplay.cc */
void relatedDisplayCreateNewDisplay(DisplayInfo *displayInfo,
                                    DlRelatedDisplayEntry *pEntry);
void relatedDisplayDataDialogPopup(Widget w);

/* amDialog.h */
void deleteAMIList (AssociatedMenuItem *head);
void destroyAMPalette(void);
void createAmDialog(Widget);

/* dm2kMonitor.c */
void plotcomAttributeInit(DlPlotcom *plotcom);
void penAttributeInit(DlPen *pen);
void plotAxisDefinitionDestroy(DlPlotAxisDefinition *axisDefinition);
void traceAttributeDestroy(DlTrace *trace);
void plotcomAttributeCopy(DlPlotcom * to, DlPlotcom * from);
void traceAttributeCopy(DlTrace * to, DlTrace * from);
void plotAxisDefinitionCopy(DlPlotAxisDefinition * from, DlPlotAxisDefinition * to);
void monitorAttributeDestroy(DlMonitor *monitor);
void monitorAttributeCopy(DlMonitor *to, DlMonitor *from);
void penAttributeCopy(DlPen * to, DlPen * from);

void overrideAttributeCopy(DlOverrideFields *to, DlOverrideFields *from);
void overrideAttributeInit(DlOverrideFields *to);
void parseOverride(DisplayInfo *displayInfo, DlOverrideFields *override);
void writeDlOverride(FILE *stream, DlOverrideFields *dlOverride, int level);

/* dm2kDynSymbol.c */
DlElement *createDlDynSymbol(DlElement * p);
DlElement *parseDynSymbol(DisplayInfo *displayInfo);

/* amDialog.c */
AssociatedMenuItem * copyAMIList (AssociatedMenuItem *head);

/* faceplateDm2k.c */
void buildDisplayFromFaceplateGroup(FaceplateGroup * fpg);
Boolean writeFaceplateGroupToFile(Widget, FaceplateGroup *, char *, Boolean);

#ifdef __cplusplus
}
#endif

/* test.cc */
/* void usetest(void);*/

Widget ProductDescriptionCreatePopupDialogShell(
  Widget         parent,		/* parent widget for the shell	*/
  const char   * name,			/* product/program name		*/
  Pixmap         namePixmap,		/* name Pixmap (or NULL)	*/
  const char   * description,		/* product description		*/
  const char   * versionInfo,		/* product version number	*/
  const char   * developedAt,		/* at and by...			*/
  int            background,		/* background color (or -1)	*/
  int            foreground);		/* foreground color (or -1)	*/

void ProductDescriptionPopupShell(
  XtAppContext   appContext,	        /* application context		*/
  Widget         parent,		/* parent widget for the shell	*/
  const char   * name,			/* product/program name		*/
  Pixmap         namePixmap,		/* name Pixmap (or NULL)	*/
  const char   * description,		/* product description		*/
  const char   * versionInfo,		/* product version number	*/
  const char   * developedAt,		/* at and by...			*/
  int            background,		/* background color (or -1)	*/
  int            foreground,		/* foreground color (or -1)	*/
  int          * popdownFlag);		/* flag to pop down shell       */


#endif  /* __PROTO_H__ */
