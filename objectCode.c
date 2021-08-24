#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include "analyze.h"

typedef struct listaCodigoAssembly
{
    tipono campo1, campo2, campo3, campo4;
    struct listaCodigoAssembly* prox;
} ListaIntermediaria;

int SR = 1;
static int contador = -1, contadorLinhasCodigoObjeto = 0, flagRetornoDeFuncao = 0, flag_slt=0;

char * convertIntString(int value)
{
    char *str=(char*)malloc(sizeof(char)*15);
    sprintf(str, "%d", value);
    return str;
}

char * newSavedRegister()
{
    if (SR == 0) SR = 1;
    else if (SR == 1) SR = 0;

    char *str=(char*)malloc(sizeof(char)*15);
    sprintf(str, "%s%d", "$S", SR);
    return str;
}

char * lastSavedRegister()
{
    char *str=(char*)malloc(sizeof(char)*15);
    sprintf(str, "%s%d", "$S", SR);
    return str;
}

/*CODIGO DA listaCodigoAssembly ASSEMBLY*/
typedef struct listaAssembly
{
    char *campo1, *campo2, *campo3, *campo4;
    struct listaAssembly* prox;
} ListaAssembly;

void geraListaInstrucoes(ListaAssembly * FinalList);

ListaAssembly * listaCodigoAssembly;

ListaAssembly* insereListaAssembly(ListaAssembly * listaCodigoAssembly, char* campo1, char* campo2, char* campo3, char* campo4)
{
    ListaAssembly* novo = (ListaAssembly*)malloc(sizeof(ListaAssembly));
    contador++;
    novo->campo1 = campo1;
    novo->campo2 = campo2;
    novo->campo3 = campo3;
    novo->campo4 = campo4;

    novo->prox = listaCodigoAssembly;
    return novo;
}

void imprimeListaAssembly(ListaAssembly* listaCodigoAssembly)
{
  fprintf(listing, "\n");

  if (listaCodigoAssembly == NULL)
    return;

  if(contadorLinhasCodigoObjeto < 10)
  fprintf(listing, "  %d ->  %s ", contadorLinhasCodigoObjeto, listaCodigoAssembly->campo1);
  else if(contadorLinhasCodigoObjeto >=10 && contadorLinhasCodigoObjeto < 100)
  fprintf(listing, " %d ->  %s ", contadorLinhasCodigoObjeto, listaCodigoAssembly->campo1);
  else
  fprintf(listing, "%d ->  %s ", contadorLinhasCodigoObjeto, listaCodigoAssembly->campo1);
  if(listaCodigoAssembly->campo2!=NULL)
  fprintf(listing, " %s ", listaCodigoAssembly->campo2);
  if(listaCodigoAssembly->campo3!=NULL)
  fprintf(listing, " %s ", listaCodigoAssembly->campo3);
  if(listaCodigoAssembly->campo4!=NULL)
  fprintf(listing, " %s ", listaCodigoAssembly->campo4);
  contadorLinhasCodigoObjeto++;
  return imprimeListaAssembly(listaCodigoAssembly->prox);
}

typedef struct parametro {
  tipono campo1, campo2;
  struct parametro* prox;
} parametro;

typedef struct filaParametro {
  parametro * inicio;
  parametro * fim;
} filaParametro;

void insereParametro(filaParametro *fila, parametro *param) {
  parametro *novo;
  novo = malloc(sizeof(parametro));
  *novo = *param;
  if (fila->fim == NULL){
    fila->fim = novo;
    fila->inicio = novo;
  }
  else {
    fila->fim->prox = novo;
    fila->fim = novo;
  }
}

void removeParametro(filaParametro *fila){
  parametro *p;
  if(fila->inicio != NULL){
    p = fila->inicio;
    fila->inicio = p->prox;
  }
  if (fila->inicio == NULL)
    fila->fim = NULL;
}

void imprimeFilaParam(filaParametro *fila)
{
while (fila->inicio != NULL)
  {
      if (fila->inicio->campo2.flag == 1){
        fprintf(listing, "\nLista de parametros: %s ", fila->inicio->campo2.endereco.variable->name);
        if (fila->inicio->campo2.endereco.variable->vetor == 1 || fila->inicio->campo2.endereco.variable->vetor == 2) //imprimir vetor
        fprintf(listing, "\nLista de parametros: posicao vetor %s %d", fila->inicio->campo2.endereco.variable->name, -9999);
      }

      else if (fila->inicio->campo2.flag == 2)
        fprintf(listing, "\nLista de parametros: %s ", fila->inicio->campo2.endereco.string);

      else if (fila->inicio->campo2.flag == 3)
        fprintf(listing, "\nLista de parametros: %d ", fila->inicio->campo2.endereco.constant);

      fila->inicio = fila->inicio->prox;
  }
}

parametro* primeiroFilaParam(filaParametro *fila)
{
  if (fila->inicio != NULL)
  {
    return fila->inicio;
  }
}


//listaCodigoAssembly de labels
typedef struct listaLabel{
    char *label;
    int posicao;
    struct listaLabel* prox;
} ListaLabel;

ListaLabel * listaLabel;

ListaLabel* insereLabel(ListaLabel * l, char* label, int posicao)
{
    ListaLabel* novo = (ListaLabel*)malloc(sizeof(ListaLabel));
    novo->label = label;
    novo->posicao = posicao;
    novo->prox = l;
    return novo;
}

void imprimeListaLabel(ListaLabel* l)
{
    fprintf(listing, "\n");

    if (l == NULL)
    {
        return;
    }
   fprintf(listing, " %s ", l->label);
    fprintf(listing, " %d ", l->posicao);
    return imprimeListaLabel(l->prox);
}

