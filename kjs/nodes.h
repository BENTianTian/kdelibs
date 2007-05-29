// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Maksim Orlovich (maksim@kde.org)
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef NODES_H_
#define NODES_H_

#include "Parser.h"
#include "internal.h"
#include "operations.h"
#include <wtf/ListRefPtr.h>
#include <wtf/Vector.h>

namespace KJS {

  class ProgramNode;
  class PropertyNameNode;
  class PropertyListNode;
  class RegExp;
  class SourceElementsNode;
  class SourceStream;
  class PackageObject;
  class FuncDeclNode;
  class FunctionBodyNode;
  class Node;

  class VarDeclVisitor;
  class FuncDeclVisitor;

  class NodeVisitor {
  public:
    virtual ~NodeVisitor() {}
    /**
     This method should be overridden by subclasses to process nodes, and 
     perhaps return pointers for replacement nodes. If the node should not be 
     changed, return 0. Otherwise, return the replacement node.

     The default implementation asks the node to visit its kids, and do 
     replacements on them if needed, but does not change anything for this node
    */
    virtual Node* visit(Node* node);
  };

  class Node {
  public:
      enum NodeType {
          UnknownNodeType,
          NullNodeType,
          BooleanNodeType,
          NumberNodeType,
          StringNodeType,
          RegExpNodeType,
          TryNodeType,
          GroupNodeType
      };

    Node();
    virtual ~Node();

    virtual NodeType type() const { return UnknownNodeType; }

    virtual JSValue *evaluate(ExecState *exec) = 0;
    UString toString() const;
    virtual void streamTo(SourceStream&) const = 0;
    int lineNo() const { return m_line; }

    void ref();
    void deref();
    unsigned refcount();
    static void clearNewNodes();

    virtual Node *nodeInsideAllParens();

    virtual bool isLocation() const { return false; }
    virtual bool isResolveNode() const { return false; }
    virtual bool isBracketAccessorNode() const { return false; }
    virtual bool isDotAccessorNode() const { return false; }
    bool isNumber() const { return type() == NumberNodeType; }
    bool isString() const { return type() == StringNodeType; }
    bool isGroupNode() const { return type() == GroupNodeType; }
    bool isTryNode() const { return type() == TryNodeType; }
    virtual bool introducesNewStaticScope () const { return false; }
    virtual bool introducesNewDynamicScope() const { return false; }
    bool introducesNewScope() const { return introducesNewStaticScope() || introducesNewDynamicScope(); }

    virtual void breakCycle() { }

    // Processes all function and variable declarations below this node, 
    // adding them to symbol table or the current object depending on the 
    // execution context..
    void processDecls(ExecState*);
    
    //Marks nodes that do symbolic lookup
    virtual bool  isDynamicResolver() const { return false; }
    
    //Tries to create a better resolver for this node; may only be called if isDynamicResolver is true.
    //If this is the best possible, returns 'this'
    virtual Node* optimizeResolver(ExecState*, FunctionBodyNode*) const { assert(false); return 0; }

    /*
      Implementations of this method should call visitor->visit on all the 
      children nodes, and if they return value is non-0, update the link to the child.
      The recurseVisitLink helper takes care of this
    */
    virtual void recurseVisit(NodeVisitor * /*visitor*/) {}

    template<typename T>
    static void recurseVisitLink(NodeVisitor* visitor, RefPtr<T>& link)
    {
        if (!link)
          return;

        T* newLink = static_cast<T*>(visitor->visit(link.get()));
        if (newLink)
          link = newLink;
    }

    template<typename T>
    static void recurseVisitLink(NodeVisitor* visitor, ListRefPtr<T>& link)
    {
        if (!link)
          return;

        T* newLink = static_cast<T*>(visitor->visit(link.get()));
        if (newLink)
          link = newLink;
    }

    Completion createErrorCompletion(ExecState *, ErrorType, const char *msg);
    Completion createErrorCompletion(ExecState *, ErrorType, const char *msg, const Identifier &);

    JSValue *throwError(ExecState *, ErrorType, const char *msg);
    JSValue *throwError(ExecState *, ErrorType, const char *msg, const char* string);
    JSValue *throwError(ExecState *, ErrorType, const char *msg, JSValue *, Node *);
    JSValue *throwError(ExecState *, ErrorType, const char *msg, const Identifier &);
    JSValue *throwError(ExecState *, ErrorType, const char *msg, JSValue *, const Identifier &);
    JSValue *throwError(ExecState *, ErrorType, const char *msg, JSValue *, Node *, Node *);
    JSValue *throwError(ExecState *, ErrorType, const char *msg, JSValue *, Node *, const Identifier &);

