#include "globals.h"
#include "symtab.h"
#include "cgen.h"

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int contadorParametros = 0;
static char* scope = "main";

static int label=0;
static int regTemp=9;
static int contalinhas = 0;

int flag_temp = 0;

BucketList st_lookupBucket ( char * name, char * scope );

char * novaLabel(){
    label++;
    char *str=(char*)malloc(sizeof(char)*15);
    sprintf(str, "%s%d", "L", label);
    return str;
}

char * buscaUltimaLabel(){
    char *str=(char*)malloc(sizeof(char)*15);
    sprintf(str, "%s%d", "L", label);
    return str;
}

char * novoRegTemp(){
    if (regTemp < 29)
    {
      regTemp++;
      char *T=(char*)malloc(sizeof(char)*15);
      sprintf(T, "%s%d", "t", regTemp);
      return T;
    }
    else
    {
      regTemp=10;
      char *T=(char*)malloc(sizeof(char)*15);
      sprintf(T, "%s%d", "t", regTemp);
      return T;
    }
}

char * buscaUltimoRegTemp(){
    char *T=(char*)malloc(sizeof(char)*15);
    sprintf(T, "%s%d", "t", regTemp);
    return T;
}


/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);

//lista quadrupla
typedef struct lista{
    tipono campo1, campo2, campo3, campo4;
    struct lista* prox;
} listaQuadrupla;

listaQuadrupla * cria_lista()
{
    return NULL;
}

listaQuadrupla* insereListaQuadrupla(listaQuadrupla * lista, tipono campo1, tipono campo2, tipono campo3, tipono campo4)
{
    listaQuadrupla* novo = (listaQuadrupla*)malloc(sizeof(listaQuadrupla));

    novo->campo1.endereco = campo1.endereco;
    novo->campo1.flag = campo1.flag;

    novo->campo2.endereco = campo2.endereco;
    novo->campo2.flag = campo2.flag;

    novo->campo3.endereco = campo3.endereco;
    novo->campo3.flag = campo3.flag;

    novo->campo4.endereco = campo4.endereco;
    novo->campo4.flag = campo4.flag;

    novo->prox = lista;

    return novo;
}

void imprimeListaQuadrupla(listaQuadrupla* lista){

    if (lista == NULL)
    {
        return;
    }
    if(contalinhas < 10)
      fprintf(listing, " %d -> ", contalinhas);
    else
      fprintf(listing, "%d -> ", contalinhas);
    contalinhas++;
    fprintf(listing, "( %s", lista->campo1.endereco.string);

    if (lista->campo2.flag == 1)
        if (lista->campo2.endereco.variable->vetor == 1)
            fprintf(listing, ", *%s ", lista->campo2.endereco.variable->name);
        else
            fprintf(listing, ", %s ", lista->campo2.endereco.variable->name);

    else if (lista->campo2.flag == 2)
        fprintf(listing, ", %s ", lista->campo2.endereco.string);

    else if (lista->campo2.flag == 3)
        fprintf(listing, ", %d ", lista->campo2.endereco.constant);



        if (lista->campo3.flag == 1)
            if (lista->campo3.endereco.variable->vetor == 1)
                fprintf(listing, ", *%s ", lista->campo3.endereco.variable->name);
            else
                fprintf(listing, ", %s ", lista->campo3.endereco.variable->name);

    else if (lista->campo3.flag == 2)
        fprintf(listing, ", %s ", lista->campo3.endereco.string);

    else if (lista->campo3.flag == 3)
        fprintf(listing, ", %d ", lista->campo3.endereco.constant);



        if (lista->campo4.flag == 1)
            if (lista->campo4.endereco.variable->vetor == 1)
                fprintf(listing, ", *%s )\n", lista->campo4.endereco.variable->name);
            else
                fprintf(listing, ", %s )\n", lista->campo4.endereco.variable->name);

    else if (lista->campo4.flag == 2)
        fprintf(listing,", %s ) \n", lista->campo4.endereco.string);

    else if (lista->campo4.flag == 3)
        fprintf(listing, ", %d )\n", lista->campo4.endereco.constant);

    return imprimeListaQuadrupla(lista->prox);
}

