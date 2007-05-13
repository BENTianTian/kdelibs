/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse kjsyyparse
#define yylex   kjsyylex
#define yyerror kjsyyerror
#define yylval  kjsyylval
#define yychar  kjsyychar
#define yydebug kjsyydebug
#define yynerrs kjsyynerrs
#define yylloc kjsyylloc

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
/* Tokens.  */
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




/* Copy the first part of user declarations.  */
#line 1 "grammar.y"


/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2006 Apple Computer, Inc.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "nodes.h"
#include "lexer.h"
#include "internal.h"

// Not sure why, but yacc doesn't add this define along with the others.
#define yylloc kjsyylloc

/* default values for bison */
#define YYDEBUG 0
#if !PLATFORM(DARWIN)
    // avoid triggering warnings in older bison
#define YYERROR_VERBOSE
#endif

extern int kjsyylex();
int kjsyyerror(const char *);
static bool allowAutomaticSemicolon();

#define AUTO_SEMICOLON do { if (!allowAutomaticSemicolon()) YYABORT; } while (0)
#define DBG(l, s, e) (l)->setLoc((s).first_line, (e).last_line)

using namespace KJS;

static bool makeAssignNode(Node*& result, Node *loc, Operator op, Node *expr);
static bool makePrefixNode(Node*& result, Node *expr, Operator op);
static bool makePostfixNode(Node*& result, Node *expr, Operator op);
static bool makeGetterOrSetterPropertyNode(PropertyNode*& result, Identifier &getOrSet, Identifier& name, ParameterNode *params, FunctionBodyNode *body);
static Node *makeFunctionCallNode(Node *func, ArgumentsNode *args);
static Node *makeTypeOfNode(Node *expr);
static Node *makeDeleteNode(Node *expr);
static StatementNode *makeImportNode(PackageNameNode *n,
				     bool wildcard, const Identifier &a);

template<typename ResolverType>
Node* makeDynamicResolver(Node* n) {
  DynamicResolver<ResolveIdentifier> *resolve = static_cast<DynamicResolver<ResolveIdentifier> *>(n);
  return new DynamicResolver<ResolverType>(resolve->identifier(), ResolverType());
}

template<typename ResolverType, typename T1>
Node* makeDynamicResolver(Node* n, T1 arg1) {
  DynamicResolver<ResolveIdentifier> *resolve = static_cast<DynamicResolver<ResolveIdentifier> *>(n);
  return new DynamicResolver<ResolverType>(resolve->identifier(), ResolverType(arg1));
}

