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

#ifndef _KJSSTRING_H_
#define _KJSSTRING_H_

#include "global.h"

namespace DOM {
  class DOMString;
};
class QString;

namespace KJS {

  class CString {
  public:
    CString();
    CString(char c);
    CString(const char *c);
    CString(const CString &);
    CString(const CString *);
    // conversions
    CString(int i);
    CString(unsigned int u);

    ~CString();

    CString &append(const CString &);
    CString &operator=(const char *c);
    CString &operator=(const CString &);
    CString &operator+=(const CString &);

    int length() const;
    void resize(unsigned int l);

    const char * ascii() const;
  private:
    char *data;
  };

  class UString {
  public:
    UString();
    UString(char c);
    UString(const char *c);
    UString(const UnicodeChar *c, int length);
    UString(const UString &);
    UString(const UString *);
    UString(const DOM::DOMString &);

    ~UString();

    void append(const UString &);

    // to be implemented when needed, e.g. in a module
    DOM::DOMString string() const;
    QString qstring() const;

    UString &operator=(const char *c);
    UString &operator=(const UString &);

    const UnicodeChar* unicode() const;
    const char* ascii() const;
    int length() const;
    UnicodeChar operator[](unsigned int pos) const;
  private:
    UnicodeChar *s;
    unsigned int l;
  };

  bool operator==(const UString& s1, const UString& s2);
  bool operator==(const CString& s1, const CString& s2);
  UString operator+(const UString& s1, const UString& s2);

}; // namespace

#endif