    JSValue *throwUndefinedVariableError(ExecState *, const Identifier &);

    void handleException(ExecState*);
    void handleException(ExecState*, JSValue*);

  protected:
    int m_line;
  private:
    virtual void processVarDecl (ExecState* state);
    virtual void processFuncDecl(ExecState* state);
    friend class VarDeclVisitor;
    friend class FuncDeclVisitor;

    // disallow assignment
    Node& operator=(const Node&);
    Node(const Node &other);
  };

  class StatementNode : public Node {
  public:
    StatementNode();
    void setLoc(int line0, int line1);
    int firstLine() const { return lineNo(); }
    int lastLine() const { return m_lastLine; }
    bool hitStatement(ExecState*);
    virtual Completion execute(ExecState *exec) = 0;
    void pushLabel(const Identifier &id) { ls.push(id); }
  protected:
    LabelStack ls;
  private:
    JSValue *evaluate(ExecState*) { return jsUndefined(); }
    int m_lastLine;
  };

  class NullNode : public Node {
  public:
    NullNode() {}
    virtual NodeType type() const { return NullNodeType; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  };

  class BooleanNode : public Node {
  public:
    BooleanNode(bool v) : val(v) {}
    bool value() const { return val; }

    virtual NodeType type() const { return BooleanNodeType; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    bool val;
  };

  class NumberNode : public Node {
  public:
    NumberNode(double v) : val(v) {}
    double value() const { return val; }
    void setValue(double v) { val = v; }

    virtual NodeType type() const { return NumberNodeType; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    double val;
  };

  class StringNode : public Node {
  public:
    StringNode(const UString *v) : val(*v) { }
    UString value() const { return val; }
    void setValue(const UString& v) { val = v; }

    virtual NodeType type() const { return StringNodeType; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    UString val;
  };

  class RegExpNode : public Node {
  public:
    RegExpNode(const UString &p, const UString &f)
      : pattern(p), flags(f) { }
    virtual NodeType type() const { return RegExpNodeType; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    UString pattern, flags;
  };

  class ThisNode : public Node {
  public:
    ThisNode() {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  };


  struct ResolveResult {
     ResolveResult() : writeValue(0) {}
 
     JSValue* evalValue;
     JSValue* writeValue;
  };
  
  //Resolver template for nodes that can be looked up directly
  //in the active scope's Activation object by index.
  template<typename Handler> class StaticResolver: public Node {
  public:
    StaticResolver(uint32_t i, const Identifier& s, const Handler& h) : handler(h), ident(s), index(i) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;

    virtual bool isLocation() const {
      return handler.isLocation();
    }

    virtual bool isResolveNode() const {
      return false;
    }

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    Handler  handler;
    Identifier ident;
    uint32_t index;
  };
  
  //Resolver template for nodes that need symbolic lookup
  //to get their location..
  template<typename Handler> class DynamicResolver : public Node {
  public:
    DynamicResolver(const Identifier &s, const Handler& h) : handler(h), ident(s) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
      
    virtual bool isLocation() const {
      return handler.isLocation();
    }
    
    virtual bool isResolveNode() const {
      return handler.isResolveNode();
    }

    virtual void recurseVisit(NodeVisitor *visitor);

    virtual bool  isDynamicResolver() const { return true; }
    virtual Node* optimizeResolver(ExecState *exec, FunctionBodyNode* node) const;
    const Identifier& identifier() const { return ident; }
  private:
    Handler    handler;
    Identifier ident;
  };

  template<typename Handler> class NonLocalResolver : public Node {
  public:
    NonLocalResolver(const Identifier &s, const Handler& h) : handler(h), ident(s) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;

    virtual bool isLocation() const {
      return handler.isLocation();
    }

    virtual bool isResolveNode() const {
      return false;
    }

    virtual void recurseVisit(NodeVisitor *visitor);

    const Identifier& identifier() const { return ident; }
  private:
    Handler    handler;
    Identifier ident;
  };


  class ResolveHandlerBase {
  public:
    bool isLocation() const { return false; }
    bool isResolveNode() const { return false; }

    //A subclass should also implement the following:
    //void streamTo(SourceStream&, const Identifier& ident) const;
    //void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident);

    //In this case, the scope pointer points to the bottom of the scope chain, since some 
    //operations (e.g. assign) use that..
    //void handleResolveFailure(ResolveResult& res, ExecState* exec, JSObject* scope, const Identifier& ident) = 0;

