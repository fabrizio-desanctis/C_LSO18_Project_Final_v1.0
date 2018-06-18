#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define CLIENT_WAIT  "In attesa di un client.\n\n"
#define CLIENT_ACCEPT "Nuovo client accettato.\n"
#define ERR_REG "[SERVER] Operazione fallita.\n\n"
#define OPERAZ_OK "[SERVER] Operazione eseguita.\n\n"
/* Struttura Recordings */
//Struttura utilizzata per salvare informazioni lette dal file.
struct TRecordings {

   uint16_t matr;
   char exam[5];
   uint16_t year;
   uint16_t vote;
 
};

typedef struct TRecordings* Recordings;


/* Lista Recordings*/
//Struttura utilizzate per creare una lista in cui verranno salvate le informazioni lette da file.
struct TListRecordings {

    Recordings r;
    struct TListRecordings* next;
};

typedef struct TListRecordings* RecordingsList;

/*Struttura VoteSum*/
//Struttura utilizzata per salvare la somma dei voti di una matricola.
struct TVoteSum {
	uint16_t matr;
	uint16_t sum; 
};

typedef struct TVoteSum* VoteSum;

/* Lista VoteSum */
// Struttura utilizzata per creare una lista i cui verranno salvate le somme dei voti delle matricole.
struct TVoteSumList {

	VoteSum i; 
	struct TVoteSumList* next; 
};

typedef struct TVoteSumList* VoteSumList; 



//FASE 1 - Lettura operazioni da file.
RecordingsList initNodeListR(Recordings r); // Inizializza un nuovo nodo Recordings.
RecordingsList appendNodeListR(RecordingsList L, Recordings r);  // Aggiunge un nodo alla fine della lista controllandone l'esistenza. La funzione ritorna sempre la testa della lista.
void freeListR(RecordingsList L); // Dealloca la lista interamente.
RecordingsList ReadToFile(RecordingsList L,char file_name[]); // Legge da file i voti già registrati, ritorna un puntatore alla lista.
RecordingsList Operation(int file_descriptor,RecordingsList L,char matr[]); // Legge da file il voto relativo alla matricola passata.
//FASE 2 - Creazione connessione e thread.
void Connection_creation(); // Crea un thread che si occupa della connessione. 
void *Connect(void*); // Procedura lanciata da un thread che si occupa di gestire i client che si connettono. 	
void *NewClient(); // Procedura lanciata da un thread che si occupa di effettuare tutte le operazioni richieste da un client.
//FASE 3 - Operazioni da svolgere.
// OPERAZIONE REG. 
RecordingsList Registra (int channel,Info coming); // Registra sulla lista un nuovo voto.
// OPERAZIONE SVA.
void freeListS (VoteSumList L); // Dealloca la lista interamente.
VoteSumList appendNodeListS (VoteSumList L, VoteSum i); // Aggiunge un nodo alla fine della lista controllandone l'esistenza. La funzione ritorna la testa della lista. 
VoteSumList initNodeListS(VoteSum i); // Inizilizza un nuovo nodo VoteSum.
VoteSumList ScorriLista ( VoteSumList L ,VoteSum somma ); // Cerca nella lista VoteSum se un nodo è presente,altrimenti lo aggiunge.
VoteSumList SommaVotiAnno ( VoteSumList L , RecordingsList R , uint16_t matr , uint16_t year ); // Somma i voti di una matricola relativi ad un anno e li inserisce nella lista VoteSumList.
//OPERAZIONE IMV.
void InviaSommaVoti ( int channel , VoteSumList L , uint16_t matr); // Invia al client la somma dei voti calcolata dalla funzione precedente.
//OPERAZIONE CIV.
void CalcolaeInviaSommaVoti ( int channel , RecordingsList R , uint16_t matr ); // Calcola ed invia al client la somma di tutti i voti di una matricola.
//OPERAZIONE CLO.
int Close( int channel ); // Chiude il collegamento con il client.
//OPERAZIONE SLE.
void Sleep ( int channel, uint16_t time ); // Mette il processo in attesa per "time" secondi. 
// FASE 4 - Scrittura su File. 
void PrinttoFile ( RecordingsList R , char file_name[]); // Scrive su file tutte le operazioni presenti sulla lista RecordingList.
