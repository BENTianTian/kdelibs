    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "debug.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int arts_debug_level = Arts::Debug::lInfo;
static bool arts_debug_abort = false;
static const char *arts_debug_prefix = "";

namespace Arts {

static class DebugInitFromEnv {
public:
	DebugInitFromEnv() {
		const char *env = getenv("ARTS_DEBUG");
		if(env)
		{
			if(strcmp(env,"debug") == 0)
				arts_debug_level = Debug::lDebug;
			else if(strcmp(env,"info") == 0)
				arts_debug_level = Debug::lInfo;
			else if(strcmp(env,"warning") == 0)
				arts_debug_level = Debug::lWarning;
			else if(strcmp(env,"quiet") == 0)
				arts_debug_level = Debug::lFatal;
			else
			{
				fprintf(stderr,
					"ARTS_DEBUG must be one of debug,info,warning,quiet\n");
			}
		}
		env = getenv("ARTS_DEBUG_ABORT");
		if(env)
			arts_debug_abort = true;
	}
} debugInitFromEnv;

};

void Arts::Debug::init(const char *prefix, Level level)
{
	arts_debug_level = level;
	arts_debug_prefix = prefix;
}

void Arts::Debug::fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
	fprintf(stderr,"\n");
	fprintf(stderr,"%s ** Fatal error (exiting) ** ", arts_debug_prefix);
    (void) vfprintf(stderr, fmt, ap);
	fprintf(stderr,"\n");
	fflush(stderr);
    va_end(ap);

	if(arts_debug_abort) abort();
	exit(1);
}

void Arts::Debug::warning(const char *fmt, ...)
{
	if(lWarning >= arts_debug_level)
	{
    	va_list ap;
    	va_start(ap, fmt);
		fprintf(stderr,"\n");
		fprintf(stderr,"%s ** Warning ** ", arts_debug_prefix);
    	(void) vfprintf(stderr, fmt, ap);
		fprintf(stderr,"\n");
		fflush(stderr);
    	va_end(ap);
	}
}

void Arts::Debug::info(const char *fmt, ...)
{
	if(lInfo >= arts_debug_level)
	{
    	va_list ap;
    	va_start(ap, fmt);
		fprintf(stderr,"%s ", arts_debug_prefix);
    	(void) vfprintf(stderr, fmt, ap);
		fprintf(stderr,"\n");
		fflush(stderr);
    	va_end(ap);
	}
}

void Arts::Debug::debug(const char *fmt, ...)
{
	if(lDebug >= arts_debug_level)
	{
    	va_list ap;
    	va_start(ap, fmt);
    	(void) vfprintf(stderr, fmt, ap);
		fprintf(stderr,"\n");
		fflush(stderr);
    	va_end(ap);
	}
}