    void recurseVisit(NodeVisitor * /*visitor*/) {}
  };

  class ResolveIdentifier : public ResolveHandlerBase {
  public:
    bool isLocation() const { return true; }
    bool isResolveNode() const { return true; }
  
    void streamTo(SourceStream&, const Identifier& ident) const;
    void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident) {
        res.evalValue = slot.getValue(exec, scope, ident);
    }
    void handleResolveFailure(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, const Identifier& ident);
  };

  class GroupNode : public Node {
  public:
    GroupNode(Node *g) : group(g) { }
    virtual NodeType type() const { return GroupNodeType; }
    virtual JSValue* evaluate(ExecState*);
    virtual Node *nodeInsideAllParens();
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> group;
  };

  class ElementNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ArrayNode ctor
    ElementNode(int e, Node *n) : next(this), elision(e), node(n) { Parser::noteNodeCycle(this); }
    ElementNode(ElementNode *l, int e, Node *n)
      : next(l->next), elision(e), node(n) { l->next = this; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<ElementNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ArrayNode;
    ListRefPtr<ElementNode> next;
    int elision;
    RefPtr<Node> node;
  };

  class ArrayNode : public Node {
  public:
    ArrayNode(int e) : elision(e), opt(true) { }
    ArrayNode(ElementNode *ele)
      : element(ele->next.release()), elision(0), opt(false) { Parser::removeNodeCycle(element.get()); }
    ArrayNode(int eli, ElementNode *ele)
      : element(ele->next.release()), elision(eli), opt(true) { Parser::removeNodeCycle(element.get()); }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<ElementNode> element;
    int elision;
    bool opt;
  };

  class PropertyNameNode : public Node {
  public:
    PropertyNameNode(double d) : numeric(d) { }
    PropertyNameNode(const Identifier &s) : str(s) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    double numeric;
    Identifier str;
  };

  class PropertyNode : public Node {
  public:
    enum Type { Constant, Getter, Setter };
    PropertyNode(PropertyNameNode *n, Node *a, Type t)
      : name(n), assign(a), type(t) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    friend class PropertyListNode;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<PropertyNameNode> name;
    RefPtr<Node> assign;
    Type type;
  };

  class PropertyListNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ObjectLiteralNode ctor
    PropertyListNode(PropertyNode *n)
      : node(n), next(this) { Parser::noteNodeCycle(this); }
    PropertyListNode(PropertyNode *n, PropertyListNode *l)
      : node(n), next(l->next) { l->next = this; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<PropertyListNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ObjectLiteralNode;
    RefPtr<PropertyNode> node;
    ListRefPtr<PropertyListNode> next;
  };

  class ObjectLiteralNode : public Node {
  public:
    ObjectLiteralNode() { }
    ObjectLiteralNode(PropertyListNode *l) : list(l->next.release()) { Parser::removeNodeCycle(list.get()); }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<PropertyListNode> list;
  };

  class BracketAccessorNode : public Node {
  public:
    BracketAccessorNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;

    virtual bool isLocation() const { return true; }
    virtual bool isBracketAccessorNode() const { return true; }
    Node *base() { return expr1.get(); }
    Node *subscript() { return expr2.get(); }

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
  };

  class DotAccessorNode : public Node {
  public:
    DotAccessorNode(Node *e, const Identifier &s) : expr(e), ident(s) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;

    virtual bool isLocation() const { return true; }
    virtual bool isDotAccessorNode() const { return true; }
    Node *base() const { return expr.get(); }
    const Identifier& identifier() const { return ident; }

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
    Identifier ident;
  };

  class ArgumentListNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ArgumentsNode ctor
    ArgumentListNode(Node *e) : next(this), expr(e) { Parser::noteNodeCycle(this); }
    ArgumentListNode(ArgumentListNode *l, Node *e)
      : next(l->next), expr(e) { l->next = this; }
    JSValue* evaluate(ExecState*);
    List evaluateList(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<ArgumentListNode> releaseNext() { return next.release(); }
    virtual void breakCycle();

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ArgumentsNode;
    ListRefPtr<ArgumentListNode> next;
    RefPtr<Node> expr;
  };

  class ArgumentsNode : public Node {
  public:
    ArgumentsNode() { }
    ArgumentsNode(ArgumentListNode *l)
      : list(l->next.release()) { Parser::removeNodeCycle(list.get()); }
    JSValue* evaluate(ExecState*);
    List evaluateList(ExecState *exec) { return list ? list->evaluateList(exec) : List::empty(); }
    virtual void streamTo(SourceStream&) const;

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<ArgumentListNode> list;
  };

