==========================
DM2K Building instructions
==========================

Install packages
----------------

This depends on the Linux distribution you use:

debian-11 and debian-12
+++++++++++++++++++++++

Enter::

  apt-get install -y make gcc g++ x11proto-dev libx11-dev libxt-dev libmotif-dev libxmu-headers libxext-dev libxmu-dev

Fedora 41
+++++++++

Enter::

  dnf install -y make perl xorg-x11-proto-devel libX11-devel libXt-devel motif-devel

Note: gcc and g++ are already installed.

Download and build EPICS Base
-----------------------------

Download EPICS Base 3.15.9 from https://epics.anl.gov/download/base/index.php

Unpack the tar file::

  tar -xvzf base-3.15.9.tar.gz

Build EPICS base::

  cd base-3.15.9
  make -sj

Create file for setting environment variable 'EPICS_BASE'::

  echo "export EPICS_BASE=$(pwd)" > setenv.sh

Set environment variable EPICS_BASE::

  . ./setenv.sh

Build DM2K
----------

Go to DM2K directory::

  cd ../DM2K-HZB

Set up location of EPICS Base::

  echo "EPICS_BASE=$EPICS_BASE" > configure/RELEASE.local

Build DM2K::

  make

After make has finished, the binary "dm2k" is in directory 'bin/linux-x86_64'.

