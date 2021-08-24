#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken( TokenType token, const char* tokenString )
{ switch (token)
  { case IF:
    case INT:
    case ELSE:
    case RETURN:
    case VOID:
    case WHILE:
    fprintf(listing,
         "reserved word: %s\n",tokenString);
      break;
    case RECEBE: fprintf(listing,"=\n"); break;
    case MENORQUE: fprintf(listing,"<\n"); break;
    case MAIORQUE: fprintf(listing,">\n"); break;
    case IGUAL: fprintf(listing,"==\n"); break;
    case DIFERENTE: fprintf(listing,"!=\n"); break;
    case MENORIGUAL: fprintf(listing,"<=\n"); break;
    case MAIORIGUAL: fprintf(listing,">=\n"); break;
    case PARENTESESESQUERDA: fprintf(listing,"(\n"); break;
    case PARENTESESDIREITA: fprintf(listing,")\n"); break;
    case COLCHETESESQUERDA: fprintf(listing,"[\n"); break;
    case COLCHETESDIREITA: fprintf(listing,"]\n"); break;
    case CHAVESESQUERDA: fprintf(listing,"{\n"); break;
    case CHAVESDIREITA: fprintf(listing,"}\n"); break;
    case PONTOEVIRGULA: fprintf(listing,";\n"); break;
    case MAIS: fprintf(listing,"+\n"); break;
    case MENOS: fprintf(listing,"-\n"); break;
    case VEZES: fprintf(listing,"*\n"); break;
    case DIVIDIR: fprintf(listing,"/\n"); break;
    case VIRGULA: fprintf(listing,",\n"); break;
    case ENDFILE: fprintf(listing,"EOF\n"); break;

    case NUM:
      fprintf(listing,
          "NUM, val= %s\n",tokenString);
      break;
    case ID:
      fprintf(listing,
          "ID, name= %s\n",tokenString);
      break;
    case ERROR:
      fprintf(listing,
          "ERROR: %s\n",tokenString);
      break;
    default: /* should never happen */
      fprintf(listing,"Unknown token: %d\n",token);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if (tree->nodekind==StmtK)
    { switch (tree->kind.stmt) {
        case IfK:
          fprintf(listing,"IF\n");
          break;
        case WhileK:
          fprintf(listing,"WHILE\n");
          break;
        case RecebeK:
          fprintf(listing,"ATRIBUICAO: \n");
          break;
        case FuncaoK:
        if (tree->type==Integer)
          fprintf(listing,"FUNCAO int %s\n",tree->attr.name);
        else
          fprintf(listing,"FUNCAO void %s\n",tree->attr.name);
          break;
        case ParametrosK:
          fprintf(listing,"PARAMETROS: \n");
          break;
        case AtivacaoK:
          fprintf(listing,"CHAMADA DA FUNCAO: %s\n",tree->attr.name);
          break;
        case ReturnK:
          fprintf(listing,"RETURN:\n");
          break;
        default:
          fprintf(listing,"DESCONHECIDO\n");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { switch (tree->kind.exp) {
        case OpK:
          fprintf(listing,"Operacao: ");
          printToken(tree->attr.op,"\0");
          break;
        case ConstK:
          fprintf(listing,"Constante: %d\n",tree->attr.val);
          break;
        case IdK:
          fprintf(listing,"Identificador: %s\n",tree->attr.name);
          break;
        case TypeK:
          if (tree->type==1)
            fprintf(listing,"Tipo: int\n");
          else
            fprintf(listing,"Tipo: void\n");
          break;
        case VetorK:
          fprintf(listing,"VETOR: %s\n",tree->attr.name);
          break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
    else fprintf(listing,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
