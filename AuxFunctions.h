#include <stdint.h>
#include <stdio.h>

/* Errori */
#define ERR_NUM "Numero di parametri non sufficiente.\n"
#define ERR_PORT "Il numero di porta deve essere un intero non negativo.\n"
#define ERR_IP "IP non corretto.\n"
#define ERR_SOCKET "Errore creazione Socket.\n"
#define ERR_BIND "Errore Bind.\n"
#define ERR_LISTEN "Errore Listen.\n"
#define ERR_CONNECT "Errore Connect.\n"
#define ERR_FILE "Il file scelto non esiste.\n"
#define ERR_READ "Errore lettura da file.\n"
#define ERR_OPERATION "Errore lettura operazione.\n"
#define CLEARSCREEN system("clear")
#define CONNECT_OK "Connessione OK.\n\n"
#define dim 5

void PrintErrClose(char *s);	//Scrive s su standard output e chiude il programma con exit(1).
int IsUInteger(char *s);	//ritorna 1 se la stringa s e' un intero senza segno, 0 altrimenti.
int IsDigit(char c);


/* Struttura Info*/

struct TInfo {

   char operation[4];
   uint16_t matr;
   char exam[5];
   uint16_t year;
   uint16_t vote;
   uint16_t time; 
};

typedef struct TInfo* Info;

/* Lista */

struct TList {

    Info i;
    struct TList* next;
};

typedef struct TList* List;

List initNodeList(Info i); // Inizializza un nuovo nodo
List appendNodeList(List L, Info i);  /* Aggiunge un nodo alla fine della lista controllandone l'esistenza. La funzione ritorna sempre la testa della lista*/
void freeList(List L); // Dealloca la lista interamente