int procuraLabel(ListaLabel* listaCodigoAssembly, char * nomeFuncao)
{
    if (listaCodigoAssembly == NULL)
        return -1;

    else if (!strcmp(listaCodigoAssembly->label, nomeFuncao))
        return listaCodigoAssembly->posicao;

    return procuraLabel(listaCodigoAssembly->prox, nomeFuncao);
}
/*FIM CODIGO listaCodigoAssembly DE LABELS*/

void verificaLoads(ListaIntermediaria * intermediario);
void verificaTemps(ListaIntermediaria * intermediario, char * equacao);
void verificaLinhas (ListaIntermediaria * ListaInterAux);

void ListAnalyzer(ListaIntermediaria * intermediario)
{
  int i, primeiraVez = 0;
  filaParametro* Fila;
  Fila = malloc(sizeof(filaParametro));
  Fila->fim = NULL;
  Fila->inicio = NULL;
  char * nomeLabel;
  int posicaoLinhaLabel;
  int posicaoDaFuncaoNaMemoria;
  char * tipoRetornoFuncao;
  char* tipoVariavel;
  int memLoc;

  char * nomeFuncao;
  char * escopo;

  parametro *param;
  param = malloc(sizeof(parametro));

  char *straux=(char*)malloc(sizeof(char)*15);
  listaCodigoAssembly = NULL;

  verificaLinhas(intermediario);

  contadorLinhasCodigoObjeto = 0;

  while (intermediario != NULL)
  {
    if (strcmp(intermediario->campo1.endereco.string,"if")==0)
    {
      nomeLabel = intermediario->campo3.endereco.string;
      posicaoLinhaLabel = procuraLabel(listaLabel, nomeLabel);
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "beq", intermediario->campo2.endereco.string, "$zero", convertIntString(posicaoLinhaLabel));
      contadorLinhasCodigoObjeto++;
    }

    if (strcmp(intermediario->campo1.endereco.string,"lab")==0)
    {
      posicaoDaFuncaoNaMemoria = st_lookup ( escopo, " " );
      tipoRetornoFuncao = st_lookupCallTypeData ( escopo, " " );
      if (intermediario->campo2.flag == 1 && !strcmp(tipoRetornoFuncao,"void") && strcmp("main", intermediario->campo2.endereco.string) && primeiraVez)
      {
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load","$S31" ,convertIntString(posicaoDaFuncaoNaMemoria), NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "jr", "$S31", NULL, NULL);
        contadorLinhasCodigoObjeto+=2;
      }
      if (intermediario->campo2.flag == 1)
      {
        escopo = intermediario->campo2.endereco.variable->name;
        primeiraVez = 1;
      }
    }

    if (strcmp(intermediario->campo1.endereco.string,"goto")==0)
    {
      nomeLabel = intermediario->campo2.endereco.string;
      posicaoLinhaLabel = procuraLabel(listaLabel, nomeLabel);
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(posicaoLinhaLabel), NULL, NULL);
      contadorLinhasCodigoObjeto++;
    }

    if (strcmp(intermediario->campo1.endereco.string,"par")==0)
    {
      param->campo1 = intermediario->campo2;
      param->campo2 = intermediario->campo3;
      insereParametro(Fila, param);
    }

    if (strcmp(intermediario->campo1.endereco.string,"call")==0)
    {
      nomeFuncao = intermediario->campo2.endereco.string;

      if (!strcmp(nomeFuncao, "interrupt"))
      {
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "interrupt", " ", " ", " ");
        contadorLinhasCodigoObjeto+=1;
      }

      else if (!strcmp(nomeFuncao, "hdreg"))
      {
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
            param = primeiroFilaParam(Fila);
            if(i == 0)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$SR4" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
            else
            if(i == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$SR5" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
            removeParametro(Fila);
        }
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "hdreg", "$SR4", "$SR5", " ");
        contadorLinhasCodigoObjeto+=4;
      }

      else if (!strcmp(nomeFuncao, "reghd"))
      {
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
            param = primeiroFilaParam(Fila);
            if(i == 0)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$SR4" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
            else
            if(i == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$SR5" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
            removeParametro(Fila);
        }
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "reghd", "$SR4", "$SR5", " ");
        contadorLinhasCodigoObjeto+=4;
      }
      else if (!strcmp(nomeFuncao, "salvaPC"))
      {
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
          param = primeiroFilaParam(Fila);
          if(i == 0)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storePC", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
          removeParametro(Fila);
        }
        contadorLinhasCodigoObjeto+=1;
      }
      else if (!strcmp(nomeFuncao, "jumpProcesso"))
      {
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
          param = primeiroFilaParam(Fila);
          if(i == 0)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
          else if(i == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
          removeParametro(Fila);
        }
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "jProc", "$S0", "$S1", " ");
        contadorLinhasCodigoObjeto+=3;
      }


      else if (!strcmp(nomeFuncao, "hdmd"))
      {
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
          param = primeiroFilaParam(Fila);
          if(i == 0)
          {
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
            //printf("%d\n", param);
          }
          else if(i == 1) //trilha
          {
            //printf("%d\n", param);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
          }
          else if(i == 2)
          {
            //printf("%d\n", param);
            if (param->campo1.endereco.variable->vetor == 1 || param->campo1.endereco.variable->vetor == 2)
            {
              if(param->campo1.endereco.variable->vetor == 1)
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
              if(param->campo1.endereco.variable->vetor == 2)
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1" ,convertIntString(param->campo2.endereco.variable->memloc) , " ");
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0" , "$S1", "$S0");
              if(!strcmp(nomeFuncao, "hdmd"))
              {
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "hdmd", "$S0", "$S1", "$S2");
              }
              contadorLinhasCodigoObjeto+=4;
            }
            else
            {
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S2" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
              if(!strcmp(nomeFuncao, "hdmd"))
              {
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "hdmd", "$S0", "$S1", "$S2");
              }
              contadorLinhasCodigoObjeto+=2;
            }
          }
          //printf("%d\n", param);
          removeParametro(Fila);
        }
      }

      else if (!strcmp(nomeFuncao, "hdmi"))
      {
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
          param = primeiroFilaParam(Fila);
          if(i == 0)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
          else
          if(i == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
          else
          if(i == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S2" ,convertIntString(param->campo1.endereco.variable->memloc) , " ");
          removeParametro(Fila);
        }
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "hdmi", "$S0", "$S1", "$S2");
        contadorLinhasCodigoObjeto+=4;
      }


      //Quando não é input nem output
      else if (strcmp(nomeFuncao, "input") && strcmp(nomeFuncao, "output"))
      {
        if (intermediario->campo3.flag == 3)//verifica se ha parametros
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
          param = primeiroFilaParam(Fila);
          memLoc = st_lookup (nomeFuncao, " ") + (i+1);

          if(param->campo1.flag == 2){//(t10, ...)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", param->campo1.endereco.string, convertIntString(memLoc), NULL);
            contadorLinhasCodigoObjeto++;
          }else if(param->campo1.flag == 1){//(x, ...) onde x é vetor depois de passado por parametro, ou (*x, ...) ou (x, ...) onde x é variavel comum

            if(param->campo1.endereco.variable->vetor == 1 || param->campo1.endereco.variable->vetor == 2)//(*x,...) ou (x,...)
            {
              if(param->campo2.flag == 1)//(x, y) ou (*x,y)
              {
                if(param->campo1.endereco.variable->vetor == 1)//(*x,y)
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                if(param->campo1.endereco.variable->vetor == 2)//(x,y)
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1",convertIntString(param->campo2.endereco.variable->memloc), NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", "$S1", "$S0");
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S0", "$S0", NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(memLoc), NULL);
                contadorLinhasCodigoObjeto+=5;
              }
              else if(param->campo2.flag == 3) //(*x, 3) ou (x,3)
              {
                if(param->campo1.endereco.variable->vetor == 1)
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                if(param->campo1.endereco.variable->vetor == 2)
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$S0", "$S0", convertIntString(param->campo2.endereco.constant));
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S0", "$S0", NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(memLoc), NULL);
                contadorLinhasCodigoObjeto+=4;
              }
              else if(param->campo2.flag == 2)//(*x, _) ou (*x, t10) ou (x,_) ou (x,t10) ondd x é vetor depois de passado por parametro
              {
                if(strcmp(param->campo2.endereco.string, "_"))//(*x,t10) ou (x, t10)
                {
                  if(param->campo1.endereco.variable->vetor == 1)//(*x,t10)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                  if(param->campo1.endereco.variable->vetor == 2)//(x,t10)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", param->campo2.endereco.string, "$S0");
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S0", "$S0", NULL);
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(memLoc), NULL);
                  contadorLinhasCodigoObjeto+=4;
                }
                else//(*x,_), ou (x,_) vetor
                {
                  if(param->campo1.endereco.variable->vetor == 1)//(*x,_) essa caso é o de passar vetor por parametro pela primeira vez
                  {
                    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(memLoc), NULL);
                    contadorLinhasCodigoObjeto+=2;
                  }
                  else//(x,_) este caso é qndo passa vetor por parametro pela segunda vez ou adiante
                  {
                    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(param->campo1.endereco.variable->memloc), NULL);
                    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(memLoc), NULL);
                    contadorLinhasCodigoObjeto+=2;
                  }
                }
              }
            }
            else{//(x, _) variavel
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0", convertIntString(param->campo1.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(memLoc), NULL);
              contadorLinhasCodigoObjeto+=2;
            }
          }else{//(3,_)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0", convertIntString(param->campo1.endereco.constant), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store","$S0", convertIntString(memLoc), NULL);
            contadorLinhasCodigoObjeto+=2;
          }

            removeParametro(Fila);
        }

        posicaoLinhaLabel = procuraLabel(listaLabel, nomeFuncao);

        if (strcmp(nomeFuncao, "main"))
        {
          listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0", convertIntString(contadorLinhasCodigoObjeto+3), NULL); //VERIFICAR A CONTAGEM DE LINHAS
          listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(st_lookup ( nomeFuncao, " " )), NULL);
          contadorLinhasCodigoObjeto+=2;
          tipoVariavel = st_lookupCallTypeData(nomeFuncao, " ");
          if(strcmp(tipoVariavel, "void"))
            flagRetornoDeFuncao = 1;
        }

        //se for main pela primeira ve cai aqui
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(posicaoLinhaLabel), NULL, NULL);
        contadorLinhasCodigoObjeto++;
      }
      else
      {
        if (!strcmp(nomeFuncao, "input"))
        {
          listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "in", intermediario->campo4.endereco.string, NULL, NULL);
          contadorLinhasCodigoObjeto++;
        }
        else
        if (!strcmp(nomeFuncao, "output"))
        {
          param = primeiroFilaParam(Fila);

          if(param->campo1.flag == 1)
          {
            if (param->campo1.endereco.variable->vetor == 1 || param->campo1.endereco.variable->vetor == 2)
            {
              if (param->campo2.flag == 1)
              {
                if(param->campo1.endereco.variable->vetor == 1)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , NULL);
                if(param->campo1.endereco.variable->vetor == 2)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1" ,convertIntString(param->campo2.endereco.variable->memloc) , NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0" , "$S1", "$S0");
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S0", "$S0", NULL);
                contadorLinhasCodigoObjeto+=4;
              }
              else if (param->campo2.flag == 2)
              {
                if(param->campo1.endereco.variable->vetor == 1)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , NULL);
                if(param->campo1.endereco.variable->vetor == 2)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0" , param->campo2.endereco.string, "$S0");
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S0", "$S0", NULL);
                contadorLinhasCodigoObjeto+=3;
              }
              else
              {
                if(param->campo1.endereco.variable->vetor == 1)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , NULL);
                if(param->campo1.endereco.variable->vetor == 2)
                  listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(param->campo1.endereco.variable->memloc) , NULL);
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$S0" , "$S0", convertIntString(param->campo2.endereco.constant));
                listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S0", "$S0", NULL);
                contadorLinhasCodigoObjeto+=3;
              }
            }
            else
            {
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0", convertIntString(param->campo1.endereco.variable->memloc), NULL);
              contadorLinhasCodigoObjeto++;
            }
          }
          else if (param->campo1.flag == 3)
          {
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0", convertIntString(param->campo1.endereco.constant), NULL);
            contadorLinhasCodigoObjeto++;
          }
          if (param->campo1.flag == 2)
          {
            removeParametro(Fila);
            if(flagRetornoDeFuncao){
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "out","$S30" , NULL, NULL);
              flagRetornoDeFuncao = 0;
            }else{
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "out",param->campo1.endereco.string , NULL, NULL);
            }
            contadorLinhasCodigoObjeto++;
          }
          else
          {
            removeParametro(Fila);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "out", "$S0", NULL, NULL);
            contadorLinhasCodigoObjeto++;
          }
        }
      }
    }

    if (strcmp(intermediario->campo1.endereco.string,"ret")==0)
    {
      posicaoDaFuncaoNaMemoria = st_lookup ( escopo, " " );
      if (intermediario->campo2.flag == 1)
      {
        if (intermediario->campo2.endereco.variable->vetor == 1 || intermediario->campo2.endereco.variable->vetor == 2)
        {
          if (intermediario->campo3.flag == 1)
          {
            if(intermediario->campo2.endereco.variable->vetor == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0" ,convertIntString(st_lookup(intermediario->campo2.endereco.variable->name, escopo)), NULL);
            if(intermediario->campo2.endereco.variable->vetor == 2)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(st_lookup(intermediario->campo2.endereco.variable->name, escopo)), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1" ,convertIntString(st_lookup(intermediario->campo3.endereco.variable->name, escopo)), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0" , "$S1", "$S0");
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S30", "$S0", " ");
            contadorLinhasCodigoObjeto+=4;
          }
          else if (intermediario->campo3.flag == 2)
          {
            if(intermediario->campo2.endereco.variable->vetor == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0" ,convertIntString(st_lookup(intermediario->campo2.endereco.variable->name, escopo)), NULL);
            if(intermediario->campo2.endereco.variable->vetor == 2)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(st_lookup(intermediario->campo2.endereco.variable->name, escopo)), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0" , intermediario->campo3.endereco.string, "$S0");
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S30", "$S0", NULL);
            contadorLinhasCodigoObjeto+=3;
          }
          else if (intermediario->campo3.flag == 3)
          {
            if(intermediario->campo2.endereco.variable->vetor == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0" ,convertIntString(st_lookup(intermediario->campo2.endereco.variable->name, escopo)), NULL);
            if(intermediario->campo2.endereco.variable->vetor == 2)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" ,convertIntString(st_lookup(intermediario->campo2.endereco.variable->name, escopo)), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$S0" , "$S0", convertIntString(intermediario->campo3.endereco.constant));
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", "$S30", "$S0", NULL);
            contadorLinhasCodigoObjeto+=3;
          }
        }
        else
        {
          listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S30", convertIntString(st_lookup(intermediario->campo2.endereco.variable->name, escopo)), NULL);
          contadorLinhasCodigoObjeto++;
        }
      }else if (intermediario->campo2.flag == 2){
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", "$S30", intermediario->campo2.endereco.string, NULL);
        contadorLinhasCodigoObjeto++;
      }
      else//retorno de constante
      {
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S30", convertIntString(intermediario->campo2.endereco.constant), NULL);
        contadorLinhasCodigoObjeto++;
      }
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load","$S31" ,convertIntString(posicaoDaFuncaoNaMemoria), NULL);
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "jr", "$S31", NULL, NULL);
      contadorLinhasCodigoObjeto+=2;
    }

    if (strcmp(intermediario->campo1.endereco.string,"asg")==0)
    {
      if(intermediario->campo2.flag == 2)
      {
        if(intermediario->campo3.endereco.variable->vetor == 1 || intermediario->campo3.endereco.variable->vetor == 2){
          if(intermediario->campo4.flag == 1){
            if(intermediario->campo3.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            if(intermediario->campo3.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1",convertIntString(intermediario->campo4.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", "$S1", "$S0");
            if(flagRetornoDeFuncao){
    					listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S30", "$S0", NULL);
    					flagRetornoDeFuncao = 0;
            }else{
            	listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", intermediario->campo2.endereco.string, "$S0", NULL);
            }
            contadorLinhasCodigoObjeto+=4;
          }else if(intermediario->campo4.flag == 2){
            if(intermediario->campo3.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            if(intermediario->campo3.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", intermediario->campo4.endereco.string, "$S0");
            if(flagRetornoDeFuncao){
    					listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S30", "$S0", NULL);
    					flagRetornoDeFuncao = 0;
            }else{
            	listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", intermediario->campo2.endereco.string, "$S0", NULL);
            }
            contadorLinhasCodigoObjeto+=3;
            }else if(intermediario->campo4.flag == 3){
              if(intermediario->campo3.endereco.variable->vetor == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              if(intermediario->campo3.endereco.variable->vetor == 2)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$S0", "$S0", convertIntString(intermediario->campo4.endereco.constant));
              if(flagRetornoDeFuncao){
    						listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S30", "$S0", NULL);
    						flagRetornoDeFuncao = 0;
              }else{
              	listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", intermediario->campo2.endereco.string, "$S0", NULL);
              }
                contadorLinhasCodigoObjeto+=3;
            }
        }else{
          if(flagRetornoDeFuncao){
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S30", convertIntString(intermediario->campo4.endereco.variable->memloc), NULL);
            flagRetornoDeFuncao = 0;
          }else{
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", intermediario->campo2.endereco.string, convertIntString(intermediario->campo4.endereco.variable->memloc), NULL);
          }
          contadorLinhasCodigoObjeto++;
        }
      }
      else if(intermediario->campo2.flag == 1)
      {
        if (intermediario->campo2.endereco.variable->vetor == 1 || intermediario->campo2.endereco.variable->vetor == 2)
        {
          if (intermediario->campo3.flag == 1)
          {
            if(intermediario->campo2.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
            if(intermediario->campo2.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", "$S1", "$S0");
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", intermediario->campo4.endereco.string, "$S0", NULL);
            contadorLinhasCodigoObjeto+=4;
          }
          else if (intermediario->campo3.flag == 2)
          {
            if(intermediario->campo2.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
            if(intermediario->campo2.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", intermediario->campo3.endereco.string, "$S0");
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", intermediario->campo4.endereco.string, "$S0", NULL);
            contadorLinhasCodigoObjeto+=3;
          }
          else if (intermediario->campo3.flag == 3)
          {
            if(intermediario->campo2.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
            if(intermediario->campo2.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$S0", "$S0", convertIntString(intermediario->campo3.endereco.constant));
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadr", intermediario->campo4.endereco.string, "$S0", NULL);
            contadorLinhasCodigoObjeto+=3;
          }
        }
        else
        {
          if(intermediario->campo3.endereco.variable->vetor == 1 || intermediario->campo3.endereco.variable->vetor == 2){ // X = vetor
            if(intermediario->campo4.flag == 1){
              if(intermediario->campo3.endereco.variable->vetor == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              if(intermediario->campo3.endereco.variable->vetor == 2)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1",convertIntString(intermediario->campo4.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", "$S1", "$S0");
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1", convertIntString(intermediario->campo2.endereco.variable->memloc),  NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S1", "$S0", " ");
              contadorLinhasCodigoObjeto+=5;
            }
            else if(intermediario->campo4.flag == 2)
            {
              if(intermediario->campo3.endereco.variable->vetor == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              if(intermediario->campo3.endereco.variable->vetor == 2)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", intermediario->campo4.endereco.string, "$S0");
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1", convertIntString(intermediario->campo2.endereco.variable->memloc),  NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S1", "$S0", NULL);
              contadorLinhasCodigoObjeto+=4;
            }
            else if(intermediario->campo4.flag == 3){
              if(intermediario->campo3.endereco.variable->vetor == 1)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              if(intermediario->campo3.endereco.variable->vetor == 2)
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$S0", "$S0", convertIntString(intermediario->campo4.endereco.constant));
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1", convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S1", "$S0", NULL);
              contadorLinhasCodigoObjeto+=4;
            }
          }
          else
          {
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0" , convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(intermediario->campo4.endereco.variable->memloc), NULL);
              contadorLinhasCodigoObjeto+=2;
          }
        }
      }
      else
      {
        if(intermediario->campo3.endereco.variable->vetor == 1 || intermediario->campo3.endereco.variable->vetor == 2){ //
          if(intermediario->campo4.flag == 1){
            if(intermediario->campo3.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            if(intermediario->campo3.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S1",convertIntString(intermediario->campo4.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", "$S1", "$S0");
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S1", convertIntString(intermediario->campo2.endereco.constant), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S1", "$S0", NULL);
            contadorLinhasCodigoObjeto+=5;
          }else if(intermediario->campo4.flag == 2){
            if(intermediario->campo3.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            if(intermediario->campo3.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$S0", intermediario->campo4.endereco.string, "$S0");
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S1", convertIntString(intermediario->campo2.endereco.constant), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S1", "$S0", NULL);
            contadorLinhasCodigoObjeto+=4;
          }else if(intermediario->campo4.flag == 3){
            if(intermediario->campo3.endereco.variable->vetor == 1)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            if(intermediario->campo3.endereco.variable->vetor == 2)
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", "$S0",convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$S0", "$S0", convertIntString(intermediario->campo4.endereco.constant));
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S1", convertIntString(intermediario->campo2.endereco.constant), NULL);
            listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storer", "$S1", "$S0", NULL);
            contadorLinhasCodigoObjeto+=4;
          }
        }
        else
        {
            if(strcmp(convertIntString(intermediario->campo2.endereco.constant),"18440")==0){
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storeP", "$S0", convertIntString(intermediario->campo4.endereco.variable->memloc), " ");
              contadorLinhasCodigoObjeto+=1;
            }
            else if(strcmp(convertIntString(intermediario->campo2.endereco.constant),"18441")==0){
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "storeC", "$S0", convertIntString(intermediario->campo4.endereco.variable->memloc), " ");
              contadorLinhasCodigoObjeto+=1;
            }
            else{
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$S0", convertIntString(intermediario->campo2.endereco.constant), NULL);
              listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "store", "$S0", convertIntString(intermediario->campo4.endereco.variable->memloc), NULL);
              contadorLinhasCodigoObjeto+=2;
            }
        }
      }
    }
    if (strcmp(intermediario->campo1.endereco.string,"soma")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "add");
    }
    if (strcmp(intermediario->campo1.endereco.string,"sub")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "sub");
    }

    if (strcmp(intermediario->campo1.endereco.string,"mul")==0)
    {
      verificaLoads(intermediario);
      if(intermediario->campo2.flag == 2 && intermediario->campo3.flag == 2){
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "beq", intermediario->campo2.endereco.string, "$zero", convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$Aux0", "$Aux0", intermediario->campo3.endereco.string);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "subi", intermediario->campo2.endereco.string, intermediario->campo2.endereco.string, "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }else if(intermediario->campo2.flag == 2){
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "beq", intermediario->campo2.endereco.string, "$zero", convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$Aux0", "$Aux0", "$S0");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "subi", intermediario->campo2.endereco.string, intermediario->campo2.endereco.string, "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }else if(intermediario->campo3.flag == 2){
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "beq", "$S0", "$zero", convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$Aux0", "$Aux0", intermediario->campo3.endereco.string);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "subi", "$S0", "$S0", "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }else{
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "beq", "$S0", "$zero", convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "add", "$Aux0", "$Aux0", "$S1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "subi", "$S0", "$S0", "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }
      contadorLinhasCodigoObjeto+=6;
    }

    if (strcmp(intermediario->campo1.endereco.string,"div")==0)
    {
      verificaLoads(intermediario);
      if(intermediario->campo2.flag == 2 && intermediario->campo3.flag == 2){
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "blt", intermediario->campo2.endereco.string, intermediario->campo3.endereco.string, convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "sub", intermediario->campo2.endereco.string, intermediario->campo2.endereco.string, intermediario->campo3.endereco.string);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$Aux0", "$Aux0", "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }else if(intermediario->campo2.flag == 2){
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "blt", intermediario->campo2.endereco.string, "$S0", convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "sub", intermediario->campo2.endereco.string, intermediario->campo2.endereco.string, "$S0");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$Aux0", "$Aux0", "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }else if(intermediario->campo3.flag == 2){
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "blt", "$S0", intermediario->campo3.endereco.string, convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "sub", "$S0", "$S0", intermediario->campo3.endereco.string);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$Aux0", "$Aux0", "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }else{
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", "$Aux0", "0", NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "blt", "$S0", "$S1", convertIntString(contador+5));
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "sub", "$S0", "$S0", "$S1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "addi", "$Aux0", "$Aux0", "1");
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "j", convertIntString(contador-2), NULL, NULL);
        listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "move", intermediario->campo4.endereco.string, "$Aux0", NULL);
      }
      contadorLinhasCodigoObjeto+=6;
    }

    if (strcmp(intermediario->campo1.endereco.string,"menor")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "slt");
    }

    if (strcmp(intermediario->campo1.endereco.string,"igual")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "set");
    }

    if (strcmp(intermediario->campo1.endereco.string,"maior")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "sgt");
    }

    if (strcmp(intermediario->campo1.endereco.string,"dif")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "sdt");
    }

    if (strcmp(intermediario->campo1.endereco.string,"menorigual")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "slet");

    }

    if (strcmp(intermediario->campo1.endereco.string,"maiorigual")==0)
    {
      verificaLoads(intermediario);
      verificaTemps(intermediario, "sget");

    }

    if (strcmp(intermediario->campo1.endereco.string,"halt")==0)
    {
	     listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "hlt", NULL, NULL, NULL);
       contadorLinhasCodigoObjeto++;
    }

    intermediario = intermediario->prox;
  }
  fprintf(listing, "\n\nGeracao do Codigo Assembly:\n");

  ListaAssembly *FinalList;
  FinalList = NULL;
  while (listaCodigoAssembly != NULL)
  {
      FinalList = insereListaAssembly(FinalList, listaCodigoAssembly->campo1, listaCodigoAssembly->campo2, listaCodigoAssembly->campo3, listaCodigoAssembly->campo4);
      listaCodigoAssembly = listaCodigoAssembly->prox;
  }
  contadorLinhasCodigoObjeto = 0;
  imprimeListaAssembly(FinalList);
  geraListaInstrucoes(FinalList);


}

