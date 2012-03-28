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
 *      Medm CDEV Interface Implementation Header file
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab. High Performance Computing Group
 *
 * Revision History:
 *   $Log: dm2kCdevP.h,v $
 *   Revision 1.2  2006-03-28 15:32:06  birke
 *   now a 3.14 application...
 *
 *   Revision 1.1  2000/02/23 16:17:12  birke
 *   += cdev-support
 *
 *
 *
 */
#ifndef _DM2K_CDEVP_H
#define _DM2K_CDEVP_H

#ifdef DM2K_CDEV

#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <cdevTypes.h>

class cdevSystem;
class cdevRequestObject;

class dm2kInputFd
{
public:
  // constructor
  dm2kInputFd        (int f, int i, dm2kInputFd* next = 0);
  // destructor
  ~dm2kInputFd       (void);

  int                fd;
  XtInputId          id;
  dm2kInputFd*       next;
};

class dm2kXInput
{
public:
  // constrcutor
  dm2kXInput  (XtAppContext context, cdevSystem* system);
  // destructor
  ~dm2kXInput (void);

  // operations
  // add a single file descriptor 
  void addInput    (int fd, XtPointer mask);
  void removeInput (int fd);

protected:
  // internal file descriptors
  dm2kInputFd* xfds_;
  
private:
  static void inputCallback (XtPointer, int*, XtInputId*);
  XtAppContext context_;

  cdevSystem* system_;

  // deny copy and assignment operations
  dm2kXInput (const dm2kXInput& input);
  dm2kXInput& operator = (const dm2kXInput& input);
};

extern dm2kXInput* dm2kGXinput;

#endif

#endif

