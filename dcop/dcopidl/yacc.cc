/* A Bison parser, made from yacc.yy
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	T_CHARACTER_LITERAL	257
# define	T_DOUBLE_LITERAL	258
# define	T_IDENTIFIER	259
# define	T_INTEGER_LITERAL	260
# define	T_STRING_LITERAL	261
# define	T_INCLUDE	262
# define	T_CLASS	263
# define	T_STRUCT	264
# define	T_LEFT_CURLY_BRACKET	265
# define	T_LEFT_PARANTHESIS	266
# define	T_RIGHT_CURLY_BRACKET	267
# define	T_RIGHT_PARANTHESIS	268
# define	T_COLON	269
# define	T_SEMICOLON	270
# define	T_PUBLIC	271
# define	T_PROTECTED	272
# define	T_TRIPE_DOT	273
# define	T_PRIVATE	274
# define	T_VIRTUAL	275
# define	T_CONST	276
# define	T_INLINE	277
# define	T_FRIEND	278
# define	T_RETURN	279
# define	T_SIGNAL	280
# define	T_SLOT	281
# define	T_TYPEDEF	282
# define	T_PLUS	283
# define	T_MINUS	284
# define	T_COMMA	285
# define	T_ASTERISK	286
# define	T_TILDE	287
# define	T_LESS	288
# define	T_GREATER	289
# define	T_AMPERSAND	290
# define	T_EXTERN	291
# define	T_EXTERN_C	292
# define	T_ACCESS	293
# define	T_ENUM	294
# define	T_NAMESPACE	295
# define	T_USING	296
# define	T_UNKNOWN	297
# define	T_TRIPLE_DOT	298
# define	T_TRUE	299
# define	T_FALSE	300
# define	T_STATIC	301
# define	T_MUTABLE	302
# define	T_EQUAL	303
# define	T_SCOPE	304
# define	T_NULL	305
# define	T_INT	306
# define	T_ARRAY_OPEN	307
# define	T_ARRAY_CLOSE	308
# define	T_CHAR	309
# define	T_DCOP	310
# define	T_DCOP_AREA	311
# define	T_DCOP_SIGNAL_AREA	312
# define	T_SIGNED	313
# define	T_UNSIGNED	314
# define	T_LONG	315
# define	T_SHORT	316
# define	T_FUNOPERATOR	317
# define	T_MISCOPERATOR	318
# define	T_SHIFT	319

#line 1 "yacc.yy"

/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <config.h>

// Workaround for a bison issue:
// bison.simple concludes from _GNU_SOURCE that stpcpy is available,
// while GNU string.h only exposes it if __USE_GNU is set.
#ifdef _GNU_SOURCE
#define __USE_GNU 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <qstring.h>

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE

extern int yylex();

// extern QString idl_lexFile;
extern int idl_line_no;
extern int function_mode;

static int dcop_area = 0;
static int dcop_signal_area = 0;

static QString in_namespace( "" );

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 66 "yacc.yy"
#ifndef YYSTYPE
typedef union
{
  long   _int;
  QString        *_str;
  unsigned short          _char;
  double _float;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 1
#endif



#define	YYFINAL		360
#define	YYFLAG		-32768
#define	YYNTBASE	66

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 319 ? yytranslate[x] : 118)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     4,     5,     8,    13,    14,    16,    17,    24,
      28,    32,    38,    39,    47,    52,    58,    61,    66,    74,
      83,    86,    88,    90,    92,    95,    96,    98,   100,   102,
     104,   106,   108,   110,   111,   115,   118,   121,   124,   126,
     130,   132,   137,   141,   143,   146,   150,   153,   155,   156,
     158,   160,   163,   166,   169,   172,   175,   178,   181,   187,
     192,   197,   202,   209,   214,   221,   228,   236,   243,   250,
     256,   260,   262,   266,   268,   270,   272,   275,   277,   279,
     281,   285,   289,   297,   307,   308,   310,   312,   315,   317,
     320,   323,   327,   330,   334,   337,   341,   344,   348,   350,
     352,   355,   357,   360,   362,   365,   368,   371,   373,   374,
     376,   380,   382,   384,   387,   390,   395,   402,   406,   408,
     411,   413,   417,   421,   424,   427,   429,   432,   436,   438,
     442,   445,   447,   448,   451,   457,   459,   461,   463,   465,
     470,   471,   473,   475,   477,   479,   481,   483,   490,   498,
     500,   504,   505,   510,   512,   516,   519,   525,   529,   535,
     543,   550,   554,   556,   558,   562,   567,   570,   571,   573,
     576,   577,   579,   583,   586,   589,   593,   599,   605,   611
};
static const short yyrhs[] =
{
      67,    69,    66,     0,     0,     8,    67,     0,    38,    11,
      66,    13,     0,     0,    56,     0,     0,     9,    78,    82,
      68,    84,    16,     0,     9,    78,    16,     0,    10,    78,
      16,     0,    10,    78,    82,    84,    16,     0,     0,    41,
       5,    11,    70,    66,    13,    83,     0,    42,    41,     5,
      16,     0,    42,     5,    50,     5,    16,     0,    37,    16,
       0,    28,    98,    78,    16,     0,    28,    10,    11,    71,
      13,    78,    16,     0,    28,    10,    78,    11,    71,    13,
      78,    16,     0,    23,   109,     0,   109,     0,   117,     0,
      85,     0,   117,    71,     0,     0,    45,     0,    46,     0,
      20,     0,    18,     0,    17,     0,    26,     0,    27,     0,
       0,    73,    74,    15,     0,    74,    15,     0,    57,    15,
       0,    58,    15,     0,     5,     0,     5,    50,    78,     0,
      78,     0,    78,    34,    99,    35,     0,   103,    17,    79,
       0,    79,     0,    80,    11,     0,    80,    31,    81,     0,
      15,    81,     0,    11,     0,     0,    16,     0,    13,     0,
      90,    84,     0,   109,    84,     0,    77,    84,     0,    85,
      84,     0,    76,    84,     0,    75,    84,     0,   117,    84,
       0,    24,     9,    78,    16,    84,     0,    24,    78,    16,
      84,     0,    24,   105,    16,    84,     0,     9,    78,    16,
      84,     0,     9,    78,    82,    84,    16,    84,     0,    10,
      78,    16,    84,     0,    10,    78,    82,    84,    16,    84,
       0,    42,     5,    50,     5,    16,    84,     0,    40,     5,
      11,    86,    13,     5,    16,     0,    40,     5,    11,    86,
      13,    16,     0,    40,    11,    86,    13,     5,    16,     0,
      40,    11,    86,    13,    16,     0,    87,    31,    86,     0,
      87,     0,     5,    49,    89,     0,     5,     0,     3,     0,
       6,     0,    30,     6,     0,    51,     0,    78,     0,    88,
       0,    88,    29,    88,     0,    88,    65,    88,     0,    28,
      78,    34,    99,    35,    78,    16,     0,    28,    78,    34,
      99,    35,    50,     5,    78,    16,     0,     0,    22,     0,
      59,     0,    59,    52,     0,    60,     0,    60,    52,     0,
      59,    62,     0,    59,    62,    52,     0,    59,    61,     0,
      59,    61,    52,     0,    60,    62,     0,    60,    62,    52,
       0,    60,    61,     0,    60,    61,    52,     0,    52,     0,
      61,     0,    61,    52,     0,    62,     0,    62,    52,     0,
      55,     0,    59,    55,     0,    60,    55,     0,    32,    93,
       0,    32,     0,     0,   100,     0,    94,    31,   100,     0,
      92,     0,    78,     0,    10,    78,     0,     9,    78,     0,
      78,    34,    96,    35,     0,    78,    34,    96,    35,    50,
      78,     0,    97,    31,    96,     0,    97,     0,    95,    93,
       0,    95,     0,    22,    95,    93,     0,    22,    95,    36,
       0,    22,    95,     0,    95,    36,     0,    95,     0,    95,
      93,     0,    98,    31,    99,     0,    98,     0,    98,    78,
     101,     0,    98,   101,     0,    44,     0,     0,    49,   102,
       0,    49,    12,    98,    14,   102,     0,     7,     0,    89,
       0,     4,     0,    72,     0,    78,    12,    94,    14,     0,
       0,    21,     0,    64,     0,    65,     0,    35,     0,    34,
       0,    49,     0,    98,    78,    12,    94,    14,    91,     0,
      98,    63,   104,    12,    94,    14,    91,     0,   102,     0,
     102,    31,   106,     0,     0,     5,    12,   106,    14,     0,
     107,     0,   107,    31,   108,     0,   105,   111,     0,    21,
     105,    49,    51,   111,     0,    21,   105,   111,     0,    78,
      12,    94,    14,   111,     0,    78,    12,    94,    14,    15,
     108,   111,     0,   103,    33,    78,    12,    14,   111,     0,
      47,   105,   111,     0,    11,     0,    16,     0,   110,   112,
      13,     0,   110,   112,    13,    16,     0,   113,   112,     0,
       0,    16,     0,    31,   116,     0,     0,     5,     0,     5,
      49,   102,     0,    93,     5,     0,   115,   114,     0,    98,
     116,    16,     0,    98,    78,    15,     6,    16,     0,    47,
      98,     5,   101,    16,     0,    48,    98,     5,   101,    16,
       0,    98,     5,    53,    89,    54,    16,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   163,   166,   170,   174,   177,   183,   184,   188,   197,
     200,   203,   206,   206,   218,   221,   224,   227,   230,   233,
     236,   239,   242,   245,   251,   252,   255,   255,   257,   257,
     257,   259,   259,   259,   262,   267,   275,   283,   295,   298,
     306,   312,   321,   325,   332,   336,   344,   348,   355,   358,
     362,   366,   370,   374,   378,   382,   386,   390,   394,   398,
     402,   406,   410,   414,   418,   422,   429,   430,   431,   432,
     436,   437,   441,   442,   446,   447,   448,   449,   450,   454,
     455,   456,   460,   470,   478,   482,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   512,   513,   517,   521,
     522,   530,   531,   532,   533,   534,   540,   551,   555,   563,
     568,   577,   582,   589,   594,   599,   604,   612,   616,   623,
     632,   640,   649,   652,   655,   662,   665,   668,   671,   674,
     680,   681,   685,   685,   685,   685,   685,   689,   711,   722,
     723,   724,   729,   734,   735,   739,   743,   747,   751,   757,
     763,   769,   782,   789,   790,   791,   795,   796,   800,   804,
     805,   808,   809,   810,   813,   817,   818,   819,   820,   821
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "T_CHARACTER_LITERAL", "T_DOUBLE_LITERAL", 
  "T_IDENTIFIER", "T_INTEGER_LITERAL", "T_STRING_LITERAL", "T_INCLUDE", 
  "T_CLASS", "T_STRUCT", "T_LEFT_CURLY_BRACKET", "T_LEFT_PARANTHESIS", 
  "T_RIGHT_CURLY_BRACKET", "T_RIGHT_PARANTHESIS", "T_COLON", 
  "T_SEMICOLON", "T_PUBLIC", "T_PROTECTED", "T_TRIPE_DOT", "T_PRIVATE", 
  "T_VIRTUAL", "T_CONST", "T_INLINE", "T_FRIEND", "T_RETURN", "T_SIGNAL", 
  "T_SLOT", "T_TYPEDEF", "T_PLUS", "T_MINUS", "T_COMMA", "T_ASTERISK", 
  "T_TILDE", "T_LESS", "T_GREATER", "T_AMPERSAND", "T_EXTERN", 
  "T_EXTERN_C", "T_ACCESS", "T_ENUM", "T_NAMESPACE", "T_USING", 
  "T_UNKNOWN", "T_TRIPLE_DOT", "T_TRUE", "T_FALSE", "T_STATIC", 
  "T_MUTABLE", "T_EQUAL", "T_SCOPE", "T_NULL", "T_INT", "T_ARRAY_OPEN", 
  "T_ARRAY_CLOSE", "T_CHAR", "T_DCOP", "T_DCOP_AREA", 
  "T_DCOP_SIGNAL_AREA", "T_SIGNED", "T_UNSIGNED", "T_LONG", "T_SHORT", 
  "T_FUNOPERATOR", "T_MISCOPERATOR", "T_SHIFT", "main", "includes", 
  "dcoptag", "declaration", "@1", "member_list", "bool_value", 
  "nodcop_area", "sigslot", "nodcop_area_begin", "dcop_area_begin", 
  "dcop_signal_area_begin", "Identifier", "super_class_name", 
  "super_class", "super_classes", "class_header", "opt_semicolon", "body", 
  "enum", "enum_list", "enum_item", "number", "int_expression", "typedef", 
  "const_qualifier", "int_type", "asterisks", "params", "type_name", 
  "templ_type_list", "templ_type", "type", "type_list", "param", 
  "default", "value", "virtual_qualifier", "operator", "function_header", 
  "values", "init_item", "init_list", "function", "function_begin", 
  "function_body", "function_lines", "function_line", 
  "Identifier_list_rest", "Identifier_list_entry", "Identifier_list", 
  "member", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    66,    66,    67,    67,    67,    68,    68,    69,    69,
      69,    69,    70,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    71,    71,    72,    72,    73,    73,
      73,    74,    74,    74,    75,    75,    76,    77,    78,    78,
      79,    79,    80,    80,    81,    81,    82,    82,    83,    83,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    85,    85,    85,    85,
      86,    86,    87,    87,    88,    88,    88,    88,    88,    89,
      89,    89,    90,    90,    91,    91,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    93,    93,    94,    94,
      94,    95,    95,    95,    95,    95,    95,    96,    96,    97,
      97,    98,    98,    98,    98,    98,    98,    99,    99,   100,
     100,   100,   101,   101,   101,   102,   102,   102,   102,   102,
     103,   103,   104,   104,   104,   104,   104,   105,   105,   106,
     106,   106,   107,   108,   108,   109,   109,   109,   109,   109,
     109,   109,   110,   111,   111,   111,   112,   112,   113,   114,
     114,   115,   115,   115,   116,   117,   117,   117,   117,   117
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     3,     0,     2,     4,     0,     1,     0,     6,     3,
       3,     5,     0,     7,     4,     5,     2,     4,     7,     8,
       2,     1,     1,     1,     2,     0,     1,     1,     1,     1,
       1,     1,     1,     0,     3,     2,     2,     2,     1,     3,
       1,     4,     3,     1,     2,     3,     2,     1,     0,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     5,     4,
       4,     4,     6,     4,     6,     6,     7,     6,     6,     5,
       3,     1,     3,     1,     1,     1,     2,     1,     1,     1,
       3,     3,     7,     9,     0,     1,     1,     2,     1,     2,
       2,     3,     2,     3,     2,     3,     2,     3,     1,     1,
       2,     1,     2,     1,     2,     2,     2,     1,     0,     1,
       3,     1,     1,     2,     2,     4,     6,     3,     1,     2,
       1,     3,     3,     2,     2,     1,     2,     3,     1,     3,
       2,     1,     0,     2,     5,     1,     1,     1,     1,     4,
       0,     1,     1,     1,     1,     1,     1,     6,     7,     1,
       3,     0,     4,     1,     3,     2,     5,     3,     5,     7,
       6,     3,     1,     1,     3,     4,     2,     0,     1,     2,
       0,     1,     3,     2,     2,     3,     5,     5,     5,     6
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       5,     5,     0,   140,     3,     5,    38,     0,     0,   141,
       0,   140,     0,     0,     0,     0,     0,     0,     0,    98,
     103,    86,    88,    99,   101,     5,   112,    23,   111,   125,
       0,     0,     0,    21,    22,     0,     0,   114,   113,     0,
       0,   112,     0,     0,   123,     0,    20,     0,     0,    16,
       0,     0,     0,     0,     0,     0,     0,     0,    87,   104,
      92,    90,    89,   105,    96,    94,   100,   102,     1,   108,
       0,   107,   124,   126,    38,     0,     0,     0,   170,     0,
       0,   162,   163,   167,   155,     4,    39,    47,   140,     9,
       7,    10,    33,   114,   113,     0,     0,   157,   122,   121,
      25,   113,     0,     0,    73,     0,    71,    12,     0,     0,
      38,   161,   132,    93,    91,    97,    95,   131,     0,   132,
     109,   120,     0,   118,   106,     0,     0,   145,   144,   146,
     142,   143,     0,   108,     0,   173,     0,   174,   175,     0,
     168,     0,   167,   141,    40,    43,     0,    46,     0,     6,
      33,     0,     0,    50,    30,    29,    28,     0,    31,    32,
       0,     0,     0,     0,    33,     0,    33,    33,    33,     0,
      33,    33,    33,    33,     0,     0,     0,     0,    25,    25,
      17,     0,     0,     0,     0,     5,     0,    14,     0,     0,
       0,     0,     0,   132,   130,   119,   115,     0,    74,   137,
      75,   135,     0,    26,    27,    77,   138,    78,    79,   136,
     172,    78,     0,   108,     0,     0,   171,   169,     0,   164,
     166,     0,    44,   140,     0,     0,   114,   113,     0,   112,
       0,     0,     0,    36,    37,     0,    35,    56,    55,    53,
      11,    54,    51,    52,    57,   156,     0,     0,     0,    24,
       0,     0,    72,     0,    69,    70,     0,    15,     0,   133,
     177,   178,     0,   158,   110,   129,     0,   117,    76,   108,
       0,     0,     0,     0,    84,   176,     0,   165,   128,     0,
      45,    42,     8,    33,    33,    33,    33,   114,    33,    33,
       0,     0,    34,   132,     0,     0,     0,    67,    68,    48,
       0,     0,   153,     0,   116,     0,    80,    81,   179,    84,
      85,   147,   160,     0,    41,    61,     0,    63,     0,    33,
      59,    60,     0,     0,    18,     0,    66,    49,    13,     0,
     151,     0,   159,   139,   148,   127,    33,    33,    58,     0,
      33,    19,   134,   149,     0,   154,    62,    64,     0,     0,
      65,   151,   152,     0,    82,   150,     0,    83,     0,     0,
       0
};

static const short yydefgoto[] =
{
      35,     3,   150,    25,   185,   176,   206,   164,   165,   166,
     167,   168,    26,   145,   146,   147,    90,   328,   169,   170,
     105,   106,   208,   209,   171,   311,    28,    77,   118,    29,
     122,   123,    30,   279,   120,   189,   343,    31,   132,    32,
     344,   302,   303,   172,    83,    84,   141,   142,   137,    78,
      79,   173
};

static const short yypact[] =
{
      53,    52,    46,   376,-32768,    18,    88,   171,   171,   239,
      97,    49,   309,   173,   213,   192,    23,   239,   239,-32768,
  -32768,   235,   265,    16,   168,   127,    43,-32768,-32768,   100,
      15,   199,   222,-32768,-32768,   221,   171,   230,   254,   171,
     171,   209,    17,   117,   107,   239,-32768,   226,   171,-32768,
     243,   245,   249,   224,   271,    22,   222,   284,-32768,-32768,
     241,   250,-32768,-32768,   257,   258,-32768,-32768,-32768,   397,
      97,   263,-32768,-32768,   255,   158,    27,   308,   285,   299,
     171,-32768,-32768,   305,-32768,-32768,-32768,-32768,   108,-32768,
     272,-32768,   332,-32768,-32768,   321,   283,-32768,-32768,-32768,
     220,   324,   320,   245,   289,   326,   312,-32768,   335,   339,
     130,-32768,   302,-32768,-32768,-32768,-32768,-32768,   184,    20,
  -32768,   263,   313,   331,-32768,   440,   111,-32768,-32768,-32768,
  -32768,-32768,   345,   397,   357,-32768,   150,-32768,-32768,   354,
  -32768,   360,   305,-32768,   333,-32768,    63,-32768,   358,-32768,
     332,   171,   171,-32768,-32768,-32768,-32768,   417,-32768,-32768,
     171,   371,   362,   363,   147,   367,   332,   332,   332,   372,
     332,   332,   332,   332,   222,   239,   370,   155,   220,   220,
  -32768,   382,   111,   115,   245,    18,   380,-32768,   118,   384,
     385,   262,   397,   302,-32768,-32768,   353,    97,-32768,-32768,
  -32768,-32768,   399,-32768,-32768,-32768,-32768,   396,     7,-32768,
  -32768,-32768,   355,   397,   196,   394,   365,-32768,   398,   395,
  -32768,   239,-32768,   108,   171,   404,   296,   314,   171,    81,
     405,   381,   375,-32768,-32768,   414,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,   425,   171,   418,-32768,
     419,   172,-32768,   424,-32768,-32768,   421,-32768,   239,-32768,
  -32768,-32768,   437,-32768,-32768,-32768,   171,-32768,-32768,   397,
     111,   111,   432,   197,   428,-32768,   222,-32768,   420,   426,
  -32768,-32768,-32768,   332,   332,   332,   332,   438,   332,   332,
     239,   448,-32768,   302,   439,   171,   444,-32768,-32768,   446,
     449,   452,   434,   222,-32768,   205,-32768,-32768,-32768,   428,
  -32768,-32768,-32768,   239,-32768,-32768,   450,-32768,   451,   332,
  -32768,-32768,   433,   455,-32768,   457,-32768,-32768,-32768,   440,
     440,   437,-32768,-32768,-32768,-32768,   332,   332,-32768,    25,
     332,-32768,-32768,   443,   461,-32768,-32768,-32768,   475,   465,
  -32768,   440,-32768,   171,-32768,-32768,   466,-32768,   483,   484,
  -32768
};

static const short yypgoto[] =
{
       9,   486,-32768,-32768,-32768,    79,-32768,-32768,   325,-32768,
  -32768,-32768,    -7,   264,-32768,   267,   -31,-32768,   -84,   489,
     -85,-32768,    14,   -82,-32768,   185,-32768,    21,  -118,     6,
     298,-32768,    34,  -276,   301,   -95,  -121,   -76,-32768,     4,
     145,-32768,   166,   175,-32768,   -37,   356,-32768,-32768,-32768,
     364,     5
};


#define	YYLAST		500


static const short yytable[] =
{
      37,    38,    41,    41,   210,    41,    97,    92,    34,   358,
      41,    41,   148,    43,   322,   214,    44,   190,   181,   111,
      74,    56,     6,    76,   194,     6,     1,   110,    53,    86,
       6,    -2,    93,    94,    68,    95,   270,   335,    41,   133,
     101,   102,   134,    42,   212,    42,    48,    71,    95,    56,
      73,    55,    57,    -2,     6,    69,     2,     5,    39,    40,
       1,     1,    41,    41,    54,    99,   225,   259,    66,   188,
       9,    10,   271,   139,   222,   348,   121,    70,    75,    42,
      75,   144,   237,   238,   239,    75,   241,   242,   243,   244,
       2,     2,   124,    41,   223,   273,    45,   288,   265,   255,
     252,    19,     6,   119,    20,   178,    39,    40,    21,    22,
      23,    24,   193,     6,   198,    70,     6,   200,   207,   211,
     253,   198,   199,     6,   200,   201,    41,    -2,    81,   143,
     258,   254,    71,    82,   177,     1,    72,   245,    36,    71,
      -2,   202,   195,    98,   226,   227,  -132,   148,   202,    19,
     229,   305,    20,   231,   263,   216,    21,    22,    23,    24,
      74,   230,   205,   203,   204,     2,    96,   119,    41,   205,
     248,    41,    41,   158,   159,   211,     6,   296,    33,   188,
      36,   207,    71,   178,   178,    41,    46,    71,   297,    49,
      41,    42,   127,   128,   256,   284,   286,    52,   191,   315,
     316,   317,   318,   121,   320,   321,    41,   129,   342,   246,
     274,   309,   177,   177,    41,   192,   144,   144,    50,   333,
      67,   287,   130,   131,    51,     6,   119,   192,   192,    39,
      40,     6,    80,    81,    85,   338,   192,   100,    82,   312,
     294,    87,    10,    70,     6,    88,    89,   119,    39,    40,
     104,    41,   346,   347,   103,   278,   350,   249,   250,   304,
     107,    10,    41,   211,   211,    87,   332,   175,    18,    88,
      91,  -171,    19,    81,   108,    20,   109,   262,    82,    21,
      22,    23,    24,    41,   306,   307,  -171,    58,   325,   112,
      59,    19,   300,   113,    20,    71,    60,    61,    21,    22,
      23,    24,   114,   119,   125,    36,    41,    87,   126,   115,
     116,    88,   283,   135,     6,   138,   136,    62,    39,    47,
      63,   140,   207,   207,   278,    87,    64,    65,   149,    88,
     285,    10,   349,   133,   174,   179,   180,     6,   182,   183,
     186,   151,   152,   184,   207,   153,   356,   278,   196,   154,
     155,   188,   156,     9,    10,   187,   157,   213,   158,   159,
     160,    19,   197,   215,    20,  -140,   218,   221,    21,    22,
      23,    24,    14,   219,   161,   224,   232,   233,   234,    17,
      18,     6,   236,   247,    19,     7,     8,    20,   240,   162,
     163,    21,    22,    23,    24,   251,   257,     9,    10,    11,
     260,   261,     6,   266,    12,   268,    39,    40,   269,   272,
     275,   277,   276,    13,   125,   290,    14,    15,    16,    10,
     282,   289,     6,    17,    18,   291,   228,    40,    19,   292,
     293,    20,   295,   134,   299,    21,    22,    23,    24,    10,
     298,   117,   301,   198,   199,     6,   200,   201,   308,    19,
     310,   313,    20,   323,   319,   324,    21,    22,    23,    24,
     326,   314,   327,   329,   330,   331,   336,   337,   339,    19,
     202,   340,    20,   341,   351,   352,    21,    22,    23,    24,
     353,   354,   357,   359,   360,   203,   204,     4,   281,   235,
     280,   205,    27,   264,   334,   267,   355,   345,   220,     0,
     217
};

static const short yycheck[] =
{
       7,     8,     9,    10,   125,    12,    43,    38,     3,     0,
      17,    18,    88,     9,   290,   133,    10,   112,   103,    56,
       5,    17,     5,    30,   119,     5,     8,     5,     5,    36,
       5,    13,    39,    40,    25,    42,    29,   313,    45,    12,
      47,    48,    15,     9,   126,    11,    12,    32,    55,    45,
      29,    17,    18,     0,     5,    12,    38,    11,     9,    10,
       8,     8,    69,    70,    41,    44,   150,   188,    52,    49,
      21,    22,    65,    80,    11,    50,    70,    34,    63,    45,
      63,    88,   166,   167,   168,    63,   170,   171,   172,   173,
      38,    38,    71,   100,    31,   213,    47,    16,   193,   184,
     182,    52,     5,    69,    55,   100,     9,    10,    59,    60,
      61,    62,   119,     5,     3,    34,     5,     6,   125,   126,
       5,     3,     4,     5,     6,     7,   133,     0,    11,    21,
      12,    16,    32,    16,   100,     8,    36,   174,    50,    32,
      13,    30,   121,    36,   151,   152,    16,   223,    30,    52,
     157,   269,    55,   160,   191,     5,    59,    60,    61,    62,
       5,   157,    51,    45,    46,    38,    49,   133,   175,    51,
     177,   178,   179,    26,    27,   182,     5,     5,     3,    49,
      50,   188,    32,   178,   179,   192,    11,    32,    16,    16,
     197,   157,    34,    35,   185,   226,   227,     5,    14,   283,
     284,   285,   286,   197,   288,   289,   213,    49,   329,   175,
      14,    14,   178,   179,   221,    31,   223,   224,     5,    14,
      52,   228,    64,    65,    11,     5,   192,    31,    31,     9,
      10,     5,    33,    11,    13,   319,    31,    11,    16,   276,
     247,    11,    22,    34,     5,    15,    16,   213,     9,    10,
       5,   258,   336,   337,    11,   221,   340,   178,   179,   266,
      11,    22,   269,   270,   271,    11,   303,    47,    48,    15,
      16,    16,    52,    11,    50,    55,     5,    15,    16,    59,
      60,    61,    62,   290,   270,   271,    31,    52,   295,     5,
      55,    52,   258,    52,    55,    32,    61,    62,    59,    60,
      61,    62,    52,   269,    49,    50,   313,    11,    53,    52,
      52,    15,    16,     5,     5,    16,    31,    52,     9,    10,
      55,    16,   329,   330,   290,    11,    61,    62,    56,    15,
      16,    22,   339,    12,    51,    11,    16,     5,    49,    13,
       5,     9,    10,    31,   351,    13,   353,   313,    35,    17,
      18,    49,    20,    21,    22,    16,    24,    12,    26,    27,
      28,    52,    31,     6,    55,    33,    12,    34,    59,    60,
      61,    62,    40,    13,    42,    17,     5,    15,    15,    47,
      48,     5,    15,    13,    52,     9,    10,    55,    16,    57,
      58,    59,    60,    61,    62,    13,    16,    21,    22,    23,
      16,    16,     5,    50,    28,     6,     9,    10,    12,    54,
      16,    16,    14,    37,    49,    34,    40,    41,    42,    22,
      16,    16,     5,    47,    48,    50,     9,    10,    52,    15,
       5,    55,    13,    15,    13,    59,    60,    61,    62,    22,
      16,    44,     5,     3,     4,     5,     6,     7,    16,    52,
      22,    31,    55,     5,    16,    16,    59,    60,    61,    62,
      16,    35,    16,    14,    12,    31,    16,    16,    35,    52,
      30,    16,    55,    16,    31,    14,    59,    60,    61,    62,
       5,    16,    16,     0,     0,    45,    46,     1,   224,   164,
     223,    51,     3,   192,   309,   197,   351,   331,   142,    -1,
     136
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

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
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

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
	    (To)[yyi] = (From)[yyi];		\
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
#define YYABORT 	goto yyabortlab
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
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


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
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
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

#ifdef YYERROR_VERBOSE

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
#endif

#line 315 "/usr/share/bison/bison.simple"


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

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

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

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
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
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
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
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
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
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

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
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 164 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 171 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 175 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 178 "yacc.yy"
{
          ;
    break;}
case 6:
#line 183 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 184 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 189 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  ;
    break;}
case 9:
#line 198 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 201 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 204 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 207 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 13:
#line 211 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 14:
#line 219 "yacc.yy"
{
          ;
    break;}
case 15:
#line 222 "yacc.yy"
{
          ;
    break;}
case 16:
#line 225 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 228 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 231 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 234 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 237 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 240 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 243 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 246 "yacc.yy"
{
	  ;
    break;}
case 34:
#line 263 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 35:
#line 268 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 36:
#line 276 "yacc.yy"
{
	  dcop_area = 1;
	  dcop_signal_area = 0;
	;
    break;}
case 37:
#line 284 "yacc.yy"
{
	  /*
	  A dcop signals area needs all dcop area capabilities,
	  e.g. parsing of function parameters.
	  */
	  dcop_area = 1;
	  dcop_signal_area = 1;
	;
    break;}
