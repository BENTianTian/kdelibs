// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *
 */

#ifndef _KJS_FUNCTION_H_
#define _KJS_FUNCTION_H_

#include "internal.h"
#include "array_instance.h"

namespace KJS {

  class Parameter;

  /**
   * Base class for all function objects.
   * It implements the hasInstance method (for instanceof, which only applies to function objects)
   * and allows to give the function a name, used in toString().
   *
   * Constructors and prototypes of internal objects (implemented in C++) directly inherit from this.
   * FunctionImp also does, for functions implemented in JS.
   */
  class InternalFunctionImp : public ObjectImp {
  public:
    /**
     * Constructor. For C++-implemented functions, @p funcProto is usually
     * static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype().imp())
     */
    InternalFunctionImp(FunctionPrototypeImp *funcProto);
    bool implementsHasInstance() const;
    Boolean hasInstance(ExecState *exec, const Value &value);

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
    Identifier name() const { return ident; }
    /// You might want to use the helper function ObjectImp::setFunctionName for this
    void setName(Identifier _ident) { ident = _ident; }

  protected:
    Identifier ident;
  };

  /**
   * @short Implementation class for functions implemented in JS.
   */
  class FunctionImp : public InternalFunctionImp {
    friend class Function;
    friend class ActivationImp;
  public:
    FunctionImp(ExecState *exec, const Identifier &n = Identifier::null());
    virtual ~FunctionImp();

    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, const Value &value, int attr = None);
    virtual bool hasProperty(ExecState *exec, const Identifier &propertyName) const;
    virtual bool deleteProperty(ExecState *exec, const Identifier &propertyName);

    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    void addParameter(const Identifier &n);
    Identifier parameterProperty(int index) const;
    // parameters in string representation, e.g. (a, b, c)
    UString parameterString() const;
    virtual CodeType codeType() const = 0;

    virtual Completion execute(ExecState *exec) = 0;
    int firstLine() const { return line0; }
    int lastLine() const { return line1; }
    int sourceId() const { return sid; }

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    Parameter *param;
    int line0;
    int line1;
    int sid;

  private:
    void processParameters(ExecState *exec, const List &);
    virtual void processVarDecls(ExecState *exec);
  };

  class DeclaredFunctionImp : public FunctionImp {
  public:
    DeclaredFunctionImp(ExecState *exec, const Identifier &n,
			FunctionBodyNode *b, const ScopeChain &sc);
    ~DeclaredFunctionImp();

    bool implementsConstruct() const;
    Object construct(ExecState *exec, const List &args);

    virtual Completion execute(ExecState *exec);
    CodeType codeType() const { return FunctionCode; }
    FunctionBodyNode *body;

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  private:
    virtual void processVarDecls(ExecState *exec);
  };

  class ActivationImp;

  class ArgumentsImp : public ObjectImp {
  public:
    ArgumentsImp(ExecState *exec, FunctionImp *func, const List &args, ActivationImp *act);

    virtual void mark();

    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual void put(ExecState *exec, const Identifier &propertyName,
		     const Value &value, int attr = None);

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;

  private:
    ActivationImp *activation;
  };

  class ActivationImp : public ObjectImp {
  public:
    ActivationImp(FunctionImp *function, const List &arguments);

    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual bool hasProperty(ExecState *exec, const Identifier &propertyName) const;
    virtual bool deleteProperty(ExecState *exec, const Identifier &propertyName);

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;

    virtual void mark();

  private:
    FunctionImp *_function;
    List _arguments;
    mutable ArgumentsImp *_argumentsObject;
  };

  class GlobalFuncImp : public InternalFunctionImp {
  public:
    GlobalFuncImp(ExecState *exec, FunctionPrototypeImp *funcProto,
		  int i, int len, const Identifier &_ident);
    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);
    virtual CodeType codeType() const;
    enum { Eval, ParseInt, ParseFloat, IsNaN, IsFinite, DecodeURI, DecodeURIComponent,
	   EncodeURI, EncodeURIComponent, Escape, UnEscape, KJSPrint };
  private:
    int id;
  };



} // namespace

#endif
