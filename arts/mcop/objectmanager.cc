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

#include "mcoputils.h"
#include "mcopconfig.h"
#include "objectmanager.h"
#include "dispatcher.h"
#include "extensionloader.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;

Object_skel *ObjectManager::create(string name)
{
	list<Factory *>::iterator i;

	/* first try: look through all factories we have */

	for(i = factories.begin();i != factories.end(); i++)
	{
		Factory *f = *i;
		if(f->interfaceName() == name + "_base") return f->createInstance();
		if(f->interfaceName() == name ) return f->createInstance();
	}

	/* second try: look if there is a suitable extension we could load */

	MCOPConfig config(string(EXTENSION_DIR) + "/" + name + ".mcopclass");
	string library = config.readEntry("Library");
	if(library != "")
	{
		ExtensionLoader *e = new ExtensionLoader(library);

		if(e->success())
		{
			// TODO: memory leak here, extension is never unloaded
			for(i = factories.begin();i != factories.end(); i++)
			{
				Factory *f = *i;
//				cerr << "Found interfaceName: " << f->interfaceName() << endl;
				if(f->interfaceName() == name + "_base") return f->createInstance();
				if(f->interfaceName() == name) return f->createInstance();
			}
		}
		else {
			cerr << "MCOP ObjectManager: warning: Could not load extension " << library << endl;
			delete e;
		}
	}
	cerr << "MCOP ObjectManager: warning: can't find implementation for " << name << endl;
	return 0;
}

void ObjectManager::registerFactory(Factory *factory)
{
	factories.push_back(factory);
}

void ObjectManager::removeFactory(Factory *factory)
{
	list<Factory *>::iterator i;

	i = factories.begin();
	while(i != factories.end())
	{
		if(*i == factory) {
			factories.erase(i);
			i = factories.begin();
		}
		else i++;
	}
}

ObjectManager *ObjectManager::_instance = 0;

ObjectManager::ObjectManager()
{
	assert(!_instance);
	_instance = this;
}

ObjectManager::~ObjectManager()
{
	assert(_instance);
	_instance = 0;
}

ObjectManager *ObjectManager::the()
{
	assert(_instance);
	return _instance;
}

/*
 * global references
 */

bool ObjectManager::addGlobalReference(Object *object, string name)
{
	bool result;

	result = Dispatcher::the()->globalComm()->put(name,object->_toString());
	if(result)
		referenceNames.push_back(name);

	return result;
}

string ObjectManager::getGlobalReference(string name)
{
	return Dispatcher::the()->globalComm()->get(name);
}

void ObjectManager::removeGlobalReferences()
{
	list<string>::iterator i;

	for(i=referenceNames.begin(); i != referenceNames.end();i++)
		Dispatcher::the()->globalComm()->erase(*i);
}
