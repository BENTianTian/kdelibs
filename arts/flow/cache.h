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

#ifndef _SYNTH_CACHE_H
#define _SYNTH_CACHE_H

#include <time.h>
#include <list>
#include <string>
#include "startupmanager.h"

class Cache;

class CachedObject
{
private:
	std::string _object_key;
	int _ref_cnt;
	time_t _lastAccess;

protected:
	void setKey(std::string key);

public:
	std::string getKey();

	time_t lastAccess();

	void decRef();
	void incRef();
	int refCnt();

	virtual bool isValid();
	virtual int memoryUsage() = 0;

	CachedObject(Cache *cache);
	virtual ~CachedObject();
};

class Cache
{
protected:
	std::list<CachedObject *> objects;
	Cache();
	~Cache();

	static Cache *_instance;

public:
	static Cache *the();
	static void shutdown();

	static long memused;

	CachedObject *get(std::string key);
	void add(CachedObject *object);

	// garbage collection; returns amount of memory used (after cleaning)
	long cleanUp(long cacheLimit);
};

class CacheShutdown :public StartupClass
{
public:
	void startup();
	void shutdown();
};

#endif