case 38:
#line 295 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 39:
#line 298 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 40:
#line 307 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 41:
#line 313 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[-3]._str) + "&lt" + *(yyvsp[-1]._str) + "&gt;" );
		yyval._str = tmp;
	  ;
    break;}
case 42:
#line 322 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 43:
#line 326 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 44:
#line 333 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 45:
#line 337 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 46:
#line 345 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 47:
#line 349 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 48:
#line 356 "yacc.yy"
{
          ;
    break;}
case 50:
#line 363 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 51:
#line 367 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 52:
#line 371 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 53:
#line 375 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 379 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 383 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 387 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 391 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 395 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 399 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 403 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 61:
#line 407 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 62:
#line 411 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 63:
#line 415 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 64:
#line 419 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 65:
#line 423 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 72:
#line 441 "yacc.yy"
{;
    break;}
case 73:
#line 442 "yacc.yy"
{;
    break;}
case 74:
#line 446 "yacc.yy"
{;
    break;}
case 75:
#line 447 "yacc.yy"
{;
    break;}
case 76:
#line 448 "yacc.yy"
{;
    break;}
case 77:
#line 449 "yacc.yy"
{;
    break;}
case 78:
#line 450 "yacc.yy"
{;
    break;}
case 79:
#line 454 "yacc.yy"
{;
    break;}
