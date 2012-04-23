/*
Daniel Padin Pazos
daniel.padin@udc.es
35484768t
grupo 3.3.1
*/

struct tprocListNode {
    char *path;
    int pidHijo;
    time_t creation;
    int priority;
    int status;
    int returned;
    struct tprocListNode *sig;
};
typedef struct tprocListNode procListNode;
procListNode *createProcList(); 
procListNode *addProcList(procListNode *l, char *path, int pidHijo,int priority);
procListNode *delProcList(procListNode *l, int pidHijo);
procListNode *cleanProcList(procListNode *l); 
void printProcList(procListNode *l);
