/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT_CONSTANT = 258,
     IDENT = 259,
     INT = 260,
     MAIN = 261,
     BREAK = 262,
     CONTINUE = 263,
     ELSE = 264,
     IF = 265,
     WHILE = 266,
     VOID = 267,
     CONST = 268,
     RETURN = 269,
     OP_AND = 270,
     OP_OR = 271,
     OP_EQ = 272,
     OP_NE = 273,
     OP_GE = 274,
     OP_LE = 275,
     LOWER_THAN_ELSE = 276
   };
#endif
/* Tokens.  */
#define INT_CONSTANT 258
#define IDENT 259
#define INT 260
#define MAIN 261
#define BREAK 262
#define CONTINUE 263
#define ELSE 264
#define IF 265
#define WHILE 266
#define VOID 267
#define CONST 268
#define RETURN 269
#define OP_AND 270
#define OP_OR 271
#define OP_EQ 272
#define OP_NE 273
#define OP_GE 274
#define OP_LE 275
#define LOWER_THAN_ELSE 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 19 "sysy.y"
{
	int		int_value;
	char *	string_value;
}
/* Line 1529 of yacc.c.  */
#line 96 "sysy.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

