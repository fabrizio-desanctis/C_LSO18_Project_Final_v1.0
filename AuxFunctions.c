#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "AuxFunctions.h"

int IsDigit(char c){
	return (c>='0' && c<='9');
}

/*Questa funzione previene che vengano inseriti dall'utente valori non interi.*/
int IsUInteger(char *s){
	int res=1, i = 0;

	while( s[i] != '\0' && IsDigit(s[i])) i++;
	
	if(i==0 || s[i]!='\0') res = 0;

	return res; 
}


/*Questa funzione si occupa di stampare su standard output un messaggio relativo ad un errore.*/
void PrintErrClose(char *s){
	write(2, s , strlen(s));
	exit(1);
}



/*Questa funzione si occupa di inizializzare un nuovo nodo della lista. Prende in input le informazioni necessarie e restituisce una nuova
occorrenza della lista.*/
List initNodeList(Info i) {
    List L = (List)malloc(sizeof(struct TList));
    L->i = i;
    L->next = NULL;
    return L;
}



/*Questa funzione si occupa di appendere un nodo direttamente alla fine della lista;
Ritorna la nuova lista. */
List appendNodeList(List L, Info i) {
    if (L != NULL) {
        L->next = appendNodeList(L->next,i);
    } else {
        L = initNodeList(i);
    }
    return L;
}


/*Questa procedura libera tutta la memoria occupata dalla lista*/
void freeList(List L) {
    if (L != NULL) {
        freeList(L->next);
        free(L);
    }
}
