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

#include "ifacerepo_impl.h"

using namespace std;

InterfaceRepo_impl::InterfaceRepo_impl()
{
	nextModuleID = 1;
}

long InterfaceRepo_impl::insertModule(const ModuleDef& newModule)
{
	long moduleID = nextModuleID++;

	vector<InterfaceDef *>::const_iterator ii;
	for(ii=newModule.interfaces.begin();
		ii != newModule.interfaces.end();ii++)
	{
		Buffer b;
		(*ii)->writeType(b);
		interfaces.push_back(new InterfaceEntry(b,moduleID));
	}

	vector<TypeDef *>::const_iterator ti;
	for(ti=newModule.types.begin();
		ti != newModule.types.end();ti++)
	{
		Buffer b;
		(*ti)->writeType(b);
		types.push_back(new TypeEntry(b,moduleID));
	}

	return moduleID;
}

void InterfaceRepo_impl::removeModule(long moduleID)
{
	list<TypeEntry *>::iterator ti;

	/* erase types */
	ti = types.begin();
	while(ti != types.end())
	{
		if((*ti)->moduleID == moduleID)
		{
			delete (*ti);
			types.erase(ti);
			ti = types.begin();
		}
		else ti++;
	}

	/* erase interfaces */
	list<InterfaceEntry *>::iterator ii;
	ii = interfaces.begin();
	while(ii != interfaces.end())
	{
		if((*ii)->moduleID == moduleID)
		{
			delete (*ii);
			interfaces.erase(ii);
			ii = interfaces.begin();
		}
		else ii++;
	}
}

InterfaceDef* InterfaceRepo_impl::queryInterface(const string& name)
{
	list<InterfaceEntry *>::iterator ii;

	for(ii = interfaces.begin();ii != interfaces.end();ii++)
	{
		if((*ii)->name == name)	/* TODO: namespace! */
		{
			Buffer b;
			(*ii)->writeType(b);
			return new InterfaceDef(b);
		}
	}
	/* TODO: what happens here? */
	assert(false);
	return 0;
}

TypeDef* InterfaceRepo_impl::queryType(const string& name)
{
	list<TypeEntry *>::iterator ti;

	for(ti = types.begin();ti != types.end();ti++)
	{
		if((*ti)->name == name)	/* TODO: namespace! */
		{
			Buffer b;
			(*ti)->writeType(b);
			return new TypeDef(b);
		}
	}

	/* TODO: what happens here? */
	assert(false);
	return 0;
}
