#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dm2k.h"
#include "faceplateBase.h"

#define TITLE_HEIGHT(fp) (fp->title ? 48 : 0)
#define NOTES_HEIGHT(fp) (fp->notes ? 32 : 0)
#define TITLE_COLOR      44
#define NOTES_COLOR      44
#define MARGIN           12


static void prepareFaceplateGroupForDisplay(FaceplateGroup * fpg)
{
  int i;

  if (fpg == NULL)
    return;
  
  for (i = 0; i < fpg->entriesNum; i++) {
    Faceplate * entry = fpg->entries[i];

    if ((entry = fpg->entries[i]) != NULL) {

      if (entry->position != NULL) 
      {
	int row, column;
	char buffer[8];
	char * tmp;

	strcpy(buffer, entry->position);
	tmp =strchr(buffer, ',');
	if (tmp)
	  *tmp = ' ';
	
	sscanf(buffer, "%d%d\n", &column, &row);
	
	entry->x = (column * fpg->w) / 8;
	entry->y = (row * fpg->h) / 2;
	entry->w = fpg->w / 8;
	entry->h = fpg->h / 2;
      }
      else {
	if (fpg->fractionBase > 0) {
	  entry->x = (fpg->w * entry->x) / fpg->fractionBase;
	  entry->y = (fpg->h * entry->y) / fpg->fractionBase;
	  entry->w = (fpg->w * entry->w) / fpg->fractionBase;
	  entry->h = (fpg->h * entry->h) / fpg->fractionBase;
	}
      }
    }
  }

  /* place for title and notes label
   */
  fpg->h += TITLE_HEIGHT(fpg) + NOTES_HEIGHT(fpg);
}

