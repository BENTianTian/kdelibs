/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NULLTOKEN = 258,
     TRUETOKEN = 259,
     FALSETOKEN = 260,
     BREAK = 261,
     CASE = 262,
     DEFAULT = 263,
     FOR = 264,
     NEW = 265,
     VAR = 266,
     CONST = 267,
     CONTINUE = 268,
     FUNCTION = 269,
     RETURN = 270,
     VOID = 271,
     DELETE = 272,
     IF = 273,
     THIS = 274,
     DO = 275,
     WHILE = 276,
     IN = 277,
     INSTANCEOF = 278,
     TYPEOF = 279,
     SWITCH = 280,
     WITH = 281,
     RESERVED = 282,
     THROW = 283,
     TRY = 284,
     CATCH = 285,
     FINALLY = 286,
     DEBUGGER = 287,
     IMPORT = 288,
     IF_WITHOUT_ELSE = 289,
     ELSE = 290,
     EQEQ = 291,
     NE = 292,
     STREQ = 293,
     STRNEQ = 294,
     LE = 295,
     GE = 296,
     OR = 297,
     AND = 298,
     PLUSPLUS = 299,
     MINUSMINUS = 300,
     LSHIFT = 301,
     RSHIFT = 302,
     URSHIFT = 303,
     PLUSEQUAL = 304,
     MINUSEQUAL = 305,
     MULTEQUAL = 306,
     DIVEQUAL = 307,
     LSHIFTEQUAL = 308,
     RSHIFTEQUAL = 309,
     URSHIFTEQUAL = 310,
     ANDEQUAL = 311,
     MODEQUAL = 312,
     XOREQUAL = 313,
     OREQUAL = 314,
     NUMBER = 315,
     STRING = 316,
     IDENT = 317,
     AUTOPLUSPLUS = 318,
     AUTOMINUSMINUS = 319
   };
#endif
#define NULLTOKEN 258
#define TRUETOKEN 259
#define FALSETOKEN 260
#define BREAK 261
#define CASE 262
#define DEFAULT 263
#define FOR 264
#define NEW 265
#define VAR 266
#define CONST 267
#define CONTINUE 268
#define FUNCTION 269
#define RETURN 270
#define VOID 271
#define DELETE 272
#define IF 273
#define THIS 274
#define DO 275
#define WHILE 276
#define IN 277
#define INSTANCEOF 278
#define TYPEOF 279
#define SWITCH 280
#define WITH 281
#define RESERVED 282
#define THROW 283
#define TRY 284
#define CATCH 285
#define FINALLY 286
#define DEBUGGER 287
#define IMPORT 288
#define IF_WITHOUT_ELSE 289
#define ELSE 290
#define EQEQ 291
#define NE 292
#define STREQ 293
#define STRNEQ 294
#define LE 295
#define GE 296
#define OR 297
#define AND 298
#define PLUSPLUS 299
#define MINUSMINUS 300
#define LSHIFT 301
#define RSHIFT 302
#define URSHIFT 303
#define PLUSEQUAL 304
#define MINUSEQUAL 305
#define MULTEQUAL 306
#define DIVEQUAL 307
#define LSHIFTEQUAL 308
#define RSHIFTEQUAL 309
#define URSHIFTEQUAL 310
#define ANDEQUAL 311
#define MODEQUAL 312
#define XOREQUAL 313
#define OREQUAL 314
#define NUMBER 315
#define STRING 316
#define IDENT 317
#define AUTOPLUSPLUS 318
#define AUTOMINUSMINUS 319




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 65 "grammar.y"
typedef union YYSTYPE {
  int                 ival;
  double              dval;
  UString             *ustr;
  Identifier          *ident;
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FunctionBodyNode    *body;
  FuncDeclNode        *func;
  FuncExprNode        *funcExpr;
  ProgramNode         *prog;
  AssignExprNode      *init;
  SourceElementsNode  *srcs;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  ElementNode         *elm;
  Operator            op;
  PropertyListNode   *plist;
  PropertyNode       *pnode;
  PropertyNameNode   *pname;
  PackageNameNode     *pkgn;
  PackageIdentNode     *pkgi;
} YYSTYPE;
/* Line 1285 of yacc.c.  */
#line 195 "grammar.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE kjsyylval;

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE kjsyylloc;