  class NewExprNode : public Node {
  public:
    NewExprNode(Node *e) : expr(e) {}
    NewExprNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallValueNode : public Node {
  public:
    FunctionCallValueNode(Node *e, ArgumentsNode *a) : expr(e), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
    RefPtr<ArgumentsNode> args;
  };

  class ResolveFunctionCall : public ResolveHandlerBase {
  public:
    ResolveFunctionCall(ArgumentsNode *a) : args(a) {}
    void streamTo(SourceStream&, const Identifier& ident) const;
    void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident);
    void handleResolveFailure(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, const Identifier& ident);
    void recurseVisit(NodeVisitor * visitor);
  private:
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallBracketNode : public Node {
  public:
    FunctionCallBracketNode(Node *b, Node *s, ArgumentsNode *a) : base(b), subscript(s), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> base;
    RefPtr<Node> subscript;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallParenBracketNode : public FunctionCallBracketNode {
  public:
    FunctionCallParenBracketNode(Node *b, Node *s, ArgumentsNode *a) : FunctionCallBracketNode(b, s, a) {}
    virtual void streamTo(SourceStream&) const;
  };

  class FunctionCallDotNode : public Node {
  public:
    FunctionCallDotNode(Node *b, const Identifier &i, ArgumentsNode *a) : base(b), ident(i), args(a) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> base;
    Identifier ident;
    RefPtr<ArgumentsNode> args;
  };

  class FunctionCallParenDotNode : public FunctionCallDotNode {
  public:
    FunctionCallParenDotNode(Node *b, const Identifier &i, ArgumentsNode *a) : FunctionCallDotNode(b, i, a) {}
    virtual void streamTo(SourceStream&) const;
  };

  class ResolvePostfix : public ResolveHandlerBase {
  public:
    ResolvePostfix(Operator o) : m_oper(o) {}
    void streamTo(SourceStream&, const Identifier& ident) const;
    void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident);
    void handleResolveFailure(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, const Identifier& ident);
  private:
    Operator m_oper;
  };

  class PostfixBracketNode : public Node {
  public:
    PostfixBracketNode(Node *b, Node *s, Operator o) : m_base(b), m_subscript(s), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_base;
    RefPtr<Node> m_subscript;
    Operator m_oper;
  };

  class PostfixDotNode : public Node {
  public:
    PostfixDotNode(Node *b, const Identifier& i, Operator o) : m_base(b), m_ident(i), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_base;
    Identifier m_ident;
    Operator m_oper;
  };

  class PostfixErrorNode : public Node { 
  public: 
    PostfixErrorNode(Node* e, Operator o) : m_expr(e), m_oper(o) {} 
    JSValue* evaluate(ExecState*); 
    virtual void streamTo(SourceStream&) const; 
  private: 
    RefPtr<Node> m_expr; 
    Operator m_oper; 
  }; 

  class ResolveDelete : public ResolveHandlerBase {
  public:
    void streamTo(SourceStream&, const Identifier& ident) const;
    void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident);
    void handleResolveFailure(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, const Identifier& ident);
  };

  class DeleteBracketNode : public Node {
  public:
    DeleteBracketNode(Node *base, Node *subscript) : m_base(base), m_subscript(subscript) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_base;
    RefPtr<Node> m_subscript;
  };

  class DeleteDotNode : public Node {
  public:
    DeleteDotNode(Node *base, const Identifier& i) : m_base(base), m_ident(i) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_base;
    Identifier m_ident;
  };

  class DeleteValueNode : public Node {
  public:
    DeleteValueNode(Node *e) : m_expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_expr;
  };

  class VoidNode : public Node {
  public:
    VoidNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class ResolveTypeOf : public ResolveHandlerBase {
  public:
    void streamTo(SourceStream&, const Identifier& ident) const;
    void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident);
    void handleResolveFailure(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, const Identifier& ident);
  };

  class TypeOfValueNode : public Node {
  public:
    TypeOfValueNode(Node *e) : m_expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_expr;
  };

  class ResolvePrefix : public ResolveHandlerBase {
  public:
    ResolvePrefix(Operator o) : m_oper(o) {}
    void streamTo(SourceStream&, const Identifier& ident) const;
    void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident);
    void handleResolveFailure(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, const Identifier& ident);
  private:
    Operator m_oper;
  };