void ListAnalyzer(listaQuadrupla * l);

tipono campoNaoUsado(){//usado para completar os parametros de inser????o na lista quadrupla
  tipono campoNaoUsado;
  campoNaoUsado.endereco.string = "_";
  campoNaoUsado.flag = 2;
  return campoNaoUsado;
}
listaQuadrupla* lista;
tipono aux, aux1, aux2, aux3, auxnew, auxnew2;
tipono auxwhile0, auxwhile1, auxwhile2, auxwhile3;
/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree)
{
  TreeNode * p1, * p2, * p3;
  switch (tree->kind.stmt)
  {
    case RecebeK:
    p1 = tree->child[0];
    p2 = tree->child[1];
    if(p2->kind.exp == OpK)//filho da direita ?? operador
    {
      cGen(p2);
      p2->aux1.endereco.string = buscaUltimoRegTemp();
      p2->aux1.flag = 2;
    }
    else if(p2->kind.stmt == AtivacaoK && p2->vetor!=1)//filho da direita ?? retorno de fun????o
    {
      cGen(p2);
      p2->aux1.endereco.string = buscaUltimoRegTemp();
      p2->aux1.flag = 2;
    }
    else
    {
      if (p2->vetor!=1)//se filho da direita n??o ?? vetor
      {
        if(p2->kind.exp == ConstK)//se filho da direita ?? constante
        {
          p2->aux1.endereco.constant = p2->attr.val;
          p2->aux1.flag = 3;
        }
        else //se filho da direita ?? vari??vel
        {
          p2->aux1.endereco.variable = st_lookupBucket(p2->attr.name, p2->scope);
          p2->aux1.flag = 1;
        }
      }
      else //quando ?? vetor
      {
        if(p2->child[0] != NULL)
        {
          if(p2->child[0]->kind.exp == OpK)//se ??ndice ?? operador. Ex variavel = y[2+4] (asg, *y, t10, t11)
          {
            cGen(p2->child[0]);
            p2->child[0]->aux2.endereco.string = buscaUltimoRegTemp();
            p2->child[0]->aux2.flag = 2;
          }
          else//se indice n??o for operador
          {
            if(p2->child[0]->kind.exp == ConstK)//se ??ndice for constante. Ex x = y[2] (asg, *y, 2, t10)
            {
              p2->child[0]->aux2.endereco.constant = p2->child[0]->attr.val;
              p2->child[0]->aux2.flag = 3;
            }
            else //se ??ndice for vari??vel. Ex x = y[x] (asg, *y, x, t10)
            {
              p2->child[0]->aux2.endereco.variable = st_lookupBucket(p2->child[0]->attr.name, p2->child[0]->scope);
              p2->child[0]->aux2.flag = 1;
            }
          }
          aux.endereco.string="asg";//(asg,...,...,...)
          aux.flag=0;
          p2->child[0]->aux3.endereco.string = novoRegTemp();//(..., ..., ..., t10)
          p2->child[0]->aux3.flag = 2;
          p2->child[0]->aux1.endereco.variable = st_lookupBucket(p2->attr.name, p2->scope);//(..., *y, ..., ...)
          p2->child[0]->aux1.flag = 1;
          lista = insereListaQuadrupla(lista, aux, p2->child[0]->aux1, p2->child[0]->aux2, p2->child[0]->aux3);

          /*
            pr??ximas duas linhas trata de inserir no pr??ximo asg da lista quadrupla, pois qndo o filho da direita ?? vetor utiliza-se o asg duas vezes no c??digo intermedi??rio, ou seja, um asg para buscar o valor (e saltar em um tempor??rio) e outro asg para saltar esse tempor??rio na posi????o destino da mem??ria (memloc)
          */
          p2->aux1.endereco.string = buscaUltimoRegTemp();
          p2->aux1.flag = 2;
        }
      }
    }
    /*tratando o filho da esquerda*/
    if(p1->vetor == 1)//Se for vetor
    {
      p2->aux2.endereco.variable = st_lookupBucket(p1->attr.name, p1->scope);//(..., ..., *y, ...)
      p2->aux2.flag = 1;
      if(p1->child[0] != NULL)
      {
        if (p1->child[0]->kind.exp == OpK)//se indice do vetor for operador. Ex y[2+4] = valor (..., ..., ..., buscaUltimoRegTemp)
        {
          cGen(p1->child[0]);
          p2->aux3.endereco.string = buscaUltimoRegTemp();
          p2->aux3.flag = 2;
        }
        else if (p1->child[0]->kind.exp == ConstK)//se indice do vetor for constante. Ex y[2] = valor (..., ..., ..., 3)
        {
          p2->aux3.endereco.constant = p1->child[0]->attr.val;
          p2->aux3.flag = 3;
        }
        else//se indice do vetor for variavel. Ex y[x] = valor (..., ..., ..., x)
        {
          p2->aux3.endereco.variable = st_lookupBucket(p1->child[0]->attr.name, p1->child[0]->scope);
          p2->aux3.flag = 1;
        }
      }
    }
    //tentar tratar qndo o ??ndice do vetor for uma chamada de fun????o Ex. y[teste()] = valor
    else//se filho da esquerda n??o for vetor. Ex x = valor
    {
      p2->aux2.endereco.string = "_";//(..., ..., _, ...)
      p2->aux2.flag = 2;
      p2->aux3.endereco.variable = st_lookupBucket(p1->attr.name, p1->scope);//(..., ..., ..., x)
      p2->aux3.flag = 1;
    }
    aux.endereco.string="asg"; //(asg, ..., ..., ...)
    aux.flag=0;
    lista = insereListaQuadrupla(lista, aux, p2->aux1, p2->aux2, p2->aux3);
    break;

    case IfK :
    p1 = tree->child[0] ;
    p2 = tree->child[1] ;
    p3 = tree->child[2] ;
    /* generate code for test expression */
    if(p3 == NULL)
    {
      cGen(p1);
      aux.endereco.string="if";
      aux.flag=0;
      p1->aux1.endereco.string = buscaUltimoRegTemp();
      p1->aux1.flag = 2;
      p1->aux2.endereco.string = novaLabel();
      p1->aux2.flag = 2;
      lista = insereListaQuadrupla(lista, aux, p1->aux1, p1->aux2, campoNaoUsado());

      cGen(p2);
      aux.endereco.string="lab";
      aux.flag=0;
      p2->aux1.endereco.string = p1->aux2.endereco.string;
      p2->aux1.flag = 2;
      lista = insereListaQuadrupla(lista, aux, p2->aux1, campoNaoUsado(), campoNaoUsado());
    }
    else
    {
      cGen(p1);
      aux.endereco.string="if";
      aux.flag=0;
      p1->aux1.endereco.string = buscaUltimoRegTemp();
      p1->aux1.flag = 2;
      p1->aux2.endereco.string = novaLabel();
      p1->aux2.flag = 2;
      lista = insereListaQuadrupla(lista, aux, p1->aux1, p1->aux2, campoNaoUsado());

      cGen(p2);
      aux.endereco.string="goto";
      aux.flag=0;
      p2->aux1.endereco.string = novaLabel();
      p2->aux1.flag = 2;
      lista = insereListaQuadrupla(lista, aux, p2->aux1, campoNaoUsado(), campoNaoUsado());

      aux1.endereco.string = p1->aux2.endereco.string;
      aux1.flag = 2;
      aux.endereco.string="lab";
      aux.flag=0;
      lista = insereListaQuadrupla(lista, aux, aux1, campoNaoUsado(), campoNaoUsado());

      cGen(p3);
      aux.endereco.string="lab";
      aux.flag=0;
      p3->aux1.endereco.string = p2->aux1.endereco.string;
      p3->aux1.flag = 2;
      lista = insereListaQuadrupla(lista, aux, p3->aux1, campoNaoUsado(), campoNaoUsado());
    }

    break;

    case WhileK:
      p1 = tree->child[0] ;
      p2 = tree->child[1] ;
      TreeNode paux1;
      TreeNode paux2;


        aux.endereco.string="lab";
        aux.flag=0;
        paux1.aux1.endereco.string = novaLabel();
        paux1.aux1.flag = 2;
        lista = insereListaQuadrupla(lista, aux, paux1.aux1, campoNaoUsado(), campoNaoUsado());

        cGen(p1);
        aux.endereco.string="if";
        aux.flag=0;
        p1->aux1.endereco.string = buscaUltimoRegTemp();
        p1->aux1.flag = 2;
        p1->aux2.endereco.string = novaLabel();
        p1->aux2.flag = 2;
        lista = insereListaQuadrupla(lista, aux, p1->aux1, p1->aux2, campoNaoUsado());


        cGen(p2);
        aux.endereco.string="goto";
        aux.flag=0;
        p2->aux1.endereco.string = paux1.aux1.endereco.string;
        p2->aux1.flag = 2;
        lista = insereListaQuadrupla(lista, aux, p2->aux1, campoNaoUsado(), campoNaoUsado());

        paux2.aux1.endereco.string = p1->aux2.endereco.string;
        paux2.aux1.flag = 2;
        aux.endereco.string="lab";
        aux.flag=0;
        lista = insereListaQuadrupla(lista, aux, paux2.aux1, campoNaoUsado(), campoNaoUsado());

        break;

    case FuncaoK:
        aux.endereco.string="lab";
        aux.flag=0;
        aux1.endereco.variable = st_lookupBucket(tree->attr.name, " ");
        aux1.flag = 1;
        lista = insereListaQuadrupla(lista, aux, aux1, campoNaoUsado(), campoNaoUsado());

        cGen(tree->child[2]);
        break;

    case ParametrosK:
        break;

    case AtivacaoK:
        p1 = tree->child[0];

        if (p1!=NULL)
        p1->contadorParametros = 0;
        else
        contadorParametros = 0;

        while (p1 != NULL)
        {
            p1->contadorParametros++;
            if (p1->kind.exp == OpK)/*qndo h?? uma opera????o como parametro. Ex teste(2+3); (call, t10, ..., ...)*/
            {
                cGen(p1);
                aux1.endereco.string = buscaUltimoRegTemp();
                aux1.flag = 2;
            }
            else if(p1->kind.stmt == AtivacaoK && p1->vetor != 1)/*qndo ha uma chamada de fun????o como parametro. Ex teste(test());*/
            {
                cGen(p1);
                aux1.endereco.string = buscaUltimoRegTemp();
                aux1.flag = 2;
            }
            else
            {
                if (p1->vetor != 1)//qndo n??o ?? vetor
                {
                    if (p1->kind.exp == ConstK)//qndo ?? uma constante como parametro. Ex teste(2);
                    {
                        aux1.endereco.constant = p1->attr.val;
                        aux1.flag = 3;
                    }
                    else//qndo ?? uma vari??vel como parametro. Ex teste(x);
                    {
                        aux1.endereco.variable = st_lookupBucket(p1->attr.name, p1->scope);
                        aux1.flag = 1;
                    }
                }
                else//qndo ?? vetor
                {
                    if(p1->child[0] != NULL)//qndo vetor tem indice
                    {
                        if(p1->child[0]->kind.exp == OpK)//qndo indice do vetor ?? um operador
                        {
                            cGen(p1->child[0]);
                            aux2.endereco.string = buscaUltimoRegTemp();
                            aux2.flag = 2;

                        }
                        else
                        {
                            if(p1->child[0]->kind.exp == ConstK)//qndo o indice do vetor ?? uma constante
                            {
                                aux2.endereco.constant = p1->child[0]->attr.val;
                                aux2.flag = 3;
                            }
                            else//qndo o indice do vetor ?? uma variavel
                            {
                                aux2.endereco.variable = st_lookupBucket(p1->child[0]->attr.name, p1->child[0]->scope);
                                aux2.flag = 1;
                            }
                        }
                        //atribunido os outros campos da lista qu??drupla para caso de vetor
                        aux.endereco.string="par";
                        aux.flag=0;
                        aux1.endereco.variable = st_lookupBucket(p1->attr.name, p1->scope);
                        aux1.flag = 1;

                        lista = insereListaQuadrupla(lista, aux, aux1, aux2, campoNaoUsado());
                    }
                }
            }
            if (p1->vetor != 1)//atribuindo os outros campos da lista qu??drupla para qndo n??o tem vetor
            {
                aux.endereco.string="par";
                aux.flag=0;

                lista = insereListaQuadrupla(lista, aux, aux1, campoNaoUsado(), campoNaoUsado());
            }
            if(p1->sibling!= NULL)//quando houver mais de um par??metro
            {
                p1->sibling->contadorParametros = p1->contadorParametros;
            }
            else
            {
                contadorParametros = p1->contadorParametros;
            }
            p1 = p1->sibling;
        }
        //(call, nomeFuncao, qtdParam, novoRegTemp) caso de chamada de fun????o com retorno
        //(call, nomeFuncao, qtdParam, _) caso de chamada de fun????o void
        aux.endereco.string="call";
        aux.flag = 0;
        aux1.endereco.string = tree->attr.name;
        aux1.flag = 2;
        aux2.endereco.constant = contadorParametros;
        aux2.flag = 3;

        if (tree->type==Integer)
            aux3.endereco.string = novoRegTemp();
        else
            aux3.endereco.string = "_";
        aux3.flag = 2;
        lista = insereListaQuadrupla(lista, aux, aux1, aux2, aux3);
        break;

    case ReturnK:
        p1 = tree->child[0];
        if(p1->kind.exp == OpK)//Ex: return 2+3;
        {
            cGen(p1);
            aux1.endereco.string = buscaUltimoRegTemp();
            aux1.flag = 2;
        }
        else if(p1->kind.stmt == AtivacaoK && p1->vetor != 1)//se retorno ?? fun????o e n??o vetor Ex: return teste();
        {

            cGen(p1);
            aux1.endereco.string = buscaUltimoRegTemp();
            aux1.flag = 2;
        }
        else
        {
            if(p1->vetor !=1 )//se retorno n??o ?? vetor
            {
                if (p1->kind.exp != ConstK)//se retorno n??o ?? constante
                {
                    aux1.endereco.variable = st_lookupBucket(p1->attr.name,p1->scope);
                    aux1.flag = 1;
                }
                else//se retorno ?? constante
                {
                    aux1.endereco.constant = p1->attr.val;
                    aux1.flag = 3;
                }
            }
            else//se retorno ?? vetor
            {
                if(p1->child[0] != NULL)
                {
                    if(p1->child[0]->kind.exp == OpK)//se retorno ?? um vetor com opera????o no ??ndice
                    {
                        cGen(p1->child[0]);
                        aux2.endereco.string = buscaUltimoRegTemp();
                        aux2.flag = 2;

                    }
                    else
                    {
                        if(p1->child[0]->kind.exp == ConstK)//se retorno ?? vetor com constante como ??ndice
                        {
                            aux2.endereco.constant = p1->child[0]->attr.val;
                            aux2.flag = 3;
                        }
                        else//se retorno ?? vetor com variavel como indice
                        {
                            aux2.endereco.variable = st_lookupBucket(p1->child[0]->attr.name, p1->child[0]->scope);
                            aux2.flag = 1;
                        }
                    }
                    //preenchendo os outros campos da lista quadrupla quando ?? vetor
                    aux.endereco.string="ret";
                    aux.flag=0;
                    aux1.endereco.variable = st_lookupBucket(p1->attr.name, p1->scope);
                    aux1.flag = 1;

                    lista = insereListaQuadrupla(lista, aux, aux1, aux2, campoNaoUsado());
                }
            }
        }
        if(p1->vetor != 1)//se n??o ?? vetor preenche-se os outros campos da lista qu??drupla
        {
            aux.endereco.string="ret";
            aux.flag=0;
            lista = insereListaQuadrupla(lista, aux, aux1, campoNaoUsado(), campoNaoUsado());
        }
        break;

    default:
        break;
    }
}

