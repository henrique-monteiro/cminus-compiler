#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include <string.h>

/* counter for variable memory locations */
static int location = 0;
static char* scope = " ";
int main_already_seem = 0;
static void declarationError(TreeNode * t, char * message);
/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}
static int primeiravez = 1;

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case RecebeK:  break;
        case IfK: break;
        case WhileK: break;
        case ReturnK: break;
        case ParametrosK: break;

        case AtivacaoK:
          if (st_lookup(t->attr.name, " ") == -1)
            declarationError(t,"Funcao nao declarada");
            break;

        case FuncaoK:
        if (primeiravez){
            st_insert("input",0,0, 0," ", "func", "int");
            st_insert("output",0,0,0, " ", "func", "void");
            st_insert("hdmi",0,0,0, " ", "func", "void");
            st_insert("hdmd",0,0,0, " ", "func", "void");
            st_insert("jumpProcesso",0,0,0, " ", "func", "void");
            st_insert("salvaPC",0,0,0, " ", "func", "void");
            st_insert("reghd",0,0,0, " ", "func", "void");
            st_insert("hdreg",0,0,0, " ", "func", "void");
            st_insert("interrupt",0,0,0, " ", "func", "void");
            primeiravez = 0;
        }

        scope = t->attr.name;
        if (strcmp(t->attr.name,"main")==0)
            main_already_seem = 1;

            if (st_lookup(t->attr.name, " ") == -1)
              {
                if (t->type==Integer)
                    st_insert(t->attr.name,t->lineno,location++,0, " ", "func", "int");

                else

                  st_insert(t->attr.name,t->lineno,location++,0," ", "func", "void");
              }
              else { declarationError(t,"nome ja utilizado por outra funcao");}
                break;

        default: break;

      }
      break;

    case ExpK:
      switch (t->kind.exp)

      {   case VetorK:
            t->scope = scope;
            st_insert(t->attr.name,t->lineno,0,1,scope, " ", " ");//
            break;
          case OpK: break;
          case ConstK: break;

          case IdK:
            if (st_lookup(t->attr.name, scope) == -1)
              declarationError(t,"variavel nao declarada neste escopo");
            else
              if (t->already_seem == 0)
              t->scope = scope;

              if (strcmp(scope," ")==0){
                if (t->vetor!=1) st_insert(t->attr.name,t->lineno,0, 0,scope, " ", " ");
            }
            else { if (t->vetor!=1) st_insert(t->attr.name,t->lineno,-999, 0,scope, " ", " ");}
              break;

              case TypeK:
                  if (t->type == Integer)
                  {
                      if (st_lookup(t->child[0]->attr.name, scope) == -1)
                      {
                          t->scope = scope;
                          if (strcmp(scope," ")==0)
                          {
                              if (t->child[0]->child[0]!=NULL)
                              {
                                  st_insert(t->child[0]->attr.name,t->child[0]->lineno,location++, 1,scope, "var", "int");
                                  location = location + t->child[0]->child[0]->attr.val - 1;
                              }
                              else
                                  st_insert(t->child[0]->attr.name,t->child[0]->lineno,location++, 0,scope, "var", "int");
                          }
                          else
                          {
                              if (t->child[0]->child[0]!=NULL)
                              {
                                  st_insert(t->child[0]->attr.name,t->child[0]->lineno,location++, 1,scope, "var", "int");
                                  location = location + t->child[0]->child[0]->attr.val - 1;
                              }
                              else if (t->vetor==1)
                              {
                                  st_insert(t->child[0]->attr.name,t->child[0]->lineno,location++, 1,scope, "var", "int");
                              }
                              else if (t->vetor==2)
                              {
                                  st_insert(t->child[0]->attr.name,t->child[0]->lineno,location++, 2,scope, "var", "int");
                              }
                              else
                                  st_insert(t->child[0]->attr.name,t->child[0]->lineno,location++, 0,scope, "var", "int");
                          }
                          t->child[0]->already_seem = 1;
                      }
                      else
                      {
                          declarationError(t,"nome já utilizado por outra variável declarada anteriormente neste escopo ou por outra função");
                      }
                }

                else
                {
                    if (t->type == Void)
                        declarationError(t,"variavel declarada como void");
                }
    break;


        default: break;
      }
      break;
    default:
      break;
  }
}



 static void mainError()
 { if (main_already_seem == 0) {fprintf(listing,"Error: main nao declarada\n");
   Error = TRUE;}
 }

 /* Function buildSymtab constructs the symbol
  * table by preorder traversal of the syntax tree
  */
void buildSymtab(TreeNode * syntaxTree)
{
  traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  { mainError();
    fprintf(listing,"\nTabela de Simbolos:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void declarationError(TreeNode * t, char * message)
{ fprintf(listing,"Na linha %d: %s ""(variável %s)""\n",t->lineno,message, t->attr.name);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op não é do tipo inteiro");

          if ((t->attr.op == IGUAL) || (t->attr.op == MENORQUE) ||  (t->attr.op == MAIORQUE) || (t->attr.op == DIFERENTE) || (t->attr.op == MENORIGUAL) || (t->attr.op == MAIORIGUAL) )
            t->type = Boolean;

          else
            t->type = Integer;
          break;

        case ConstK: t->type = Integer; break;

        case IdK: t->type = Integer; break;

          case VetorK:
            t->vetor = 1;
            t->type = Integer;
            break;

          case TypeK: break;

          break;
        default:
          break;
      }
      break;

    case StmtK:
      switch (t->kind.stmt)

      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;

        case RecebeK:
          if (t->child[0]->type != t->child[1]->type)
            typeError(t->child[0],"assignment error: different types");
          break;

        case WhileK:
          if (t->child[0]->type == Integer)
            typeError(t->child[1],"while test is not Boolean");
              break;

        case AtivacaoK:
          if (tipoFuncao(t->attr.name)==1)
          t->type = Integer;
          else
          t->type = Void;
          break;


        case FuncaoK: break;
        default: break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
