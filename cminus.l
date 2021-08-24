%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
static int yylex(void);
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  [a-zA-Z][a-zA-Z0-9_]*
newline     \n
whitespace  [ \t\r]+

%%

"else"          {return ELSE;}
"if"            {return IF;}
"int"           {return INT;}
"return"        {return RETURN;}
"void"          {return VOID;}
"while"         {return WHILE;}
"=="             {return IGUAL;}
"="            {return RECEBE;}
"!="            {return DIFERENTE;}
"<"             {return MENORQUE;}
"<="           	{return MENORIGUAL;}
">"             {return MAIORQUE;}
">="            {return MAIORIGUAL;}
"+"             {return MAIS;}
"-"             {return MENOS;}
"*"             {return VEZES;}
"/"             {return DIVIDIR;}
"("             {return PARENTESESESQUERDA;}
")"             {return PARENTESESDIREITA;}
"["             {return COLCHETESESQUERDA;}
"]"             {return COLCHETESDIREITA;}
"{"             {return CHAVESESQUERDA;}
"}"             {return CHAVESDIREITA;}
";"             {return PONTOEVIRGULA;}
","             {return VIRGULA;}

{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}

{whitespace}    {/* skip whitespace */}
"/*"			{
			  char c, cantes;

			  c = input();
			  do
                  	  {
		    	    cantes = c;
			    c = input();

                    	    if (cantes== EOF)
			      return ERROR;
                    	    if (cantes == '\n')
			      lineno++;
                  	  }
			  while (cantes != '*' || c != '/');
                	}
.               	{return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}
