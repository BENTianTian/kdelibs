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
     KHTML_RULE_SYM = 274,
     KHTML_DECLS_SYM = 275,
     KHTML_VALUE_SYM = 276,
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
#define KHTML_RULE_SYM 274
#define KHTML_DECLS_SYM 275
#define KHTML_VALUE_SYM 276
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
//#define CSS_DEBUG

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
    qDebug( "%s", x );
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
#define YYLAST   579

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  57
/* YYNRULES -- Number of rules. */
#define YYNRULES  155
/* YYNRULES -- Number of states. */
#define YYNSTATES  286

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
     349,   352,   355,   356,   358,   362,   365,   368,   371,   372,
     374,   377,   380,   383,   386,   389,   391,   393,   396,   399,
     402,   405,   408,   411,   414,   417,   420,   423,   426,   429,
     432,   435,   438,   441,   444,   447,   450,   456,   460,   463,
     467,   471,   474,   480,   484,   486
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      63,     0,    -1,    69,    68,    70,    72,    76,    -1,    64,
      67,    -1,    65,    67,    -1,    66,    67,    -1,    24,    50,
      67,    88,    67,    51,    -1,    25,    50,    67,   104,    51,
      -1,    26,    50,    67,   109,    51,    -1,    -1,    67,     3,
      -1,    -1,    68,     4,    -1,    68,     3,    -1,    -1,    22,
      67,    10,    67,    52,    -1,    22,     1,   117,    -1,    22,
       1,    52,    -1,    -1,    70,    71,    68,    -1,    18,    67,
      78,    67,    79,    52,    -1,    18,     1,   117,    -1,    18,
       1,    52,    -1,    -1,    73,    68,    -1,    23,    67,    75,
      78,    67,    52,    -1,    11,    -1,    -1,    74,    67,    -1,
      -1,    76,    77,    68,    -1,    88,    -1,    81,    -1,    84,
      -1,    85,    -1,   116,    -1,   115,    -1,    10,    -1,    47,
      -1,    -1,    80,    -1,    83,    -1,    80,    53,    67,    83,
      -1,    80,     1,    -1,    20,    67,    80,    50,    67,    82,
      51,    -1,    -1,    82,    88,    67,    -1,    11,    67,    -1,
      19,     1,   117,    -1,    19,     1,    52,    -1,    21,     1,
     117,    -1,    21,     1,    52,    -1,    54,    67,    -1,    55,
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
      -1,   105,   106,    -1,   105,    -1,     1,   118,     1,    -1,
       1,    -1,   106,    52,    67,    -1,     1,    52,    67,    -1,
       1,   118,     1,    52,    67,    -1,   105,   106,    52,    67,
      -1,   105,     1,    52,    67,    -1,   105,     1,   118,     1,
      52,    67,    -1,   107,    13,    67,   109,   108,    -1,    11,
      67,    -1,    27,    67,    -1,    -1,   111,    -1,   109,   110,
     111,    -1,   109,     1,    -1,    60,    67,    -1,    53,    67,
      -1,    -1,   112,    -1,    87,   112,    -1,    10,    67,    -1,
      11,    67,    -1,    47,    67,    -1,    49,    67,    -1,   114,
      -1,   113,    -1,    46,    67,    -1,    45,    67,    -1,    31,
      67,    -1,    32,    67,    -1,    33,    67,    -1,    34,    67,
      -1,    35,    67,    -1,    36,    67,    -1,    37,    67,    -1,
      38,    67,    -1,    39,    67,    -1,    40,    67,    -1,    41,
      67,    -1,    42,    67,    -1,    43,    67,    -1,    29,    67,
      -1,    28,    67,    -1,    30,    67,    -1,    44,    67,    -1,
      48,    67,   109,    59,    67,    -1,    48,    67,     1,    -1,
      12,    67,    -1,    61,     1,   117,    -1,    61,     1,    52,
      -1,     1,   117,    -1,    50,     1,   118,     1,    51,    -1,
      50,     1,    51,    -1,   117,    -1,   118,     1,   117,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   240,   240,   242,   243,   244,   247,   254,   260,   285,
     287,   290,   292,   293,   296,   298,   303,   304,   307,   309,
     319,   330,   333,   339,   340,   344,   348,   351,   353,   356,
     358,   368,   370,   371,   372,   373,   374,   377,   379,   382,
     386,   390,   395,   400,   406,   420,   422,   431,   439,   443,
     448,   452,   457,   459,   460,   463,   465,   468,   488,   502,
     516,   522,   526,   547,   553,   558,   563,   570,   572,   580,
     582,   583,   587,   608,   611,   616,   626,   632,   639,   640,
     641,   644,   653,   655,   661,   685,   691,   699,   703,   706,
     709,   712,   715,   720,   722,   725,   731,   737,   745,   749,
     754,   757,   763,   771,   775,   781,   787,   792,   798,   806,
     831,   838,   840,   843,   848,   861,   867,   871,   874,   879,
     881,   882,   883,   889,   890,   891,   893,   898,   900,   901,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   921,   930,   941,   948,
     955,   963,   973,   975,   978,   980
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
  "FONT_FACE_SYM", "CHARSET_SYM", "NAMESPACE_SYM", "KHTML_RULE_SYM", 
  "KHTML_DECLS_SYM", "KHTML_VALUE_SYM", "IMPORTANT_SYM", "QEMS", "EMS", 
  "EXS", "PXS", "CMS", "MMS", "INS", "PTS", "PCS", "DEGS", "RADS", 
  "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", "DIMEN", "PERCENTAGE", 
  "NUMBER", "URI", "FUNCTION", "UNICODERANGE", "'{'", "'}'", "';'", "','", 
  "'+'", "'>'", "'-'", "']'", "'='", "')'", "'/'", "'@'", "$accept", 
  "stylesheet", "khtml_rule", "khtml_decls", "khtml_value", "maybe_space", 
  "maybe_sgml", "maybe_charset", "import_list", "import", 
  "maybe_namespace", "namespace", "ns_prefix", "maybe_ns_prefix", 
  "rule_list", "rule", "string_or_uri", "maybe_media_list", "media_list", 
  "media", "ruleset_list", "medium", "page", "font_face", "combinator", 
  "unary_operator", "ruleset", "selector_list", "selector", 
  "simple_selector", "ns_element", "ns_selector", "element_name", 
  "specifier_list", "specifier", "class", "ns_attrib_id", "attrib_id", 
  "attrib", "match", "ident_or_string", "pseudo", "declaration_list", 
  "decl_list", "declaration", "property", "prio", "expr", "operator", 
  "term", "unary_term", "function", "hexcolor", "invalid_at", 
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
     104,   104,   104,   105,   105,   105,   105,   105,   105,   106,
     107,   108,   108,   109,   109,   109,   110,   110,   110,   111,
     111,   111,   111,   111,   111,   111,   111,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   113,   113,   114,   115,
     115,   116,   117,   117,   118,   118
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
       1,     3,     1,     3,     3,     5,     4,     4,     6,     5,
       2,     2,     0,     1,     3,     2,     2,     2,     0,     1,
       2,     2,     2,     2,     2,     1,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     5,     3,     2,     3,
       3,     2,     5,     3,     1,     3
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
      79,    80,   102,     9,     0,     0,    98,     0,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,    56,    55,     0,     0,   113,   119,   126,
     125,     0,     9,    11,    29,    11,   153,   154,     0,    15,
      70,    95,     0,     9,    81,     0,    71,     0,    60,     9,
       9,    63,     9,     9,     0,    64,     0,    72,    73,    67,
      76,    66,    75,     9,     0,   110,     7,     0,    99,     9,
       9,   121,   122,   148,   143,   142,   144,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     145,   128,   127,   123,     0,   124,   120,   115,     8,     9,
       9,     0,     0,     0,    27,    19,     0,    24,     0,    96,
       0,     9,     0,     0,     0,    83,     6,     0,     0,    52,
      53,    62,    65,   104,   101,     9,     0,     9,   103,     0,
     147,     0,   117,   116,   114,    22,    21,    37,    38,     9,
      26,     9,     0,     0,     0,     9,     0,     0,    11,    32,
      33,    34,    31,    36,    35,   152,   155,     0,    84,     9,
      82,    88,    89,    90,    91,    92,    85,    87,     9,     0,
       0,     9,   107,     0,   106,     0,     9,    39,    28,     9,
     151,     0,     0,     0,     0,    30,    97,     0,    57,   105,
       9,     9,   109,   146,     9,     0,     0,    41,     0,    49,
      48,     0,    51,    50,   150,   149,    94,    93,     9,   108,
     111,    47,    20,    43,     9,    25,     9,     0,     0,    45,
      86,    42,     0,    44,     9,    46
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     5,     6,     7,     8,   218,    19,     9,    30,    93,
      94,    95,   201,   202,   166,   208,   199,   255,   256,   209,
     282,   257,   210,   211,   114,    85,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,   174,   175,    50,   228,
     268,    51,    54,    55,    56,    57,   252,    86,   161,    87,
      88,    89,    90,   213,   214,    97,    98
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -166
static const short yypact[] =
{
     149,    17,   -31,   -28,   -17,   101,  -166,  -166,  -166,  -166,
      45,    18,  -166,  -166,  -166,  -166,   104,   104,   104,   142,
      44,  -166,  -166,  -166,  -166,   314,    14,   464,  -166,  -166,
     126,   102,    52,   108,  -166,   103,   146,  -166,  -166,   144,
    -166,     3,   178,  -166,   201,    21,  -166,   153,  -166,  -166,
    -166,  -166,    75,  -166,   121,    43,   124,   165,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,   533,   312,  -166,  -166,  -166,
    -166,    95,  -166,  -166,  -166,  -166,  -166,  -166,   181,  -166,
    -166,  -166,   177,  -166,  -166,    13,  -166,    90,  -166,  -166,
    -166,  -166,  -166,  -166,   321,   104,   153,  -166,  -166,  -166,
    -166,   104,  -166,  -166,   200,   104,  -166,    98,   150,  -166,
    -166,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   417,   104,  -166,  -166,  -166,  -166,
    -166,   504,   131,   100,   110,   142,   119,   142,   112,  -166,
     314,   108,   144,   194,    42,  -166,  -166,    14,   314,   104,
     104,  -166,   104,   104,   135,  -166,   209,  -166,   104,   464,
    -166,   363,   104,   104,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,    96,   161,   216,  -166,   217,   246,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,   170,   104,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,   213,
     208,  -166,   104,   147,   104,   258,  -166,   115,   104,  -166,
    -166,   196,   115,   202,   205,   142,  -166,    31,  -166,   104,
    -166,  -166,  -166,   104,  -166,   199,    34,  -166,    85,  -166,
    -166,    39,  -166,  -166,  -166,  -166,  -166,  -166,  -166,   104,
     104,   104,  -166,  -166,  -166,  -166,  -166,    28,   115,   104,
    -166,  -166,   228,  -166,  -166,   104
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -166,  -166,  -166,  -166,  -166,    -1,   -85,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,    54,  -166,    23,  -166,
    -166,   -18,  -166,  -166,  -166,  -166,  -165,  -166,    88,   -88,
    -166,   166,   227,   230,   -33,  -166,  -166,   105,  -166,  -166,
    -166,  -166,    99,  -166,   222,  -166,  -166,  -152,  -166,   120,
     195,  -166,  -166,  -166,  -166,    -7,   -43
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, parse error.  */
#define YYTABLE_NINF -119
static const short yytable[] =
{
      11,   212,   191,    22,   108,    16,    17,    18,   165,   124,
     167,    25,    26,    27,   122,    52,    23,    23,    10,    12,
      -9,    23,    13,    32,   171,    53,   181,    -9,    24,    38,
     172,    23,   117,    14,    23,   273,   105,   235,   118,   107,
     273,   266,   267,   115,   127,    31,   121,   221,   222,   223,
     224,   225,   125,   109,    53,    23,   110,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   217,   122,   186,   280,   -40,   274,    23,   276,
     163,   164,   274,    23,  -100,    20,   162,    21,    -9,   226,
     227,    15,   170,    23,    99,    -9,   197,    23,   177,   178,
     197,   179,   180,    23,   101,   182,   102,   284,    23,    -2,
     203,   200,   183,   245,   100,    20,   254,   123,   188,   189,
      33,    34,    35,    36,    37,    38,    39,   275,   204,   205,
     206,   176,    -9,   198,    91,    28,    29,   198,    20,    92,
     185,   103,    20,    96,   120,   196,    -9,   104,   192,   193,
     106,   216,    20,   215,    -9,    34,    35,    36,    37,    -9,
      -9,     1,   126,     2,     3,     4,   129,   216,   130,   111,
     207,    20,   168,   195,   232,    20,   234,   231,   169,   -54,
     -54,   -54,   -54,   -54,   -54,   -54,   240,    20,   237,   250,
     238,   184,   187,    -9,   242,   219,    -9,    -9,    -9,   111,
     233,    20,    -9,    34,    35,    36,    37,   241,   243,   -54,
     -54,   -54,   -54,   -54,   -54,   -54,   216,   247,   -58,   246,
     249,   -58,   112,   113,   260,   253,   263,   265,   258,    33,
      34,    35,    36,    37,    38,    39,    20,   244,   259,   269,
     270,   272,    20,   271,   262,    20,   239,   264,   -59,   157,
     281,   -59,   112,   113,   248,   261,   230,   277,  -118,  -118,
    -118,   173,   119,   278,   116,   279,   229,   128,   220,   283,
     156,   194,     0,   285,     0,   251,  -118,  -118,  -118,  -118,
    -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,
    -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,     0,  -112,
    -112,   159,  -118,   157,  -118,     0,     0,    23,   160,     0,
       0,     0,  -118,  -118,  -118,    33,    34,    35,    36,    37,
      38,    39,    33,    34,    35,    36,    37,    38,    39,     0,
    -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,
    -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,
    -118,  -118,     0,   158,   157,   159,  -118,     0,  -118,     0,
       0,     0,   160,  -118,  -118,  -118,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,
    -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,
    -118,  -118,  -118,     0,     0,     0,   159,  -118,   190,  -118,
      23,     0,   236,   160,     0,     0,     0,    58,    59,    60,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    23,     0,     0,
       0,    83,     0,    84,    58,    59,    60,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    58,    59,    60,     0,    83,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,    83,     0,
      84,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79
};

static const short yycheck[] =
{
       1,   166,   154,    10,     1,     6,     7,     8,    93,    52,
      95,    12,    13,    14,    47,     1,     3,     3,     1,    50,
       3,     3,    50,    24,    11,    11,   114,    10,    10,    16,
      17,     3,    11,    50,     3,     1,    37,   189,    17,    40,
       1,    10,    11,    44,     1,     1,    47,     5,     6,     7,
       8,     9,    53,    50,    11,     3,    53,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,   170,   116,   127,    57,    52,    53,     3,    50,
      91,    92,    53,     3,    51,    50,     1,    52,     3,    57,
      58,     0,   103,     3,    52,    10,    10,     3,   109,   110,
      10,   112,   113,     3,    11,   116,    13,   282,     3,     0,
       1,    11,   123,   208,    16,    50,    11,    52,   129,   130,
      11,    12,    13,    14,    15,    16,    17,    52,    19,    20,
      21,    51,    47,    47,    18,     3,     4,    47,    50,    23,
      52,    48,    50,    51,     1,   162,     3,    11,   159,   160,
      16,   168,    50,    51,    11,    12,    13,    14,    15,    16,
      17,    22,    51,    24,    25,    26,    52,   184,    13,     1,
      61,    50,     1,    52,   185,    50,   187,    52,    11,    11,
      12,    13,    14,    15,    16,    17,   203,    50,   199,    52,
     201,     1,    52,    50,   205,    11,    53,    54,    55,     1,
       1,    50,    59,    12,    13,    14,    15,     1,     1,    11,
      12,    13,    14,    15,    16,    17,   233,   228,    50,    59,
     231,    53,    54,    55,   241,   236,   243,   244,   239,    11,
      12,    13,    14,    15,    16,    17,    50,     1,    52,   250,
     251,    52,    50,   254,    52,    50,   202,    52,    50,     1,
     278,    53,    54,    55,    51,   242,   178,   268,    10,    11,
      12,   105,    45,   274,    44,   276,   177,    55,   173,    51,
      85,   161,    -1,   284,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    53,    54,     1,    56,    -1,    -1,     3,    60,    -1,
      -1,    -1,    10,    11,    12,    11,    12,    13,    14,    15,
      16,    17,    11,    12,    13,    14,    15,    16,    17,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,     1,    53,    54,    -1,    56,    -1,
      -1,    -1,    60,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    -1,    -1,    53,    54,     1,    56,
       3,    -1,    59,    60,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,     3,    -1,    -1,
      -1,    54,    -1,    56,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    10,    11,    12,    -1,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    22,    24,    25,    26,    63,    64,    65,    66,    69,
       1,    67,    50,    50,    50,     0,    67,    67,    67,    68,
      50,    52,   117,     3,    10,    67,    67,    67,     3,     4,
      70,     1,    67,    11,    12,    13,    14,    15,    16,    17,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
     100,   103,     1,    11,   104,   105,   106,   107,    10,    11,
      12,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    54,    56,    87,   109,   111,   112,   113,
     114,    18,    23,    71,    72,    73,    51,   117,   118,    52,
      16,    11,    13,    48,    11,    67,    16,    67,     1,    50,
      53,     1,    54,    55,    86,    67,    95,    11,    17,    94,
       1,    67,    96,    52,   118,    67,    51,     1,   106,    52,
      13,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,   112,     1,    51,    53,
      60,   110,     1,    67,    67,    68,    76,    68,     1,    11,
      67,    11,    17,    93,    98,    99,    51,    67,    67,    67,
      67,    91,    67,    67,     1,    52,   118,    52,    67,    67,
       1,   109,    67,    67,   111,    52,   117,    10,    47,    78,
      11,    74,    75,     1,    19,    20,    21,    61,    77,    81,
      84,    85,    88,   115,   116,    51,   117,    91,    67,    11,
      99,     5,     6,     7,     8,     9,    57,    58,   101,   104,
      90,    52,    67,     1,    67,   109,    59,    67,    67,    78,
     117,     1,    67,     1,     1,    68,    59,    67,    51,    67,
      52,    27,   108,    67,    11,    79,    80,    83,    67,    52,
     117,    80,    52,   117,    52,   117,    10,    11,   102,    67,
      67,    67,    52,     1,    53,    52,    50,    67,    67,    67,
      57,    83,    82,    51,    88,    67
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
	if ( yyvsp[-1].mediaList && p->styleElement && p->styleElement->isCSSStyleSheet() )
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
	yyval.mediaList = new MediaListImpl();
    }
    break;

  case 41:
#line 391 "parser.y"
    {
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
    }
    break;

  case 42:
#line 395 "parser.y"
    {
	yyval.mediaList = yyvsp[-3].mediaList;
	if (yyval.mediaList)
	    yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
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
#line 440 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 49:
#line 443 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 50:
#line 449 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 51:
#line 452 "parser.y"
    {
      yyval.rule = 0;
    }
    break;

  case 52:
#line 458 "parser.y"
    { yyval.relation = CSSSelector::Sibling; }
    break;

  case 53:
#line 459 "parser.y"
    { yyval.relation = CSSSelector::Child; }
    break;

  case 54:
#line 460 "parser.y"
    { yyval.relation = CSSSelector::Descendant; }
    break;

  case 55:
#line 464 "parser.y"
    { yyval.val = -1; }
    break;

  case 56:
#line 465 "parser.y"
    { yyval.val = 1; }
    break;

  case 57:
#line 469 "parser.y"
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
#line 489 "parser.y"
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
#line 502 "parser.y"
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
#line 516 "parser.y"
    {
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    }
    break;

  case 61:
#line 523 "parser.y"
    {
	yyval.selector = yyvsp[0].selector;
    }
    break;

  case 62:
#line 526 "parser.y"
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
#line 547 "parser.y"
    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    }
    break;

  case 64:
#line 554 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    }
    break;

  case 65:
#line 558 "parser.y"
    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    }
    break;

  case 66:
