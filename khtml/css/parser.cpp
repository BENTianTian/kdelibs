/* A Bison parser, made from parser.y, by GNU bison 1.75.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON	1

/* Pure parsers.  */
#define YYPURE	1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse cssyyparse
#define yylex   cssyylex
#define yyerror cssyyerror
#define yylval  cssyylval
#define yychar  cssyychar
#define yydebug cssyydebug
#define yynerrs cssyynerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     S = 258,
     SGML_CD = 259,
     INCLUDES = 260,
     DASHMATCH = 261,
     BEGINSWITH = 262,
     ENDSWITH = 263,
     CONTAINS = 264,
     STRING = 265,
     IDENT = 266,
     HASH = 267,
     IMPORT_SYM = 268,
     PAGE_SYM = 269,
     MEDIA_SYM = 270,
     FONT_FACE_SYM = 271,
     CHARSET_SYM = 272,
     NAMESPACE_SYM = 273,
     KONQ_RULE_SYM = 274,
     KONQ_DECLS_SYM = 275,
     KONQ_VALUE_SYM = 276,
     IMPORTANT_SYM = 277,
     QEMS = 278,
     EMS = 279,
     EXS = 280,
     PXS = 281,
     CMS = 282,
     MMS = 283,
     INS = 284,
     PTS = 285,
     PCS = 286,
     DEGS = 287,
     RADS = 288,
     GRADS = 289,
     MSECS = 290,
     SECS = 291,
     HERZ = 292,
     KHERZ = 293,
     DIMEN = 294,
     PERCENTAGE = 295,
     NUMBER = 296,
     URI = 297,
     FUNCTION = 298,
     UNICODERANGE = 299
   };
#endif
#define S 258
#define SGML_CD 259
#define INCLUDES 260
#define DASHMATCH 261
#define BEGINSWITH 262
#define ENDSWITH 263
#define CONTAINS 264
#define STRING 265
#define IDENT 266
#define HASH 267
#define IMPORT_SYM 268
#define PAGE_SYM 269
#define MEDIA_SYM 270
#define FONT_FACE_SYM 271
#define CHARSET_SYM 272
#define NAMESPACE_SYM 273
#define KONQ_RULE_SYM 274
#define KONQ_DECLS_SYM 275
#define KONQ_VALUE_SYM 276
#define IMPORTANT_SYM 277
#define QEMS 278
#define EMS 279
#define EXS 280
#define PXS 281
#define CMS 282
#define MMS 283
#define INS 284
#define PTS 285
#define PCS 286
#define DEGS 287
#define RADS 288
#define GRADS 289
#define MSECS 290
#define SECS 291
#define HERZ 292
#define KHERZ 293
#define DIMEN 294
#define PERCENTAGE 295
#define NUMBER 296
#define URI 297
#define FUNCTION 298
#define UNICODERANGE 299




/* Copy the first part of user declarations.  */
#line 1 "parser.y"


/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002-2003 Lars Knoll (knoll@kde.org)
 *  Copyright (c) 2003 Apple Computer
 *  Copyright (C) 2003 Dirk Mueller (mueller@kde.org)
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
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <stdlib.h>

#include <dom/dom_string.h>
#include <xml/dom_docimpl.h>
#include <css/css_ruleimpl.h>
#include <css/css_stylesheetimpl.h>
#include <css/css_valueimpl.h>
#include <misc/htmlhashes.h>
#include "cssparser.h"

#include <assert.h>
#include <kdebug.h>
// #define CSS_DEBUG

using namespace DOM;

//
// The following file defines the function
//     const struct props *findProp(const char *word, int len)
//
// with 'props->id' a CSS property in the range from CSS_PROP_MIN to
// (and including) CSS_PROP_TOTAL-1

// turn off inlining to void warning with newer gcc
#undef __inline
#define __inline
#include "cssproperties.c"
#include "cssvalues.c"
#undef __inline

int DOM::getPropertyID(const char *tagStr, int len)
{
    const struct props *propsPtr = findProp(tagStr, len);
    if (!propsPtr)
        return 0;

    return propsPtr->id;
}

static inline int getValueID(const char *tagStr, int len)
{
    const struct css_value *val = findValue(tagStr, len);
    if (!val)
        return 0;

    return val->id;
}


#define YYDEBUG 0
#define YYMAXDEPTH 0
#define YYPARSE_PARAM parser


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

#ifndef YYSTYPE
#line 85 "parser.y"
typedef union {
    CSSRuleImpl *rule;
    CSSSelector *selector;
    QPtrList<CSSSelector> *selectorList;
    bool ok;
    MediaListImpl *mediaList;
    CSSMediaRuleImpl *mediaRule;
    CSSRuleListImpl *ruleList;
    ParseString string;
    float val;
    int prop_id;
    unsigned int attribute;
    unsigned int element;
    unsigned int ns;
    CSSSelector::Relation relation;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
} yystype;
/* Line 193 of /usr/share/bison/yacc.c.  */
#line 273 "parser.tab.c"
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} yyltype;
# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

/* Copy the second part of user declarations.  */
#line 106 "parser.y"


static inline int cssyyerror(const char *x ) {
#ifdef CSS_DEBUG
    qDebug( x );
#else
    Q_UNUSED( x );
#endif
    return 1;
}

static int cssyylex( YYSTYPE *yylval ) {
    return CSSParser::current()->lex( yylval );
}




/* Line 213 of /usr/share/bison/yacc.c.  */
#line 311 "parser.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];	\
	}					\
      while (0)
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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  15
#define YYLAST   635

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  58
/* YYNRULES -- Number of rules. */
#define YYNRULES  157
/* YYNRULES -- Number of states. */
#define YYNSTATES  292

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   299

