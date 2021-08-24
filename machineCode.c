#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include "analyze.h"

int posicaoInstrucao = 100;
int formatoInstrucao;

typedef struct listaAssembly
{
    char *opcode, *campo2, *campo3, *campo4;
    struct listaAssembly* prox;
} ListaAssembly;

char * concatena(char* a0, char a1, char a2)
{
    char *str=(char*)malloc(sizeof(char)*90);
    if (a2!= ' ')
    {
        sprintf(str, "%s%c%c", a0, a1, a2);
    }
    else
    {
      sprintf(str, "%s%c", a0, a1);
    }
    return str;
}

char * EncontraRegistrador(char *Cifrao)
{
    if(!strcmp(Cifrao, "$zero"))
    {
      Cifrao = "5'd0";
    }
    else if(!strcmp(Cifrao, "$S0"))
    {
      Cifrao = "5'd1";
    }
    else if(!strcmp(Cifrao, "$S1"))
    {
      Cifrao = "5'd2";
    }
    else if(!strcmp(Cifrao, "$S2"))
    {
      Cifrao = "5'd3";
    }
    else if(!strcmp(Cifrao, "$Aux0"))
    { //usado para instrucoes MUL e DIV
      Cifrao = "5'd4";
    }
    else if(!strcmp(Cifrao, "$SR4"))
    {
      Cifrao = "5'd5";
    }
    else if(!strcmp(Cifrao, "$SR5"))
    {
      Cifrao = "5'd6";
    }
    else if(!strcmp(Cifrao, "$S30")){
      Cifrao = "5'd30";
    }
    else if(!strcmp(Cifrao, "$S31"))
    {
      Cifrao = "5'd31";
    }
    else if(Cifrao[0] == 't')
    {
      Cifrao = concatena("5'd", Cifrao[1], Cifrao[2]);
    }
    return Cifrao;
}


char * novaInstrucao()
{
    char *str=(char*)malloc(sizeof(char)*15);
    sprintf(str, "%s%d%s", "hd[", posicaoInstrucao,"] <= {");
    posicaoInstrucao++;
    return str;
}

void insereInstrucao(ListaAssembly * lista, int formatoInstrucao, char* opcode)
{
    char *instrucao;
    char *campo2;
    char *campo3;
    char *campo4;
    if(formatoInstrucao == 1)
    {
      campo2 = EncontraRegistrador(lista->campo2);
      campo3 = EncontraRegistrador(lista->campo3);
      campo4 = EncontraRegistrador(lista->campo4);
      fprintf(listing, "%s %s, %s, %s, %s, 11'd0 };\n", novaInstrucao(), opcode, campo2, campo3, campo4);
    }
    else if(formatoInstrucao == 2)
    {
      campo2 = EncontraRegistrador(lista->campo2);
      campo3 = EncontraRegistrador(lista->campo3);
      campo4 = lista->campo4;
      fprintf(listing, "%s %s, %s, %s, 16'd%s };\n", novaInstrucao(), opcode, campo2, campo3, campo4);
    }
    else if(formatoInstrucao == 3)
    {
      fprintf(listing, "%s %s, 26'd0 };\n", novaInstrucao(), opcode);
    }
    else if(formatoInstrucao == 4)
    {
      campo2 = EncontraRegistrador(lista->campo2);
      campo3 = EncontraRegistrador(lista->campo3);
      fprintf(listing, "%s %s, %s, %s, 16'd0 };\n", novaInstrucao(), opcode, campo2, campo3);
    }
    else if(formatoInstrucao == 5)
    {
      campo2 = lista->campo2;
      fprintf(listing, "%s %s, 16'd%s, 10'd0 };\n", novaInstrucao(), opcode, campo2);
    }
    else if(formatoInstrucao == 6)
    {
      campo2 = EncontraRegistrador(lista->campo2);
      fprintf(listing, "%s %s, %s, 21'd0 };\n", novaInstrucao(), opcode, campo2);
    }
    else if(formatoInstrucao == 7)
    {
      campo2 = EncontraRegistrador(lista->campo2);
      campo3 = lista->campo3;
      fprintf(listing, "%s %s, %s, 16'd%s, 5'd0 };\n", novaInstrucao(), opcode, campo2, campo3);
    }
}