case 80:
#line 455 "yacc.yy"
{;
    break;}
case 81:
#line 456 "yacc.yy"
{;
    break;}
case 82:
#line 461 "yacc.yy"
{
		if (dcop_area) {
 		  QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\"><PARAM %3</TYPEDEF>\n");
		  *tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		  yyval._str = tmp;
		} else {
		  yyval._str = new QString("");
		}
	  ;
    break;}
case 83:
#line 471 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 84:
#line 479 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 85:
#line 483 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 86:
#line 489 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 87:
#line 490 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 88:
#line 491 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 89:
#line 492 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 90:
#line 493 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 91:
#line 494 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 92:
#line 495 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 93:
#line 496 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 94:
#line 497 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 95:
#line 498 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 96:
#line 499 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 97:
#line 500 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 98:
#line 501 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 99:
#line 502 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 100:
#line 503 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 101:
#line 504 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 102:
#line 505 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 103:
#line 506 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 104:
#line 507 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 105:
#line 508 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 108:
#line 518 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 110:
#line 523 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 112:
#line 531 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 113:
#line 532 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 114:
#line 533 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 115:
#line 534 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 116:
#line 540 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 117:
#line 552 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 118:
#line 556 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 119:
#line 564 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 120:
#line 569 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 121:
#line 578 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 122:
#line 582 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 123:
#line 589 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 124:
#line 594 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 125:
#line 599 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 126:
#line 605 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 127:
#line 613 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 128:
#line 617 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 129:
#line 624 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 130:
#line 633 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 131:
#line 641 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 132:
#line 650 "yacc.yy"
{
	  ;
    break;}
