#include <errno.h>
#include <time.h>
#include "dm2k.h"
#include "faceplateBase.h"


#define LINE_SIZE        10000

const char groupComments[]     = "groupComments";
const char groupTitle[]        = "groupTitle";
const char groupNotes[]        = "groupNotes";
const char groupX[]            = "groupX";
const char groupY[]            = "groupY"; 
const char groupWidth[]        = "groupWidth";
const char groupHeight[]       = "groupHeight";
const char groupFractionBase[] = "groupFractionBase";


const char faceplatePosition[] = "faceplatePosition";
const char faceplateX[]        = "faceplateX";
const char faceplateY[]        = "faceplateY"; 
const char faceplateWidth[]    = "faceplateWidth";
const char faceplateHeight[]   = "faceplateHeight"; 
const char faceplateAdl[]      = "faceplateAdl";
const char faceplateMacro[]    = "faceplateMacro"; 

void destroyFaceplateGroup(FaceplateGroup * fpg)
{
  int i;

  if (fpg == NULL)
    return;

  DM2KFREE(fpg->name);
  DM2KFREE(fpg->title);
  DM2KFREE(fpg->notes);
  DM2KFREE(fpg->comments);

  if (fpg->entries != NULL) {
    for (i = 0; i < fpg->entriesNum; i++) {
      if (fpg->entries[i] != NULL) {
	DM2KFREE(fpg->entries[i]->position);
	DM2KFREE(fpg->entries[i]->adl);
	DM2KFREE(fpg->entries[i]->macro);
	DM2KFREE(fpg->entries[i]);
      }
    }
  }

  DM2KFREE(fpg->entries);

  DM2KFREE(fpg);
}


static Boolean fillInFaceplateGroup(FaceplateGroup * fpg, FILE * stream)
{
  char        line[LINE_SIZE+1];
  Faceplate * entry;

  while(fgets(line, LINE_SIZE, stream)) 
  {
    if (line[0] != ';')
    {
      char * equal;
      int    len = strlen(line);

      if (line[len-1] == '\n')
	line[len-1] = '\0';

      equal = strchr(line, '=');
      if (equal == NULL) {
	fprintf(stderr, "Error in faceplate file\n"
		"The mistake in line:(%s)\n",
		line);
	continue;
      }
      
      equal++;

#define STRNCMP(s) strncmp(line, s, sizeof(s)-1) == 0

      if      (STRNCMP(groupComments)) /* */
      {
	renewString(&fpg->comments,equal);
      }
      else if (STRNCMP(groupTitle))    /* */
      {
	renewString(&fpg->title,equal);
      }
      else if (STRNCMP(groupNotes))    /* */
      {
	renewString(&fpg->notes,equal);
      }
      else if (STRNCMP(groupX))        /* */
      {
	fpg->x = atoi(equal);
      }
      else if (STRNCMP(groupY))        /* */
      {
	fpg->y = atoi(equal);
      }
      else if (STRNCMP(groupWidth))    /* */
      {
	fpg->w = (unsigned int)atoi(equal);
      }
      else if (STRNCMP(groupHeight))   /* */
      {
	fpg->h = (unsigned int)atoi(equal);
      }
      else if (STRNCMP(groupFractionBase))  /* */
      {
	fpg->fractionBase = atoi(equal);
      }
      else if (STRNCMP(faceplatePosition))  /* */
      {
	fpg->entriesNum++;
	REALLOC(Faceplate*, fpg->entries, fpg->entriesNum);
	if (fpg->entries == NULL) 
	  return True;
	
	fpg->entries[fpg->entriesNum -1] = NULL;

	entry = DM2KALLOC(Faceplate);
	if (entry == NULL)
	  return True;

	renewString(&entry->position,equal);
	
	fpg->entries[fpg->entriesNum -1] = entry;

	if (equal != NULL) {
	  equal = strchr(equal, ',');

	  if (equal != NULL) {
	    *equal = ' ';
	  }
	  else {
	    fprintf(stderr, "Error in faceplate file\n"
		    "The mistake in line:(%s)\n",
		    line);

	    DM2KFREE(entry->position);
	    entry->position = NULL;

	    fpg->entriesNum--;
	    REALLOC(Faceplate*, fpg->entries, fpg->entriesNum);
	    if (fpg->entries == NULL) 
	      return True;
	  }
	}

      }
      else if (STRNCMP(faceplateX))         /* */
      {
	fpg->entriesNum++;
	REALLOC(Faceplate*, fpg->entries, fpg->entriesNum);
	if (fpg->entries == NULL) 
	  return True;
	
	fpg->entries[fpg->entriesNum -1] = NULL;

	entry = DM2KALLOC(Faceplate);
	if (entry == NULL)
	  return True;

	entry->x = atoi(equal);

	fpg->entries[fpg->entriesNum -1] = entry;
      }
      else if (STRNCMP(faceplateY))         /* */
      {
	fpg->entries[fpg->entriesNum -1]->y = atoi(equal);
      }
      else if (STRNCMP(faceplateWidth))     /* */
      {
	fpg->entries[fpg->entriesNum -1]->w = (unsigned int)atoi(equal);
      }
      else if (STRNCMP(faceplateHeight))    /* */
      {
	fpg->entries[fpg->entriesNum -1]->h = (unsigned int)atoi(equal);
      }
      else if (STRNCMP(faceplateAdl))       /* */
      {
	entry = fpg->entries[fpg->entriesNum -1];
	renewString(&entry->adl,equal);
      }
      else if (STRNCMP(faceplateMacro))     /* */
      {
	entry = fpg->entries[fpg->entriesNum -1];
	renewString(&entry->macro,equal);
      }
      else{
	fprintf(stderr, "Error in faceplate file\n"
		"The mistake in line:(%s)\n",
		line);
      }

#undef STRNCMP

    }

  }

  return False;
}