void buildDisplayFromFaceplateGroup(FaceplateGroup * fpg)
{
  FILE        * filePtr;
  DisplayInfo * displayInfo = NULL;
  DisplayInfo * entryDisplayInfo;
  int i;

  if (fpg == NULL)
    return;
  
  prepareFaceplateGroupForDisplay(fpg);

  for (i = 0; i < fpg->entriesNum; i++) {
    DlElement   * disEl;

    if (fpg->entries[i] != NULL && fpg->entries[i]->adl != NULL) {

      filePtr = dmOpenUseableFile(fpg->entries[i]->adl);
      if (filePtr == NULL) {
	perror(fpg->entries[i]->adl);
	continue;
      }

      entryDisplayInfo = createInitializedDisplayInfo();
      if (entryDisplayInfo == NULL)
	return;
      
      entryDisplayInfo->filePtr = filePtr;
      dmDisplayListParse2(NULL, entryDisplayInfo, filePtr, 
			  fpg->entries[i]->macro, fpg->entries[i]->adl);

      fclose(filePtr);

      /* resize and shift
       */
      {
	unsigned int  width;
	unsigned int  height;
	float         widthRatio;
	float         heightRatio;


	disEl = FirstDlElement(entryDisplayInfo->dlElementList);
	while (disEl != NULL && disEl->type != DL_Display)
	  disEl = disEl->next;

	if (disEl == NULL) {
	  /* destroy display info (todo)*/
	  continue;
	}
	
	width  = disEl->structure.display->object.width;
	height = disEl->structure.display->object.height;
	
	widthRatio  = (float)fpg->entries[i]->w / (float)width;
	heightRatio = (float)fpg->entries[i]->h / (float)height;

	resizeDlElementList (entryDisplayInfo->dlElementList,
			     0,
			     0, /*TITLE_HEIGHT(fpg),*/
			     widthRatio,
			     heightRatio);

	if (fpg->entries[i]->x != 0 
	    || fpg->entries[i]->y + TITLE_HEIGHT(fpg) != 0)
	  shiftDlElementList(entryDisplayInfo->dlElementList,
			     fpg->entries[i]->x,
			     fpg->entries[i]->y + TITLE_HEIGHT(fpg));

      }

      /* add rectangle element to make a background for faceplate
       */
      {
	DlElement   * el;
	DlRectangle * dlRectangle;

	el = createDlRectangle(NULL);

	if (el != NULL) {
	  dlRectangle = el->structure.rectangle;
	  
	  dlRectangle->object.x      = fpg->entries[i]->x;
	  dlRectangle->object.y      = fpg->entries[i]->y + TITLE_HEIGHT(fpg);
	  dlRectangle->object.width  = fpg->entries[i]->w;
	  dlRectangle->object.height = fpg->entries[i]->h;

	  dlRectangle->dynAttr.vis = V_STATIC;
	  dlRectangle->dynAttr.clr = STATIC;
	  dlRectangle->attr.fill   = F_SOLID;
	  dlRectangle->attr.clr    = disEl->structure.display->bclr;

	  insertAfter(entryDisplayInfo->dlElementList, disEl, el);
	}
      }

      if (displayInfo == NULL) {
	disEl->structure.display->object.x      = fpg->x;
	disEl->structure.display->object.y      = fpg->y;
	disEl->structure.display->object.width  = fpg->w;
	disEl->structure.display->object.height = fpg->h;

	/* add Text element to show title
	 */
	if (fpg->title != NULL)
	  {
	    DlElement * el;
	    DlText    * dlText;

	    el = createDlText(NULL);

	    if (el != NULL) {
	      dlText = el->structure.text;
	  
	      dlText->object.x      = 0;
	      dlText->object.y      = 0;
	      dlText->object.width  = fpg->w;
	      dlText->object.height = TITLE_HEIGHT(fpg) - MARGIN;

	      dlText->textix = STRDUP(fpg->title);
	      dlText->alignment = ALIGNMENT_C;

	      dlText->dynAttr.vis = V_STATIC;
	      dlText->dynAttr.clr = STATIC;
	      dlText->attr.fill   = F_SOLID;
	      dlText->attr.clr    = TITLE_COLOR;

	      insertAfter(entryDisplayInfo->dlElementList, disEl, el);
	    }
	  }

	/* add Text element to show notes
	 */
	if (fpg->notes != NULL)
	  {
	    DlElement * el;
	    DlText    * dlText;

	    el = createDlText(NULL);

	    if (el != NULL) {
	      dlText = el->structure.text;
	  
	      dlText->object.x      = 0;
	      dlText->object.y      = fpg->h - TITLE_HEIGHT(fpg) + MARGIN;
	      dlText->object.width  = fpg->w;
	      dlText->object.height = TITLE_HEIGHT(fpg) - MARGIN;

	      dlText->textix = STRDUP(fpg->notes);
	      dlText->alignment = ALIGNMENT_C;

	      dlText->dynAttr.vis = V_STATIC;
	      dlText->dynAttr.clr = STATIC;
	      dlText->attr.fill   = F_SOLID;
	      dlText->attr.clr    = TITLE_COLOR;

	      insertAfter(entryDisplayInfo->dlElementList, disEl, el);
	    }
	  }


	displayInfo = entryDisplayInfo;
      }
      else {
	removeDlElement(entryDisplayInfo->dlElementList, disEl);
	
	insertDlListAfter(displayInfo->dlElementList,
			  LastDlElement(displayInfo->dlElementList),
			  entryDisplayInfo->dlElementList);
	
	emptyDlList(entryDisplayInfo->dlElementList);
	destroyDlDisplayList(entryDisplayInfo->selectedDlElementList);
	
	DM2KFREE(entryDisplayInfo->selectedDlElementList);
	DM2KFREE(entryDisplayInfo->dlElementList);
	DM2KFREE(entryDisplayInfo->dlFile);
	DM2KFREE(entryDisplayInfo->dlColormap);
	DM2KFREE(entryDisplayInfo);
      }
    }
  }  

  if (displayInfo != NULL) {
    renewString(&displayInfo->dlFile->name, fpg->name);

    displayInfo->next = NULL;
    displayInfo->prev = displayInfoListTail;

    if (displayInfoListHead == NULL) displayInfoListHead = displayInfo;
    if (displayInfoListTail) displayInfoListTail->next = displayInfo;

    displayInfoListTail = displayInfo;

    currentDisplayInfo = displayInfo;
    currentDisplayInfo->newDisplay = False;

    displayInfo->fromRelatedDisplayExecution = True;

    updateTaskInit(displayInfo);
    dmTraverseDisplayList(displayInfo);
  }

}

#define FACEPLATE_SUFFIX ".mfp"

static const char *templateSuffix = ".template";

