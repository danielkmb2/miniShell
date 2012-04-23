/*
Daniel Padin Pazos
daniel.padin@udc.es
35484768t
grupo 3.3.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "procList.h"

procListNode *createProcList()
{
    return NULL;
}
procListNode *addProcList(procListNode *l, char *path, int pidHijo,int priority)
{
    procListNode *tmp;

    tmp = malloc(sizeof(procListNode));
    tmp->path = path;
    tmp->pidHijo = pidHijo;
    tmp->returned = INT_MAX;
    tmp->creation = time(0);
    tmp->priority = priority;
    tmp->status = 4;/*comienza en ejecución*/ 
    tmp->sig = l;
    l = tmp;

    return l;
}

procListNode *delProcList(procListNode *l, int pid)
{
    procListNode* tmp;
    procListNode* toDel;

    tmp=l;
    if(tmp==NULL)
    {
        printf("error en la lista (1)\n");
        return l;
    } 
    else

        if(tmp->pidHijo==pid)
        {
            l=tmp->sig;
            free(tmp->path);
            free(tmp);
            return l;/*la lista queda vacía*/
        }

        else
            while(1)
            {
                if(tmp->sig->pidHijo==pid)
                {  
                    toDel=tmp->sig;
                    tmp->sig=toDel->sig;
                    free(toDel->path);
                    free(toDel);
                    return l;
                }
                else
                    tmp=tmp->sig;
            }
    return l;
}

procListNode* cleanProcList(procListNode *l)
{
    procListNode *tmp;

    while (l)
    {
        tmp = l;
        free(tmp->path);
        l = l->sig;
        free (tmp);
    }
    return l;
}

void printProcList(procListNode *l)
{
    procListNode *tmp;
    tmp = l;

    while (tmp)
    {
        printf("%s :%d\n",tmp->path,tmp->pidHijo);
        tmp = tmp->sig;
    }
}
