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

#include <qapplication.h>
#include <khtml.h>
#include <html_document.h>

#include "kjs.h"

extern int yyparse();

int main(int argc, char **argv)
{
  const int BufferSize = 20000;
  char code[BufferSize];

  if (argc != 2) {
    fprintf(stderr, "You have to specify a filename\n");
    return -1;
  }

  const char *file = argv[1];

  FILE *f = fopen(file, "r");
  if (!f) {
    fprintf(stderr, "Error opening file.\n");
    return -1;
  }
  int num = fread(code, 1, BufferSize, f);
  code[num] = '\0';
  if(num == BufferSize - 1)
    fprintf(stderr, "Warning: File may have been too long.\n");

  QApplication a(argc, argv);

  KJSWorld *kjs = new KJSWorld(0L);

  kjs->evaluate(code);

  fprintf(stderr, "OK.\n");
}

// dummy implemenations to let us compile a standalone
// version of testkjs without linking to libkhtml
using namespace DOM;
DOMString::DOMString() { }
DOMString::DOMString(int) { }
DOMString::DOMString(QChar *, unsigned int, bool) { }
DOMString::~DOMString() { }
uint DOMString::length() const { return 0; }
QChar *DOMString::unicode() const { return 0; }
Node::Node() { }
Node::~Node() { }
Document::Document() { }
Document::~Document() { }
HTMLDocument::HTMLDocument() { }
HTMLDocument::~HTMLDocument() { }
HTMLDocument::HTMLDocument(HTMLDocumentImpl *) { }
DOMString HTMLDocument::title() const { return 0; }
DOMString HTMLDocument::URL() const { return 0; }
void HTMLDocument::write(const DOM::DOMString &) { }
