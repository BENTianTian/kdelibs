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

#include <stdio.h>
#include <string.h>

#include "kjs.h"
#include "object.h"
#include "nodes.h"
#include "lexer.h"

extern int kjsyyparse();

#ifdef KJS_DEBUG_MEM
extern const char* typeName[];
#endif

using namespace KJS;

KJSLexer*    KJSWorld::lexer     = 0L;
KJSContext*  KJSWorld::context   = 0L;
KJSGlobal*   KJSWorld::global    = 0L;
ProgramNode* KJSWorld::prog      = 0L;
KJSError*    KJSWorld::error     = 0L;

KJSWorld::KJSWorld(KHTMLWidget *w)
  : htmlw(w)
{
  printf("KJSWorld::KJSWorld()\n");

  // due to yacc's C nature the parser isn't reentrant
  assert(lexer == 0);
  assert(context == 0);
  assert(global == 0);
  assert(prog == 0);

  error = 0L;
}

KJSWorld::~KJSWorld()
{
}

bool KJSWorld::evaluate(const char *code)
{
  return evaluate(UString(code).unicode(), strlen(code));
}

bool KJSWorld::evaluate(const KJS::UnicodeChar *code, unsigned int length)
{
  lexer = new KJSLexer(UString(code, length));

  int parseError = kjsyyparse();

  // we can safely get rid of the parser now
  delete lexer;
  lexer = 0L;

  if (parseError) {
    fprintf(stderr, "JavaScript parse error.\n");
    KJS::Node::deleteAllNodes();
    return false;
  }

  KJSWorld::error = 0L;

  global = new KJSGlobal(htmlw);

  context = new KJSContext();

  assert(prog);
  Ptr res = prog->evaluate();
  res.release();

  if (KJSWorld::error)
    KJSWorld::error->deref();

  delete context;
  global->deref();

#ifdef KJS_DEBUG_MEM
  if (KJSO::count != 0) {
    fprintf(stderr, "MEMORY LEAK: %d unfreed objects\n", KJS::KJSO::count);
    KJSO *o = KJS::KJSO::firstObject;
    while (o) {
      fprintf(stderr, "id = %d type = %d %s refCount = %d\n",
	      o->objId, o->type(), typeName[o->type()], o->refCount);
      o = o->nextObject;
    }
  }
#endif

  Node::deleteAllNodes();

  return true;
}