case 133:
#line 653 "yacc.yy"
{
	  ;
    break;}
case 134:
#line 656 "yacc.yy"
{
	  ;
    break;}
case 135:
#line 663 "yacc.yy"
{
          ;
    break;}
case 136:
#line 666 "yacc.yy"
{
          ;
    break;}
case 137:
#line 669 "yacc.yy"
{
          ;
    break;}
case 138:
#line 672 "yacc.yy"
{
          ;
    break;}
case 139:
#line 675 "yacc.yy"
{
          ;
    break;}
case 140:
#line 680 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 141:
#line 681 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 147:
#line 690 "yacc.yy"
{
	     if (dcop_area || dcop_signal_area) {
		QString* tmp = 0;
                tmp = new QString(
                        "    <%4>\n"
                        "        %2\n"
                        "        <NAME>%1</NAME>"
                        "%3\n"
                        "     </%5>\n");
		*tmp = tmp->arg( *(yyvsp[-4]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
                *tmp = tmp->arg( *(yyvsp[-2]._str) );
                
                QString tagname = (dcop_signal_area) ? "SIGNAL" : "FUNC";
                QString attr = (yyvsp[0]._int) ? " qual=\"const\"" : "";
                *tmp = tmp->arg( QString("%1%2").arg(tagname).arg(attr) );
                *tmp = tmp->arg( QString("%1").arg(tagname) );
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 148:
#line 712 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 149:
#line 722 "yacc.yy"
{;
    break;}
case 150:
#line 723 "yacc.yy"
{;
    break;}
case 151:
#line 724 "yacc.yy"
{;
    break;}
case 152:
#line 729 "yacc.yy"
{;
    break;}
case 153:
#line 734 "yacc.yy"
{;
    break;}
case 154:
#line 735 "yacc.yy"
{;
    break;}
case 155:
#line 740 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 156:
#line 744 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 157:
#line 748 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 158:
#line 752 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 159:
#line 758 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 160:
#line 764 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 161:
#line 770 "yacc.yy"
{
              if (dcop_area) {
                 if (dcop_signal_area)
                     yyerror("DCOP signals cannot be static");
                 else
                     yyerror("DCOP functions cannot be static");
              } else {
                 yyval._str = new QString();
              }  
	  ;
    break;}
case 162:
#line 783 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 166:
#line 795 "yacc.yy"
{;
    break;}
case 167:
#line 796 "yacc.yy"
{;
    break;}
case 171:
#line 808 "yacc.yy"
{;
    break;}
case 172:
#line 809 "yacc.yy"
{;
    break;}
case 173:
#line 810 "yacc.yy"
{;
    break;}
case 174:
#line 813 "yacc.yy"
{;
    break;}
case 175:
#line 817 "yacc.yy"
{;
    break;}
case 176:
#line 818 "yacc.yy"
{;
    break;}
case 177:
#line 819 "yacc.yy"
{;
    break;}
case 178:
#line 820 "yacc.yy"
{;
    break;}
case 179:
#line 821 "yacc.yy"
{;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

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
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
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
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

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

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 824 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
