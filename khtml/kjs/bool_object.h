/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#ifndef _BOOL_OBJECT_H_
#define _BOOL_OBJECT_H_

#include "object.h"

namespace KJS {

  enum { IDBool2S, IDBoolvalOf };

  class BooleanObject : public KJSInternalFunction {
  public:
    BooleanObject(KJSPrototype *boolProto, KJSPrototype *funcProto);
    KJSO* execute();
  };

  class BooleanConstructor : public KJSConstructor {
  public:
    BooleanConstructor(KJSPrototype *bp, KJSPrototype *fp);
    KJSObject* construct(KJSArgList *);
  private:
    KJSPrototype *boolProto;
  };

  class BooleanPrototype : public KJSPrototype {
  public:
    BooleanPrototype(KJSPrototype *objProto, KJSPrototype *funcProto);
  };

  class BooleanProtoFunc : public KJSInternalFunction {
  public:
    BooleanProtoFunc(int i, KJSPrototype *funcProto);
    KJSO *execute();
  private:
    int id;
  };

}; // namespace

#endif
