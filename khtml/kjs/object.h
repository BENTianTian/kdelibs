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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <assert.h>

#include "global.h"
#include "kjsstring.h"

namespace KJS {

union Value {
  bool b;
  double d;
  UString *s;
  Compl c;
};

// forward declarations
class KJSProperty;
class KJSArgList;
class KJSParamList;
class Node;
class StatementNode;

class KJSO {
public:
  KJSO() { init(); }
  virtual ~KJSO();
public:
  void init();
  virtual Type type() const { assert(!"Undefined type()"); return Undefined; };
  bool isA(Type t) const { return (type() == t); }
  bool isObject() const { return (type() >= Object); }

#ifdef KJS_DEBUG_MEM
  static int count;
  static KJSO* firstObject;
  KJSO* nextObject, *prevObject;
  int objId;
  static int lastId;
#endif

  // Properties
  KJSO *prototype() const { return proto; }
  Class getClass() const { return cl; }
  KJSO *get(const CString &p) const;
  bool hasProperty(const CString &p) const;
  void put(const CString &p, KJSO *v, int attr = None);
  bool canPut(const CString &p) const;
  void deleteProperty(const CString &p);
  KJSO *defaultValue(Hint hint = NoneHint);
  void dump(int level = 0);
  virtual KJSO *construct() { return 0L; }

  //private:
public:
  int refCount;
public:
  KJSO *ref() { refCount++; return this; }
  void deref() { assert(refCount > 0); if(!--refCount) delete this; }

  // Reference
  KJSO *getBase();
  CString getPropertyName();
  KJSO *getValue();
  ErrorCode putValue(KJSO *v);

  // internal value
  bool bVal() { assert(type()==Boolean); return val.b; }
  double dVal()  { assert(type()==Number); return val.d; }
  const UString sVal()  { assert(type()==String); return *(val.s); }
  Compl cVal() { assert(type()==Completion); return val.c; }
  bool isValueCompletion() { assert(type()==Completion); return (complVal); }
  KJSO *complValue() { assert(type()==Completion); return complVal; }

  // function call
  bool implementsCall() const { return (type() == InternalFunction ||
				  type() == DeclaredFunction ||
				  type() == AnonymousFunction); }
  KJSO *executeCall(KJSO *thisV, KJSArgList *args);
  KJSO* (*call)(KJSO*);

  // constructor
  bool implementsConstruct() const { return true; /* TODO */ }
  KJSO *construct(KJSO *) { /* TODO */ return 0L; }

protected:
  KJSO *proto;
  Class cl;
  KJSProperty *prop;

protected:
  Value val;

  // references:
protected:
  KJSO *base;
  CString propname;

  // completion:
protected:
  KJSO *complVal;
};

class Ptr {
public:
  Ptr() { obj = 0L; }
  Ptr(KJSO *o) { assert(o); obj = o; }
  ~Ptr() { if (obj) obj->deref(); }
  Ptr *operator=(KJSO *o) { if (obj) obj->deref(); obj = o; return this; }
  KJSO* operator->() { return obj; }

