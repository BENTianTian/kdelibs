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

#ifndef MCOPUTILS_H
#define MCOPUTILS_H

#include <string>

namespace Arts {

class MCOPUtils {
public:
	/**
	 * Returns the full pathname to a file in the mcop directory which
	 * is called "name". It will also care that no other characters than
	 * A-Z,a-z,0-9,-,_ occur.
	 * 
	 * The result is something like /tmp/mcop-<username>/name, the directory
	 * will be created when necessary.
	 */
	static std::string createFilePath(std::string name);

	/**
	 * Returns the fully qualified hostname, such as "www.kde.org" (of course
	 * this may fail due to misconfiguration).
	 *
	 * The result is "localhost" if nothing at all can be found out.
	 */
	static std::string getFullHostname();

	/**
	 * Returns configuration values from .mcoprc
	 */
	static std::string readConfigEntry(const std::string& key,
										const std::string& defaultValue = "");

	/**
	 * Makes an interface ID from string - if the given string is already
	 * known, the returned IID will be the one returned last time. If not,
	 * a new IID is generated
	 */
	static unsigned long makeIID(const std::string& interfaceName);
};

};
#endif /* MCOPUTILS_H */