#line 563 "parser.y"
    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = 0xffffffff;
    }
    break;

  case 67:
#line 571 "parser.y"
    { yyval.element = (yyvsp[-1].ns<<16) | yyvsp[0].element; }
    break;

  case 68:
#line 572 "parser.y"
    {
        /* according to the specs this one matches all namespaces if no
	   default namespace has been specified otherwise the default namespace */
	CSSParser *p = static_cast<CSSParser *>(parser);
	yyval.element = (p->defaultNamespace<<16) | yyvsp[0].element;
    }
    break;

  case 69:
#line 581 "parser.y"
    { yyval.ns = 0; }
    break;

  case 70:
#line 582 "parser.y"
    { yyval.ns = 1; /* #### insert correct namespace id here */ }
    break;

  case 71:
#line 583 "parser.y"
    { yyval.ns = 0xffff; }
    break;

  case 72:
#line 588 "parser.y"
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
	    yyval.element = doc->getId(NodeImpl::ElementId, dtag.implementation(), false, true);
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
#line 608 "parser.y"
    { yyval.element = 0xffff; }
    break;

  case 74:
#line 612 "parser.y"
    {
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    }
    break;

  case 75:
#line 616 "parser.y"
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
#line 626 "parser.y"
    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    }
    break;

  case 77:
