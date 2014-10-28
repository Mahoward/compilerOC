// $Id: tokens.h,v 1.7 2014-10-09 18:39:25-07 - - $

#ifndef __TOKENS_H__
#define __TOKENS_H__

#define YYEOF     0 
#define IDENT   260
#define NUMBER  261
#define ERROR   262

extern int yy_flex_debug;
extern char* yytext;
extern int yylex (void); 

#endif