template<typename ResolverType, typename T1, typename T2>
Node* makeDynamicResolver(Node* n, T1 arg1, T2 arg2) {
  DynamicResolver<ResolveIdentifier> *resolve = static_cast<DynamicResolver<ResolveIdentifier> *>(n);
  return new DynamicResolver<ResolverType>(resolve->identifier(), ResolverType(arg1, arg2));
}




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 86 "grammar.y"
{
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
}
/* Line 187 of yacc.c.  */
#line 346 "grammar.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
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


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 371 "grammar.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  212
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1528

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  104
/* YYNRULES -- Number of rules.  */
#define YYNRULES  310
/* YYNRULES -- Number of states.  */
#define YYNSTATES  545

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    78,     2,     2,     2,    80,    83,     2,
      67,    68,    79,    75,    69,    76,    74,    65,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    66,    88,
      81,    87,    82,    86,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    72,     2,    73,    84,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    85,    71,    77,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    27,    33,    40,    42,    46,    48,    51,
      55,    60,    62,    64,    66,    68,    72,    76,    80,    86,
      89,    94,    95,    97,    99,   102,   104,   106,   111,   115,
     119,   121,   126,   130,   134,   136,   139,   141,   144,   147,
     150,   155,   159,   162,   165,   170,   174,   177,   181,   183,
     187,   189,   191,   193,   195,   197,   200,   203,   205,   208,
     211,   214,   217,   220,   223,   226,   229,   232,   235,   238,
     241,   244,   246,   248,   250,   252,   254,   258,   262,   266,
     268,   272,   276,   280,   282,   286,   290,   292,   296,   300,
     302,   306,   310,   314,   316,   320,   324,   328,   330,   334,
     338,   342,   346,   350,   354,   356,   360,   364,   368,   372,
     376,   378,   382,   386,   390,   394,   398,   402,   404,   408,
     412,   416,   420,   422,   426,   430,   434,   438,   440,   444,
     448,   452,   456,   458,   462,   464,   468,   470,   474,   476,
     480,   482,   486,   488,   492,   494,   498,   500,   504,   506,
     510,   512,   516,   518,   522,   524,   528,   530,   534,   536,
     540,   542,   546,   548,   554,   556,   562,   564,   570,   572,
     576,   578,   582,   584,   588,   590,   592,   594,   596,   598,
     600,   602,   604,   606,   608,   610,   612,   614,   618,   620,
     624,   626,   630,   632,   634,   636,   638,   640,   642,   644,
     646,   648,   650,   652,   654,   656,   658,   660,   662,   664,
     667,   671,   675,   679,   681,   685,   687,   691,   693,   696,
     698,   701,   705,   709,   711,   715,   717,   720,   723,   726,
     728,   731,   734,   740,   748,   755,   761,   771,   782,   790,
     799,   809,   810,   812,   813,   815,   818,   821,   825,   829,
     832,   835,   839,   843,   846,   849,   853,   857,   863,   869,
     873,   879,   880,   882,   884,   887,   891,   896,   899,   903,
     907,   911,   915,   920,   928,   938,   941,   944,   946,   950,
     956,   962,   966,   970,   976,   982,   988,   995,  1000,  1006,
    1012,  1019,  1021,  1025,  1028,  1032,  1033,  1035,  1037,  1040,
    1042
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     190,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    60,
      -1,    61,    -1,    65,    -1,    52,    -1,    62,    -1,    61,
      -1,    60,    -1,    91,    66,   145,    -1,    62,    62,    67,
      68,   189,    -1,    62,    62,    67,   188,    68,   189,    -1,
      92,    -1,    93,    69,    92,    -1,    95,    -1,    70,    71,
      -1,    70,    93,    71,    -1,    70,    93,    69,    71,    -1,
      19,    -1,    90,    -1,    96,    -1,    62,    -1,    67,   149,
      68,    -1,    72,    98,    73,    -1,    72,    97,    73,    -1,
      72,    97,    69,    98,    73,    -1,    98,   145,    -1,    97,
      69,    98,   145,    -1,    -1,    99,    -1,    69,    -1,    99,
      69,    -1,    94,    -1,   187,    -1,   100,    72,   149,    73,
      -1,   100,    74,    62,    -1,    10,   100,   106,    -1,    95,
      -1,   101,    72,   149,    73,    -1,   101,    74,    62,    -1,
      10,   100,   106,    -1,   100,    -1,    10,   102,    -1,   101,
      -1,    10,   102,    -1,   100,   106,    -1,   104,   106,    -1,
     104,    72,   149,    73,    -1,   104,    74,    62,    -1,   101,
     106,    -1,   105,   106,    -1,   105,    72,   149,    73,    -1,
     105,    74,    62,    -1,    67,    68,    -1,    67,   107,    68,
      -1,   145,    -1,   107,    69,   145,    -1,   102,    -1,   104,
      -1,   103,    -1,   105,    -1,   108,    -1,   108,    44,    -1,
     108,    45,    -1,   109,    -1,   109,    44,    -1,   109,    45,
      -1,    17,   113,    -1,    16,   113,    -1,    24,   113,    -1,
      44,   113,    -1,    63,   113,    -1,    45,   113,    -1,    64,
     113,    -1,    75,   113,    -1,    76,   113,    -1,    77,   113,
      -1,    78,   113,    -1,   110,    -1,   112,    -1,   111,    -1,
     112,    -1,   113,    -1,   115,    79,   113,    -1,   115,    65,
     113,    -1,   115,    80,   113,    -1,   114,    -1,   116,    79,
     113,    -1,   116,    65,   113,    -1,   116,    80,   113,    -1,
     115,    -1,   117,    75,   115,    -1,   117,    76,   115,    -1,
     116,    -1,   118,    75,   115,    -1,   118,    76,   115,    -1,
     117,    -1,   119,    46,   117,    -1,   119,    47,   117,    -1,
     119,    48,   117,    -1,   118,    -1,   120,    46,   117,    -1,
     120,    47,   117,    -1,   120,    48,   117,    -1,   119,    -1,
     121,    81,   119,    -1,   121,    82,   119,    -1,   121,    40,
     119,    -1,   121,    41,   119,    -1,   121,    23,   119,    -1,
     121,    22,   119,    -1,   119,    -1,   122,    81,   119,    -1,
     122,    82,   119,    -1,   122,    40,   119,    -1,   122,    41,
     119,    -1,   122,    23,   119,    -1,   120,    -1,   123,    81,
     119,    -1,   123,    82,   119,    -1,   123,    40,   119,    -1,
     123,    41,   119,    -1,   123,    23,   119,    -1,   123,    22,
     119,    -1,   121,    -1,   124,    36,   121,    -1,   124,    37,
     121,    -1,   124,    38,   121,    -1,   124,    39,   121,    -1,
     122,    -1,   125,    36,   122,    -1,   125,    37,   122,    -1,
     125,    38,   122,    -1,   125,    39,   122,    -1,   123,    -1,
     126,    36,   121,    -1,   126,    37,   121,    -1,   126,    38,
     121,    -1,   126,    39,   121,    -1,   124,    -1,   127,    83,
     124,    -1,   125,    -1,   128,    83,   125,    -1,   126,    -1,
     129,    83,   124,    -1,   127,    -1,   130,    84,   127,    -1,
     128,    -1,   131,    84,   128,    -1,   129,    -1,   132,    84,
     127,    -1,   130,    -1,   133,    85,   130,    -1,   131,    -1,
     134,    85,   131,    -1,   132,    -1,   135,    85,   130,    -1,
     133,    -1,   136,    43,   133,    -1,   134,    -1,   137,    43,
     134,    -1,   135,    -1,   138,    43,   133,    -1,   136,    -1,
     139,    42,   136,    -1,   137,    -1,   140,    42,   137,    -1,
     138,    -1,   141,    42,   136,    -1,   139,    -1,   139,    86,
     145,    66,   145,    -1,   140,    -1,   140,    86,   146,    66,
     146,    -1,   141,    -1,   141,    86,   145,    66,   145,    -1,
     142,    -1,   108,   148,   145,    -1,   143,    -1,   108,   148,
     146,    -1,   144,    -1,   109,   148,   145,    -1,    87,    -1,
      49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,
      54,    -1,    55,    -1,    56,    -1,    58,    -1,    59,    -1,
      57,    -1,   145,    -1,   149,    69,   145,    -1,   146,    -1,
     150,    69,   146,    -1,   147,    -1,   151,    69,   145,    -1,
     153,    -1,   154,    -1,   159,    -1,   164,    -1,   165,    -1,
     166,    -1,   167,    -1,   170,    -1,   171,    -1,   172,    -1,
     173,    -1,   174,    -1,   180,    -1,   181,    -1,   182,    -1,
     183,    -1,   185,    -1,    70,    71,    -1,    70,   191,    71,
      -1,    11,   155,    88,    -1,    11,   155,     1,    -1,   157,
      -1,   155,    69,   157,    -1,   158,    -1,   156,    69,   158,
      -1,    62,    -1,    62,   162,    -1,    62,    -1,    62,   163,
      -1,    12,   160,    88,    -1,    12,   160,     1,    -1,   161,
      -1,   160,    69,   161,    -1,    62,    -1,    62,   162,    -1,
      87,   145,    -1,    87,   146,    -1,    88,    -1,   151,    88,
      -1,   151,     1,    -1,    18,    67,   149,    68,   152,    -1,
      18,    67,   149,    68,   152,    35,   152,    -1,    20,   152,
      21,    67,   149,    68,    -1,    21,    67,   149,    68,   152,
      -1,     9,    67,   169,    88,   168,    88,   168,    68,   152,
      -1,     9,    67,    11,   156,    88,   168,    88,   168,    68,
     152,    -1,     9,    67,   108,    22,   149,    68,   152,    -1,
       9,    67,    11,    62,    22,   149,    68,   152,    -1,     9,
      67,    11,    62,   163,    22,   149,    68,   152,    -1,    -1,
     149,    -1,    -1,   150,    -1,    13,    88,    -1,    13,     1,
      -1,    13,    62,    88,    -1,    13,    62,     1,    -1,     6,
      88,    -1,     6,     1,    -1,     6,    62,    88,    -1,     6,
      62,     1,    -1,    15,    88,    -1,    15,     1,    -1,    15,
     149,    88,    -1,    15,   149,     1,    -1,    26,    67,   149,
      68,   152,    -1,    25,    67,   149,    68,   175,    -1,    70,
     176,    71,    -1,    70,   176,   179,   176,    71,    -1,    -1,
     177,    -1,   178,    -1,   177,   178,    -1,     7,   149,    66,
      -1,     7,   149,    66,   191,    -1,     8,    66,    -1,     8,
      66,   191,    -1,    62,    66,   152,    -1,    28,   149,    88,
      -1,    28,   149,     1,    -1,    29,   153,    31,   153,    -1,
      29,   153,    30,    67,    62,    68,   153,    -1,    29,   153,
      30,    67,    62,    68,   153,    31,   153,    -1,    32,    88,
      -1,    32,     1,    -1,    62,    -1,   184,    74,    62,    -1,
      33,   184,    74,    79,    88,    -1,    33,   184,    74,    79,
       1,    -1,    33,   184,    88,    -1,    33,   184,     1,    -1,
      33,    62,    87,   184,    88,    -1,    33,    62,    87,   184,
       1,    -1,    14,    62,    67,    68,   189,    -1,    14,    62,
      67,   188,    68,   189,    -1,    14,    67,    68,   189,    -1,
      14,    67,   188,    68,   189,    -1,    14,    62,    67,    68,
     189,    -1,    14,    62,    67,   188,    68,   189,    -1,    62,
      -1,   188,    69,    62,    -1,    70,    71,    -1,    70,   191,
      71,    -1,    -1,   191,    -1,   192,    -1,   191,   192,    -1,
     186,    -1,   152,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   212,   212,   213,   214,   215,   216,   217,   222,   230,
     231,   232,   236,   237,   238,   243,   244,   248,   249,   250,
     252,   256,   257,   258,   259,   260,   265,   266,   267,   271,
     272,   277,   278,   282,   283,   287,   288,   289,   290,   291,
     295,   296,   297,   298,   302,   303,   307,   308,   312,   313,
     314,   315,   319,   320,   321,   322,   326,   327,   331,   332,
     336,   337,   341,   342,   346,   347,   348,   352,   353,   354,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   371,   372,   376,   377,   381,   382,   383,   384,   388,
     389,   391,   393,   398,   399,   400,   404,   405,   407,   412,
     413,   414,   415,   419,   420,   421,   422,   426,   427,   428,
     429,   430,   431,   432,   436,   437,   438,   439,   440,   441,
     446,   447,   448,   449,   450,   451,   453,   457,   458,   459,
     460,   461,   465,   466,   468,   470,   472,   477,   478,   480,
     481,   483,   488,   489,   493,   494,   499,   500,   504,   505,
     509,   510,   515,   516,   521,   522,   526,   527,   532,   533,
     538,   539,   543,   544,   549,   550,   555,   556,   560,   561,
     566,   567,   571,   572,   577,   578,   583,   584,   589,   590,
     595,   596,   601,   602,   607,   608,   609,   610,   611,   612,
     613,   614,   615,   616,   617,   618,   622,   623,   627,   628,
     632,   633,   637,   638,   639,   640,   641,   642,   643,   644,
     645,   646,   647,   648,   649,   650,   651,   652,   653,   657,
     658,   662,   663,   667,   668,   673,   674,   679,   680,   684,
     685,   689,   690,   694,   695,   700,   701,   705,   709,   713,
     717,   718,   722,   724,   729,   730,   731,   733,   735,   743,
     745,   750,   751,   755,   756,   760,   761,   762,   763,   767,
     768,   769,   770,   774,   775,   776,   777,   781,   785,   789,
     790,   795,   796,   800,   801,   805,   806,   810,   811,   815,
     819,   820,   824,   825,   826,   831,   832,   836,   837,   841,
     843,   845,   847,   849,   851,   856,   857,   862,   863,   865,
     866,   871,   872,   876,   877,   881,   882,   886,   887,   891,
     892
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN",
  "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", "CONST", "CONTINUE",
  "FUNCTION", "RETURN", "VOID", "DELETE", "IF", "THIS", "DO", "WHILE",
  "IN", "INSTANCEOF", "TYPEOF", "SWITCH", "WITH", "RESERVED", "THROW",
  "TRY", "CATCH", "FINALLY", "DEBUGGER", "IMPORT", "IF_WITHOUT_ELSE",
  "ELSE", "EQEQ", "NE", "STREQ", "STRNEQ", "LE", "GE", "OR", "AND",
  "PLUSPLUS", "MINUSMINUS", "LSHIFT", "RSHIFT", "URSHIFT", "PLUSEQUAL",
  "MINUSEQUAL", "MULTEQUAL", "DIVEQUAL", "LSHIFTEQUAL", "RSHIFTEQUAL",
  "URSHIFTEQUAL", "ANDEQUAL", "MODEQUAL", "XOREQUAL", "OREQUAL", "NUMBER",
  "STRING", "IDENT", "AUTOPLUSPLUS", "AUTOMINUSMINUS", "'/'", "':'", "'('",
  "')'", "','", "'{'", "'}'", "'['", "']'", "'.'", "'+'", "'-'", "'~'",
  "'!'", "'*'", "'%'", "'<'", "'>'", "'&'", "'^'", "'|'", "'?'", "'='",
  "';'", "$accept", "Literal", "PropertyName", "Property", "PropertyList",
  "PrimaryExpr", "PrimaryExprNoBrace", "ArrayLiteral", "ElementList",
  "ElisionOpt", "Elision", "MemberExpr", "MemberExprNoBF", "NewExpr",
  "NewExprNoBF", "CallExpr", "CallExprNoBF", "Arguments", "ArgumentList",
  "LeftHandSideExpr", "LeftHandSideExprNoBF", "PostfixExpr",
  "PostfixExprNoBF", "UnaryExprCommon", "UnaryExpr", "UnaryExprNoBF",
  "MultiplicativeExpr", "MultiplicativeExprNoBF", "AdditiveExpr",
  "AdditiveExprNoBF", "ShiftExpr", "ShiftExprNoBF", "RelationalExpr",
  "RelationalExprNoIn", "RelationalExprNoBF", "EqualityExpr",
  "EqualityExprNoIn", "EqualityExprNoBF", "BitwiseANDExpr",
  "BitwiseANDExprNoIn", "BitwiseANDExprNoBF", "BitwiseXORExpr",
  "BitwiseXORExprNoIn", "BitwiseXORExprNoBF", "BitwiseORExpr",
  "BitwiseORExprNoIn", "BitwiseORExprNoBF", "LogicalANDExpr",
  "LogicalANDExprNoIn", "LogicalANDExprNoBF", "LogicalORExpr",
  "LogicalORExprNoIn", "LogicalORExprNoBF", "ConditionalExpr",
  "ConditionalExprNoIn", "ConditionalExprNoBF", "AssignmentExpr",
  "AssignmentExprNoIn", "AssignmentExprNoBF", "AssignmentOperator", "Expr",
  "ExprNoIn", "ExprNoBF", "Statement", "Block", "VariableStatement",
  "VariableDeclarationList", "VariableDeclarationListNoIn",
  "VariableDeclaration", "VariableDeclarationNoIn", "ConstStatement",
  "ConstDeclarationList", "ConstDeclaration", "Initializer",
  "InitializerNoIn", "EmptyStatement", "ExprStatement", "IfStatement",
  "IterationStatement", "ExprOpt", "ExprNoInOpt", "ContinueStatement",
  "BreakStatement", "ReturnStatement", "WithStatement", "SwitchStatement",
  "CaseBlock", "CaseClausesOpt", "CaseClauses", "CaseClause",
  "DefaultClause", "LabelledStatement", "ThrowStatement", "TryStatement",
  "DebuggerStatement", "PackageName", "ImportStatement",
  "FunctionDeclaration", "FunctionExpr", "FormalParameterList",
  "FunctionBody", "Program", "SourceElements", "SourceElement", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,    47,    58,    40,    41,    44,
     123,   125,    91,    93,    46,    43,    45,   126,    33,    42,
      37,    60,    62,    38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    90,    90,    90,    90,    90,    90,    91,
      91,    91,    92,    92,    92,    93,    93,    94,    94,    94,
      94,    95,    95,    95,    95,    95,    96,    96,    96,    97,
      97,    98,    98,    99,    99,   100,   100,   100,   100,   100,
     101,   101,   101,   101,   102,   102,   103,   103,   104,   104,
     104,   104,   105,   105,   105,   105,   106,   106,   107,   107,
     108,   108,   109,   109,   110,   110,   110,   111,   111,   111,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   113,   113,   114,   114,   115,   115,   115,   115,   116,
     116,   116,   116,   117,   117,   117,   118,   118,   118,   119,
     119,   119,   119,   120,   120,   120,   120,   121,   121,   121,
     121,   121,   121,   121,   122,   122,   122,   122,   122,   122,
     123,   123,   123,   123,   123,   123,   123,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   126,   126,   126,
     126,   126,   127,   127,   128,   128,   129,   129,   130,   130,
     131,   131,   132,   132,   133,   133,   134,   134,   135,   135,
     136,   136,   137,   137,   138,   138,   139,   139,   140,   140,
     141,   141,   142,   142,   143,   143,   144,   144,   145,   145,
     146,   146,   147,   147,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   149,   149,   150,   150,
     151,   151,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   153,
     153,   154,   154,   155,   155,   156,   156,   157,   157,   158,
     158,   159,   159,   160,   160,   161,   161,   162,   163,   164,
     165,   165,   166,   166,   167,   167,   167,   167,   167,   167,
     167,   168,   168,   169,   169,   170,   170,   170,   170,   171,
     171,   171,   171,   172,   172,   172,   172,   173,   174,   175,
     175,   176,   176,   177,   177,   178,   178,   179,   179,   180,
     181,   181,   182,   182,   182,   183,   183,   184,   184,   185,
     185,   185,   185,   185,   185,   186,   186,   187,   187,   187,
     187,   188,   188,   189,   189,   190,   190,   191,   191,   192,
     192
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     5,     6,     1,     3,     1,     2,     3,
       4,     1,     1,     1,     1,     3,     3,     3,     5,     2,
       4,     0,     1,     1,     2,     1,     1,     4,     3,     3,
       1,     4,     3,     3,     1,     2,     1,     2,     2,     2,
       4,     3,     2,     2,     4,     3,     2,     3,     1,     3,
       1,     1,     1,     1,     1,     2,     2,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     3,     3,     3,     1,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     1,     3,     3,     3,     1,     3,     3,
       3,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       1,     3,     3,     3,     3,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     5,     1,     5,     1,     5,     1,     3,
       1,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     3,     3,     1,     3,     1,     3,     1,     2,     1,
       2,     3,     3,     1,     3,     1,     2,     2,     2,     1,
       2,     2,     5,     7,     6,     5,     9,    10,     7,     8,
       9,     0,     1,     0,     1,     2,     2,     3,     3,     2,
       2,     3,     3,     2,     2,     3,     3,     5,     5,     3,
       5,     0,     1,     1,     2,     3,     4,     2,     3,     3,
       3,     3,     4,     7,     9,     2,     2,     1,     3,     5,
       5,     3,     3,     5,     5,     5,     6,     4,     5,     5,
       6,     1,     3,     2,     3,     0,     1,     1,     2,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     305,     2,     3,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     5,     6,
      24,     0,     0,     7,     0,     0,    31,     0,     0,     0,
       0,   239,    22,    40,    23,    46,    62,    63,    67,    83,
      84,    89,    96,   103,   120,   137,   146,   152,   158,   164,
     170,   176,   182,   200,     0,   310,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   309,     0,   306,   307,   260,     0,   259,
     253,     0,     0,    24,     0,    35,    17,    44,    47,    36,
     227,     0,   223,   235,     0,   233,   256,     0,   255,     0,
     264,   263,    44,    60,    61,    64,    81,    82,    85,    93,
      99,   107,   127,   142,   148,   154,   160,   166,   172,   178,
     196,     0,    64,    71,    70,     0,     0,     0,    72,     0,
       0,     0,     0,   286,   285,   287,     0,    73,    75,     0,
      74,    76,     0,   219,     0,    33,     0,     0,    32,    77,
      78,    79,    80,     0,     0,     0,    52,     0,     0,    53,
      68,    69,   185,   186,   187,   188,   189,   190,   191,   192,
     195,   193,   194,   184,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   241,
       0,   240,     1,   308,   262,   261,     0,    64,   114,   132,
     144,   150,   156,   162,   168,   174,   180,   198,   254,     0,
      44,    45,     0,     0,    11,    10,     9,    18,     0,    15,
       0,     0,     0,    43,     0,   228,   222,     0,   221,   236,
     232,     0,   231,   258,   257,     0,    48,     0,     0,    49,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   266,     0,   265,
       0,     0,     0,     0,     0,   281,   280,     0,     0,     0,
     292,     0,   291,   279,    25,   220,    31,    27,    26,    29,
      34,    56,     0,    58,     0,    42,     0,    55,   183,    91,
      90,    92,    97,    98,   104,   105,   106,   126,   125,   123,
     124,   121,   122,   138,   139,   140,   141,   147,   153,   159,
     165,   171,     0,   201,   229,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   251,    39,     0,   301,     0,
       0,     0,     0,     0,    19,     0,    38,   237,   224,   234,
       0,     0,     0,    51,   179,    87,    86,    88,    94,    95,
     100,   101,   102,   113,   112,   110,   111,   108,   109,   128,
     129,   130,   131,   143,   149,   155,   161,   167,     0,   197,
       0,     0,     0,     0,     0,     0,   282,   287,     0,   288,
       0,     0,    57,     0,    41,    54,     0,     0,     0,   230,
       0,   251,     0,    64,   181,   119,   117,   118,   115,   116,
     133,   134,   135,   136,   145,   151,   157,   163,   169,     0,
     199,   252,     0,     0,     0,     0,   297,     0,     0,     0,
      12,    20,    16,    37,   295,     0,    50,     0,   242,     0,
     245,   271,   268,   267,     0,   294,     0,   293,   290,   289,
      28,    30,    59,   177,     0,   238,     0,   229,   226,     0,
       0,     0,   251,   299,     0,   303,     0,   298,   302,     0,
       0,   296,   173,     0,   244,     0,     0,   272,   273,     0,
       0,     0,   230,   251,   248,   175,     0,   300,   304,    13,
       0,   243,     0,     0,   269,   271,   274,   283,   249,     0,
       0,     0,    14,   275,   277,     0,     0,   250,     0,   246,
     276,   278,   270,   284,   247
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    42,   238,   239,   240,    95,    96,    44,   156,   157,
     158,   112,    45,   113,    46,   114,    47,   166,   312,   132,
      48,   116,    49,   117,   118,    51,   119,    52,   120,    53,
     121,    54,   122,   219,    55,   123,   220,    56,   124,   221,
      57,   125,   222,    58,   126,   223,    59,   127,   224,    60,
     128,   225,    61,   129,   226,    62,   130,   227,    63,   348,
     451,   228,    64,    65,    66,    67,   101,   345,   102,   346,
      68,   104,   105,   245,   429,    69,    70,    71,    72,   452,
     229,    73,    74,    75,    76,    77,   472,   506,   507,   508,
     525,    78,    79,    80,    81,   146,    82,    83,    99,   370,
     456,    84,    85,    86
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -379
static const yytype_int16 yypact[] =
{
     990,  -379,  -379,  -379,    16,   -56,   175,   -35,   -20,    17,
      -1,   590,  1450,  1450,   -13,  -379,  1070,     2,  1450,    10,
      46,  1450,    12,    27,    60,  1450,  1450,  -379,  -379,  -379,
      70,  1450,  1450,  -379,  1450,   670,    83,  1450,  1450,  1450,
    1450,  -379,  -379,  -379,  -379,   126,  -379,   129,   392,  -379,
    -379,  -379,   108,   204,   270,   255,   288,    88,   115,   110,
     171,    -5,  -379,  -379,     3,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,   223,   990,  -379,  -379,    31,  -379,
    1146,   175,   135,  -379,   168,  -379,  -379,   174,  -379,  -379,
     144,    19,  -379,   144,    20,  -379,  -379,    33,  -379,   186,
    -379,  -379,   174,  -379,   247,   521,  -379,  -379,  -379,   111,
     226,   308,   300,   292,   187,   173,   189,   229,    -2,  -379,
    -379,    23,   199,  -379,  -379,  1450,   267,  1450,  -379,  1450,
    1450,    28,   220,  -379,  -379,   205,    29,  -379,  -379,  1070,
    -379,  -379,   242,  -379,   750,  -379,   -28,  1222,   225,  -379,
    -379,  -379,  -379,  1298,  1450,   236,  -379,  1450,   246,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  1450,  1450,  1450,  1450,  1450,  1450,
    1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,
    1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  -379,
    1450,  -379,  -379,  -379,  -379,  -379,   251,   210,   308,   143,
     296,   262,   254,   291,   351,    95,  -379,  -379,   327,   314,
     174,  -379,   344,    -6,  -379,  -379,   353,  -379,   350,  -379,
     146,  1450,   355,  -379,  1450,  -379,  -379,   -35,  -379,  -379,
    -379,   -20,  -379,  -379,  -379,    93,  -379,  1450,   356,  -379,
    -379,  -379,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,
    1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,
    1450,  1450,  1450,  1450,  1450,  1450,  1450,  -379,  1450,  -379,
     274,   354,   283,   289,   310,  -379,  -379,   359,    12,   358,
    -379,    98,  -379,  -379,  -379,  -379,    83,  -379,  -379,  -379,
    -379,  -379,   315,  -379,    94,  -379,   113,  -379,  -379,  -379,
    -379,  -379,   111,   111,   226,   226,   226,   308,   308,   308,
     308,   308,   308,   300,   300,   300,   300,   292,   187,   173,
     189,   229,   357,  -379,    71,    40,  -379,  1450,  1450,  1450,
    1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,  1450,
    1450,  1450,  1450,  1450,  1450,  1450,  -379,   100,  -379,   360,
     317,   364,  1450,   244,  -379,   165,  -379,  -379,  -379,  -379,
     360,   319,   183,  -379,  -379,  -379,  -379,  -379,   111,   111,
     226,   226,   226,   308,   308,   308,   308,   308,   308,   300,
     300,   300,   300,   292,   187,   173,   189,   229,   366,  -379,
    1070,  1450,  1070,   363,  1070,   372,  -379,  -379,    32,  -379,
      35,  1374,  -379,  1450,  -379,  -379,  1450,  1450,  1450,   413,
     376,  1450,   321,   521,  -379,   308,   308,   308,   308,   308,
     143,   143,   143,   143,   296,   262,   254,   291,   351,   374,
    -379,   383,   365,   360,   323,   830,  -379,   360,   394,   154,
    -379,  -379,  -379,  -379,  -379,   360,  -379,  1450,   419,   332,
    -379,   452,  -379,  -379,   393,  -379,   398,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,   336,  -379,  1450,   375,  -379,   377,
    1070,  1450,  1450,  -379,   360,  -379,   910,  -379,  -379,   360,
     338,  -379,  -379,  1070,  -379,  1450,   104,   452,  -379,    12,
    1070,   340,  -379,  1450,  -379,  -379,   395,  -379,  -379,  -379,
     360,  -379,    14,   400,  -379,   452,  -379,   433,  -379,  1070,
     399,  1070,  -379,   990,   990,   397,    12,  -379,  1070,  -379,
     990,   990,  -379,  -379,  -379
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -379,  -379,  -379,    96,  -379,  -379,     0,  -379,  -379,   166,
    -379,     4,  -379,     8,  -379,  -379,  -379,   -38,  -379,   136,
    -379,  -379,  -379,    15,    26,  -379,  -121,  -379,  -144,  -379,
      11,  -379,   -59,    -7,  -379,  -191,   116,  -379,  -149,   112,
    -379,  -145,   117,  -379,  -133,   119,  -379,  -132,   114,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -114,  -341,  -379,   -29,
      -8,  -379,  -379,   -15,   -17,  -379,  -379,  -379,   234,    43,
    -379,  -379,   231,   382,     6,  -379,  -379,  -379,  -379,  -378,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,   -39,  -379,   -18,
    -379,  -379,  -379,  -379,  -379,   188,  -379,  -379,  -379,  -249,
    -355,  -379,   -27,   -83
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      43,   136,   213,   131,   209,   142,   381,   434,   154,   169,
      97,    90,   337,   141,    98,    50,    43,    87,   106,   184,
     246,   250,   449,   450,   287,   464,   152,   100,   143,   295,
     300,    50,   214,   475,   253,    43,   478,   207,   133,   134,
     285,   306,   103,   309,   138,   307,   324,   325,   326,   313,
      50,   147,   148,   489,   135,   338,   368,   150,   151,   243,
     339,   109,   369,   159,   160,   161,   162,   322,   323,   137,
     318,   213,   210,   340,   256,   341,   259,   139,    88,   107,
     533,   208,    35,   288,   286,    43,   262,   485,   247,   251,
     403,   211,   288,   427,   342,   230,   343,   288,   493,   231,
      50,   218,   497,   301,    89,   108,   476,   248,   252,   430,
     501,   289,   523,   140,   516,   144,   296,   302,   454,   215,
     477,   254,   145,   479,   390,   391,   392,   290,   431,   292,
     377,   293,   294,   404,   303,   530,   149,   362,   405,   517,
     333,   334,   335,   336,   519,   388,   389,   115,   384,    43,
     515,   406,   155,   407,    43,   368,   314,   115,   428,   316,
     419,   380,   368,   288,    50,   532,   349,   424,   453,    50,
     115,   203,   408,   185,   409,   524,   263,   420,     1,     2,
       3,   363,   288,   350,   351,    91,   425,   186,   187,    92,
     264,   265,   366,   163,    15,   205,   163,   232,   164,   204,
     165,   167,   233,   168,   327,   328,   329,   330,   331,   332,
     500,   319,   320,   321,   206,   373,   368,   374,   399,   400,
     401,   402,   499,   212,   352,   353,   217,    27,   234,   235,
     236,   244,   347,   375,   288,    28,    29,    93,   463,   237,
      33,   163,    34,   260,   261,    94,   241,    36,   242,   382,
     297,   298,   288,   255,   260,   261,   466,   282,   460,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     281,   115,   284,   115,   283,   115,   115,   193,   194,   188,
     189,   416,   393,   394,   395,   396,   397,   398,   291,   385,
     386,   387,   299,   115,   310,   195,   196,   183,   315,   115,
     115,   266,   267,   115,   234,   235,   236,   481,   317,   482,
     304,   288,   483,   344,   163,   461,   190,   191,   192,   257,
     115,   258,   271,   272,   199,   200,   201,   202,   277,   278,
     279,   280,   354,   355,   356,   357,   197,   198,   359,   432,
     273,   274,   410,   288,   115,   358,   115,   440,   441,   442,
     443,   412,   288,   502,   268,   269,   270,   413,   288,   218,
     435,   436,   437,   438,   439,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   360,   115,   414,   288,
     115,   275,   276,   422,   423,   457,   458,   465,   458,   490,
     288,   494,   458,   115,   361,   468,   364,   470,   115,   473,
     504,   288,   365,   469,   510,   288,   520,   458,   529,   288,
      43,   367,    43,   213,    43,   371,   372,   376,   383,   484,
     417,   411,   115,   426,   115,    50,   415,    50,   496,    50,
     455,   459,   467,   471,   474,   486,   170,   171,   487,   218,
     491,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   288,   492,   503,    43,   498,   213,   213,   505,
     419,   509,   428,   531,   536,   513,   534,   538,   542,   462,
      50,   445,   421,   488,   444,   514,   448,   446,   511,   183,
     447,   378,   379,   115,   433,   249,   535,   418,   521,   526,
      43,     0,   527,   512,     0,   528,    43,   522,     0,   433,
     433,   115,   218,    43,     0,    50,   540,   541,   115,     0,
      43,    50,     0,     0,   537,     0,   539,     0,    50,   543,
       0,     0,     0,   544,     0,    50,     0,     0,     0,    43,
       0,    43,     0,    43,    43,     0,     0,     0,    43,     0,
      43,    43,     0,     0,    50,     0,    50,   115,    50,    50,
       0,     0,     0,    50,     0,    50,    50,   115,     0,   115,
       0,     0,   115,   115,   433,   260,   261,   115,     0,     0,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   110,     0,     1,     2,     3,     0,     0,     0,     0,
      91,     0,     0,   115,    92,     0,    12,    13,   183,    15,
       0,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,     0,   115,     0,     0,     0,     0,   433,   115,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,   115,    27,     0,     0,     0,     0,     0,     0,   115,
      28,    29,    93,    31,    32,    33,     0,    34,     0,     0,
      94,     0,    36,     0,     0,    37,    38,    39,    40,     0,
       0,     0,     0,     1,     2,     3,     4,     0,   111,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,    18,    19,    20,     0,    21,    22,
       0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,     0,     0,
      35,   153,    36,     0,     0,    37,    38,    39,    40,     0,
       0,     0,     0,     1,     2,     3,     4,     0,    41,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,    18,    19,    20,     0,    21,    22,
       0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,     0,     0,
      35,   305,    36,     0,     0,    37,    38,    39,    40,     0,
       0,     0,     0,     1,     2,     3,     4,     0,    41,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,    18,    19,    20,     0,    21,    22,
       0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,     0,     0,
      35,   495,    36,     0,     0,    37,    38,    39,    40,     0,
       0,     0,     0,     1,     2,     3,     4,     0,    41,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,    18,    19,    20,     0,    21,    22,
       0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,     0,     0,
      35,   518,    36,     0,     0,    37,    38,    39,    40,     0,
       0,     0,     0,     1,     2,     3,     4,     0,    41,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,    18,    19,    20,     0,    21,    22,
       0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,     0,     0,
      35,     0,    36,     0,     0,    37,    38,    39,    40,     0,
       0,     0,     0,     1,     2,     3,     4,     0,    41,     5,
       6,     7,     8,     9,     0,    11,    12,    13,    14,    15,
      16,    17,     0,     0,    18,    19,    20,     0,    21,    22,
       0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,    33,     0,    34,     0,     0,
      35,     0,    36,     0,     0,    37,    38,    39,    40,     1,
       2,     3,     0,     0,     0,     0,    91,   216,    41,     0,
      92,     0,    12,    13,     0,    15,     0,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    93,    31,
      32,    33,     0,    34,     0,     0,    94,     0,    36,     0,
       0,    37,    38,    39,    40,     1,     2,     3,     0,     0,
       0,     0,    91,     0,     0,     0,    92,     0,    12,    13,
       0,    15,     0,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    25,    26,     0,     0,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,     0,    28,    29,    93,    31,    32,    33,     0,    34,
       0,     0,    94,     0,    36,   308,     0,    37,    38,    39,
      40,     1,     2,     3,     0,     0,     0,     0,    91,     0,
       0,     0,    92,     0,    12,    13,     0,    15,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      93,    31,    32,    33,     0,    34,   311,     0,    94,     0,
      36,     0,     0,    37,    38,    39,    40,     1,     2,     3,
       0,     0,     0,     0,    91,     0,     0,     0,    92,     0,
      12,    13,     0,    15,     0,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,    28,    29,    93,    31,    32,    33,
       0,    34,     0,     0,    94,     0,    36,   480,     0,    37,
      38,    39,    40,     1,     2,     3,     0,     0,     0,     0,
      91,     0,     0,     0,    92,     0,    12,    13,     0,    15,
       0,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    93,    31,    32,    33,     0,    34,     0,     0,
      94,     0,    36,     0,     0,    37,    38,    39,    40
};

