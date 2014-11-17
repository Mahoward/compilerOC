%{
#include "lyutils.h"
#include "astree.h"
%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON
%token TOK_ORD TOK_CHR ROOT TOK_INDEX TOK_NEWSTRING

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL TOK_DECLID
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_RETURNVOID TOK_FUNCTION TOK_PARAMLIST TOK_PROTOTYPE

%right      TOK_IF TOK_THEN TOK_ELSE
%right      '='
%left       TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left       '+' '-'
%left       '*' '/' '%'
%right      TOK_POS TOK_NEG '!' TOK_ORD TOK_CHR
%left       '[' ']' '.' '(' ')'
%nonassoc   TOK_NEW
%right      POS "u+" NEG "u-"

%start start

%%

start     : program             { yyparse_astree = $1; }
          ;
program   : program structdef   { $$ = adopt1($1, $2); }
          | program function    { $$ = adopt1($1, $2); }
          | program statement   { $$ = adopt1($1, $2); }
          | program error '}'   { $$ = $1; }
          | program error ';'   { $$ = $1; }
          |                     { $$ = new_parseroot(); }
          ;

structdef : TOK_STRUCT TOK_IDENT '{'mfdecl'}'
                                { $$ = adopt2($1,
                                       adopt1sym($2, NULL,
                                       TOK_TYPEID), $4);
                                      free_ast2($3, $5); }
          | TOK_STRUCT TOK_IDENT '{''}'
                                { $$ = adopt1($1,
                                       adopt1sym($2, NULL,
                                       TOK_TYPEID));
                                      free_ast2($3, $4); }
          ;

mfdecl    : mfdecl fielddecl';' { $$ = adopt1($1, $2);
                                       free_ast($3); }
          | fielddecl';'        { $$ = $1; free_ast($2); }
          ;

fielddecl : basetype TOK_ARRAY TOK_IDENT
                                { $$ = adopt2($1, $2,
                                        adopt1sym($3, NULL,
                                        TOK_FIELD)); }
          | basetype TOK_IDENT  { $$ = adopt1($1,
                                        adopt1sym($2, NULL,
                                        TOK_FIELD)); }
          ;

basetype  : TOK_VOID            { $$ = $1; }
          | TOK_BOOL            { $$ = $1; }
          | TOK_CHAR            { $$ = $1; }
          | TOK_INT             { $$ = $1; }
          | TOK_STRING          { $$ = $1; }
          | TOK_IDENT           { $$ = adopt1sym($1, NULL,
                                        TOK_FIELD); }
          ;

prototype : identdecl'(' midecl ')'';'
                                { $$ = adopt1($1,
                                        adopt1sym($3, NULL,
                                        TOK_PARAMLIST));
                                       free_ast2($2, $4); }
          | identdecl'('')'';'  { $$ = adopt1($1,$4);
                                       free_ast2($2, $3); }
          ;

function  : identdecl'(' midecl ')'block
                                { $$ = adopt2($1,
                                        adopt1sym($3, NULL,
                                        TOK_PARAMLIST), $5);
                                       free_ast2($2, $4); }
          | identdecl'('')'block
                                { $$ = adopt1($1, $4);
                                       free_ast2($2, $3); }
          ;

midecl    : midecl','identdecl  { $$ = adopt1($1, $3);
                                  free_ast($2); }
          | identdecl           { $$ = $1 }
          ;

identdecl : basetype TOK_ARRAY TOK_IDENT
                                { $$ = adopt2($1, $2,
                                        adopt1sym($3, NULL,
                                        TOK_DECLID)); }
          | basetype TOK_IDENT  { $$ = adopt1($1,
                                        adopt1sym($2, NULL,
                                        TOK_DECLID)); }
          ;

block     : '{'mstate'}'        { $$ = $2;
                                       free_ast2($1, $3); }
          ;

mstate    : mstate statement    { $$ = adopt1($1, $2); }
          | statement           { $$ = $1}
          ;

statement : ';'                 { free_ast($1); }
          | block               { $$ = $1; }
          | vardecl             { $$ = $1; }
          | while               { $$ = $1; }
          | ifelse              { $$ = $1; }
          | return              { $$ = $1; }
          | expr';'             { $$ = $1;
                                       free_ast($2); }
          ;

vardecl   : identdecl '=' expr ';'
                                { $$ = adopt1($1, $3);
                                       free_ast2($2, $4); }
          ;

while     : TOK_WHILE '('expr')'statement
                                { $$ = adopt2($1, $3, $5);
                                       free_ast2($2, $4); }
          ;

ifelse    : TOK_IF '('expr')'statement TOK_ELSE statement
                                { $$ = adopt2(adopt1sym($1,
                                        $3, TOK_IFELSE),
                                        $5,$7);
                                       free_ast2($2, $4); }
          | TOK_IF '('expr')'statement
                                { $$ = adopt2($1, $3, $5);
                                       free_ast2($2, $4); }
          ;

return    : TOK_RETURN expr';'
                                { $$ = adopt1($1, $2); }
          | TOK_RETURN';'       { $$ = adopt1sym($1, NULL,
                                        TOK_RETURNVOID);
                                       free_ast($2); }
          ;

expr      : expr '=' expr       { $$ = adopt2($2, $1, $3); }
          | expr TOK_EQ expr    { $$ = adopt2($2, $1, $3); }
          | expr TOK_NE expr    { $$ = adopt2($2, $1, $3); }
          | expr TOK_LT expr    { $$ = adopt2($2, $1, $3); }
          | expr TOK_LE expr    { $$ = adopt2($2, $1, $3); }
          | expr TOK_GT expr    { $$ = adopt2($2, $1, $3); }
          | expr TOK_GE expr    { $$ = adopt2($2, $1, $3); }
          | expr '+' expr       { $$ = adopt2($2, $1, $3); }
          | expr '-' expr       { $$ = adopt2($2, $1, $3); }
          | expr '*' expr       { $$ = adopt2($2, $1, $3); }
          | expr '/' expr       { $$ = adopt2($2, $1, $3); }
          | expr '%' expr       { $$ = adopt2($2, $1, $3); }
          | '+' expr %prec TOK_POS
                                { $$ = adopt1sym ($1, $2,
                                        TOK_POS); }
          | '-' expr %prec TOK_NEG
                                { $$ = adopt1sym ($1, $2,
                                        TOK_NEG); }
          | '!' expr            { $$ = adopt1 ($1, $2); }
          | TOK_ORD expr        { $$ = adopt1 ($1, $2); }
          | TOK_CHR expr        { $$ = adopt1 ($1, $2); }
          | allocator           { $$ = $1; }
          | call                { $$ = $1; }
          | variable            { $$ = $1; }
          | constant            { $$ = $1; }
          | '(' expr ')'        { $$ = $2; free_ast2 ($1, $3); }
          ;

mexpr     : mexpr expr       { $$ = adopt1($1, $2); }
          | expr ','            { $$ = $1; free_ast($2); }

allocator : TOK_NEW TOK_IDENT '('')'
                                { $$ = adopt1($1,
                                        adopt1sym($2, NULL,
                                        TOK_TYPEID)); }
          | TOK_NEW TOK_STRING '('expr')'
                                { $$ = adopt1sym($1, $4,
                                        TOK_NEWSTRING); }
          | TOK_NEW basetype '['expr']'
                                { $$ = adopt1(adopt1sym($1, $2,
                                        TOK_NEWARRAY), $4); }
          ;

call      : TOK_IDENT '('')'    { $$ = adopt1sym($1, NULL,
                                        TOK_CALL);
                                       free_ast2($2, $3);}
          | TOK_IDENT '('mexpr')'
                                { $$ = adopt1sym($1, $3,
                                        TOK_CALL);}
          ;

variable  : TOK_IDENT           { $$ = $1; }
          | expr '[' expr ']'   { $$ = adopt1($1,
                                        adopt1sym($3, NULL,
                                        TOK_INDEX)); }
          | expr '.' TOK_IDENT  { $$ = adopt1($1,
                                        adopt1sym($3, NULL,
                                       TOK_FIELD)); }
          ;

constant  : TOK_INTCON                 { $$ = $1; }
          | TOK_CHARCON                { $$ = $1; }
          | TOK_STRINGCON              { $$ = $1; }
          | TOK_TRUE                   { $$ = $1; }
          | TOK_FALSE                  { $$ = $1; }
          | TOK_NULL                   { $$ = $1; }
          ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}
