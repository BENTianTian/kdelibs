/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <dom_string.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>

#include <kjs/types.h>
#include <kjs/function.h>
#include <kjs/operations.h>
#include <kurl.h>
#include <kio/kprotocolmanager.h>
#include "kjs_navigator.h"
#include "khtml_part.h"

using namespace KJS;

namespace KJS {

  class Plugins : public HostImp {
  public:
    Plugins() { }
    virtual KJSO get(const UString &p) const;
  };

  class PluginsFunc : public DOMFunction {
  public:
    PluginsFunc() { };
    Completion tryExecute(const List &);
  };

  class NavigatorFunc : public InternalFunctionImp {
  public:
    NavigatorFunc(KHTMLPart *p) : part(p) { }
    Completion execute(const List &);
  private:
  KHTMLPart *part;
  };
};

KJSO Navigator::get(const UString &p) const
{
  KURL url = part->url();
  QString userAgent = KProtocolManager::userAgentForHost(url.host());

  if (p == "javaEnabled")
     return Function (new NavigatorFunc(part));
  else if (p == "appCodeName")
    return String("Mozilla");
  else if (p == "appName") {
    // If we find "Mozilla" but not "(compatible, ...)" we are a real Netscape
    if (userAgent.find(QString::fromLatin1("Mozilla")) >= 0 &&
	userAgent.find(QString::fromLatin1("compatible")) == -1)
      return String("Netscape");
    if (userAgent.find(QString::fromLatin1("Microsoft")) >= 0 ||
	userAgent.find(QString::fromLatin1("MSIE")) >= 0)
      return String("Microsoft Internet Explorer");
    return String("Konqueror");
  } else if (p == "appVersion"){
    // We assume the string is something like Mozilla/version (properties)
    return String(userAgent.mid(userAgent.find('/') + 1));
  } else if (p == "language") {
    return String(KGlobal::locale()->language() == "C" ? QString::fromLatin1("en") : KGlobal::locale()->language());
  } else if (p == "userAgent") {
    return String(userAgent);
  } else if (p == "platform") {
    // danimo: yet another evil hack, but necessary to spoof some sites...
    return String((userAgent.find(QString::fromLatin1("Win"),0,false)==-1) ?
           QString::fromLatin1("X11") : QString::fromLatin1("Win32"));
  } else if (p == "plugins")
    return KJSO(new Plugins());
  else
    return Undefined();
}

KJSO Plugins::get(const UString &p) const
{
  if (p == "refresh")
    return Function(new PluginsFunc());

  return Undefined();
}

Completion PluginsFunc::tryExecute(const List &)
{
  return Completion(Normal, Undefined());
}

Completion NavigatorFunc::execute(const List &)
{
  // javaEnabled()
  return Completion(Normal, Boolean(part->javaEnabled()));
}
