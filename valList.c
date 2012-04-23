/*
Daniel Padin Pazos
daniel.padin@udc.es
35484768t
grupo 3.3.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "valList.h"

valListNode *createValList()
{
	return NULL;
}

valListNode *insertValList(valListNode *l, char *variable, char *valor)
{
	valListNode *tmp;

	tmp = malloc(sizeof(valListNode));
	tmp->variable = variable;
	tmp->valor = valor;
	tmp->sig = l;
	l = tmp;

	return l;
}
 
void updateValList(valListNode *n, char *valor)
 {
 	if (n)
	{
		free(n->valor);
		n->valor = valor;
	} 
	else 
	{
		fprintf(stderr, "Error: actualizarLista: valListNode no valido\n");
	}
 }

valListNode *searchValList(valListNode *l, char *variable)
{
	valListNode *tmp;
	tmp = l;

	while (tmp)
	{
		if (!strcmp(variable, tmp->variable))
			return tmp;
		tmp = tmp->sig;
	}
	return NULL;
}

void cleanValList(valListNode *l)
{
	valListNode *tmp;

	while (l)
	{
		tmp = l;
		l = l->sig;
		free(tmp->variable);
		free(tmp->valor);
		free(tmp);
	}
}

void printValList(valListNode *l)
{
	valListNode *tmp;
	tmp = l;

	while (tmp)
	{
		printf("%s=%s\n",tmp->variable, tmp->valor);
		tmp = tmp->sig;
	}
}