#line 633 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 81:
#line 645 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 82:
#line 654 "parser.y"
    { yyval.attribute = (yyvsp[-1].ns<<16) | yyvsp[0].attribute; }
    break;

  case 83:
#line 655 "parser.y"
    {
	/* opposed to elements, these only match for non namespaced attributes */
	yyval.attribute = yyvsp[0].attribute;
    }
    break;

  case 84:
#line 662 "parser.y"
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
	    yyval.attribute = doc->getId(NodeImpl::AttributeId, dattr.implementation(), false, true);
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
#line 686 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    }
    break;

  case 86:
#line 691 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = (CSSSelector::Match)yyvsp[-4].val;
	yyval.selector->value = domString(yyvsp[-2].string);
    }
    break;

  case 87:
#line 700 "parser.y"
    {
	yyval.val = CSSSelector::Exact;
    }
    break;

  case 88:
#line 703 "parser.y"
    {
	yyval.val = CSSSelector::List;
    }
    break;

  case 89:
#line 706 "parser.y"
    {
	yyval.val = CSSSelector::Hyphen;
    }
    break;

  case 90:
#line 709 "parser.y"
    {
	yyval.val = CSSSelector::Begin;
    }
    break;

  case 91:
#line 712 "parser.y"
    {
	yyval.val = CSSSelector::End;
    }
    break;

  case 92:
