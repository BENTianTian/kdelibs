/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Timo Hummel <timo.hummel@sap.com>
 *                    Tom Braun <braunt@fh-konstanz.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * This file is used to catch signals which would normally
 * crash the application (like segmentation fault, floating
 * point exception and such).
 */

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "kcrash.h"


KCrash::HandlerType KCrash::_emergencySaveFunction = 0;
KCrash::HandlerType KCrash::_crashHandler = 0;
QString *KCrash::appName = 0;
QString *KCrash::appPath = 0;

// This function sets the function which should be called when the 
// application crashes and the
// application is asked to try to save its data.
void
KCrash::setEmergencySaveFunction (HandlerType saveFunction)
{
  _emergencySaveFunction = saveFunction;
  
  /* 
   * We need at least the default crash handler for 
   * emergencySaveFunction to be called
   */
  if (_emergencySaveFunction && !_crashHandler)
    _crashHandler = defaultCrashHandler;
}


// This function sets the function which should be responsible for 
// the application crash handling.

void
KCrash::setCrashHandler (HandlerType handler)
{
  if (!handler)
    signal (SIGSEGV, SIG_DFL);
  else
    signal (SIGSEGV, handler);

  _crashHandler = handler;
}

void
KCrash::defaultCrashHandler (int signal)
{
  // Handle possible recursions
  static int crashRecursionCounter = 0;
  kdDebug(0) << "KCrash: crashing.... crashRecursionCounter = " << crashRecursionCounter << "\n";
  kdDebug(0) << "KCrash: Appname = " << appName << " apppath = " << appPath << "\n";

  crashRecursionCounter++;

  if (crashRecursionCounter < 2) {
    if (_emergencySaveFunction) {
      _emergencySaveFunction (signal);
    }
    crashRecursionCounter++; // 
  }
  
  if (crashRecursionCounter < 3 && appName) {
      // this code is leaking, but this should not hurt cause we will do a
      // exec() afterwards. exec() is supposed to clean up.
      char * argv[14]; // don't forget to update this
      int i = 0;

      // argument 0 has to be drkonqi
      argv[i++] = qstrdup("drkonqi");

      // we have already tested this
      argv[i++] = "--appname";
      argv[i++] = qstrdup(appName->latin1());

      // only add apppath if it's not NULL
      if (appPath) {
	argv[i++] = qstrdup("--apppath");
	argv[i++] = qstrdup(appPath->latin1());
      }

      // signal number -- will never be NULL
      QCString tmp;
      tmp.setNum(signal);
      argv[i++] = qstrdup("--signal");
      argv[i++] = qstrdup(tmp.data());

      const KInstance *instance = KGlobal::_instance;
      const KAboutData *about = instance ? instance->aboutData() : 0;
      if (about) {
	if (!about->version().isNull()) {
	  argv[i++] = qstrdup("--appversion");
	  argv[i++] = qstrdup(about->version().utf8());
	}

	if (!about->programName().isNull()) {
	  argv[i++] = qstrdup("--programname");
	  argv[i++] = qstrdup(about->programName().utf8());
	}

	if (!about->bugAddress().isNull()) {
	  argv[i++] = qstrdup("--bugaddress");
	  argv[i++] = qstrdup(about->bugAddress().utf8());
	}
      }

      // NULL terminated list
      argv[i++] = NULL;

      execvp("drkonqi", argv);

      // we could clean up here
      // i = 0;
      // while (argv[i])
      //   free(argv[i++]);
  }
  else
    kdDebug(0) << "Unknown appname\n";
   
  if (crashRecursionCounter < 4)
  {
     kdDebug(0) << "Unable to start dr. konqi\n";
  }
  _exit(255);
}

