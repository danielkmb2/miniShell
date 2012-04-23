/*
Daniel Padin Pazos
daniel.padin@udc.es
35484768t
grupo 3.3.1
*/

struct tvalListNode {
		char *variable;
		char *valor;
		struct tvalListNode *sig;
};
typedef struct tvalListNode valListNode;
valListNode *createValList(); /* Inicializa a NULL*/
valListNode *insertValList(valListNode *l, char *variable, char *valor);
void updateValList(valListNode *n, char *valor);
valListNode *searchValList(valListNode *l, char *variable); /* Se non atopa o elemento
											   devolve NULL*/
void cleanValList(valListNode *l); /* Libera a memoria*/
void printValList(valListNode *l);
