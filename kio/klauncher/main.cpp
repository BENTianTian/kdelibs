/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include "config.h"

#include "kapp.h"
#include "klauncher.h"
#include "kcmdlineargs.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <qcstring.h>


extern "C" { int start_launcher(int); }

static void sig_handler(int)
{
   KLauncher::destruct(255);
}

int
start_launcher(int socket)
{
   QCString cname = KApplication::launcher();
   char *name = cname.data();
   // Started via kdeinit.
   KCmdLineArgs::init(1, (char **) &name, name, "A service launcher.", 
                       "v0.1");
   KLauncher::addCmdLineOptions();

   // WABA: Make sure not to enable session management.
   putenv("SESSION_MANAGER=");

   KLauncher launcher(socket);

   signal( SIGHUP, sig_handler);
   signal( SIGTERM, sig_handler);
   signal( SIGSEGV, sig_handler);

#ifdef __USE_GNU 
#warning Temporary hack to get KLocale initialised
#endif
   (void)launcher.config();

   launcher.exec();
   return 0;
}

int main(/*int argc, char **argv*/)
{
   return 0;
}