  class PrefixErrorNode : public Node { 
  public: 
    PrefixErrorNode(Node* e, Operator o) : m_expr(e), m_oper(o) {} 
    JSValue* evaluate(ExecState*); 
    virtual void streamTo(SourceStream&) const; 
  private: 
    RefPtr<Node> m_expr; 
    Operator m_oper; 
  }; 

  class PrefixBracketNode : public Node {
  public:
    PrefixBracketNode(Node *b, Node *s, Operator o) : m_base(b), m_subscript(s), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_base;
    RefPtr<Node> m_subscript;
    Operator m_oper;
  };

  class PrefixDotNode : public Node {
  public:
    PrefixDotNode(Node *b, const Identifier& i, Operator o) : m_base(b), m_ident(i), m_oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> m_base;
    Identifier m_ident;
    Operator m_oper;
  };

  class UnaryPlusNode : public Node {
  public:
    UnaryPlusNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class NegateNode : public Node {
  public:
    NegateNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class BitwiseNotNode : public Node {
  public:
    BitwiseNotNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class LogicalNotNode : public Node {
  public:
    LogicalNotNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class BinaryOperatorNode : public Node {
  public:
    BinaryOperatorNode(Node* e1, Node* e2, Operator op)
      : expr1(e1), expr2(e2), oper(op) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor* visitor);
  private:
    JSValue* operatorIn(ExecState* exec, JSValue* v1, JSValue* v2);
    JSValue* operatorInstanceOf(ExecState* exec, JSValue* v1, JSValue* v2);
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
    Operator oper;
  };

  /**
   * expr1 && expr2, expr1 || expr2
   */
  class BinaryLogicalNode : public Node {
  public:
    BinaryLogicalNode(Node *e1, Operator o, Node *e2) :
      expr1(e1), expr2(e2), oper(o) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
    Operator oper;
  };

  /**
   * The ternary operator, "logical ? expr1 : expr2"
   */
  class ConditionalNode : public Node {
  public:
    ConditionalNode(Node *l, Node *e1, Node *e2) :
      logical(l), expr1(e1), expr2(e2) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> logical;
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
  };

  class AssignErrorNode : public Node { 
  public: 
    AssignErrorNode(Node* left, Operator oper, Node* right) 
      : m_left(left), m_oper(oper), m_right(right) {} 
    JSValue* evaluate(ExecState*); 
    virtual void streamTo(SourceStream&) const; 
  protected: 
    RefPtr<Node> m_left; 
    Operator m_oper; 
    RefPtr<Node> m_right;
  };

  class ResolveAssign : public ResolveHandlerBase {
  public:
    ResolveAssign(Operator oper, Node *right)
      : m_oper(oper), m_right(right) {}
    void streamTo(SourceStream&, const Identifier& ident) const;
    void handleResolveSuccess(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, PropertySlot& slot, const Identifier& ident);
    void handleResolveFailure(ResolveResult& res, Node* node, ExecState* exec, JSObject* scope, const Identifier& ident);
    void recurseVisit(NodeVisitor * visitor);
  protected:
    Operator m_oper;
    RefPtr<Node> m_right;
  };

  class AssignBracketNode : public Node {
  public:
    AssignBracketNode(Node *base, Node *subscript, Operator oper, Node *right)
      : m_base(base), m_subscript(subscript), m_oper(oper), m_right(right) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> m_base;
    RefPtr<Node> m_subscript;
    Operator m_oper;
    RefPtr<Node> m_right;
  };

  class AssignDotNode : public Node {
  public:
    AssignDotNode(Node *base, const Identifier& ident, Operator oper, Node *right)
      : m_base(base), m_ident(ident), m_oper(oper), m_right(right) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<Node> m_base;
    Identifier m_ident;
    Operator m_oper;
    RefPtr<Node> m_right;
  };

  class CommaNode : public Node {
  public:
    CommaNode(Node *e1, Node *e2) : expr1(e1), expr2(e2) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
  };

  class AssignExprNode : public Node {
  public:
    AssignExprNode(Node *e) : expr(e) {}
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class VarDeclNode : public Node {
  public:
    enum Type { Variable, Constant };
    VarDeclNode(const Identifier &id, AssignExprNode *in, Type t);
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);

    virtual void processVarDecl(ExecState*);
  private:
    Type varType;
    Identifier ident;
    RefPtr<AssignExprNode> init;
  };

  class VarDeclListNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the ForNode/VarStatementNode ctor
    VarDeclListNode(VarDeclNode *v) : next(this), var(v) { Parser::noteNodeCycle(this); }
    VarDeclListNode(VarDeclListNode *l, VarDeclNode *v)
      : next(l->next), var(v) { l->next = this; }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<VarDeclListNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class ForNode;
    friend class VarStatementNode;
    ListRefPtr<VarDeclListNode> next;
    RefPtr<VarDeclNode> var;
  };

  class VarStatementNode : public StatementNode {
  public:
    VarStatementNode(VarDeclListNode *l) : next(l->next.release()) { Parser::removeNodeCycle(next.get()); }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<VarDeclListNode> next;
  };

  class BlockNode : public StatementNode {
  public:
    BlockNode(SourceElementsNode *s);
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  protected:
    RefPtr<SourceElementsNode> source;
  };

  class EmptyStatementNode : public StatementNode {
  public:
    EmptyStatementNode() { } // debug
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
  };

  class ExprStatementNode : public StatementNode {
  public:
    ExprStatementNode(Node *e) : expr(e) { }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class IfNode : public StatementNode {
  public:
    IfNode(Node *e, StatementNode *s1, StatementNode *s2)
      : expr(e), statement1(s1), statement2(s2) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
    RefPtr<StatementNode> statement1;
    RefPtr<StatementNode> statement2;
  };

  class DoWhileNode : public StatementNode {
  public:
    DoWhileNode(StatementNode *s, Node *e) : statement(s), expr(e) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<StatementNode> statement;
    RefPtr<Node> expr;
  };

  class WhileNode : public StatementNode {
  public:
    WhileNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
    RefPtr<StatementNode> statement;
  };

  class ForNode : public StatementNode {
  public:
    ForNode(Node *e1, Node *e2, Node *e3, StatementNode *s) :
      expr1(e1), expr2(e2), expr3(e3), statement(s) {}
    ForNode(VarDeclListNode *e1, Node *e2, Node *e3, StatementNode *s) :
      expr1(e1->next.release()), expr2(e2), expr3(e3), statement(s) { Parser::removeNodeCycle(expr1.get()); }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr1;
    RefPtr<Node> expr2;
    RefPtr<Node> expr3;
    RefPtr<StatementNode> statement;
  };

  class ForInNode : public StatementNode {
  public:
    ForInNode(Node *l, Node *e, StatementNode *s);
    ForInNode(const Identifier &i, AssignExprNode *in, Node *e, StatementNode *s);
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    Identifier ident;
    RefPtr<AssignExprNode> init;
    RefPtr<Node> lexpr;
    RefPtr<Node> expr;
    RefPtr<VarDeclNode> varDecl;
    RefPtr<StatementNode> statement;
  };

  class ContinueNode : public StatementNode {
  public:
    ContinueNode() { }
    ContinueNode(const Identifier &i) : ident(i) { }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    Identifier ident;
  };

  class BreakNode : public StatementNode {
  public:
    BreakNode() { }
    BreakNode(const Identifier &i) : ident(i) { }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
  private:
    Identifier ident;
  };

  class ReturnNode : public StatementNode {
  public:
    ReturnNode(Node *v) : value(v) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> value;
  };

  class WithNode : public StatementNode {
  public:
    WithNode(Node *e, StatementNode *s) : expr(e), statement(s) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool introducesNewDynamicScope() const { return true; }
  private:
    RefPtr<Node> expr;
    RefPtr<StatementNode> statement;
  };

  class LabelNode : public StatementNode {
  public:
    LabelNode(const Identifier &l, StatementNode *s) : label(l), statement(s) { }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    Identifier label;
    RefPtr<StatementNode> statement;
  };

  class ThrowNode : public StatementNode {
  public:
    ThrowNode(Node *e) : expr(e) {}
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    RefPtr<Node> expr;
  };

  class TryNode : public StatementNode {
  public:
    TryNode(StatementNode *b, const Identifier &e, StatementNode *c, StatementNode *f)
      : tryBlock(b), exceptionIdent(e), catchBlock(c), finallyBlock(f) { }
    virtual NodeType type() const { return TryNodeType; }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);

    void recurseVisitNonCatch(NodeVisitor *visitor);
  private:
    RefPtr<StatementNode> tryBlock;
    Identifier exceptionIdent;
    RefPtr<StatementNode> catchBlock;
    RefPtr<StatementNode> finallyBlock;
  };

  class ParameterNode : public Node {
  public:
    // list pointer is tail of a circular list, cracked in the FuncDeclNode/FuncExprNode ctor
    ParameterNode(const Identifier &i) : id(i), next(this) { Parser::noteNodeCycle(this); }
    ParameterNode(ParameterNode *next, const Identifier &i)
      : id(i), next(next->next) { next->next = this; }
    JSValue* evaluate(ExecState*);
    const Identifier& ident() const { return id; }
    ParameterNode *nextParam() const { return next.get(); }
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<ParameterNode> releaseNext() { return next.release(); }
    virtual void breakCycle();

    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class FuncDeclNode;
    friend class FuncExprNode;
    Identifier id;
    ListRefPtr<ParameterNode> next;
  };

  // inherited by ProgramNode

  /**
   This AST node corresponds to the function body in the AST, but is used to 
   keep track of much of the information relevant to the whole function,
   such as parameter names and symbol tables. This is because there are both function 
   declarations and expressions, so there is no natural single place to put this stuff 
   above the body
  */
  class FunctionBodyNode : public BlockNode {
  private:
    struct Symbol {
      Symbol(const Identifier& _name, int _attr, FuncDeclNode* _funcDecl) : name(_name), funcDecl(_funcDecl), attr(_attr) {}
      Symbol() {}
      Identifier    name;
      FuncDeclNode* funcDecl;
      int           attr;
    };

    struct Parameter {
      Parameter() {}
      Parameter(const Identifier &n, int id) : name(n), symbolID(id) { }
      Identifier name;
      int        symbolID;
    };
  public:
    FunctionBodyNode(SourceElementsNode *);
    int sourceId() { return m_sourceId; }
    const UString& sourceURL() { return m_sourceURL; }
    
    //////////////////////////////////////////////////////////////////////
    // Symbol table functions
    //////////////////////////////////////////////////////////////////////
    bool builtSymbolList()    const { return m_builtSymbolList; }
    void setBuiltSymbolList()       { m_builtSymbolList = true; }
    int  numLocals()          const { return m_symbolList.size(); }
    int  getLocalAttr(int id) const { return m_symbolList[id-1].attr; }
    FuncDeclNode* getLocalFuncDecl(int id) const { return m_symbolList[id-1].funcDecl; }
    
    //Returns -1 if not found..
    int  lookupSymbolID(const Identifier &ident) const;
     
    //////////////////////////////////////////////////////////////////////
    // Parameter array functions
    //////////////////////////////////////////////////////////////////////
    void addParam(const Identifier& ident); //Also creates the symbol for the
                                            //parameter, caled on creating
                                            //the enclosing expr or decl

    void addVarDecl(const Identifier& ident, int attr, ExecState* exec);
    void addFunDecl(const Identifier& ident, int attr, FuncDeclNode* funcDecl);

    int numParams() const { return m_paramList.size(); }
    int paramSymbolID(int pos) const { return m_paramList[pos].symbolID; }
    const Identifier& paramName(int pos) const { return m_paramList[pos].name; }
   
  private:
    UString m_sourceURL;
    int m_sourceId : 31;
    bool m_builtSymbolList : 1;

    int  addSymbol(const Identifier& ident, int attr, FuncDeclNode* funcDecl = 0);

    // Note: the convention here is that the position 0
    // is the index 1, position 1 is the index 2, etc.
    WTF::Vector<Symbol> m_symbolList;
    
    // This is the map of the above, the jsValues are the IDs.
    // ### ugh. Lazy-lazy-lazy. But this works, too.
    PropertyMap m_symbolTable;

    // The list of parameters, and their local IDs in the
    WTF::Vector<Parameter> m_paramList;
  };

  class FuncExprNode : public Node {
  public:
    FuncExprNode(const Identifier &i, FunctionBodyNode *b, ParameterNode *p = 0)
      : ident(i), param(p ? p->next.release() : 0), body(b) { if (p) { Parser::removeNodeCycle(param.get()); } addParams(); }
    virtual JSValue *evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool introducesNewStaticScope() const { return true; }
  private:
    void addParams();
    // Used for streamTo
    friend class PropertyNode;
    Identifier ident;
    RefPtr<ParameterNode> param;
    RefPtr<FunctionBodyNode> body;
  };

  class FuncDeclNode : public StatementNode {
  public:
    FuncDeclNode(const Identifier &i, FunctionBodyNode *b)
      : ident(i), body(b) { addParams(); }
    FuncDeclNode(const Identifier &i, ParameterNode *p, FunctionBodyNode *b)
      : ident(i), param(p->next.release()), body(b) { Parser::removeNodeCycle(param.get()); addParams(); }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    virtual bool introducesNewStaticScope() const { return true; }

    virtual void processFuncDecl(ExecState*);
    FunctionImp* makeFunctionObject(ExecState*);
  private:
    void addParams();
    Identifier ident;
    RefPtr<ParameterNode> param;
    RefPtr<FunctionBodyNode> body;
  };

  // A linked list of source element nodes
  class SourceElementsNode : public StatementNode {
  public:
    // list pointer is tail of a circular list, cracked in the BlockNode (or subclass) ctor
    SourceElementsNode(StatementNode*);
    SourceElementsNode(SourceElementsNode *s1, StatementNode *s2);

    Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    PassRefPtr<SourceElementsNode> releaseNext() { return next.release(); }
    virtual void breakCycle();
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    friend class BlockNode;
    friend class CaseClauseNode;
    RefPtr<StatementNode> node;
    ListRefPtr<SourceElementsNode> next;
  };

  class CaseClauseNode : public Node {
  public:
      CaseClauseNode(Node *e) : expr(e) { }
      CaseClauseNode(Node *e, SourceElementsNode *s)
      : expr(e), source(s->next.release()) { Parser::removeNodeCycle(source.get()); }
      JSValue* evaluate(ExecState*);
      Completion evalStatements(ExecState*);
      virtual void streamTo(SourceStream&) const;
      virtual void recurseVisit(NodeVisitor *visitor);
  private:
      RefPtr<Node> expr;
      RefPtr<SourceElementsNode> source;
  };

  class ClauseListNode : public Node {
  public:
      // list pointer is tail of a circular list, cracked in the CaseBlockNode ctor
      ClauseListNode(CaseClauseNode *c) : clause(c), next(this) { Parser::noteNodeCycle(this); }
      ClauseListNode(ClauseListNode *n, CaseClauseNode *c)
      : clause(c), next(n->next) { n->next = this; }
      JSValue* evaluate(ExecState*);
      CaseClauseNode *getClause() const { return clause.get(); }
      ClauseListNode *getNext() const { return next.get(); }
      virtual void streamTo(SourceStream&) const;
      PassRefPtr<ClauseListNode> releaseNext() { return next.release(); }
      virtual void breakCycle();
      virtual void recurseVisit(NodeVisitor *visitor);
  private:
      friend class CaseBlockNode;
      RefPtr<CaseClauseNode> clause;
      ListRefPtr<ClauseListNode> next;
  };

  class CaseBlockNode : public Node {
  public:
      CaseBlockNode(ClauseListNode *l1, CaseClauseNode *d, ClauseListNode *l2);
      JSValue* evaluate(ExecState*);
      Completion evalBlock(ExecState *exec, JSValue *input);
      virtual void streamTo(SourceStream&) const;
      virtual void recurseVisit(NodeVisitor *visitor);
  private:
      RefPtr<ClauseListNode> list1;
      RefPtr<CaseClauseNode> def;
      RefPtr<ClauseListNode> list2;
  };

  class SwitchNode : public StatementNode {
  public:
      SwitchNode(Node *e, CaseBlockNode *b) : expr(e), block(b) { }
      virtual Completion execute(ExecState*);
      virtual void streamTo(SourceStream&) const;
      virtual void recurseVisit(NodeVisitor *visitor);
  private:
      RefPtr<Node> expr;
      RefPtr<CaseBlockNode> block;
  };

  class ProgramNode : public FunctionBodyNode {
  public:
    ProgramNode(SourceElementsNode *s);
  };

  class PackageNameNode : public Node {
  public:
    PackageNameNode(const Identifier &i) : names(0), id(i) { }
    PackageNameNode(PackageNameNode *n,
                    const Identifier &i) : names(n), id(i) { }
    JSValue* evaluate(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
    
    Completion loadSymbol(ExecState* exec, bool wildcard);
    PackageObject* resolvePackage(ExecState* exec);

  private:
    PackageObject* resolvePackage(ExecState* exec,
				  JSObject* baseObject, Package* basePackage);
    RefPtr<PackageNameNode> names;
    Identifier id;
  };

  class ImportStatement : public StatementNode {
  public:
    ImportStatement(PackageNameNode *n) : name(n), wld(false) {}
    void enableWildcard() { wld = true; }
    void setAlias(const Identifier &a) { al = a; }
    virtual Completion execute(ExecState*);
    virtual void streamTo(SourceStream&) const;
    virtual void recurseVisit(NodeVisitor *visitor);
  private:
    virtual void processVarDecl (ExecState* state);
    RefPtr<PackageNameNode> name;
    Identifier al;
    bool wld;
  };

} // namespace

#endif