void geraListaInstrucoes(ListaAssembly * lista)
{

  fprintf(listing,"\n\nGeracao do Codigo de Maquina:\n\n");

  while (lista != NULL)
  {
    if (strcmp(lista->opcode,"add")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd0");
    }
    if (strcmp(lista->opcode,"andi")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd12");
    }
    if (strcmp(lista->opcode,"sub")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd1");
    }
    if (strcmp(lista->opcode,"subi")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd3");
    }
    if (strcmp(lista->opcode,"and")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd4");
    }
    if (strcmp(lista->opcode,"addi")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd2");
    }
    if (strcmp(lista->opcode,"or")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd5");
    }
    if (strcmp(lista->opcode,"ori")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd13");
    }
    if (strcmp(lista->opcode,"slt")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd6");
    }
    if (strcmp(lista->opcode,"slti")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd14");
    }
    if (strcmp(lista->opcode,"sget")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd9");
    }
    if (strcmp(lista->opcode,"slet")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd7");
    }
    if (strcmp(lista->opcode,"load")==0)
    {
      formatoInstrucao = 7;
      insereInstrucao(lista, formatoInstrucao, "6'd16");
    }
    if (strcmp(lista->opcode,"loadi")==0)
    {
      formatoInstrucao = 7;
      insereInstrucao(lista, formatoInstrucao, "6'd18");
    }
    if (strcmp(lista->opcode,"store")==0)
    {
      formatoInstrucao = 7;
      insereInstrucao(lista, formatoInstrucao, "6'd17");
    }
    if (strcmp(lista->opcode,"move")==0)
    {
      formatoInstrucao = 4;
      insereInstrucao(lista, formatoInstrucao, "6'd21");
    }
    if (strcmp(lista->opcode,"beq")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd22");
    }
    if (strcmp(lista->opcode,"bne")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd23");
    }
    if (strcmp(lista->opcode,"j")==0)
    {
      formatoInstrucao = 5;
      insereInstrucao(lista, formatoInstrucao, "6'd26");
    }
    if (strcmp(lista->opcode,"jr")==0)
    {
      formatoInstrucao = 6;
      insereInstrucao(lista, formatoInstrucao, "6'd27");
    }
    if (strcmp(lista->opcode,"nop")==0)
    {
      formatoInstrucao = 3;
      insereInstrucao(lista, formatoInstrucao, "6'd28");
    }
    if (strcmp(lista->opcode,"hlt")==0)
    {
      formatoInstrucao = 3;
      insereInstrucao(lista, formatoInstrucao, "6'd29");
    }
    if (strcmp(lista->opcode,"in")==0)
    {
      formatoInstrucao = 6;
      insereInstrucao(lista, formatoInstrucao, "6'd30");
    }
    if (strcmp(lista->opcode,"out")==0)
    {
      formatoInstrucao = 6;
      insereInstrucao(lista, formatoInstrucao, "6'd31");
    }
    if (strcmp(lista->opcode,"bgt")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd24");
    }
    if (strcmp(lista->opcode,"blt")==0)
    {
      formatoInstrucao = 2;
      insereInstrucao(lista, formatoInstrucao, "6'd25");
    }
    if (strcmp(lista->opcode,"loadr")==0)
    {
      formatoInstrucao = 4;
      insereInstrucao(lista, formatoInstrucao, "6'd19");
    }
    if (strcmp(lista->opcode,"storer")==0)
    {
      formatoInstrucao = 4;
      insereInstrucao(lista, formatoInstrucao, "6'd20");
    }
    if (strcmp(lista->opcode,"not")==0)
    {
      formatoInstrucao = 4;
      insereInstrucao(lista, formatoInstrucao, "6'd15");
    }
    if (strcmp(lista->opcode,"sgt")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd8");
    }
    if (strcmp(lista->opcode,"set")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd10");
    }
    if (strcmp(lista->opcode,"sdt")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd11");
    }
    if (strcmp(lista->opcode,"hdmi")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd32");
    }
    if (strcmp(lista->opcode,"hdmd")==0)
    {
      formatoInstrucao = 1;
      insereInstrucao(lista, formatoInstrucao, "6'd33");
    }
    if (strcmp(lista->opcode,"jProc")==0)
    {
      formatoInstrucao = 4;
      insereInstrucao(lista, formatoInstrucao, "6'd34");
    }

    if (strcmp(lista->opcode,"storePC")==0)
    {
      formatoInstrucao = 7;
      insereInstrucao(lista, formatoInstrucao, "6'd36");
    }
    if (strcmp(lista->opcode,"hdreg")==0)
    {
      formatoInstrucao = 4;
      insereInstrucao(lista, formatoInstrucao, "6'd37");
    }
    if (strcmp(lista->opcode,"reghd")==0)
    {
      formatoInstrucao = 4;
      insereInstrucao(lista, formatoInstrucao, "6'd38");
    }
    if (strcmp(lista->opcode,"interrupt")==0)
    {
      formatoInstrucao = 3;
      insereInstrucao(lista, formatoInstrucao, "6'd35");
    }

    lista = lista->prox;
  }
  fprintf(listing,"\n");
}