FaceplateGroup * createFaceplateGroup(char * fName)
{
  FaceplateGroup * fpg;
  FILE           * stream;

  fpg = DM2KALLOC(FaceplateGroup);
  if (fpg == NULL)
    return NULL;

  if (fName == NULL)
    return fpg;

  stream = fopen(fName, "ra");
  if (stream== NULL) {
    destroyFaceplateGroup(fpg);
    return NULL;
  }

  fpg->name = STRDUP(fName);

  if (fillInFaceplateGroup(fpg, stream)) {
    destroyFaceplateGroup(fpg);
    fpg = NULL;
  }

  fclose(stream);

  return fpg;
}

void writeFaceplateGroupToStream(FaceplateGroup * fpg, FILE * stream)
{
  int    i;
  time_t t = time(NULL);
  char s[256];
  char * userName = cuserid(NULL);

  if (fpg == NULL || stream == NULL) 
    return;

  strftime(s, 256, "%a %d/%m/%Y-%H:%M", localtime(&t));
  fprintf(stream,";\n; File was generated by DM2K's Faceplate tool\n");
  fprintf(stream,"; at %s by %s.\n;\n", s, userName?userName:"unknowen user");

  if (fpg->title != NULL)
    fprintf(stream,"%s=%s\n", groupTitle, fpg->title);

  if (fpg->notes != NULL)
    fprintf(stream,"%s=%s\n", groupNotes, fpg->notes);

  if (fpg->comments != NULL)
    fprintf(stream,"%s=%s\n", groupComments, fpg->comments);


  fprintf(stream,"%s=%d\n", groupX, fpg->x);
  fprintf(stream,"%s=%d\n", groupY, fpg->y);
  fprintf(stream,"%s=%d\n", groupWidth, fpg->w);
  fprintf(stream,"%s=%d\n", groupHeight, fpg->h);


  if (fpg->fractionBase > 0)
    fprintf(stream,"%s=%d\n", groupFractionBase, fpg->fractionBase);


  if (fpg->entries != NULL) {
    for (i = 0; i < fpg->entriesNum; i++) {

      fprintf(stream,";\n; faceplate %d\n;\n", i+1);

      if (fpg->entries[i]->position != NULL) {
	fprintf(stream,"%s=%s\n", faceplatePosition,fpg->entries[i]->position);
      }
      else {
	fprintf(stream,"%s=%d\n", faceplateX, fpg->entries[i]->x);
	fprintf(stream,"%s=%d\n", faceplateY, fpg->entries[i]->y);
	fprintf(stream,"%s=%d\n", faceplateWidth, fpg->entries[i]->w);
	fprintf(stream,"%s=%d\n", faceplateHeight, fpg->entries[i]->h);
      }

      if (fpg->entries[i]->adl != NULL)
	fprintf(stream,"%s=%s\n", faceplateAdl, fpg->entries[i]->adl);
      
      if (fpg->entries[i]->macro != NULL)
	fprintf(stream,"%s=%s\n", faceplateMacro, fpg->entries[i]->macro);
    }
  }
}

Boolean appendNewFaceplate(
   FaceplateGroup * fpg, 
   const char     * position,
   int              x,
   int              y,
   unsigned int     w,
   unsigned int     h,
   const char     * adl,
   const char     * macro)
{
  Faceplate * entry;

  if (fpg == NULL)
    return True;

  fpg->entriesNum++;

  REALLOC(Faceplate*, fpg->entries, fpg->entriesNum);
  entry = DM2KALLOC(Faceplate);

  if (fpg->entries == NULL || entry == NULL) {
    fpg->entriesNum--;
    return True;
  }
  
  fpg->entries[fpg->entriesNum -1] = entry;

  entry->position = position && *position ? STRDUP(position) : NULL;
  entry->x        = x;
  entry->y        = y;
  entry->w        = w;
  entry->h        = h;
  entry->adl      = STRDUP(adl);
  entry->macro    = STRDUP(macro);

  return False;
}

void freeFaceplate(Faceplate * entry)
{
  if (entry == NULL)
    return;

  DM2KFREE(entry->position);
  DM2KFREE(entry->adl);
  DM2KFREE(entry->macro);
}

void destroyFaceplate(FaceplateGroup * fpg, int num)
{
  int i;

  if (fpg == NULL
      || fpg->entries == NULL 
      || num >= fpg->entriesNum
      || fpg->entries[num] == NULL)
    return;
  
  freeFaceplate(fpg->entries[num]);

  for (i = num; i < fpg->entriesNum-1; i++)
    fpg->entries[i] = fpg->entries[i+1];
  
  fpg->entriesNum--;
  REALLOC(Faceplate*, fpg->entries, fpg->entriesNum);
}
