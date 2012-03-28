#ifndef _faceplateGUI_h
#define _faceplateGUI_h 1

#include <stdlib.h>

#include "dm2k.h"
#include "faceplateBase.h"

#define LINK_WIDGET_NAME "wFaceplateDlg"

typedef enum {
  FP_DESTROY_DLG,
  FP_FILE_NEW,
  FP_FILE_OPEN,
  FP_FILE_SAVE,
  FP_FILE_SAVEAS,
  FP_FILE_CLOSE,
  FP_GROUP_ATTR_COMMENTS,
  FP_GROUP_ATTR_TITLE,
  FP_GROUP_ATTR_NOTES,
  FP_INPIXEL,
  FP_FRACTIONBASE,
  FP_FACEPLATE_CELL_00,
  FP_FACEPLATE_CELL_10,
  FP_FACEPLATE_CELL_20,
  FP_FACEPLATE_CELL_30,
  FP_FACEPLATE_CELL_40,
  FP_FACEPLATE_CELL_50,
  FP_FACEPLATE_CELL_60,
  FP_FACEPLATE_CELL_70,
  FP_FACEPLATE_CELL_01,
  FP_FACEPLATE_CELL_11,
  FP_FACEPLATE_CELL_21,
  FP_FACEPLATE_CELL_31,
  FP_FACEPLATE_CELL_41,
  FP_FACEPLATE_CELL_51,
  FP_FACEPLATE_CELL_61,
  FP_FACEPLATE_CELL_71,
  FP_FACEPLATE_CELL_RESET,
  FP_FACEPLATE_ADL_FILE,
  FP_FACEPLATE_ADL_FILE_BROWSE,
  FP_FACEPLATE_MACRO_PARAM,
  FP_FACEPLATE_ADD_NEW,
  FP_FACEPLATE_APPLY,
  FP_FACEPLATE_DELETE,
  FP_ADL_FILE_SELECT_CANCEL,
  FP_ADL_FILE_SELECT_OK,
  FP_FILE_SELECT_CANCEL,
  FP_FILE_SELECT_OK
} FaceplateConstants;

typedef struct _FaceplateGUI {
  FaceplateGroup * fpg;
  int              current;
  Boolean          saveIt;
  char             position[8];
  Widget           highligthedCell;
  Widget wFaceplateDlgShell;
  Widget wFaceplateDlg;
  Widget wFileNew;
  Widget wFileOpen;
  Widget wFileSave;
  Widget wFileSaveAs;
  Widget wFileClose;
  Widget wComments;
  Widget wTitle;
  Widget wNotes;
  Widget wX;
  Widget wY;
  Widget wHeight;
  Widget wWidth;
  Widget wInPixelTGL;
  Widget wFractionBaseTGL;
  Widget wFractionBaseTXT;
  Widget wGroupForm;
  Widget wGroupRowColumn;
  Widget wCell00;
  Widget wCell10;
  Widget wCell20;
  Widget wCell30;
  Widget wCell40;
  Widget wCell50;
  Widget wCell60;
  Widget wCell70;
  Widget wCell01;
  Widget wCell11;
  Widget wCell21;
  Widget wCell31;
  Widget wCell41;
  Widget wCell51;
  Widget wCell61;
  Widget wCell71;
  Widget wFaceplateX;
  Widget wFaceplateY;
  Widget wFaceplateHeigth;
  Widget wFaceplateWidth;
  Widget wSelectedAdlFile;
  Widget wFaceplateAdlFileBrowse;
  Widget wFaceplateMacroRowColomn;
  Widget wFaceplateReset;
  Widget wAddNewFaceplate;
  Widget wApplayChanges;
  Widget wDeleteFaceplate;

  Widget wAdlSelectDlg;
  Widget wAdlSelectDlgSelection;

  Widget wSelectFaceplateShell;
  Widget wSelectFaceplateDlg;

} FaceplateGUI;

extern void create_shell5 (Widget parent);
extern void create_adlSelectDlg (Widget parent);
extern void create_shell2 (Widget parent);

#endif