void verificaLoads(ListaIntermediaria * intermediario)
{
  SR = 1;
  if (intermediario->campo2.flag == 1)//(...,x, ...,...)
  {
    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", newSavedRegister(), convertIntString(intermediario->campo2.endereco.variable->memloc), NULL);
    contadorLinhasCodigoObjeto++;
  }
  else if (intermediario->campo2.flag == 3)//(..., 3, ..., ...)
  {
    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", newSavedRegister(), convertIntString(intermediario->campo2.endereco.constant), NULL);
    contadorLinhasCodigoObjeto++;
  }
  if (intermediario->campo3.flag == 1)//(..., ..., x, ...)
  {
    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "load", newSavedRegister(), convertIntString(intermediario->campo3.endereco.variable->memloc), NULL);
    contadorLinhasCodigoObjeto++;
  }
  else if (intermediario->campo3.flag == 3)//(..., ..., 3, ...)
  {
    listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, "loadi", newSavedRegister(), convertIntString(intermediario->campo3.endereco.constant), NULL);
    contadorLinhasCodigoObjeto++;
  }
}

void verificaTemps(ListaIntermediaria * intermediario, char * equacao)//verifica qndo tem variaveis
{
    if(intermediario->campo2.flag == 2 && intermediario->campo3.flag == 2){//(..., t10, ..., ...)
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, equacao, intermediario->campo4.endereco.string, intermediario->campo2.endereco.string, intermediario->campo3.endereco.string);
      contadorLinhasCodigoObjeto++;
    }else if (intermediario->campo2.flag == 2){
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, equacao, intermediario->campo4.endereco.string, intermediario->campo2.endereco.string, "$S0");
      contadorLinhasCodigoObjeto++;
    }else if (intermediario->campo3.flag == 2){
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, equacao, intermediario->campo4.endereco.string, "$S0", intermediario->campo3.endereco.string);
      contadorLinhasCodigoObjeto++;
    }else{
      listaCodigoAssembly = insereListaAssembly(listaCodigoAssembly, equacao, intermediario->campo4.endereco.string, "$S0", "$S1");
      contadorLinhasCodigoObjeto++;
    }
}