/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{
  TreeNode * p1, * p2;
  switch (tree->kind.exp)
  {
  case ConstK :
    break;
  case IdK :
    break;
  case TypeK:
    break;
  case VetorK:
    break;
  case OpK :
    p1 = tree->child[0];
    p2 = tree->child[1];
    if(p1->kind.exp == OpK)
    {
      cGen(p1);
      p1->aux1.endereco.string = buscaUltimoRegTemp();
      p1->aux1.flag = 2;
    }
    else if(p1->kind.stmt == AtivacaoK && p1->vetor != 1)
    {
      cGen(p1);
      p1->aux1.endereco.string = buscaUltimoRegTemp();
      p1->aux1.flag = 2;
    }
    else
    {
      if(p1->vetor != 1)
      {
        if(p1->kind.exp != ConstK)
        {
          p1->aux1.endereco.variable = st_lookupBucket(p1->attr.name,p1->scope);
          p1->aux1.flag = 1;
        }
        else
        {
          p1->aux1.endereco.constant = p1->attr.val;
          p1->aux1.flag = 3;
        }
      }
      else
      {
        if(p1->child[0] != NULL)
        {
          if(p1->child[0]->kind.exp == OpK)
          {
            cGen(p1->child[0]);
            p1->aux2.endereco.string = buscaUltimoRegTemp();
            p1->aux2.flag = 2;
          }
          else
          {
            if(p1->child[0]->kind.exp == ConstK)
            {
              p1->aux2.endereco.constant = p1->child[0]->attr.val;
              p1->aux2.flag = 3;
            }
            else
            {
              p1->aux2.endereco.variable = st_lookupBucket(p1->child[0]->attr.name, p1->child[0]->scope);
              p1->aux2.flag = 1;
            }
          }
          aux.endereco.string="asg";
          aux.flag=0;
          p1->aux1.endereco.variable = st_lookupBucket(p1->attr.name, p1->scope);
          p1->aux1.flag = 1;
          p1->aux3.endereco.string = novoRegTemp();
          p1->aux3.flag = 2;
          lista = insereListaQuadrupla(lista, aux, p1->aux1, p1->aux2, p1->aux3);
          p1->aux1.endereco.string = buscaUltimoRegTemp();
          p1->aux1.flag = 2;
        }
      }
    }
    if(p2->kind.exp == OpK)
    {
      cGen(p2);
      p2->aux2.endereco.string = buscaUltimoRegTemp();
      p2->aux2.flag = 2;
    }
    else if(p2->kind.stmt == AtivacaoK && p2->vetor != 1)
    {
      auxnew = aux1;
      cGen(p2);
      aux1 = auxnew;
      p2->aux2.endereco.string = buscaUltimoRegTemp();
      p2->aux2.flag = 2;
    }
    else
    {
      if(p2->vetor != 1)
      {
        if(p2->kind.exp != ConstK)
        {
          p2->aux2.endereco.variable = st_lookupBucket(p2->attr.name,p2->scope);
          p2->aux2.flag = 1;
        }
        else
        {
          p2->aux2.endereco.constant = p2->attr.val;
          p2->aux2.flag = 3;
        }
      }
      else
      {
        if(p2->child[0] != NULL)
        {
          if(p2->child[0]->kind.exp == OpK)
          {
            cGen(p2->child[0]);
            p2->aux2.endereco.string = buscaUltimoRegTemp();
            p2->aux2.flag = 2;
          }
          else
          {
            if(p2->child[0]->kind.exp == ConstK)
            {
              p2->aux2.endereco.constant = p2->child[0]->attr.val;
              p2->aux2.flag = 3;
            }
            else
            {
              p2->aux2.endereco.variable = st_lookupBucket(p2->child[0]->attr.name, p2->child[0]->scope);
              p2->aux2.flag = 1;
            }
          }
          aux.endereco.string="asg";
          aux.flag=0;
          p2->aux1.endereco.variable = st_lookupBucket(p2->attr.name, p2->scope);
          p2->aux1.flag = 1;
          p2->aux3.endereco.string = novoRegTemp();
          p2->aux3.flag = 2;
          lista = insereListaQuadrupla(lista, aux, p2->aux1, p2->aux2, p2->aux3);
          p2->aux2.endereco.string = buscaUltimoRegTemp();
          p2->aux2.flag = 2;
        }
      }
    }
    aux.flag=0;
    p1->aux3.endereco.string = novoRegTemp();
    p1->aux3.flag = 2;
    switch (tree->attr.op)
    {
      case MAIS :
      aux.endereco.string="soma";
      break;

      case MENOS :
      aux.endereco.string="sub";
      break;

      case VEZES :
      aux.endereco.string="mul";
      break;

      case DIVIDIR :
      aux.endereco.string="div";
      break;

      case MENORQUE :
      aux.endereco.string="menor";
      break;

      case MAIORQUE :
      aux.endereco.string="maior";
      break;

      case MENORIGUAL :
      aux.endereco.string="menorigual";
      break;

      case MAIORIGUAL :
      aux.endereco.string="maiorigual";
      break;

      case IGUAL :
      aux.endereco.string="igual";
      break;



      case DIFERENTE :
      aux.endereco.string="dif";
      break;


      default:
      break;
    }
    lista = insereListaQuadrupla(lista, aux, p1->aux1, p2->aux2, p1->aux3);

    break;



    default:
        break;
    }
} /* genExp */

