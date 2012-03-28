/***
 *** Print Utilities (currently supporting only PostScript)
 ***
 *** MDA - 28 June 1990
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <X11/Xlib.h>

/*
 * routine through which all graphic print requests are channeled
 */

#define MY_FREE(pointer) if (pointer) {free ((char*)pointer); pointer = NULL;}
#define COMBUFSIZE 256

void utilPrint(display, window, fileName)
     Display *display;
     Window  window;
     char    *fileName;
{
  char commandBuffer[COMBUFSIZE];
  char *newFileName = NULL;
  char *psFileName = NULL;
  FILE *fo;
  char *theprinter;
  char *printCmd;
  /*
   * if printer is not specified than use system env
   */
  printCmd = getenv("EPICS_PS_PRINT_CMD");
  theprinter = getenv("PSPRINTER");

  
  if ((theprinter == NULL) && (printCmd == NULL)) {
    fprintf(stderr,"\nutilPrint: EPICS_PS_PRINT_CMD and PSPRINTER environment variables ");
    fprintf(stderr,"not set, printing disallowed\n");
    return;
  }
  
  /* return if not enough information around */
  if (display == (Display *)NULL 
      || window == (Window)NULL
      || fileName == (char *)NULL) 
  {
    fprintf(stderr,"\nutilPrint: internal error ");
    fprintf(stderr,"there is not enough information\n");
    return;
  }

  /*
   * make unique file name for dump file
   */
  if (strlen(fileName) + 25 >= COMBUFSIZE) {
    fprintf(stderr,"\nutilPrint: internal error ");
    fprintf(stderr,"file name is too long %d char's\n", strlen(fileName));
    return;
  }

  newFileName = (char *) malloc (strlen(fileName) + 20);
  if (newFileName == NULL) {
    fprintf(stderr,"\nCannot alloc memory\n");
    MY_FREE(newFileName);
    MY_FREE(psFileName);
    return;
  }
  sprintf(newFileName,"%s%d",fileName, getpid());


  /*** first try: printScreen.tcl ***/
  sprintf(commandBuffer, "printScreen.tcl 0x%x", (int)window);
  if (system(commandBuffer) == 0) return;
  fprintf(stderr, "meta-print command '%s' seems to have failed\n -> trying print-commands on my own\n", commandBuffer);

  /*** second try: pipe of print commands ***/
  if (printCmd) {
     sprintf(commandBuffer, "xwd -id 0x%x | xwdtopnm | pnmtops -rle | %s", (int)window, printCmd );
  } else 
     sprintf(commandBuffer, "xwd -id 0x%x | xwdtopnm | pnmtops -rle | lp -d %s",  (int)window, theprinter);
  if (system(commandBuffer) == 0) return;

  fprintf(stderr, "print command '%s' seems to have failed\n -> trying builtin print-support (won't work on true-color systems!)\n", commandBuffer);

  /*** third try: do-it-yourself ***/
  /*
   * dump X window in xwd file format
   */
  xwd(display, window, newFileName);

  /*
   * postscript file name
   */
  psFileName = (char *) malloc (strlen(newFileName) + 5);
  if (psFileName == NULL) {
    fprintf(stderr,"\nCannot alloc memory\n");
    MY_FREE(newFileName);
    MY_FREE(psFileName);
    return;
  }
  sprintf(psFileName,"%s.ps", newFileName);


  /*
   * convert xwd format file to postscript
   */
  {
     char printCmd[128];
     char *myArgv[64];
     int myArgc = 0;
    
    myArgv[myArgc++] = "xwd2ps";
    myArgv[myArgc++] = "-d";
    myArgv[myArgc++] = "-t";
/*
    myArgv[myArgc++] = "-s";
    myArgv[myArgc++] = "The Title";
    myArgv[myArgc++] = "-L";
    myArgv[myArgc++] = "-b";
*/
    myArgv[myArgc++] = newFileName;
    
    fo = fopen(psFileName,"w+");
    if (fo == NULL) 
    {
      fprintf(stderr,"\nutilPrint:  unable to open file: %s",psFileName);
      sprintf(commandBuffer,"rm %s &",newFileName);
      system(commandBuffer);
      MY_FREE(newFileName);
      MY_FREE(psFileName);
      return;
    }
    xwd2ps(myArgc,myArgv,fo);
    fclose(fo);
  }

  /*
   * run system command
   */
  if (printCmd) {
     sprintf(commandBuffer, "cat %s | %s", psFileName, printCmd );
  } else 
     sprintf(commandBuffer, "lp -d%s %s", theprinter, psFileName);
  printf("executing '%s'\n", commandBuffer);
  system(commandBuffer);

  sprintf(commandBuffer,"rm -f %s %s", psFileName, newFileName);
  system(commandBuffer);

  MY_FREE(newFileName);
  MY_FREE(psFileName);
}
