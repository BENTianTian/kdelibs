/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <strings.h>
#include <iostream.h>

#include "object.h"

using namespace KJS;

KJSProperty::KJSProperty(const CString &n, KJSO *o, int attr)
{
  name = n;
  object = o->ref();
  attribute = attr;
}

// ECMA 8.6.2.1
KJSO *KJSO::get(const CString &p) const
{
  if (prop) {
    KJSProperty *pr = prop;
    while (pr) {
      if (pr->name == p)
	return pr->object->ref();
      pr = pr->next;
    }
  }
  if (!prototype())
    return new KJSUndefined();

  return prototype()->get(p);
}

// ECMA 8.6.2.2
void KJSO::put(const CString &p, KJSO *v, int attr)
{
  if (!canPut(p))
    return;

  KJSProperty *pr;

  if (prop) {
    pr = prop;
    while (pr) {
      if (pr->name == p) {
	// replace old value
	pr->object = v->ref();
	pr->attribute = attr;
	return;
      }
      pr = pr->next;
    }
  }

  // add new property
  pr = new KJSProperty(p, v, attr);
  pr->next = prop;
  prop = pr;
}

// ECMA 8.6.2.3
bool KJSO::canPut(const CString &p) const
{
  if (prop) {
    KJSProperty *pr = prop;
    while (pr) {
      if (pr->name == p)
	return !(pr->attribute & ReadOnly);
      pr = pr->next;
    }
  }
  if (!prototype())
    return true;

  return prototype()->canPut(p);
}

// ECMA 8.6.2.4
bool KJSO::hasProperty(const CString &p) const
{
  if (!prop)
    return false;

  KJSProperty *pr = prop;
  while (pr) {
    if (pr->name == p)
      return true;
    pr = pr->next;
  }

  if (!prototype())
    return false;

  return prototype()->hasProperty(p);
}

// ECMA 8.6.2.5
void KJSO::deleteProperty(const CString &p)
{
  if (prop) {
    KJSProperty *pr = prop;
    KJSProperty **prev = &prop;
    while (pr) {
      if (pr->name == p) {
	*prev = pr->next;
	delete pr;
	return;
      }
      prev = &(pr->next);
      pr = pr->next;
    }
  }
}

// ECMA 8.6.2.6 (new draft)
KJSO* KJSO::defaultValue(Hint hint)
{
  Ptr o, s;

  if (hint == NoneHint)
    hint = NumberHint;

  if (hint == StringHint)
    o = get("toString");
  else
    o = get("valueOf");

  if (o->implementsCall()) { // spec says "not primitive type" but ...
    s = o->executeCall(this, 0L);
    if (!s->isA(Object))
      return s->ref();
  }

  if (hint == StringHint)
    o = get("valueOf");
  else
    o = get("toString");

  if (o->implementsCall()) {
    s = o->executeCall(this, 0L);
    if (!s->isA(Object))
      return s->ref();
  }

  return new KJSError(ErrNoDefault, this);
}