Boolean writeFaceplateGroupToFile(
   Widget           parent,				  
   FaceplateGroup * fpg,
   char           * filename,
   Boolean          overwrite)
{
  char        * suffix;
  char          f1[MAX_FILE_CHARS], f2[MAX_FILE_CHARS+4];
  char          warningString[2*MAX_FILE_CHARS];
  int           strLen1, strLen2, strLen3, strLen4;
  int           status;
  FILE        * stream;
  Boolean       brandNewFile = False;
  Boolean       templateException = False;
  struct stat   statBuf;

  if (filename == NULL) 
    return False;

  strLen1 = STRLEN(filename);
  strLen2 = STRLEN(DISPLAY_FILE_BACKUP_SUFFIX);
  strLen3 = STRLEN(FACEPLATE_SUFFIX);
  strLen4 = STRLEN(templateSuffix);

  if (strLen1 >= MAX_FILE_CHARS) {
    dm2kPrintf("Path too Long %s\n:",filename);
    return False;
  }

  /* search for the position of the .mfp suffix 
   */
  strcpy(f1,filename);
  suffix = strstr(f1,FACEPLATE_SUFFIX);

  if ((suffix) && (suffix == f1 + strLen1 - strLen3)) {
    /* chop off the .mfp suffix */
    *suffix = '\0';
    strLen1 = strLen1 - strLen3;
  } else {
    /* search for the position of the .template suffix */
    suffix = strstr(f1,templateSuffix);
    if ((suffix) && (suffix == f1 + strLen1 - strLen4)) { 
      /* this is a .template special case */
      templateException = True;
    }
  }

  /* create the backup file name with suffux _BAK.mfp
   */
  strcpy(f2,f1);
  strcat(f2,DISPLAY_FILE_BACKUP_SUFFIX);
  strcat(f2,FACEPLATE_SUFFIX);

  /* append the .mfp suffix ;
   * check for the special case .template ;
   */

  if (!templateException) 
    strcat(f1,FACEPLATE_SUFFIX);

  /* See whether the file already exists. 
   */
  if (access(f1,W_OK) == -1) {
    if (errno == ENOENT) {
      brandNewFile = True;
    } else {
      sprintf(warningString,"Fail to create/write file :\n%s",filename);
      (void)getUserChoiseViaPopupQuestionDialog 
	(parent,
	 DM2K_DIALOG_MESSAGE_LABEL, 
	 warningString,
	 DM2K_DIALOG_OK_BUTTON,     "Yes",
	 DM2K_DIALOG_TITLE,         "Warning",
	 NULL);

      return False;
    }
  } 
  else {
    /* file exists, see whether the user want to overwrite the file. 
     */
    if (!overwrite) {
	int button;

	sprintf(warningString,"Do you want to overwrite file :\n%s",f1);

	button = getUserChoiseViaPopupQuestionDialog 
	  (parent,
	   DM2K_DIALOG_MESSAGE_LABEL, 
	   warningString,
	   DM2K_DIALOG_OK_BUTTON,     "Yes",
	   DM2K_DIALOG_CANCEL_BUTTON, "No",
	   DM2K_DIALOG_TITLE,         "Warning",
	   NULL);

	switch (button) {
	case 0 : 
	  /* Yes, Save the file */
	  break;
        default :
	  /* No, return */
          return False;
      }
    }
    
    /* see whether the backup file can be overwritten 
     */
    if (access(f2,W_OK) == -1) {
      if (errno != ENOENT) {
        sprintf(warningString,"Cannot write backup file :\n%s",filename);

	(void)getUserChoiseViaPopupQuestionDialog 
	  (parent,
	   DM2K_DIALOG_MESSAGE_LABEL, 
	   warningString,
	   DM2K_DIALOG_OK_BUTTON,     "Ok",
	   DM2K_DIALOG_TITLE,         "Warning",
	   NULL);

        return False;
      }
    }

    status = stat(f1,&statBuf);
    if (status) {
      dm2kPrintf("Failed to read status of file %s\n",filename);
      return False;
    }

    status = rename(f1,f2);
    if (status) {
      dm2kPrintf("Cannot rename file %s\n",filename);
      return False;
    }
  }

  stream = fopen(f1,"w");
  if (stream == NULL) {
    sprintf(warningString,"Fail to create/write file :\n%s",filename);
    (void)getUserChoiseViaPopupQuestionDialog 
      (parent,
       DM2K_DIALOG_MESSAGE_LABEL, 
       warningString,
       DM2K_DIALOG_OK_BUTTON,     "Ok",
       DM2K_DIALOG_TITLE,         "Warning",
       NULL);
    return False;
  }

  writeFaceplateGroupToStream(fpg, stream);

  fclose(stream);

  if (brandNewFile == 0) {
    chmod(f1,statBuf.st_mode);
  }

  return True;
}
