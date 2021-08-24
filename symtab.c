#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

BucketList st_lookupBucket ( char * name, char * scope );

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the hash table */
static BucketList hashTable[SIZE];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
int tipoFuncao (char * name)
{
  int h = hash(name);
  int kasa;
    BucketList l =  hashTable[h];
    while((l != NULL) && !((strcmp(name,l->name)==0)))
    l = l->next;
    if (l == NULL)
      return 2;
    else if (strcmp(l->typedata,"int")==0)
      return 1;
    else
      return 0;

}

void st_insert( char * name, int lineno, int loc, int vector ,char * scope, char * typeID, char * typedata )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while (  (l != NULL) && !(  (strcmp(name,l->name)==0) && ((strcmp(scope,l->scope)==0) || (strcmp(l->scope," ")==0))    ) )
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->scope = scope;
    l->vetor = vector;
    l->typeID = typeID;
    l->typedata = typedata;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = hashTable[h];
    hashTable[h] = l; }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name, char * scope )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while   (  (l != NULL) && !(  (strcmp(name,l->name)==0) && ((strcmp(scope,l->scope)==0) || (strcmp(l->scope," ")==0))    ) )
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

int st_lookupLinhaRetorno ( char * name, char * scope )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while   (  (l != NULL) && !(  (strcmp(name,l->name)==0) && ((strcmp(scope,l->scope)==0) || (strcmp(l->scope," ")==0))    ) )
    l = l->next;
  if (l == NULL) return -1;
  else return l->linha_de_retorno;
}

char* st_lookupCallTemp ( char * name, char * scope )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while   (  (l != NULL) && !(  (strcmp(name,l->name)==0) && ((strcmp(scope,l->scope)==0) || (strcmp(l->scope," ")==0))    ) )
    l = l->next;
  if (l == NULL) return " ";
  else return l->tempretorno;
}

char* st_lookupCallTypeData ( char * name, char * scope )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while   (  (l != NULL) && !(  (strcmp(name,l->name)==0) && ((strcmp(scope,l->scope)==0) || (strcmp(l->scope," ")==0))    ) )
    l = l->next;
  if (l == NULL) return " ";
  else return l->typedata;
}

BucketList st_lookupBucket ( char * name, char * scope )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while   (  (l != NULL) && !(  (strcmp(name,l->name)==0) && ((strcmp(scope,l->scope)==0) || (strcmp(l->scope," ")==0))    ) )
    l = l->next;
  if (l == NULL) return NULL;
  else return l;
}

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing,"Variable Name  MemLoc  Scope     TypeID   Type   Line numbers\n");
  fprintf(listing,"-------------  ------  -----     ------   -----  -----------\n");
  for (i=0;i<SIZE;++i)
  { if (hashTable[i] != NULL)
    { BucketList l = hashTable[i];
      while (l != NULL)
      {if ((strcmp(l->name,"input")!=0) && (strcmp(l->name,"output")!=0))
    {
        LineList t = l->lines;
        fprintf(listing,"%-14s ",l->name);
        fprintf(listing,"%-6d  ",l->memloc);
        fprintf(listing,"%-9s ",l->scope);
        if (l->vetor==1)
          fprintf(listing,"%-8s ","vector");
        else
        if (l->vetor==2)
          fprintf(listing,"%-8s ","p-vec");
        else
          fprintf(listing,"%-8s ",l->typeID);
        fprintf(listing,"%-6s ",l->typedata);
        while (t != NULL)
        { fprintf(listing,"%2d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        }
        l = l->next;
      }
    }
  }
} /* printSymTab */
