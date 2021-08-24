%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include <stdio.h>
#include <string.h>

#define YYSTYPE TreeNode *
int yyerror(char * message);
static char * savedName; /* for use in RECEBEments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);
char* scope;

%}
%start programa
%token ENDFILE PONTOEVIRGULA VIRGULA
%token IF INT ELSE RETURN VOID WHILE
%token RECEBE MENORQUE MAIORQUE IGUAL DIFERENTE MENORIGUAL MAIORIGUAL
%token CHAVESESQUERDA CHAVESDIREITA COLCHETESESQUERDA COLCHETESDIREITA
%token ID NUM
%left MAIS MENOS
%left VEZES DIVIDIR
%token PARENTESESESQUERDA PARENTESESDIREITA
%token ERROR
%expect 1

%% /* Grammar for C- */

programa            : declaracao_lista
                            {savedTree = $1;}
                    ;

declaracao_lista    : declaracao
                            {$$ = $1;}

                    | declaracao_lista declaracao
                            {YYSTYPE t = $1;
                            if (t != NULL)
                            { while (t->sibling != NULL)
                                 t = t->sibling;
                                 t->sibling = $2;
                                 $$ = $1;}
                              else $$ = $2;}
                    ;

declaracao          : var_declaracao
                            {$$ = $1;}

                    | fun_declaracao
                            {$$ = $1;}
                    ;

var_declaracao      : tipo_especificador identificador PONTOEVIRGULA
                              {$$ = $1;
                              $$->child[0] = $2;}

                    | tipo_especificador identificador COLCHETESESQUERDA num COLCHETESDIREITA PONTOEVIRGULA
                              {$$ = $1;
                              $2->vetor = 1;
                              $$->child[0] = $2;
                              $2->child[0] = $4;}
                    ;

tipo_especificador  : INT
                              {$$ = newExpNode(TypeK);
                              $$->type=Integer;
                              $$->attr.name = copyString(tokenString);}

                    | VOID
                              {$$ = newExpNode(TypeK);
                              $$->type=Void;
                              $$->attr.name = copyString(tokenString);}
                    ;

fun_declaracao      : tipo_especificador identificador PARENTESESESQUERDA params PARENTESESDIREITA composto_decl
                               {$$ = newStmtNode(FuncaoK);
                               $$->attr.name = $2->attr.name;
                               $$->child[1] = $4;
                               $$->child[2] = $6;

                               if (!strcmp($1->attr.name,"int"))
                                    $$->type = Integer;
                               else
                                    $$->type = Void;}
                    ;

params              : param_lista
                              {$$ = newStmtNode(ParametrosK);
                               $$->child[0] = $1;}

                    | VOID
                              {$$ = NULL;}
                    ;

param_lista         : param_lista VIRGULA param
                              {YYSTYPE t = $1;
                              if (t != NULL)
                              { while (t->sibling != NULL)
                                   t = t->sibling;
                                   t->sibling = $3;
                                   $$ = $1;}
                                else $$ = $3;}
                    | param
                              {$$ = $1;}
                    ;

param               : tipo_especificador identificador
                              {$$ = $1;
                              $$->child[0] = $2;}

                    | tipo_especificador identificador COLCHETESESQUERDA COLCHETESDIREITA
                              {$$ = $1;
                              $$->vetor = 2;
                              $$->child[0] = $2;}
                    ;

composto_decl       : CHAVESESQUERDA local_declaracoes statement_lista CHAVESDIREITA
                              {YYSTYPE t = $2;
                               if (t != NULL){
                                   while (t->sibling != NULL)
                                       t = t->sibling;
                               t->sibling = $3;
                               $$ = $2;
                               } else
                                     $$ = $3;}
                    ;

local_declaracoes   : local_declaracoes var_declaracao
                              {YYSTYPE t = $1;
                              if (t != NULL)
                              { while (t->sibling != NULL)
                                   t = t->sibling;
                                   t->sibling = $2;
                                   $$ = $1;}
                                else $$ = $2;}

                    | vazio { $$ = NULL; }
                    ;

statement_lista     : statement_lista statement
                              {YYSTYPE t = $1;
                              if (t != NULL)
                              { while (t->sibling != NULL)
                                   t = t->sibling;
                                   t->sibling = $2;
                                   $$ = $1;}
                                else $$ = $2;}

                    | vazio { $$ = NULL; }
                    ;

statement           : expressao_decl
                              {$$ = $1;}
                    | composto_decl
                              {$$ = $1;}
                    | selecao_decl
                              {$$ = $1;}
                    | iteracao_decl
                              {$$ = $1;}
                    | retorno_decl
                              {$$ = $1;}
                    ;

expressao_decl      : expressao PONTOEVIRGULA
                              {$$ = $1;}
                    | PONTOEVIRGULA
                              {$$ = NULL;}
                    ;