void verificaLinhaLoads(ListaIntermediaria * intermediario)
{
  if (intermediario->campo2.flag == 1)
  {
    contadorLinhasCodigoObjeto++;
  }
  else if (intermediario->campo2.flag == 3)
  {
    contadorLinhasCodigoObjeto++;
  }
  if (intermediario->campo3.flag == 1)
  {
    contadorLinhasCodigoObjeto++;
  }
  else if (intermediario->campo3.flag == 3)
  {
    contadorLinhasCodigoObjeto++;
  }
}

void verificaLinhasTemps(ListaIntermediaria * intermediario, char * equacao)
{
    if(intermediario->campo2.flag == 2 && intermediario->campo3.flag == 2){
      contadorLinhasCodigoObjeto++;
    }else if (intermediario->campo2.flag == 2){
      contadorLinhasCodigoObjeto++;
    }else if (intermediario->campo3.flag == 2){
      contadorLinhasCodigoObjeto++;
    }else{
      contadorLinhasCodigoObjeto++;
    }
}

void verificaLinhas (ListaIntermediaria * intermediario)
{
  int i, primeiraVez = 0;
  filaParametro* Fila;
  Fila = malloc(sizeof(filaParametro));
  Fila->fim = NULL;
  Fila->inicio = NULL;
  char * nomeFuncao;
  char * escopo =  " ";
  char * tipoRetornoFuncao;
  int memLoc;

  parametro *param;
  param = malloc(sizeof(parametro));
  contadorLinhasCodigoObjeto = 0;
  listaLabel = NULL;

  while (intermediario != NULL)
  {
    if (strcmp(intermediario->campo1.endereco.string,"if")==0)
    {
      contadorLinhasCodigoObjeto++;
    }

    if (strcmp(intermediario->campo1.endereco.string,"lab")==0)
    {
      tipoRetornoFuncao = st_lookupCallTypeData ( escopo, " " );
      if (intermediario->campo2.flag == 1 && !strcmp(tipoRetornoFuncao,"void") && strcmp("main", intermediario->campo2.endereco.string) && primeiraVez)
      {
        contadorLinhasCodigoObjeto+=2;
      }
      if (intermediario->campo2.flag == 1)
      {
        listaLabel = insereLabel(listaLabel, intermediario->campo2.endereco.variable->name, contadorLinhasCodigoObjeto);
        primeiraVez = 1;
        escopo = intermediario->campo2.endereco.variable->name;
      }else if (intermediario->campo2.flag == 2)
      {
        listaLabel = insereLabel(listaLabel, intermediario->campo2.endereco.string, contadorLinhasCodigoObjeto);
      }
    }

    if (strcmp(intermediario->campo1.endereco.string,"goto")==0)
    {
      contadorLinhasCodigoObjeto++;
    }

    if (strcmp(intermediario->campo1.endereco.string,"par")==0)
    {
      param->campo1 = intermediario->campo2;
      param->campo2 = intermediario->campo3;
      insereParametro(Fila, param);
    }

    if (strcmp(intermediario->campo1.endereco.string,"call")==0)
    {
      nomeFuncao = intermediario->campo2.endereco.string;

      if (!strcmp(nomeFuncao, "interrupt"))
      {
        contadorLinhasCodigoObjeto+=1;
      }

      else if (!strcmp(nomeFuncao, "hdreg"))
      {
          contadorLinhasCodigoObjeto+=3;
      }

      else if (!strcmp(nomeFuncao, "reghd"))
      {
          contadorLinhasCodigoObjeto+=3;
      }
      else if (!strcmp(nomeFuncao, "salvaPC"))
      {
        contadorLinhasCodigoObjeto+=1;
      }
      else if (!strcmp(nomeFuncao, "jumpProcesso"))
      {
        contadorLinhasCodigoObjeto+=3;
      }

      else if (!strcmp(nomeFuncao, "hdmd"))
      {
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
          param = primeiroFilaParam(Fila);
          if(i == 2)
          {
            if (param->campo1.endereco.variable->vetor == 1 || param->campo1.endereco.variable->vetor == 2)
              contadorLinhasCodigoObjeto+=4;
            else
              contadorLinhasCodigoObjeto+=2;
          }
          removeParametro(Fila);
        }
        contadorLinhasCodigoObjeto+=2;
      }

      else if (!strcmp(nomeFuncao, "hdmi"))
      {
        contadorLinhasCodigoObjeto+=4;
      }
      else
      if (strcmp(nomeFuncao, "input") && strcmp(nomeFuncao, "output"))
      {
        if (intermediario->campo3.flag == 3)
        for (i = 0; i < intermediario->campo3.endereco.constant; i++)
        {
            param = primeiroFilaParam(Fila);
            nomeFuncao = intermediario->campo2.endereco.string;
            memLoc = st_lookup (nomeFuncao, " ") + (i+1);

            if(param->campo1.flag == 2){
              contadorLinhasCodigoObjeto++;
            }else if(param->campo1.flag == 1){
                if(param->campo1.endereco.variable->vetor == 1 || param->campo1.endereco.variable->vetor == 2){
                  if(param->campo2.flag == 1){
                    contadorLinhasCodigoObjeto+=5;
                  }else if(param->campo2.flag == 3){
                    contadorLinhasCodigoObjeto+=4;
                  }else if(param->campo2.flag == 2){
                    if(strcmp(param->campo2.endereco.string, "_")){
                      contadorLinhasCodigoObjeto+=4;
                    }else{
                      contadorLinhasCodigoObjeto+=2;
                    }
                  }
                }
                else{
                  contadorLinhasCodigoObjeto+=2;
                }
            }else{
              contadorLinhasCodigoObjeto+=2;
            }

            removeParametro(Fila);
        }

        if (strcmp(nomeFuncao, "main"))
        {
          contadorLinhasCodigoObjeto+=2;
        }
        contadorLinhasCodigoObjeto++;
      }
      else
      {
        if (!strcmp(nomeFuncao, "input"))
        {
          contadorLinhasCodigoObjeto++;
        }
        else
        if (!strcmp(nomeFuncao, "output"))
        {
          param = primeiroFilaParam(Fila);

          if(param->campo1.flag == 1){ //quando é vetor
            if (param->campo1.endereco.variable->vetor == 1 || param->campo1.endereco.variable->vetor == 2)
            {
              if (param->campo2.flag == 1)
              {
                contadorLinhasCodigoObjeto+=4;
              }
              else if (param->campo2.flag == 2)
              {
                contadorLinhasCodigoObjeto+=3;
              }
              else
              {
                contadorLinhasCodigoObjeto+=3;
              }
            }
            else
            {
              contadorLinhasCodigoObjeto++;
            }
          }
          else if (param->campo1.flag == 3)
          {
            contadorLinhasCodigoObjeto++;
          }
          if (param->campo1.flag == 2)
          {
            removeParametro(Fila);
            contadorLinhasCodigoObjeto++;
          }
          else
          {
            removeParametro(Fila);
            contadorLinhasCodigoObjeto++;
          }
        }
      }
    }

    if (strcmp(intermediario->campo1.endereco.string,"ret")==0)
    {
      if (intermediario->campo2.flag == 1)
      {
        if (intermediario->campo2.endereco.variable->vetor == 1 || intermediario->campo2.endereco.variable->vetor == 2)
        {
          if (intermediario->campo3.flag == 1)
          {
            contadorLinhasCodigoObjeto+=4;
          }
          else if (intermediario->campo3.flag == 2)
          {
            contadorLinhasCodigoObjeto+=3;
          }
          else if (intermediario->campo3.flag == 3)
          {
            contadorLinhasCodigoObjeto+=3;
          }
        }
        else
        {
          contadorLinhasCodigoObjeto++;
        }
      }
      else if (intermediario->campo2.flag == 2){
        contadorLinhasCodigoObjeto++;
      }
      else
      {
        contadorLinhasCodigoObjeto++;
      }
      contadorLinhasCodigoObjeto+=2;
    }

    if (strcmp(intermediario->campo1.endereco.string,"asg")==0)
    {
      if(intermediario->campo2.flag == 2) // X = temporario
      {
            if(intermediario->campo3.endereco.variable->vetor == 1 || intermediario->campo3.endereco.variable->vetor == 2){ // X = vetor
                if(intermediario->campo4.flag == 1){
                  contadorLinhasCodigoObjeto+=4;
                }else if(intermediario->campo4.flag == 2){
                  contadorLinhasCodigoObjeto+=3;
                }else if(intermediario->campo4.flag == 3){
                  contadorLinhasCodigoObjeto+=3;
                }
            }else{ // X = variavel
              contadorLinhasCodigoObjeto++;
            }
      }
      else if(intermediario->campo2.flag == 1) // X = Y
      {
          if (intermediario->campo2.endereco.variable->vetor == 1 || intermediario->campo2.endereco.variable->vetor == 2) // Y = vetor
          {
              if (intermediario->campo3.flag == 1)
              {
                  contadorLinhasCodigoObjeto+=4;
              }
              else if (intermediario->campo3.flag == 2)
              {
                  contadorLinhasCodigoObjeto+=3;
              }
              else if (intermediario->campo3.flag == 3)
              {
                  contadorLinhasCodigoObjeto+=3;
              }
          }
          else //x = y
          {
            if(intermediario->campo3.endereco.variable->vetor == 1 || intermediario->campo3.endereco.variable->vetor == 2){ // X = vetor
                if(intermediario->campo4.flag == 1){
                  contadorLinhasCodigoObjeto+=5;
                }else if(intermediario->campo4.flag == 2){
                  contadorLinhasCodigoObjeto+=4;
                }else if(intermediario->campo4.flag == 3){
                  contadorLinhasCodigoObjeto+=4;
                }
              }else{
                contadorLinhasCodigoObjeto+=2;
              }
          }
      }
      else
      {
        if(intermediario->campo3.endereco.variable->vetor == 1 || intermediario->campo3.endereco.variable->vetor == 2){ // X = vetor
            if(intermediario->campo4.flag == 1){
              contadorLinhasCodigoObjeto+=5;
            }else if(intermediario->campo4.flag == 2){
              contadorLinhasCodigoObjeto+=4;
            }else if(intermediario->campo4.flag == 3){
              contadorLinhasCodigoObjeto+=4;
            }
        }else{
          if(strcmp(convertIntString(intermediario->campo2.endereco.constant),"18440")==0){
            contadorLinhasCodigoObjeto+=1;
          }
          else if(strcmp(convertIntString(intermediario->campo2.endereco.constant),"18441")==0){
            contadorLinhasCodigoObjeto+=1;
          }
          else{
          contadorLinhasCodigoObjeto+=2;
          }
        }
      }

    }
    if (strcmp(intermediario->campo1.endereco.string,"soma")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "add");
    }
    if (strcmp(intermediario->campo1.endereco.string,"sub")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "sub");
    }

    if (strcmp(intermediario->campo1.endereco.string,"mul")==0)
    {
      verificaLinhaLoads(intermediario);
      contadorLinhasCodigoObjeto+=6;
    }

    if (strcmp(intermediario->campo1.endereco.string,"div")==0)
    {
      verificaLinhaLoads(intermediario);
      contadorLinhasCodigoObjeto+=6;
    }

    if (strcmp(intermediario->campo1.endereco.string,"menor")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "slt");
    }

    if (strcmp(intermediario->campo1.endereco.string,"igual")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "set");
    }

    if (strcmp(intermediario->campo1.endereco.string,"maior")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "sgt");
    }

    if (strcmp(intermediario->campo1.endereco.string,"dif")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "sdt");
    }

    if (strcmp(intermediario->campo1.endereco.string,"menorigual")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "menorigual");
    }

    if (strcmp(intermediario->campo1.endereco.string,"maiorigual")==0)
    {
      verificaLinhaLoads(intermediario);
      verificaLinhasTemps(intermediario, "maiorigual");
    }

    if (strcmp(intermediario->campo1.endereco.string,"halt")==0)
    {
       contadorLinhasCodigoObjeto++;
    }

    intermediario = intermediario->prox;
  }
  contadorLinhasCodigoObjeto = 0;

  }