  operator KJSO*() { return obj; }
  void release() { obj->deref(); obj = 0L; }
  KJSO *ref() { obj->ref(); return obj; }
  Ptr(const Ptr &) { assert(0); }
private:
  Ptr& operator=(const Ptr &);
  KJSO *obj;
};

KJSO *zeroRef(KJSO *obj);

class KJSProperty {
public:
  KJSProperty(const CString &n, KJSO *o, int attr = None);
  Type type() const { return Property; }
public:
  CString name;
  int attribute;
  Ptr object;
  KJSProperty *next;
};


class KJSReference : public KJSO {
public:
  KJSReference(KJSO *b, const CString &s);
  ~KJSReference();
  Type type() const { return Reference; }
};

class KJSNull : public KJSO {
public:
  KJSNull() { }
  Type type() const { return Null; }
};

class KJSNumber : public KJSO {
public:
  KJSNumber(int i) { val.d = static_cast<double>(i); }
  KJSNumber(double d) { val.d = d; }
  Type type() const { return Number; }
};

class KJSString : public KJSO {
public:
  KJSString(const UString &s) { val.s = new UString(s); }
  Type type() const { return String; }
};

class KJSUndefined : public KJSO {
public:
  Type type() const { return Undefined; }

};

class KJSBoolean : public KJSO {
public:
  KJSBoolean(bool b) { val.b = b; }
  Type type() const { return Boolean; }
};

typedef KJSO* (*fPtr)();

class KJSFunction : public KJSO {
public:
  KJSFunction() { attr = ImplicitNone; }
  void processParameters(KJSArgList *);
  virtual KJSO* execute() = 0;
  virtual bool hasAttribute(FunctionAttribute a) const { return (attr & a); }
  virtual CodeType codeType() = 0;
protected:
  FunctionAttribute attr;
  KJSParamList *param;
};

class KJSInternalFunction : public KJSFunction {
public:
  KJSInternalFunction(KJSO* (*f)()) { param = 0L; func = f; }
  Type type() const { return InternalFunction; }
  KJSO* execute() { return (*func)(); }
  CodeType codeType() { return HostCode; }
private:
  KJSO* (*func)();
};

class KJSDeclaredFunction : public KJSFunction {
public:
  KJSDeclaredFunction(KJSParamList *p, StatementNode *b);
  Type type() const { return DeclaredFunction; }
  KJSO* execute();
  CodeType codeType() { return FunctionCode; }
private:
  StatementNode *block;
};

class KJSAnonymousFunction : public KJSFunction {
public:
  KJSAnonymousFunction() { /* TODO */ }
  Type type() const { return AnonymousFunction; }
  KJSO* execute() { /* TODO */ }
  CodeType codeType() { return AnonymousCode; }
};

class KJSCompletion : public KJSO {
public:
  KJSCompletion(Compl c, KJSO *v = 0L)
    { val.c = c; complVal = v ? v->ref() : 0L; }
  virtual ~KJSCompletion() { if (complVal) complVal->deref(); }
  Type type() const { return Completion; }
};

class KJSObject : public KJSO {
public:
  KJSObject() {}
  Type type() const { return Object; }
  virtual SubType subType() const = 0;
};

class KJSScope : public KJSO {
public:
  KJSScope(KJSO *o) : next(0L) { object = o->ref(); }
  Type type() const { return Scope; }
  void append(KJSO *o) { next = new KJSScope(o); next->next = 0L; }

  Ptr object;
  KJSScope *next;
};

class KJSActivation : public KJSO {
public:
  KJSActivation(KJSFunction *f, KJSArgList *args);
  virtual ~KJSActivation();
  Type type() const { return Object; }
private:
  KJSFunction *func;
};

class KJSArguments : public KJSO {
public:
  KJSArguments(KJSFunction *func, KJSArgList *args);
  Type type() const { return Object; }
};

class KJSContext {
public:
  KJSContext(CodeType type = GlobalCode, KJSContext *callingContext = 0L,
	     KJSFunction *func = 0L, KJSArgList *args = 0L, KJSO *thisV = 0L);
  virtual ~KJSContext();
  KJSScope *firstScope() const { return scopeChain; }
  void insertScope(KJSO *s);
  KJSScope *copyOfChain() { /* TODO */ return scopeChain; }

  KJSO *variableObject() const { return variable; }
public:
  KJSO *thisValue;
  KJSActivation *activation;
private:
  KJSO *variable;
  KJSScope *scopeChain;
};

class KJSGlobal : public KJSO {
public:
  KJSGlobal();
  Type type() const { return Object; }
private:
  static KJSO* eval();
};

class KJSArgList;

class KJSArg {
  friend KJSArgList;
public:
  KJSArg(KJSO *o) : next(0L) { obj = o->ref(); }
  KJSArg *nextArg() const { return next; }
  KJSO *object() { return obj; }
private:
  Ptr obj;
  KJSArg *next;
};

class KJSArgList {
public:
  KJSArgList() : first(0L) {}
  ~KJSArgList();
  KJSArgList *append(KJSO *o);
  KJSArg *firstArg() const { return first; }
  int count() const;
private:
  KJSArg *first;
};

class KJSParamList {
public:
  KJSParamList(int s) : size(s){ param = new CString[s]; }
  ~KJSParamList() { delete [] param; }
  int count() const { return size; }
  void insert(int i, const CString &s) { if (i<size) param[i] = s; }
  const char *at(int i) { if (i<size)
                            return param[i].ascii();
                          else
			    return 0L; }
private:
  int size;
  CString *param;
};

class KJSError : public KJSO {
public:
  KJSError(ErrorCode e, Node *n);
  KJSError(ErrorCode e, KJSO *o);
  Type type() const { return Error; }
  ErrorCode number() const { return errNo; }
private:
  ErrorCode errNo;
  int line;
};

};

#endif