selecao_decl        : IF PARENTESESESQUERDA expressao PARENTESESDIREITA statement
                          {$$ = newStmtNode(IfK);
                          $$->child[0] = $3;
                          $$->child[1] = $5;}

                    | IF PARENTESESESQUERDA expressao PARENTESESDIREITA statement ELSE statement
                          {$$ = newStmtNode(IfK);
                          $$->child[0] = $3;
                          $$->child[1] = $5;
                          $$->child[2] = $7;}
                    ;

iteracao_decl       : WHILE PARENTESESESQUERDA expressao PARENTESESDIREITA statement
                          {$$ = newStmtNode(WhileK);
                          $$->child[0]=$3;
                          $$->child[1]=$5;
                          }
                    ;

retorno_decl        : RETURN PONTOEVIRGULA
                          {$$ = newStmtNode(ReturnK);}
                    | RETURN expressao PONTOEVIRGULA
                          {$$ = newStmtNode(ReturnK);
                          $$->child[0] = $2;}
                    ;

expressao           : var RECEBE expressao
                          {$$ = newStmtNode(RecebeK);
                          $$->child[0] = $1;
                          $$->child[1] = $3;}

                    | simples_expressao
                          {$$ = $1;}
                    ;

var                 : identificador
                            {$$ = $1;}

                    | identificador COLCHETESESQUERDA expressao COLCHETESDIREITA
                            {$$ = newExpNode(VetorK);
                            $1->vetor = 1;
                            $$->attr.name = $1->attr.name;
                            $$->child[0] = $3;}
                    ;

simples_expressao   : soma_expressao relacional soma_expressao
                            {$$ = newExpNode(OpK);
                            $$->attr.op = $2->attr.op;
                            $$->child[0] = $1;
                            $$->child[1] = $3;}

                    | soma_expressao
                            {$$ = $1;}
                    ;

relacional          : MENORIGUAL
                            {$$=newExpNode(OpK);
                            $$->attr.op=MENORIGUAL;}
                    | MENORQUE
                            {$$=newExpNode(OpK);
                            $$->attr.op=MENORQUE;}
                    | MAIORQUE

                            {$$=newExpNode(OpK);
                            $$->attr.op=MAIORQUE;}
                    | MAIORIGUAL
                            {$$=newExpNode(OpK);
                            $$->attr.op=MAIORIGUAL;}
                    | IGUAL
                            {$$=newExpNode(OpK);
                            $$->attr.op=IGUAL;}
                    | DIFERENTE
                            {$$=newExpNode(OpK);
                            $$->attr.op=DIFERENTE;}
                    ;

soma_expressao      : soma_expressao MAIS termo
                            {$$ = newExpNode(OpK);
                            $$->attr.op = MAIS;
                            $$->child[0] = $1;
                            $$->child[1] = $3;}

                    | soma_expressao MENOS termo
                            {$$ = newExpNode(OpK);
                            $$->attr.op = MENOS;
                            $$->child[0] = $1;
                            $$->child[1] = $3;}

                    | termo
                            {$$ = $1;}
                    ;

termo               : termo VEZES fator
                            {$$ = newExpNode(OpK);
                            $$->attr.op = VEZES;
                            $$->child[0] = $1;
                            $$->child[1] = $3;}

                    |termo DIVIDIR fator
                            {$$ = newExpNode(OpK);
                            $$->attr.op = DIVIDIR;
                            $$->child[0] = $1;
                            $$->child[1] = $3;}

                    | fator
                            {$$ = $1;}
                    ;

fator		            : PARENTESESESQUERDA expressao PARENTESESDIREITA
                            {$$ = $2;}
                    | var
                            {$$ = $1;}
		                | ativacao
                            {$$ = $1;}
		                | num
                            {$$ = $1;}
		                ;

ativacao	          : identificador PARENTESESESQUERDA args PARENTESESDIREITA
                            {$$ = newStmtNode(AtivacaoK);
                            $$->attr.name = $1->attr.name;
                            $$->child[0] = $3;}
                    ;

args                :    arg_lista { $$ = $1; }
                    |    vazio
                    ;

arg_lista           : arg_lista VIRGULA expressao
                            {YYSTYPE t = $1;
                            if (t != NULL)
                            { while (t->sibling != NULL)
                                 t = t->sibling;
                                 t->sibling = $3;
                                 $$ = $1;}
                            else $$ = $3;}
		                | expressao
                            {$$ = $1;}
		                ;

identificador       : ID
                          {$$ = newExpNode(IdK);
                          $$->attr.name = copyString(tokenString);}
                    ;
num	                : NUM
                          {$$ = newExpNode(ConstK);
                           $$->attr.val = atoi(copyString(tokenString));}
                    ;

vazio: {$$ = NULL;};
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}