static void cGen( TreeNode * tree)
{
    if (tree != NULL)
    {
        switch (tree->nodekind)
        {
        case StmtK:
            genStmt(tree);
            break;
        case ExpK:
            genExp(tree);
            break;
        default:
            break;
        }
        cGen(tree->sibling);
    }
}

void codeGen(TreeNode * syntaxTree, char * codefile)
{
    fprintf(listing, "\n\nGeracao do Codigo Intermediario:\n\n");
    lista = cria_lista();
    char * s = malloc(strlen(codefile)+7);
    strcpy(s,"File: ");
    strcat(s,codefile);
    aux.endereco.string="call";
    aux.flag=0;
    aux1.endereco.string = "main";
    aux1.flag = 2;
    lista = insereListaQuadrupla(lista, aux, aux1, campoNaoUsado(), campoNaoUsado());
    cGen(syntaxTree);
    aux.endereco.string="halt";
    aux.flag=0;
    lista = insereListaQuadrupla(lista, aux, campoNaoUsado(), campoNaoUsado(), campoNaoUsado());
    listaQuadrupla *listafinal;
    listafinal = cria_lista();
    while (lista != NULL)
    {
        listafinal = insereListaQuadrupla(listafinal, lista->campo1, lista->campo2, lista->campo3, lista->campo4);
        lista = lista->prox;
    }
    imprimeListaQuadrupla(listafinal);
    ListAnalyzer(listafinal);
}