#line 715 "parser.y"
    {
	yyval.val = CSSSelector::Contain;
    }
    break;

  case 95:
#line 726 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 96:
#line 732 "parser.y"
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 97:
#line 737 "parser.y"
    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->simpleSelector = yyvsp[-1].selector;
        yyval.selector->value = domString(yyvsp[-3].string);
    }
    break;

  case 98:
#line 746 "parser.y"
    {
	yyval.ok = yyvsp[0].ok;
    }
    break;

  case 99:
#line 749 "parser.y"
    {
	yyval.ok = yyvsp[-1].ok;
	if ( yyvsp[0].ok )
	    yyval.ok = yyvsp[0].ok;
    }
    break;

  case 100:
#line 754 "parser.y"
    {
	yyval.ok = yyvsp[0].ok;
    }
    break;

  case 101:
#line 757 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 102:
#line 763 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping all declarations" << endl;
#endif
    }
    break;

  case 103:
#line 772 "parser.y"
    {
	yyval.ok = yyvsp[-2].ok;
    }
    break;

  case 104:
#line 775 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 105:
#line 781 "parser.y"
    {
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 106:
#line 787 "parser.y"
    {
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    }
    break;

  case 107:
#line 792 "parser.y"
    {
	yyval.ok = yyvsp[-3].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 108:
#line 798 "parser.y"
    {
	yyval.ok = yyvsp[-5].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    }
    break;

  case 109:
#line 807 "parser.y"
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
#line 832 "parser.y"
    {
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    }
    break;

  case 111:
#line 839 "parser.y"
    { yyval.b = true; }
    break;

  case 112:
#line 840 "parser.y"
    { yyval.b = false; }
    break;

  case 113:
#line 844 "parser.y"
    {
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    }
    break;

  case 114:
#line 848 "parser.y"
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

  case 115:
#line 861 "parser.y"
    {
	delete yyvsp[-1].valueList;
	yyval.valueList = 0;
    }
    break;

  case 116:
#line 868 "parser.y"
    {
	yyval.tok = '/';
    }
    break;

  case 117:
#line 871 "parser.y"
    {
	yyval.tok = ',';
    }
    break;

  case 118:
#line 874 "parser.y"
    {
        yyval.tok = 0;
  }
    break;

  case 119:
#line 880 "parser.y"
    { yyval.value = yyvsp[0].value; }
    break;

  case 120:
#line 881 "parser.y"
    { yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; }
    break;

  case 121:
#line 882 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; }
    break;

  case 122:
#line 883 "parser.y"
    {
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  }
    break;

  case 123:
#line 889 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; }
    break;

  case 124:
#line 890 "parser.y"
    { yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ }
    break;

  case 125:
#line 891 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; }
    break;

  case 126:
#line 893 "parser.y"
    {
      yyval.value = yyvsp[0].value;
  }
    break;

  case 127:
#line 899 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; }
    break;

  case 128:
#line 900 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; }
    break;

  case 129:
#line 901 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; }
    break;

  case 130:
#line 902 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; }
    break;

  case 131:
#line 903 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; }
    break;

  case 132:
#line 904 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; }
    break;

  case 133:
#line 905 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; }
    break;

  case 134:
#line 906 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; }
    break;

  case 135:
#line 907 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; }
    break;

  case 136:
#line 908 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; }
    break;

  case 137:
#line 909 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; }
    break;

  case 138:
#line 910 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; }
    break;

  case 139:
#line 911 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; }
    break;

  case 140:
#line 912 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; }
    break;

  case 141:
#line 913 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; }
    break;

  case 142:
#line 914 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; }
    break;

  case 143:
#line 915 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; }
    break;

  case 144:
#line 916 "parser.y"
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; }
    break;

  case 145:
#line 917 "parser.y"
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION; }
    break;

  case 146:
#line 922 "parser.y"
    {
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  }
    break;

  case 147:
#line 930 "parser.y"
    {
      Function *f = new Function;
      f->name = yyvsp[-2].string;
      f->args = 0;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  }
    break;

  case 148:
#line 942 "parser.y"
    { yyval.string = yyvsp[-1].string; }
    break;

  case 149:
#line 949 "parser.y"
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    }
    break;

  case 150:
#line 955 "parser.y"
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    }
    break;

  case 151:
#line 964 "parser.y"
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    }
    break;


    }

/* Line 1016 of /usr/share/bison/yacc.c.  */
#line 2454 "parser.tab.c"

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


#line 983 "parser.y"