static const yytype_int16 yycheck[] =
{
       0,    16,    85,    11,     1,    22,   255,   348,    35,    47,
       6,    67,   203,    21,     6,     0,    16,     1,     1,    48,
       1,     1,   363,   364,     1,   380,    34,    62,     1,     1,
       1,    16,     1,     1,     1,    35,     1,    42,    12,    13,
      42,    69,    62,   157,    18,    73,   190,   191,   192,   163,
      35,    25,    26,   431,    67,   204,    62,    31,    32,    97,
     205,    62,    68,    37,    38,    39,    40,   188,   189,    67,
     184,   154,    69,   206,   112,   207,   114,    67,    62,    62,
      66,    86,    70,    69,    86,    85,   115,   428,    69,    69,
     281,    88,    69,    22,   208,    91,   210,    69,   453,    91,
      85,    90,   457,    74,    88,    88,    74,    88,    88,    69,
     465,    88,     8,    67,   492,    88,    88,    88,   367,    88,
      88,    88,    62,    88,   268,   269,   270,   135,    88,   137,
     244,   139,   140,   282,   149,   513,    66,    42,   283,   494,
     199,   200,   201,   202,   499,   266,   267,    11,   262,   149,
     491,   284,    69,   285,   154,    62,   164,    21,    87,   167,
      62,    68,    62,    69,   149,   520,    23,    73,    68,   154,
      34,    83,   286,    65,   288,    71,    65,    79,     3,     4,
       5,    86,    69,    40,    41,    10,    73,    79,    80,    14,
      79,    80,   230,    67,    19,    85,    67,    62,    72,    84,
      74,    72,    67,    74,   193,   194,   195,   196,   197,   198,
     459,   185,   186,   187,    43,    69,    62,    71,   277,   278,
     279,   280,    68,     0,    81,    82,    90,    52,    60,    61,
      62,    87,    22,   241,    69,    60,    61,    62,    73,    71,
      65,    67,    67,    44,    45,    70,    72,    72,    74,   257,
      30,    31,    69,    67,    44,    45,    73,    84,   372,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      83,   135,    43,   137,    85,   139,   140,    22,    23,    75,
      76,   298,   271,   272,   273,   274,   275,   276,    21,   263,
     264,   265,    87,   157,    69,    40,    41,    87,    62,   163,
     164,    75,    76,   167,    60,    61,    62,   421,    62,   423,
      68,    69,   426,    62,    67,    71,    46,    47,    48,    72,
     184,    74,    22,    23,    36,    37,    38,    39,    36,    37,
      38,    39,    36,    37,    38,    39,    81,    82,    84,   347,
      40,    41,    68,    69,   208,    83,   210,   354,   355,   356,
     357,    68,    69,   467,    46,    47,    48,    68,    69,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,    85,   241,    68,    69,
     244,    81,    82,    68,    69,    68,    69,    68,    69,    68,
      69,    68,    69,   257,    43,   410,    69,   412,   262,   414,
      68,    69,    88,   411,    68,    69,    68,    69,    68,    69,
     410,    67,   412,   496,   414,    62,    66,    62,    62,   427,
      62,    67,   286,    66,   288,   410,    67,   412,   455,   414,
      70,    67,    66,    70,    62,    22,    44,    45,    62,   428,
      66,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    69,    88,    35,   455,    62,   540,   541,     7,
      62,    68,    87,    68,    31,    88,    66,    68,    71,   373,
     455,   359,   306,   430,   358,   490,   362,   360,   486,    87,
     361,   247,   251,   347,   348,   103,   525,   299,   503,   507,
     490,    -1,   509,   487,    -1,   510,   496,   505,    -1,   363,
     364,   365,   491,   503,    -1,   490,   533,   534,   372,    -1,
     510,   496,    -1,    -1,   529,    -1,   531,    -1,   503,   536,
      -1,    -1,    -1,   538,    -1,   510,    -1,    -1,    -1,   529,
      -1,   531,    -1,   533,   534,    -1,    -1,    -1,   538,    -1,
     540,   541,    -1,    -1,   529,    -1,   531,   411,   533,   534,
      -1,    -1,    -1,   538,    -1,   540,   541,   421,    -1,   423,
      -1,    -1,   426,   427,   428,    44,    45,   431,    -1,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     1,    -1,     3,     4,     5,    -1,    -1,    -1,    -1,
      10,    -1,    -1,   467,    14,    -1,    16,    17,    87,    19,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   486,    -1,    -1,    -1,    -1,   491,   492,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,   505,    52,    -1,    -1,    -1,    -1,    -1,    -1,   513,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    24,    25,    26,    -1,    28,    29,
      -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    71,    72,    -1,    -1,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    24,    25,    26,    -1,    28,    29,
      -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    71,    72,    -1,    -1,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    24,    25,    26,    -1,    28,    29,
      -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    71,    72,    -1,    -1,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    24,    25,    26,    -1,    28,    29,
      -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    71,    72,    -1,    -1,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    24,    25,    26,    -1,    28,    29,
      -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,     9,
      10,    11,    12,    13,    -1,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    24,    25,    26,    -1,    28,    29,
      -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    78,     3,
       4,     5,    -1,    -1,    -1,    -1,    10,    11,    88,    -1,
      14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    70,    -1,    72,    -1,
      -1,    75,    76,    77,    78,     3,     4,     5,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    14,    -1,    16,    17,
      -1,    19,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    64,    65,    -1,    67,
      -1,    -1,    70,    -1,    72,    73,    -1,    75,    76,    77,
      78,     3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    -1,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    78,     3,     4,     5,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    14,    -1,
      16,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,    65,
      -1,    67,    -1,    -1,    70,    -1,    72,    73,    -1,    75,
      76,    77,    78,     3,     4,     5,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    14,    -1,    16,    17,    -1,    19,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    78
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    24,    25,
      26,    28,    29,    32,    33,    44,    45,    52,    60,    61,
      62,    63,    64,    65,    67,    70,    72,    75,    76,    77,
      78,    88,    90,    95,    96,   101,   103,   105,   109,   111,
     112,   114,   116,   118,   120,   123,   126,   129,   132,   135,
     138,   141,   144,   147,   151,   152,   153,   154,   159,   164,
     165,   166,   167,   170,   171,   172,   173,   174,   180,   181,
     182,   183,   185,   186,   190,   191,   192,     1,    62,    88,
      67,    10,    14,    62,    70,    94,    95,   100,   102,   187,
      62,   155,   157,    62,   160,   161,     1,    62,    88,    62,
       1,    88,   100,   102,   104,   108,   110,   112,   113,   115,
     117,   119,   121,   124,   127,   130,   133,   136,   139,   142,
     145,   149,   108,   113,   113,    67,   152,    67,   113,    67,
      67,   149,   153,     1,    88,    62,   184,   113,   113,    66,
     113,   113,   149,    71,   191,    69,    97,    98,    99,   113,
     113,   113,   113,    67,    72,    74,   106,    72,    74,   106,
      44,    45,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    87,   148,    65,    79,    80,    75,    76,
      46,    47,    48,    22,    23,    40,    41,    81,    82,    36,
      37,    38,    39,    83,    84,    85,    43,    42,    86,     1,
      69,    88,     0,   192,     1,    88,    11,   108,   119,   122,
     125,   128,   131,   134,   137,   140,   143,   146,   150,   169,
     100,   102,    62,    67,    60,    61,    62,    71,    91,    92,
      93,    72,    74,   106,    87,   162,     1,    69,    88,   162,
       1,    69,    88,     1,    88,    67,   106,    72,    74,   106,
      44,    45,   148,    65,    79,    80,    75,    76,    46,    47,
      48,    22,    23,    40,    41,    81,    82,    36,    37,    38,
      39,    83,    84,    85,    43,    42,    86,     1,    69,    88,
     149,    21,   149,   149,   149,     1,    88,    30,    31,    87,
       1,    74,    88,   152,    68,    71,    69,    73,    73,   145,
      69,    68,   107,   145,   149,    62,   149,    62,   145,   113,
     113,   113,   115,   115,   117,   117,   117,   119,   119,   119,
     119,   119,   119,   121,   121,   121,   121,   124,   127,   130,
     133,   136,   145,   145,    62,   156,   158,    22,   148,    23,
      40,    41,    81,    82,    36,    37,    38,    39,    83,    84,
      85,    43,    42,    86,    69,    88,   106,    67,    62,    68,
     188,    62,    66,    69,    71,   149,    62,   145,   157,   161,
      68,   188,   149,    62,   145,   113,   113,   113,   115,   115,
     117,   117,   117,   119,   119,   119,   119,   119,   119,   121,
     121,   121,   121,   124,   127,   130,   133,   136,   145,   145,
      68,    67,    68,    68,    68,    67,   153,    62,   184,    62,
      79,    98,    68,    69,    73,    73,    66,    22,    87,   163,
      69,    88,   149,   108,   146,   119,   119,   119,   119,   119,
     122,   122,   122,   122,   125,   128,   131,   134,   137,   146,
     146,   149,   168,    68,   188,    70,   189,    68,    69,    67,
     145,    71,    92,    73,   189,    68,    73,    66,   152,   149,
     152,    70,   175,   152,    62,     1,    74,    88,     1,    88,
      73,   145,   145,   145,   149,   146,    22,    62,   158,   168,
      68,    66,    88,   189,    68,    71,   191,   189,    62,    68,
     188,   189,   145,    35,    68,     7,   176,   177,   178,    68,
      68,   149,   163,    88,   152,   146,   168,   189,    71,   189,
      68,   152,   149,     8,    71,   179,   178,   153,   152,    68,
     168,    68,   189,    66,    66,   176,    31,   152,    68,   152,
     191,   191,    71,   153,   152
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 212 "grammar.y"
    { (yyval.node) = new NullNode(); ;}
    break;

  case 3:
#line 213 "grammar.y"
    { (yyval.node) = new BooleanNode(true); ;}
    break;

  case 4:
#line 214 "grammar.y"
    { (yyval.node) = new BooleanNode(false); ;}
    break;

  case 5:
#line 215 "grammar.y"
    { (yyval.node) = new NumberNode((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 6:
#line 216 "grammar.y"
    { (yyval.node) = new StringNode((yyvsp[(1) - (1)].ustr)); ;}
    break;

  case 7:
#line 217 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode(l->pattern, l->flags);
                                        ;}
    break;

  case 8:
#line 222 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode("=" + l->pattern, l->flags);
                                        ;}
    break;

  case 9:
#line 230 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 10:
#line 231 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(Identifier(*(yyvsp[(1) - (1)].ustr))); ;}
    break;

  case 11:
#line 232 "grammar.y"
    { (yyval.pname) = new PropertyNameNode((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 12:
#line 236 "grammar.y"
    { (yyval.pnode) = new PropertyNode((yyvsp[(1) - (3)].pname), (yyvsp[(3) - (3)].node), PropertyNode::Constant); ;}
    break;

  case 13:
#line 237 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (5)].ident), *(yyvsp[(2) - (5)].ident), 0, (yyvsp[(5) - (5)].body))) YYABORT; ;}
    break;

  case 14:
#line 239 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (6)].ident), *(yyvsp[(2) - (6)].ident), (yyvsp[(4) - (6)].param), (yyvsp[(6) - (6)].body))) YYABORT; ;}
    break;

  case 15:
#line 243 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(1) - (1)].pnode)); ;}
    break;

  case 16:
#line 244 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(3) - (3)].pnode), (yyvsp[(1) - (3)].plist)); ;}
    break;

  case 18:
#line 249 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 19:
#line 250 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (3)].plist)); ;}
    break;

  case 20:
#line 252 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (4)].plist)); ;}
    break;

  case 21:
#line 256 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 24:
#line 259 "grammar.y"
    { (yyval.node) = new DynamicResolver<ResolveIdentifier>(*(yyvsp[(1) - (1)].ident), ResolveIdentifier()); ;}
    break;

  case 25:
#line 260 "grammar.y"
    { (yyval.node) = ((yyvsp[(2) - (3)].node)->isResolveNode() || (yyvsp[(2) - (3)].node)->isGroupNode()) ?
                                            (yyvsp[(2) - (3)].node) : new GroupNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 26:
#line 265 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].ival)); ;}
    break;

  case 27:
#line 266 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].elm)); ;}
    break;

  case 28:
#line 267 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(4) - (5)].ival), (yyvsp[(2) - (5)].elm)); ;}
    break;

  case 29:
#line 271 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (2)].ival), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 30:
#line 273 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (4)].elm), (yyvsp[(3) - (4)].ival), (yyvsp[(4) - (4)].node)); ;}
    break;

  case 31:
#line 277 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 33:
#line 282 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 34:
#line 283 "grammar.y"
    { (yyval.ival) = (yyvsp[(1) - (2)].ival) + 1; ;}
    break;

  case 36:
#line 288 "grammar.y"
    { (yyval.node) = (yyvsp[(1) - (1)].funcExpr); ;}
    break;

  case 37:
#line 289 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 38:
#line 290 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 39:
#line 291 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 41:
#line 296 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 42:
#line 297 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 43:
#line 298 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 45:
#line 303 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 47:
#line 308 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 48:
#line 312 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 49:
#line 313 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 50:
#line 314 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 51:
#line 315 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 52:
#line 319 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 53:
#line 320 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 54:
#line 321 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 55:
#line 322 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 56:
#line 326 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 57:
#line 327 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[(2) - (3)].alist)); ;}
    break;

  case 58:
#line 331 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (1)].node)); ;}
    break;

  case 59:
#line 332 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (3)].alist), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 65:
#line 347 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 66:
#line 348 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 68:
#line 353 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 69:
#line 354 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 70:
#line 358 "grammar.y"
    { (yyval.node) = makeDeleteNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 71:
#line 359 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 72:
#line 360 "grammar.y"
    { (yyval.node) = makeTypeOfNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 73:
#line 361 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 74:
#line 362 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 75:
#line 363 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 76:
#line 364 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 77:
#line 365 "grammar.y"
    { (yyval.node) = new UnaryPlusNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 78:
#line 366 "grammar.y"
    { (yyval.node) = new NegateNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 79:
#line 367 "grammar.y"
    { (yyval.node) = new BitwiseNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 80:
#line 368 "grammar.y"
    { (yyval.node) = new LogicalNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 86:
#line 382 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '*'); ;}
    break;

  case 87:
#line 383 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '/'); ;}
    break;

  case 88:
#line 384 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node),'%'); ;}
    break;

  case 90:
#line 390 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '*'); ;}
    break;

  case 91:
#line 392 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '/'); ;}
    break;

  case 92:
#line 394 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node),'%'); ;}
    break;

  case 94:
#line 399 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '+'); ;}
    break;

  case 95:
#line 400 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '-'); ;}
    break;

  case 97:
#line 406 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '+'); ;}
    break;

  case 98:
#line 408 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '-'); ;}
    break;

  case 100:
#line 413 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpLShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 101:
#line 414 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpRShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 102:
#line 415 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpURShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 104:
#line 420 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpLShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 105:
#line 421 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpRShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 106:
#line 422 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpURShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 108:
#line 427 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 109:
#line 428 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 110:
#line 429 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 111:
#line 430 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 112:
#line 431 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 113:
#line 432 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 115:
#line 437 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 116:
#line 438 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 117:
#line 439 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 118:
#line 440 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 119:
#line 442 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 121:
#line 447 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 122:
#line 448 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 123:
#line 449 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 124:
#line 450 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 125:
#line 452 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 126:
#line 453 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 128:
#line 458 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 129:
#line 459 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 130:
#line 460 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 131:
#line 461 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 133:
#line 467 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 134:
#line 469 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 135:
#line 471 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 136:
#line 473 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 138:
#line 479 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 139:
#line 480 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 140:
#line 482 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 141:
#line 484 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 143:
#line 489 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 145:
#line 495 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 147:
#line 500 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 149:
#line 505 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 151:
#line 511 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 153:
#line 517 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 155:
#line 522 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 157:
#line 528 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 159:
#line 534 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 161:
#line 539 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 163:
#line 545 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 165:
#line 551 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 167:
#line 556 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 169:
#line 562 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 171:
#line 567 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 173:
#line 573 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 175:
#line 579 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 177:
#line 585 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 179:
#line 591 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node))) YYABORT; ;}
    break;

  case 181:
#line 597 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node))) YYABORT; ;}
    break;

  case 183:
#line 603 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node))) YYABORT; ;}
    break;

  case 184:
#line 607 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 185:
#line 608 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 186:
#line 609 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 187:
#line 610 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 188:
#line 611 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 189:
#line 612 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 190:
#line 613 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 191:
#line 614 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 192:
#line 615 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 193:
#line 616 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 194:
#line 617 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 195:
#line 618 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 197:
#line 623 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 199:
#line 628 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 201:
#line 633 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 219:
#line 657 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[(2) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 220:
#line 658 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.stat), (yylsp[(3) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 221:
#line 662 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 222:
#line 663 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 223:
#line 667 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 224:
#line 669 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 225:
#line 673 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 226:
#line 675 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 227:
#line 679 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 228:
#line 680 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Variable); ;}
    break;

  case 229:
#line 684 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 230:
#line 685 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Variable); ;}
    break;

  case 231:
#line 689 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 232:
#line 690 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 233:
#line 694 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 234:
#line 696 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 235:
#line 700 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 236:
#line 701 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Constant); ;}
    break;

  case 237:
#line 705 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 238:
#line 709 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 239:
#line 713 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); ;}
    break;

  case 240:
#line 717 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 241:
#line 718 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 242:
#line 723 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 243:
#line 725 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[(3) - (7)].node), (yyvsp[(5) - (7)].stat), (yyvsp[(7) - (7)].stat)); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(4) - (7)])); ;}
    break;

  case 244:
#line 729 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[(2) - (6)].stat), (yyvsp[(5) - (6)].node)); DBG((yyval.stat), (yylsp[(1) - (6)]), (yylsp[(3) - (6)]));;}
    break;

  case 245:
#line 730 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 246:
#line 732 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(3) - (9)].node), (yyvsp[(5) - (9)].node), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 247:
#line 734 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(4) - (10)].vlist), (yyvsp[(6) - (10)].node), (yyvsp[(8) - (10)].node), (yyvsp[(10) - (10)].stat)); DBG((yyval.stat), (yylsp[(1) - (10)]), (yylsp[(9) - (10)])); ;}
    break;

  case 248:
#line 736 "grammar.y"
    {
                                            Node *n = (yyvsp[(3) - (7)].node)->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            (yyval.stat) = new ForInNode(n, (yyvsp[(5) - (7)].node), (yyvsp[(7) - (7)].stat));
                                            DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(6) - (7)]));
                                        ;}
    break;

  case 249:
#line 744 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (8)].ident), 0, (yyvsp[(6) - (8)].node), (yyvsp[(8) - (8)].stat)); DBG((yyval.stat), (yylsp[(1) - (8)]), (yylsp[(7) - (8)])); ;}
    break;

  case 250:
#line 746 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (9)].ident), (yyvsp[(5) - (9)].init), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 251:
#line 750 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 253:
#line 755 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 255:
#line 760 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 256:
#line 761 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 257:
#line 762 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 258:
#line 763 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 259:
#line 767 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 260:
#line 768 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 261:
#line 769 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 262:
#line 770 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 263:
#line 774 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 264:
#line 775 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 265:
#line 776 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 266:
#line 777 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 267:
#line 781 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 268:
#line 785 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].cblk)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 269:
#line 789 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (3)].clist), 0, 0); ;}
    break;

  case 270:
#line 791 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (5)].clist), (yyvsp[(3) - (5)].ccl), (yyvsp[(4) - (5)].clist)); ;}
    break;

  case 271:
#line 795 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 273:
#line 800 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (1)].ccl)); ;}
    break;

  case 274:
#line 801 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (2)].clist), (yyvsp[(2) - (2)].ccl)); ;}
    break;

  case 275:
#line 805 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 276:
#line 806 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].srcs)); ;}
    break;

  case 277:
#line 810 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 278:
#line 811 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[(3) - (3)].srcs)); ;}
    break;

  case 279:
#line 815 "grammar.y"
    { (yyvsp[(3) - (3)].stat)->pushLabel(*(yyvsp[(1) - (3)].ident)); (yyval.stat) = new LabelNode(*(yyvsp[(1) - (3)].ident), (yyvsp[(3) - (3)].stat)); ;}
    break;

  case 280:
#line 819 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 281:
#line 820 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 282:
#line 824 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (4)].stat), CommonIdentifiers::shared()->nullIdentifier, 0, (yyvsp[(4) - (4)].stat)); DBG((yyval.stat), (yylsp[(1) - (4)]), (yylsp[(2) - (4)])); ;}
    break;

  case 283:
#line 825 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (7)].stat), *(yyvsp[(5) - (7)].ident), (yyvsp[(7) - (7)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(2) - (7)])); ;}
    break;

  case 284:
#line 827 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (9)].stat), *(yyvsp[(5) - (9)].ident), (yyvsp[(7) - (9)].stat), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(2) - (9)])); ;}
    break;

  case 285:
#line 831 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 286:
#line 832 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 287:
#line 836 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 288:
#line 837 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode((yyvsp[(1) - (3)].pkgn), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 289:
#line 841 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (5)].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); ;}
    break;

  case 290:
#line 843 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (5)].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); AUTO_SEMICOLON; ;}
    break;

  case 291:
#line 845 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (3)].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 292:
#line 847 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (3)].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 293:
#line 849 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(4) - (5)].pkgn), false, *(yyvsp[(2) - (5)].ident));
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); ;}
    break;

  case 294:
#line 851 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(4) - (5)].pkgn), false, *(yyvsp[(2) - (5)].ident));
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); AUTO_SEMICOLON; ;}
    break;

  case 295:
#line 856 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (5)].ident), (yyvsp[(5) - (5)].body)); ;}
    break;

  case 296:
#line 858 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(4) - (6)].param), (yyvsp[(6) - (6)].body)); ;}
    break;

  case 297:
#line 862 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[(4) - (4)].body)); ;}
    break;

  case 298:
#line 864 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[(5) - (5)].body), (yyvsp[(3) - (5)].param)); ;}
    break;

  case 299:
#line 865 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (5)].ident), (yyvsp[(5) - (5)].body)); ;}
    break;

  case 300:
#line 867 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(6) - (6)].body), (yyvsp[(4) - (6)].param)); ;}
    break;

  case 301:
#line 871 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 302:
#line 872 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[(1) - (3)].param), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 303:
#line 876 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0); DBG((yyval.body), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 304:
#line 877 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.body), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 305:
#line 881 "grammar.y"
    { Parser::accept(new ProgramNode(0)); ;}
    break;

  case 306:
#line 882 "grammar.y"
    { Parser::accept(new ProgramNode((yyvsp[(1) - (1)].srcs))); ;}
    break;

  case 307:
#line 886 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (1)].stat)); ;}
    break;

  case 308:
#line 887 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (2)].srcs), (yyvsp[(2) - (2)].stat)); ;}
    break;

  case 309:
#line 891 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].func); ;}
    break;

  case 310:
#line 892 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].stat); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3537 "grammar.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 895 "grammar.y"


static bool makeAssignNode(Node*& result, Node *loc, Operator op, Node *expr)
{ 
    Node *n = loc->nodeInsideAllParens();

    if (!n->isLocation())
        return false;

    if (n->isResolveNode()) {
        result = makeDynamicResolver<ResolveAssign>(n, op, expr);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        result = new AssignBracketNode(bracket->base(), bracket->subscript(), op, expr);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        result = new AssignDotNode(dot->base(), dot->identifier(), op, expr);
    }

    return true;
}

static bool makePrefixNode(Node*& result, Node *expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return false;
    
    if (n->isResolveNode()) {
        result = makeDynamicResolver<ResolvePrefix>(n, op);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        result = new PrefixBracketNode(bracket->base(), bracket->subscript(), op);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        result = new PrefixDotNode(dot->base(), dot->identifier(), op);
    }

    return true;
}

static bool makePostfixNode(Node*& result, Node *expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return false;
    
    if (n->isResolveNode()) {
        result = makeDynamicResolver<ResolvePostfix>(n, op);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        result = new PostfixBracketNode(bracket->base(), bracket->subscript(), op);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        result = new PostfixDotNode(dot->base(), dot->identifier(), op);
    }

    return true;
}

static Node *makeFunctionCallNode(Node *func, ArgumentsNode *args)
{
    Node *n = func->nodeInsideAllParens();
    
    if (!n->isLocation())
        return new FunctionCallValueNode(func, args);
    else if (n->isResolveNode()) {
        return makeDynamicResolver<ResolveFunctionCall>(n, args);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        if (n != func)
            return new FunctionCallParenBracketNode(bracket->base(), bracket->subscript(), args);
        else
            return new FunctionCallBracketNode(bracket->base(), bracket->subscript(), args);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        if (n != func)
            return new FunctionCallParenDotNode(dot->base(), dot->identifier(), args);
        else
            return new FunctionCallDotNode(dot->base(), dot->identifier(), args);
    }
}

static Node *makeTypeOfNode(Node *expr)
{
    Node *n = expr->nodeInsideAllParens();

    if (n->isResolveNode())
        return makeDynamicResolver<ResolveTypeOf>(n);
    else
        return new TypeOfValueNode(n);
}

static Node *makeDeleteNode(Node *expr)
{
    Node *n = expr->nodeInsideAllParens();
    
    if (!n->isLocation())
        return new DeleteValueNode(expr);
    else if (n->isResolveNode()) {
        return makeDynamicResolver<ResolveDelete>(n);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        return new DeleteBracketNode(bracket->base(), bracket->subscript());
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        return new DeleteDotNode(dot->base(), dot->identifier());
    }
}

static bool makeGetterOrSetterPropertyNode(PropertyNode*& result, Identifier& getOrSet, Identifier& name, ParameterNode *params, FunctionBodyNode *body)
{
    PropertyNode::Type type;
    
    if (getOrSet == "get")
        type = PropertyNode::Getter;
    else if (getOrSet == "set")
        type = PropertyNode::Setter;
    else
        return false;
    
    result = new PropertyNode(new PropertyNameNode(name), 
                              new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, body, params), type);

    return true;
}

static StatementNode *makeImportNode(PackageNameNode *n,
				     bool wildcard, const Identifier &a)
{
    ImportStatement *stat = new ImportStatement(n);
    if (wildcard)
	stat->enableWildcard();
    stat->setAlias(a);

    return stat;
}

/* called by yyparse on error */
int yyerror(const char *)
{
// fprintf(stderr, "ERROR: %s at line %d\n", s, KJS::Lexer::curr()->lineNo());
    return 1;
}

/* may we automatically insert a semicolon ? */
static bool allowAutomaticSemicolon()
{
    return yychar == '}' || yychar == 0 || Lexer::curr()->prevTerminator();
}