#define YYTRANSLATE(X) \
  ((unsigned)(X) <= YYMAXUTOK ? yytranslate[X] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    59,    17,    54,    53,    56,    14,    60,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    13,    52,
       2,    58,    55,     2,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    15,     2,    57,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,    16,    51,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     9,    12,    15,    18,    25,    31,    37,
      38,    41,    42,    45,    48,    49,    55,    59,    63,    64,
      68,    75,    79,    83,    84,    87,    94,    96,    97,   100,
     101,   105,   107,   109,   111,   113,   115,   117,   119,   121,
     122,   124,   126,   131,   134,   142,   143,   147,   150,   154,
     158,   162,   166,   169,   172,   173,   175,   177,   183,   185,
     190,   193,   195,   199,   202,   205,   209,   212,   215,   217,
     219,   222,   225,   227,   229,   231,   234,   237,   239,   241,
     243,   245,   248,   251,   253,   256,   261,   270,   272,   274,
     276,   278,   280,   282,   284,   286,   289,   293,   299,   301,
     304,   306,   310,   312,   316,   320,   326,   331,   336,   343,
     344,   353,   360,   362,   365,   368,   369,   371,   375,   378,
     381,   384,   385,   387,   390,   393,   396,   399,   402,   404,
     406,   409,   412,   415,   418,   421,   424,   427,   430,   433,
     436,   439,   442,   445,   448,   451,   454,   457,   460,   463,
     469,   472,   476,   480,   483,   489,   493,   495
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      63,     0,    -1,    69,    68,    70,    72,    76,    -1,    64,
      67,    -1,    65,    67,    -1,    66,    67,    -1,    24,    50,
      67,    88,    67,    51,    -1,    25,    50,    67,   104,    51,
      -1,    26,    50,    67,   110,    51,    -1,    -1,    67,     3,
      -1,    -1,    68,     4,    -1,    68,     3,    -1,    -1,    22,
      67,    10,    67,    52,    -1,    22,     1,   118,    -1,    22,
       1,    52,    -1,    -1,    70,    71,    68,    -1,    18,    67,
      78,    67,    79,    52,    -1,    18,     1,   118,    -1,    18,
       1,    52,    -1,    -1,    73,    68,    -1,    23,    67,    75,
      78,    67,    52,    -1,    11,    -1,    -1,    74,    67,    -1,
      -1,    76,    77,    68,    -1,    88,    -1,    81,    -1,    84,
      -1,    85,    -1,   117,    -1,   116,    -1,    10,    -1,    47,
      -1,    -1,    80,    -1,    83,    -1,    80,    53,    67,    83,
      -1,    80,     1,    -1,    20,    67,    80,    50,    67,    82,
      51,    -1,    -1,    82,    88,    67,    -1,    11,    67,    -1,
      19,     1,   118,    -1,    19,     1,    52,    -1,    21,     1,
     118,    -1,    21,     1,    52,    -1,    54,    67,    -1,    55,
      67,    -1,    -1,    56,    -1,    54,    -1,    89,    50,    67,
     104,    51,    -1,    90,    -1,    89,    53,    67,    90,    -1,
      89,     1,    -1,    91,    -1,    90,    86,    91,    -1,    90,
       1,    -1,    92,    67,    -1,    92,    95,    67,    -1,    95,
      67,    -1,    93,    94,    -1,    94,    -1,    16,    -1,    11,
      16,    -1,    17,    16,    -1,    11,    -1,    17,    -1,    96,
      -1,    95,    96,    -1,    95,     1,    -1,    12,    -1,    97,
      -1,   100,    -1,   103,    -1,    14,    11,    -1,    93,    99,
      -1,    99,    -1,    11,    67,    -1,    15,    67,    98,    57,
      -1,    15,    67,    98,   101,    67,   102,    67,    57,    -1,
      58,    -1,     5,    -1,     6,    -1,     7,    -1,     8,    -1,
       9,    -1,    11,    -1,    10,    -1,    13,    11,    -1,    13,
      13,    11,    -1,    13,    48,    67,    91,    59,    -1,   106,
      -1,   105,   106,    -1,   105,    -1,     1,   119,     1,    -1,
       1,    -1,   106,    52,    67,    -1,     1,    52,    67,    -1,
       1,   119,     1,    52,    67,    -1,   105,   106,    52,    67,
      -1,   105,     1,    52,    67,    -1,   105,     1,   119,     1,
      52,    67,    -1,    -1,   108,    13,    67,   110,   109,   107,
     108,     1,    -1,   108,    13,    67,     1,   110,   109,    -1,
     109,    -1,    11,    67,    -1,    27,    67,    -1,    -1,   112,
      -1,   110,   111,   112,    -1,   110,     1,    -1,    60,    67,
      -1,    53,    67,    -1,    -1,   113,    -1,    87,   113,    -1,
      10,    67,    -1,    11,    67,    -1,    47,    67,    -1,    49,
      67,    -1,   115,    -1,   114,    -1,    46,    67,    -1,    45,
      67,    -1,    31,    67,    -1,    32,    67,    -1,    33,    67,
      -1,    34,    67,    -1,    35,    67,    -1,    36,    67,    -1,
      37,    67,    -1,    38,    67,    -1,    39,    67,    -1,    40,
      67,    -1,    41,    67,    -1,    42,    67,    -1,    43,    67,
      -1,    29,    67,    -1,    28,    67,    -1,    30,    67,    -1,
      44,    67,    -1,    48,    67,   110,    59,    67,    -1,    12,
      67,    -1,    61,     1,   118,    -1,    61,     1,    52,    -1,
       1,   118,    -1,    50,     1,   119,     1,    51,    -1,    50,
       1,    51,    -1,   118,    -1,   119,     1,   118,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   240,   240,   242,   243,   244,   247,   254,   260,   285,
     287,   290,   292,   293,   296,   298,   303,   304,   307,   309,
     319,   330,   333,   339,   340,   344,   348,   351,   353,   356,
     358,   368,   370,   371,   372,   373,   374,   377,   379,   382,
     386,   390,   395,   400,   406,   420,   422,   431,   453,   457,
     462,   466,   471,   473,   474,   477,   479,   482,   502,   516,
     530,   536,   540,   561,   567,   572,   577,   584,   586,   594,
     596,   597,   601,   622,   625,   630,   640,   646,   653,   654,
     655,   658,   667,   669,   675,   699,   705,   713,   717,   720,
     723,   726,   729,   734,   736,   739,   745,   751,   759,   763,
     768,   771,   777,   785,   789,   795,   801,   806,   812,   821,
     820,   846,   852,   859,   866,   868,   871,   876,   889,   895,
     899,   902,   907,   909,   910,   911,   917,   918,   919,   921,
     926,   928,   929,   930,   931,   932,   933,   934,   935,   936,
     937,   938,   939,   940,   941,   942,   943,   944,   945,   949,
     964,   971,   978,   986,  1012,  1014,  1017,  1019
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "S", "SGML_CD", "INCLUDES", "DASHMATCH", 
  "BEGINSWITH", "ENDSWITH", "CONTAINS", "STRING", "IDENT", "HASH", "':'", 
  "'.'", "'['", "'|'", "'*'", "IMPORT_SYM", "PAGE_SYM", "MEDIA_SYM", 
  "FONT_FACE_SYM", "CHARSET_SYM", "NAMESPACE_SYM", "KONQ_RULE_SYM", 
  "KONQ_DECLS_SYM", "KONQ_VALUE_SYM", "IMPORTANT_SYM", "QEMS", "EMS", 
  "EXS", "PXS", "CMS", "MMS", "INS", "PTS", "PCS", "DEGS", "RADS", 
  "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", "DIMEN", "PERCENTAGE", 
  "NUMBER", "URI", "FUNCTION", "UNICODERANGE", "'{'", "'}'", "';'", "','", 
  "'+'", "'>'", "'-'", "']'", "'='", "')'", "'/'", "'@'", "$accept", 
  "stylesheet", "konq_rule", "konq_decls", "konq_value", "maybe_space", 
  "maybe_sgml", "maybe_charset", "import_list", "import", 
  "maybe_namespace", "namespace", "ns_prefix", "maybe_ns_prefix", 
  "rule_list", "rule", "string_or_uri", "maybe_media_list", "media_list", 
  "media", "ruleset_list", "medium", "page", "font_face", "combinator", 
  "unary_operator", "ruleset", "selector_list", "selector", 
  "simple_selector", "ns_element", "ns_selector", "element_name", 
  "specifier_list", "specifier", "class", "ns_attrib_id", "attrib_id", 
  "attrib", "match", "ident_or_string", "pseudo", "declaration_list", 
  "decl_list", "declaration", "@1", "property", "prio", "expr", 
  "operator", "term", "unary_term", "function", "hexcolor", "invalid_at", 
  "invalid_rule", "invalid_block", "invalid_block_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,    58,    46,    91,   124,    42,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     123,   125,    59,    44,    43,    62,    45,    93,    61,    41,
      47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    62,    63,    63,    63,    63,    64,    65,    66,    67,
      67,    68,    68,    68,    69,    69,    69,    69,    70,    70,
      71,    71,    71,    72,    72,    73,    74,    75,    75,    76,
      76,    77,    77,    77,    77,    77,    77,    78,    78,    79,
      79,    80,    80,    80,    81,    82,    82,    83,    84,    84,
      85,    85,    86,    86,    86,    87,    87,    88,    89,    89,
      89,    90,    90,    90,    91,    91,    91,    92,    92,    93,
      93,    93,    94,    94,    95,    95,    95,    96,    96,    96,
      96,    97,    98,    98,    99,   100,   100,   101,   101,   101,
     101,   101,   101,   102,   102,   103,   103,   103,   104,   104,
     104,   104,   104,   105,   105,   105,   105,   105,   105,   107,
     106,   106,   106,   108,   109,   109,   110,   110,   110,   111,
     111,   111,   112,   112,   112,   112,   112,   112,   112,   112,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   114,
     115,   116,   116,   117,   118,   118,   119,   119
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     5,     2,     2,     2,     6,     5,     5,     0,
       2,     0,     2,     2,     0,     5,     3,     3,     0,     3,
       6,     3,     3,     0,     2,     6,     1,     0,     2,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     1,     4,     2,     7,     0,     3,     2,     3,     3,
       3,     3,     2,     2,     0,     1,     1,     5,     1,     4,
       2,     1,     3,     2,     2,     3,     2,     2,     1,     1,
       2,     2,     1,     1,     1,     2,     2,     1,     1,     1,
       1,     2,     2,     1,     2,     4,     8,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     3,     5,     1,     2,
       1,     3,     1,     3,     3,     5,     4,     4,     6,     0,
       8,     6,     1,     2,     2,     0,     1,     3,     2,     2,
       2,     0,     1,     2,     2,     2,     2,     2,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     5,
       2,     3,     3,     2,     5,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      14,     0,     0,     0,     0,     0,     9,     9,     9,    11,
       0,     0,     9,     9,     9,     1,     3,     4,     5,    18,
       0,    17,    16,    10,     9,     0,     0,     0,    13,    12,
      23,     0,     0,    72,    77,     0,     0,     9,    69,    73,
       9,     0,     0,    61,     9,     0,    68,     0,    74,    78,
      79,    80,   102,     9,     9,     0,     0,    98,     0,   112,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     9,    56,    55,     0,     0,   116,
     122,   129,   128,     0,     9,    11,    29,    11,   155,   156,
       0,    15,    70,    95,     0,     9,    81,     0,    71,     0,
      60,     9,     9,    63,     9,     9,     0,    64,     0,    72,
      73,    67,    76,    66,    75,     9,     0,   113,   114,     7,
       0,    99,     9,     9,   124,   125,   150,   146,   145,   147,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   148,   131,   130,   126,     0,   127,   123,
     118,     8,     9,     9,     0,     0,     0,    27,    19,     0,
      24,     0,    96,     0,     9,     0,     0,     0,    83,     6,
       0,     0,    52,    53,    62,    65,   104,   101,     9,     0,
       9,   103,     0,     0,   120,   119,   117,    22,    21,    37,
      38,     9,    26,     9,     0,     0,     0,     9,     0,     0,
      11,    32,    33,    34,    31,    36,    35,   154,   157,     0,
      84,     9,    82,    88,    89,    90,    91,    92,    85,    87,
       9,     0,     0,     9,   107,     0,   106,     0,     0,     9,
      39,    28,     9,   153,     0,     0,     0,     0,    30,    97,
       0,    57,   105,     9,     0,   109,   149,     9,     0,     0,
      41,     0,    49,    48,     0,    51,    50,   152,   151,    94,
      93,     9,   108,   111,     0,    47,    20,    43,     9,    25,
       9,     0,     0,     0,    45,    86,   110,    42,     0,    44,
       9,    46
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     5,     6,     7,     8,   220,    19,     9,    30,    95,
      96,    97,   203,   204,   169,   210,   201,   258,   259,   211,
     288,   260,   212,   213,   116,    87,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,   177,   178,    50,   230,
     271,    51,    55,    56,    57,   274,    58,    59,    88,   164,
      89,    90,    91,    92,   215,   216,    99,   100
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -224
static const short yypact[] =
{
     128,    46,   -48,   -25,   -15,    51,  -224,  -224,  -224,  -224,
      -2,   112,  -224,  -224,  -224,  -224,    35,    35,    35,   126,
      94,  -224,  -224,  -224,  -224,   319,   215,   520,  -224,  -224,
       3,   169,    38,   105,  -224,     6,   115,  -224,  -224,   127,
    -224,    95,   154,  -224,   210,   117,  -224,   124,  -224,  -224,
    -224,  -224,    66,  -224,  -224,    98,   220,   108,   133,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,   589,   317,  -224,
    -224,  -224,  -224,   141,  -224,  -224,  -224,  -224,  -224,  -224,
     162,  -224,  -224,  -224,   161,  -224,  -224,    17,  -224,    37,
    -224,  -224,  -224,  -224,  -224,  -224,   326,    35,   124,  -224,
    -224,  -224,  -224,    35,  -224,  -224,   175,    35,    35,  -224,
     107,   138,  -224,  -224,    35,    35,    35,    35,    35,    35,
      35,    35,    35,    35,    35,    35,    35,    35,    35,    35,
      35,    35,    35,    35,    35,    35,    35,   520,    35,  -224,
    -224,  -224,  -224,  -224,   560,   123,   109,    21,   126,    86,
     126,   198,  -224,   319,   105,   127,   216,   205,  -224,  -224,
     215,   319,    35,    35,  -224,    35,    35,   134,  -224,   235,
    -224,    35,   473,   368,    35,    35,  -224,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,     8,   194,   245,  -224,   249,   250,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,   195,
      35,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,   202,   180,  -224,    35,   149,    35,   560,   419,  -224,
      34,    35,  -224,  -224,   153,    34,   165,   193,   126,  -224,
      19,  -224,    35,  -224,   263,  -224,    35,  -224,   203,    56,
    -224,    39,  -224,  -224,   132,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,    35,  -224,   246,    35,  -224,  -224,  -224,  -224,
    -224,     1,   257,    34,    35,  -224,  -224,  -224,   269,  -224,
    -224,    35
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -224,  -224,  -224,  -224,  -224,    -1,   -87,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,    55,  -224,    15,  -224,
    -224,   -22,  -224,  -224,  -224,  -224,  -168,  -224,    84,   -89,
    -224,   171,   223,   225,   -33,  -224,  -224,   100,  -224,  -224,
    -224,  -224,   144,  -224,   231,  -224,    14,  -223,  -148,  -224,
     157,   226,  -224,  -224,  -224,  -224,    -7,   -36
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, parse error.  */
#define YYTABLE_NINF -122
static const short yytable[] =
{
      11,   214,    12,    22,    23,    16,    17,    18,   168,   193,
     170,    25,    26,    27,   124,   255,   126,   103,   199,   104,
      23,    93,    23,    32,    23,    13,    94,   184,   174,   269,
     270,   273,   202,    38,   175,    14,   107,    23,    23,   109,
      23,    23,    23,   117,   238,   257,   123,    10,    20,    -9,
      21,    15,   127,   128,   105,   200,    -9,   277,   285,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   219,   124,    -2,   205,   179,   254,
     101,   279,   166,   167,   189,    31,   110,    33,    34,    35,
      36,    37,    38,    39,   173,   206,   207,   208,   -40,   278,
     180,   181,    23,   182,   183,    23,    20,   185,   125,   199,
     290,   102,    24,   248,   186,   122,   106,    -9,   119,    28,
      29,   191,   192,   277,   120,    -9,    34,    35,    36,    37,
      -9,    -9,   165,   108,    -9,   111,   133,   209,   112,   129,
       1,    -9,     2,     3,     4,   113,   200,    20,   198,   188,
     132,   194,   195,   171,   218,   -54,   -54,   -54,   -54,   -54,
     -54,   -54,   172,    20,    -9,   197,   187,    -9,    -9,    -9,
     218,   113,   280,    -9,    20,   278,   233,   234,    -9,   236,
     190,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   243,    20,
     240,   253,   241,    20,   -58,   262,   245,   -58,   114,   115,
     223,   224,   225,   226,   227,    20,    52,   265,    23,    20,
      98,   130,    34,    35,    36,    37,    53,   221,   218,   250,
     -59,    53,   252,   -59,   114,   115,   235,   263,   256,   266,
     268,   261,    54,    20,    20,   267,   244,    54,    20,   217,
     246,   247,   272,   251,   249,   276,   275,    53,   286,   242,
     264,   287,   228,   229,   160,   232,  -115,  -115,   121,   118,
     281,  -100,  -115,  -121,  -121,  -121,   222,   283,   176,   284,
      33,    34,    35,    36,    37,    38,    39,   131,   282,   291,
      54,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,
    -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,
    -121,  -121,  -121,   159,  -115,  -115,   162,  -121,   160,  -121,
     289,   196,    23,   163,   231,     0,     0,  -121,  -121,  -121,
      33,    34,    35,    36,    37,    38,    39,    33,    34,    35,
      36,    37,    38,    39,     0,  -121,  -121,  -121,  -121,  -121,
    -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,
    -121,  -121,  -121,  -121,  -121,  -121,  -121,     0,   161,   160,
     162,  -121,     0,  -121,     0,     0,     0,   163,  -121,  -121,
    -121,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -121,  -121,  -121,  -121,
    -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,
    -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,     0,     0,
     160,   162,  -121,     0,  -121,     0,     0,   239,   163,  -121,
    -115,  -121,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    54,  -121,  -121,  -121,
    -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,
    -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,  -121,     0,
       0,     0,   162,  -121,   237,  -121,    23,     0,     0,   163,
       0,     0,     0,    60,    61,    62,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    23,     0,     0,     0,    85,     0,    86,
      60,    61,    62,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      60,    61,    62,     0,    85,     0,    86,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
       0,     0,     0,     0,    85,     0,    86,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81
};

static const short yycheck[] =
{
       1,   169,    50,    10,     3,     6,     7,     8,    95,   157,
      97,    12,    13,    14,    47,   238,    52,    11,    10,    13,
       3,    18,     3,    24,     3,    50,    23,   116,    11,    10,
      11,   254,    11,    16,    17,    50,    37,     3,     3,    40,
       3,     3,     3,    44,   192,    11,    47,     1,    50,     3,
      52,     0,    53,    54,    48,    47,    10,     1,    57,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,   173,   118,     0,     1,    51,   237,
      52,    52,    93,    94,   130,     1,     1,    11,    12,    13,
      14,    15,    16,    17,   105,    19,    20,    21,    52,    53,
     111,   112,     3,   114,   115,     3,    50,   118,    52,    10,
     288,    16,    10,   210,   125,     1,    11,     3,    11,     3,
       4,   132,   133,     1,    17,    11,    12,    13,    14,    15,
      16,    17,     1,    16,     3,    50,    13,    61,    53,    51,
      22,    10,    24,    25,    26,     1,    47,    50,   165,    52,
      52,   162,   163,     1,   171,    11,    12,    13,    14,    15,
      16,    17,    11,    50,    50,    52,     1,    53,    54,    55,
     187,     1,    50,    59,    50,    53,    52,   188,    47,   190,
      52,    11,    12,    13,    14,    15,    16,    17,   205,    50,
     201,    52,   203,    50,    50,    52,   207,    53,    54,    55,
       5,     6,     7,     8,     9,    50,     1,    52,     3,    50,
      51,     1,    12,    13,    14,    15,    11,    11,   235,   230,
      50,    11,   233,    53,    54,    55,     1,   244,   239,   246,
     247,   242,    27,    50,    50,    52,     1,    27,    50,    51,
       1,     1,   253,    51,    59,    52,   257,    11,     1,   204,
     245,   283,    57,    58,     1,   181,    51,    52,    45,    44,
     271,    51,    52,    10,    11,    12,   176,   278,   107,   280,
      11,    12,    13,    14,    15,    16,    17,    56,   274,   290,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    87,    51,    52,    53,    54,     1,    56,
      51,   164,     3,    60,   180,    -1,    -1,    10,    11,    12,
      11,    12,    13,    14,    15,    16,    17,    11,    12,    13,
      14,    15,    16,    17,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    51,     1,
      53,    54,    -1,    56,    -1,    -1,    -1,    60,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    -1,
       1,    53,    54,    -1,    56,    -1,    -1,    59,    60,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      -1,    -1,    53,    54,     1,    56,     3,    -1,    -1,    60,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,     3,    -1,    -1,    -1,    54,    -1,    56,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      10,    11,    12,    -1,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    22,    24,    25,    26,    63,    64,    65,    66,    69,
       1,    67,    50,    50,    50,     0,    67,    67,    67,    68,
      50,    52,   118,     3,    10,    67,    67,    67,     3,     4,
      70,     1,    67,    11,    12,    13,    14,    15,    16,    17,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
     100,   103,     1,    11,    27,   104,   105,   106,   108,   109,
      10,    11,    12,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    54,    56,    87,   110,   112,
     113,   114,   115,    18,    23,    71,    72,    73,    51,   118,
     119,    52,    16,    11,    13,    48,    11,    67,    16,    67,
       1,    50,    53,     1,    54,    55,    86,    67,    95,    11,
      17,    94,     1,    67,    96,    52,   119,    67,    67,    51,
       1,   106,    52,    13,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,   113,
       1,    51,    53,    60,   111,     1,    67,    67,    68,    76,
      68,     1,    11,    67,    11,    17,    93,    98,    99,    51,
      67,    67,    67,    67,    91,    67,    67,     1,    52,   119,
      52,    67,    67,   110,    67,    67,   112,    52,   118,    10,
      47,    78,    11,    74,    75,     1,    19,    20,    21,    61,
      77,    81,    84,    85,    88,   116,   117,    51,   118,    91,
      67,    11,    99,     5,     6,     7,     8,     9,    57,    58,
     101,   104,    90,    52,    67,     1,    67,     1,   110,    59,
      67,    67,    78,   118,     1,    67,     1,     1,    68,    59,
      67,    51,    67,    52,   110,   109,    67,    11,    79,    80,
      83,    67,    52,   118,    80,    52,   118,    52,   118,    10,
      11,   102,    67,   109,   107,    67,    52,     1,    53,    52,
      50,    67,   108,    67,    67,    57,     1,    83,    82,    51,
      88,    67
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

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
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)           \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX	yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX	yylex (&yylval)
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
} while (0)
# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*-----------------------------.
| Print this symbol on YYOUT.  |
`-----------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yysymprint (FILE* yyout, int yytype, YYSTYPE yyvalue)
#else
yysymprint (yyout, yytype, yyvalue)
    FILE* yyout;
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyout, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyout, yytoknum[yytype], yyvalue);
# endif
    }
  else
    YYFPRINTF (yyout, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyout, ")");
}
#endif /* YYDEBUG. */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yydestruct (int yytype, YYSTYPE yyvalue)
#else
yydestruct (yytype, yyvalue)
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  switch (yytype)
    {
      default:
        break;
    }
}



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif




int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of parse errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

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

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with.  */

  if (yychar <= 0)		/* This means end of input.  */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more.  */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

      /* We have to keep this `#if YYDEBUG', since we use variables
	 which are defined only if `YYDEBUG' is set.  */
      YYDPRINTF ((stderr, "Next token is "));
      YYDSYMPRINT ((stderr, yychar1, yylval));
      YYDPRINTF ((stderr, "\n"));
    }

  /* If the proper action on seeing token YYCHAR1 is to reduce or to
     detect an error, take that action.  */
  yyn += yychar1;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yychar1)
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

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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



#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn - 1, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] >= 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif
  switch (yyn)
    {
        case 6:
#line 248 "parser.y"
    {
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = yyvsp[-2].rule;
    }
    break;

  case 7:
#line 255 "parser.y"
    {
	/* can be empty */
    }
    break;

  case 8:
#line 261 "parser.y"
    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-1].valueList ) {
	    p->valueList = yyvsp[-1].valueList;
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got property for " << p->id <<
		(p->important?" important":"")<< endl;
	    bool ok =
#endif
		p->parseValue( p->id, p->important );
#ifdef CSS_DEBUG
	    if ( !ok )
		kdDebug( 6080 ) << "     couldn't parse value!" << endl;
#endif
	}
#ifdef CSS_DEBUG
	else
	    kdDebug( 6080 ) << "     no value found!" << endl;
#endif
	delete p->valueList;
	p->valueList = 0;
    }
    break;

  case 15:
#line 298 "parser.y"
    {
#ifdef CSS_DEBUG
     kdDebug( 6080 ) << "charset rule: " << qString(yyvsp[-2].string) << endl;
#endif
 }
    break;

  case 19:
#line 309 "parser.y"
    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 }
    break;

  case 20:
#line 320 "parser.y"
    {
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "@import: " << qString(yyvsp[-3].string) << endl;
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( p->styleElement && p->styleElement->isCSSStyleSheet() )
	    yyval.rule = new CSSImportRuleImpl( p->styleElement, domString(yyvsp[-3].string), yyvsp[-1].mediaList );
	else
	    yyval.rule = 0;
    }
    break;

  case 21:
#line 330 "parser.y"
    {
        yyval.rule = 0;
    }
    break;

  case 22:
#line 333 "parser.y"
    {
        yyval.rule = 0;
    }
    break;

  case 27:
#line 352 "parser.y"
    { yyval.string.string = 0; yyval.string.length = 0; }
    break;

  case 30:
#line 358 "parser.y"
    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 }
    break;

  case 39:
#line 383 "parser.y"
    {
	yyval.mediaList = 0;
    }
    break;

  case 41:
#line 391 "parser.y"
    {
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    }
    break;

  case 42:
#line 395 "parser.y"
    {
	yyval.mediaList = yyvsp[-3].mediaList;
	if (yyval.mediaList)
	    yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    }
    break;

  case 43:
#line 400 "parser.y"
    {
	delete yyvsp[-1].mediaList;
	yyval.mediaList = 0;
    }
    break;

  case 44:
#line 407 "parser.y"
    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].mediaList && yyvsp[-1].ruleList &&
	     p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	    yyval.rule = new CSSMediaRuleImpl( static_cast<CSSStyleSheetImpl*>(p->styleElement), yyvsp[-4].mediaList, yyvsp[-1].ruleList );
	} else {
	    yyval.rule = 0;
	    delete yyvsp[-4].mediaList;
	    delete yyvsp[-1].ruleList;
	}
    }
    break;

  case 45:
#line 421 "parser.y"
    { yyval.ruleList = 0; }
    break;

  case 46:
#line 422 "parser.y"
    {
      yyval.ruleList = yyvsp[-2].ruleList;
      if ( yyvsp[-1].rule ) {
	  if ( !yyval.ruleList ) yyval.ruleList = new CSSRuleListImpl();
	  yyval.ruleList->append( yyvsp[-1].rule );
      }
  }
    break;

  case 47:
#line 432 "parser.y"
    {
      yyval.string = yyvsp[-1].string;
  }
    break;

  case 48:
#line 454 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 49:
#line 457 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 50:
#line 463 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 51:
#line 466 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 52:
#line 472 "parser.y"
    { yyval.relation = CSSSelector::Sibling; }
    break;

  case 53:
#line 473 "parser.y"
    { yyval.relation = CSSSelector::Child; }
    break;

  case 54:
#line 474 "parser.y"
    { yyval.relation = CSSSelector::Descendant; }
    break;

  case 55:
#line 478 "parser.y"
    { yyval.val = -1; }
    break;

  case 56:
#line 479 "parser.y"
    { yyval.val = 1; }
    break;

  case 57:
#line 483 "parser.y"
    {
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "got ruleset" << endl << "  selector:" << endl;
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].selectorList && yyvsp[-1].ok && p->numParsedProperties ) {
	    CSSStyleRuleImpl *rule = new CSSStyleRuleImpl( p->styleElement );
	    CSSStyleDeclarationImpl *decl = p->createStyleDeclaration( rule );
	    rule->setSelector( yyvsp[-4].selectorList );
	    rule->setDeclaration(decl);
	    yyval.rule = rule;
	} else {
	    yyval.rule = 0;
	    delete yyvsp[-4].selectorList;
	    p->clearProperties();
	}
    }
    break;

  case 58:
#line 503 "parser.y"
    {
	if ( yyvsp[0].selector ) {
	    yyval.selectorList = new QPtrList<CSSSelector>;
            yyval.selectorList->setAutoDelete( true );
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	    yyval.selectorList->append( yyvsp[0].selector );
	} else {
	    yyval.selectorList = 0;
	}
    }
    break;

  case 59:
#line 516 "parser.y"
    {
	if ( yyvsp[-3].selectorList && yyvsp[0].selector ) {
	    yyval.selectorList = yyvsp[-3].selectorList;
	    yyval.selectorList->append( yyvsp[0].selector );
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	} else {
	    delete yyvsp[-3].selectorList;
	    delete yyvsp[0].selector;
	    yyval.selectorList = 0;
	}
    }
    break;

  case 60:
#line 530 "parser.y"
    {
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    }
    break;

  case 61:
#line 537 "parser.y"
    {
	yyval.selector = yyvsp[0].selector;
    }
    break;

  case 62:
#line 540 "parser.y"
    {
	if ( !yyvsp[-2].selector || !yyvsp[0].selector ) {
	    delete yyvsp[-2].selector;
	    delete yyvsp[0].selector;
	    yyval.selector = 0;
	} else {
	    yyval.selector = yyvsp[0].selector;
	    CSSSelector *end = yyvsp[0].selector;
	    while( end->tagHistory )
		end = end->tagHistory;
	    end->relation = yyvsp[-1].relation;
	    end->tagHistory = yyvsp[-2].selector;
	    if ( yyvsp[-1].relation == CSSSelector::Descendant ||
		 yyvsp[-1].relation == CSSSelector::Child ) {
		CSSParser *p = static_cast<CSSParser *>(parser);
		DOM::DocumentImpl *doc = p->document();
		if ( doc )
		    doc->setUsesDescendantRules(true);
	    }
	}
    }
    break;

  case 63:
#line 561 "parser.y"
    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    }
    break;

  case 64:
#line 568 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    }
    break;

  case 65:
#line 572 "parser.y"
    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    }
    break;

  case 66:
#line 577 "parser.y"
    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = 0xffffffff;
    }
    break;

  case 67:
#line 585 "parser.y"
    { yyval.element = (yyvsp[-1].ns<<16) | yyvsp[0].element; }
    break;

  case 68:
#line 586 "parser.y"
    {
        /* according to the specs this one matches all namespaces if no
	   default namespace has been specified otherwise the default namespace */
	CSSParser *p = static_cast<CSSParser *>(parser);
	yyval.element = (p->defaultNamespace<<16) | yyvsp[0].element;
    }
    break;

  case 69:
#line 595 "parser.y"
    { yyval.ns = 0; }
    break;

  case 70:
#line 596 "parser.y"
    { yyval.ns = 1; /* #### insert correct namespace id here */ }
    break;

  case 71:
#line 597 "parser.y"
    { yyval.ns = 0xffff; }
    break;

  case 72:
#line 602 "parser.y"
    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();
	QString tag = qString(yyvsp[0].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		tag = tag.lower();
	    const DOMString dtag(tag);
#ifdef APPLE_CHANGES
            yyval.element = doc->tagId(0, dtag.implementation(), false);
#else
	    yyval.element = doc->elementNames()->getId(dtag.implementation(), false);
#endif
	} else {
	    yyval.element = khtml::getTagID(tag.lower().ascii(), tag.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown tags
// 	    assert($$ != 0);
	}
    }
    break;

  case 73:
#line 622 "parser.y"
    { yyval.element = 0xffff; }
    break;

  case 74:
#line 626 "parser.y"
    {
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    }
    break;

  case 75:
#line 630 "parser.y"
    {
	yyval.selector = yyvsp[-1].selector;
	if ( yyval.selector ) {
            CSSSelector *end = yyvsp[-1].selector;
            while( end->tagHistory )
                end = end->tagHistory;
            end->relation = CSSSelector::SubSelector;
            end->tagHistory = yyvsp[0].selector;
	}
    }
    break;

  case 76:
#line 640 "parser.y"
    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    }
    break;

  case 77:
#line 647 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 81:
#line 659 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 82:
#line 668 "parser.y"
    { yyval.attribute = (yyvsp[-1].ns<<16) | yyvsp[0].attribute; }
    break;

  case 83:
#line 669 "parser.y"
    {
	/* opposed to elements, these only match for non namespaced attributes */
	yyval.attribute = yyvsp[0].attribute;
    }
    break;

  case 84:
#line 676 "parser.y"
    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();

	QString attr = qString(yyvsp[-1].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		attr = attr.lower();
	    const DOMString dattr(attr);
#ifdef APPLE_CHANGES
            yyval.attribute = doc->attrId(0, dattr.implementation(), false);
#else
	    yyval.attribute = doc->attrNames()->getId(dattr.implementation(), false);
#endif
	} else {
	    yyval.attribute = khtml::getAttrID(attr.lower().ascii(), attr.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown attributes
	    assert(yyval.attribute != 0);
	    }
    }
    break;

  case 85:
#line 700 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    }
    break;

  case 86:
#line 705 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = (CSSSelector::Match)yyvsp[-4].val;
	yyval.selector->value = domString(yyvsp[-2].string);
    }
    break;

  case 87:
#line 714 "parser.y"
    {
	yyval.val = CSSSelector::Exact;
    }
    break;

  case 88:
#line 717 "parser.y"
    {
	yyval.val = CSSSelector::List;
    }
    break;

  case 89:
#line 720 "parser.y"
    {
	yyval.val = CSSSelector::Hyphen;
    }
    break;

  case 90:
#line 723 "parser.y"
    {
	yyval.val = CSSSelector::Begin;
    }
    break;

  case 91:
#line 726 "parser.y"
    {
	yyval.val = CSSSelector::End;
    }
    break;

  case 92:
#line 729 "parser.y"
    {
	yyval.val = CSSSelector::Contain;
    }
    break;

  case 95:
#line 740 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 96:
#line 746 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 97:
#line 751 "parser.y"
    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->simpleSelector = yyvsp[-1].selector;
        yyval.selector->value = domString(yyvsp[-3].string);
    }
    break;

  case 98:
#line 760 "parser.y"
    {
	yyval.ok = yyvsp[0].ok;
    }
    break;

  case 99:
#line 763 "parser.y"
    {
	yyval.ok = yyvsp[-1].ok;
	if ( yyvsp[0].ok )
	    yyval.ok = yyvsp[0].ok;
    }
    break;

  case 100:
#line 768 "parser.y"
    {
	yyval.ok = yyvsp[0].ok;
    }
    break;

  case 101:
#line 771 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 102:
#line 777 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping all declarations" << endl;
#endif
    }
    break;

  case 103:
#line 786 "parser.y"
    {
	yyval.ok = yyvsp[-2].ok;
    }
    break;

  case 104:
#line 789 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 105:
#line 795 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 106:
#line 801 "parser.y"
    {
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    }
    break;

  case 107:
#line 806 "parser.y"
    {
	yyval.ok = yyvsp[-3].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 108:
#line 812 "parser.y"
    {
	yyval.ok = yyvsp[-5].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 109:
#line 821 "parser.y"
    {
	yyval.ok = false;
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].prop_id && yyvsp[-1].valueList ) {
	    p->valueList = yyvsp[-1].valueList;
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got property: " << yyvsp[-4].prop_id <<
		(yyvsp[0].b?" important":"")<< endl;
#endif
	        bool ok = p->parseValue( yyvsp[-4].prop_id, yyvsp[0].b );
                if ( ok )
		    yyval.ok = ok;
#ifdef CSS_DEBUG
	        else
		    kdDebug( 6080 ) << "     couldn't parse value!" << endl;
#endif
	} else {
            delete yyvsp[-1].valueList;
        }
	delete p->valueList;
	p->valueList = 0;
    }
    break;

  case 110:
#line 843 "parser.y"
    {
        yyval.ok = false;
    }
    break;

  case 111:
#line 847 "parser.y"
    {
	/* letter-spacing: .none; */
        delete yyvsp[-1].valueList;
        yyval.ok = false;
    }
    break;

  case 112:
#line 853 "parser.y"
    {
        /* div { text-align: center; !important } */
        yyval.ok = false;
    }
    break;

  case 113:
#line 860 "parser.y"
    {
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    }
    break;

  case 114:
#line 867 "parser.y"
    { yyval.b = true; }
    break;

  case 115:
#line 868 "parser.y"
    { yyval.b = false; }
    break;

  case 116:
#line 872 "parser.y"
    {
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    }
    break;

  case 117:
#line 876 "parser.y"
    {
	yyval.valueList = yyvsp[-2].valueList;
	if ( yyval.valueList ) {
	    if ( yyvsp[-1].tok ) {
		Value v;
		v.id = 0;
		v.unit = Value::Operator;
		v.iValue = yyvsp[-1].tok;
		yyval.valueList->addValue( v );
	    }
	    yyval.valueList->addValue( yyvsp[0].value );
	}
    }
    break;

  case 118:
#line 889 "parser.y"
    {
	delete yyvsp[-1].valueList;
	yyval.valueList = 0;
    }
    break;

  case 119:
#line 896 "parser.y"
    {
	yyval.tok = '/';
    }
    break;

  case 120:
#line 899 "parser.y"
    {
	yyval.tok = ',';
    }
    break;

  case 121:
#line 902 "parser.y"
    {
        yyval.tok = 0;
  }
    break;

  case 122:
#line 908 "parser.y"
    { yyval.value = yyvsp[0].value; }
    break;

  case 123:
#line 909 "parser.y"
    { yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; }
    break;

  case 124:
#line 910 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; }
    break;

  case 125:
#line 911 "parser.y"
    {
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  }
    break;

  case 126:
#line 917 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; }
    break;

  case 127:
#line 918 "parser.y"
    { yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ }
    break;

  case 128:
#line 919 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; }
    break;

  case 129:
#line 921 "parser.y"
    {
      yyval.value = yyvsp[0].value;
  }
    break;

  case 130:
#line 927 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; }
    break;

  case 131:
#line 928 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; }
    break;

  case 132:
#line 929 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; }
    break;

  case 133:
#line 930 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; }
    break;

  case 134:
#line 931 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; }
    break;

  case 135:
#line 932 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; }
    break;

  case 136:
#line 933 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; }
    break;

  case 137:
#line 934 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; }
    break;

  case 138:
#line 935 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; }
    break;

  case 139:
#line 936 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; }
    break;

  case 140:
#line 937 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; }
    break;

  case 141:
#line 938 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; }
    break;

  case 142:
#line 939 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; }
    break;

  case 143:
#line 940 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; }
    break;

  case 144:
#line 941 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; }
    break;

  case 145:
#line 942 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; }
    break;

  case 146:
#line 943 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; }
    break;

  case 147:
#line 944 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; }
    break;

  case 148:
#line 945 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION; }
    break;

  case 149:
#line 950 "parser.y"
    {
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  }
    break;

  case 150:
#line 965 "parser.y"
    { yyval.string = yyvsp[-1].string; }
    break;

  case 151:
#line 972 "parser.y"
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    }
    break;

  case 152:
#line 978 "parser.y"
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    }
    break;

  case 153:
#line 987 "parser.y"
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    }
    break;


    }

/* Line 1016 of /usr/share/bison/yacc.c.  */
#line 2482 "parser.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyssp > yyss)
	    {
	      YYDPRINTF ((stderr, "Error: popping "));
	      YYDSYMPRINT ((stderr,
			    yystos[*yyssp],
			    *yyvsp));
	      YYDPRINTF ((stderr, "\n"));
	      yydestruct (yystos[*yyssp], *yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yydestruct (yychar1, yylval);
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

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

      YYDPRINTF ((stderr, "Error: popping "));
      YYDSYMPRINT ((stderr,
		    yystos[*yyssp], *yyvsp));
      YYDPRINTF ((stderr, "\n"));

      yydestruct (yystos[yystate], *yyvsp);
      yyvsp--;
      yystate = *--yyssp;


#if YYDEBUG
      if (yydebug)
	{
	  short *yyssp1 = yyss - 1;
	  YYFPRINTF (stderr, "Error: state stack now");
	  while (yyssp1 != yyssp)
	    YYFPRINTF (stderr, " %d", *++yyssp1);
	  YYFPRINTF (stderr, "\n");
	}
#endif
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1022 "parser.y"



