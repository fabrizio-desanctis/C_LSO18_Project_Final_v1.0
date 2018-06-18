#include "AuxFunctions.h"
#include "server.h"

//Parametri
int listen_port; // Porta d'ascolto.
int sock; // Socket.
char file_name[30]; // Nome del file.
RecordingsList Registro = NULL; // Registro in cui salviamo tutte le operazioni.
VoteSumList SommaVoti = NULL;  // Lista in cui salviamo tutte le somme dei voti. 
//Mutex
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex per la stampa su file.
pthread_mutex_t registro_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex per l'operazione REG.
pthread_mutex_t sva_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex per l'operazione SVA. 

int main( int argc, char* argv[] ){
	char buff[60]; // Variabile per stampa a video.

	//Controllo dei parametri del programma
	if (argc < 3 ) PrintErrClose(ERR_NUM);
	if ( !IsUInteger(argv[1]) ) PrintErrClose(ERR_PORT);
	
	//Assegnazione dei parametri a variabili globali
	listen_port = atoi(argv[1]);
	strcpy(file_name,argv[2]);

	/*INIZIO PROGRAMMA*/
	CLEARSCREEN;
	sprintf(buff , "Porta: %d\n", listen_port );	
	write(1, buff , strlen(buff)); 
	Registro = ReadToFile(Registro,file_name); //Inizio Fase 1 - Lettura operazioni da file.

	Connection_creation();	// Inizio Fase 2 - Creazione e connessione thread. 
	
return 0;
}

/* FUNZIONI */

//FASE 1 - Lettura operazioni da file.


/*Questa funzione si occupa di inizializzare un nuovo nodo della lista. Prende in input le informazioni necessarie e restituisce una nuova
occorrenza della lista.*/
RecordingsList initNodeListR(Recordings r) {
    RecordingsList L = (RecordingsList)malloc(sizeof(struct TListRecordings));
    L->r = r;
    L->next = NULL;
    return L;
}



/*Questa funzione si occupa di appendere un nodo direttamente alla fine della lista;
Ritorna la nuova lista. */
RecordingsList appendNodeListR(RecordingsList L, Recordings r) {
    if (L != NULL) {
        L->next = appendNodeListR(L->next,r);
    } else {
        L = initNodeListR(r);
    }
    return L;
}

/*Questa procedura libera tutta la memoria occupata dalla lista*/
void freeListR(RecordingsList L) {
    if (L != NULL) {
        freeListR(L->next);
        free(L);
    }
}

/*Questa funzione si occupa di leggere dal file tutti gli esami già registrati. Prende in input una lista in cui verranno inseriti gli esami registrati ed il nome del file dal quale prelevarli. Ritorna la testa della lista.*/
RecordingsList ReadToFile(RecordingsList L,char file_name[]){

	char matr[5];
	int file_descriptor,read_value;
	Recordings information = (Recordings)malloc(sizeof(struct TRecordings));
	

	/* Apertura file */
	file_descriptor = open(file_name,O_RDONLY|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO); 
	if( file_descriptor == -1 ) PrintErrClose(ERR_FILE); 

        /*Lettura file*/
	read_value = read(file_descriptor,matr,4*sizeof(char));
	matr[4] = '\0';

	while(read_value != 0 && matr[0]!='\n'){ // Continua finchè non ci sono più caratteri da leggere.
		if(read_value == -1)
		   PrintErrClose(ERR_READ);
	
      		L = Operation(file_descriptor,L,matr); 
		lseek(file_descriptor, (off_t)1, SEEK_CUR);  //Salta il carattere fine riga;
	
        /*Lettura operazione successiva*/
	read_value = read(file_descriptor,matr,4*sizeof(char));
	matr[4] = '\0';   }
	
	close(file_descriptor); //Chiudi file.
return L;
}

/*Questa funzione si occupa di leggere dal file le informazioni che riguardano l'esame di una determinata matricola. Ritorna la testa della lista.*/
RecordingsList Operation(int file_descriptor,RecordingsList L,char matr[]){

        /*Allocazione struttura*/
     	Recordings information = (Recordings)malloc(sizeof(struct TRecordings));
        information->matr = atoi(matr);   

	char *string=malloc(dim*sizeof(char)); //Stringa acquisita;

	/*Riempimento struttura*/
	        lseek(file_descriptor, (off_t)1, SEEK_CUR);  //Salta i ":";
		if(read(file_descriptor,information->exam,4*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci esame.
	         information->exam[4] = '\0';
		lseek(file_descriptor, (off_t)1, SEEK_CUR); //Salta i ":";
		if(read(file_descriptor,string,1*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci anno accademico.
	        string[1] = '\0';
                information->year = atoi(string);
		lseek(file_descriptor, (off_t)1, SEEK_CUR); //Salta i ":";
		if(read(file_descriptor,string,2*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci voto;
	        string[2] = '\0';
		information->vote = atoi(string);
	
                
        L = appendNodeListR(L,information); //Inserisci in lista
return L;
}




//FASE 2 - Creazione connessione e thread.

/*Questa procedura si occupa di creare il thread che gestirà tutte le connessioni.*/
void Connection_creation(){

	pthread_t tid_connect[1];
	pthread_create( tid_connect , NULL , Connect, NULL);
	pthread_join( tid_connect[0] , NULL);	
}

/*quesat procedura viene lanciata alla creazione di un nuovo thread nella funzione precedente e si occupa di inizializzare tutti i parametri necessari per la connessione e di accettare nuovi client creando un thread ogni volta che se ne connette uno.*/
void *Connect(void* arg){
	
	int work = 1; // Condizione di uscita del ciclo.
	int *channel; // Identificatore del client.
	pthread_t tid[1]; // Tid del thread.
	struct sockaddr_in info; // Struttura nella quale salviamo le informazioni per la connessione.
	/*Inizio connessione.*/
	sock = socket(AF_INET,SOCK_STREAM,0);

	if (sock == -1 ) PrintErrClose(ERR_SOCKET);

	info.sin_family = AF_INET;
	info.sin_port = htons( listen_port );
	info.sin_addr.s_addr = INADDR_ANY;

	if( bind(sock, (struct sockaddr*)&info, sizeof(info)) != 0) PrintErrClose(ERR_BIND);
	if ( listen(sock,70) != 0) PrintErrClose(ERR_LISTEN);

	/*Inizio accettazione nuovi client*/
	while(work == 1){
		channel = malloc(sizeof(int));
		write(1,CLIENT_WAIT,strlen(CLIENT_WAIT)*sizeof(char));
		(*channel) = accept(sock,NULL,NULL); // Nuovo client accettato.
		pthread_create( tid , NULL , NewClient, channel);  // Creazione nuovo thread per gestione del client.
		write(1,CLIENT_ACCEPT,strlen(CLIENT_ACCEPT)*sizeof(char));
	
		
}		
	close(sock); // Chiusura della connessione.
}



/* Questa procedura viene lanciata ogni volta che si connette un client. Si occupa di leggere e completare le operazioni inviate dal client. Prima di terminare effettua una scrittura su file.*/

void *NewClient(int *param){
	int channel=(*param); // Identificatore del client.  
	int num_oper; // Numero operazioni.
	Info coming = (Info)malloc(sizeof(Info)); // Salviamo le informazioni lette dal client.
	char temp[5]; // Variabile temporanea.
	
		read(channel,temp,2*sizeof(char)); // Lettura del numero di operazioni dal client.
		temp[2] = '\0';
		num_oper = atoi(temp); // Conversione da stringa ad intero.
		
		do{
			// Inizio lettura informazioni dal client.
			read(channel,coming->operation,3*sizeof(char));
		 	read(channel,temp,4*sizeof(char));
			temp[4] = '\0';
			coming->matr = atoi(temp);
			read(channel,coming->exam,4*sizeof(char));
			read(channel,temp,1*sizeof(char));
			temp[1] = '\0';
			coming->year = atoi(temp);
			read(channel,temp,2*sizeof(char));
			temp[2] = '\0';
			coming->vote = atoi(temp);
			read(channel,temp,1*sizeof(char));
			temp[1] = '\0';
			coming->time = atoi(temp);
		
			
			if(strcmp(coming->operation,"REG") == 0){  //E' un'operazione "REG".
				pthread_mutex_lock(&registro_lock); // Blocca il mutex.
				Registro = Registra(channel,coming); 
				pthread_mutex_unlock(&registro_lock); // Rilascia il mutex. 
			}
		
			else if(strcmp(coming->operation,"SVA") == 0){ //E' un'operazione "SVA".
				pthread_mutex_lock(&sva_lock); // Blocca il mutex.
				SommaVoti = SommaVotiAnno(SommaVoti,Registro,coming->matr,coming->year);
				pthread_mutex_unlock(&sva_lock); // Rilascia il mutex. 
				write(channel,OPERAZ_OK,strlen(OPERAZ_OK));
				}
	   			

			else if(strcmp(coming->operation,"IMV") == 0) //E' un'operazione "IMV".
				InviaSommaVoti(channel,SommaVoti,coming->matr);
	   			

			else if(strcmp(coming->operation,"CIV") == 0) //E' un'operazione "CIV".
           			CalcolaeInviaSommaVoti(channel,Registro,coming->matr);

			else if(strcmp(coming->operation,"CLO") == 0){ //E' un'operazione "CLO".
	   			num_oper = 0; // Setta a 0 il numero di operazioni ancora da effettuare.
				write(channel,OPERAZ_OK,strlen(OPERAZ_OK)); }

			else if(strcmp(coming->operation,"SLE") == 0) //E' un'operazione "SLE".
				Sleep(channel,coming->time);
	   			
		if(num_oper != 0) // Se numero operazioni è diverso da 0. 
        	num_oper--; // Decrementa numero operazioni.

		}while(num_oper !=0); // Contiua finchè non sono state completate tutte le operazioni.
		
		
		Close(channel); // Chiude il canale di comunicazione con il client.
		pthread_mutex_lock(&print_lock); // Blocca il mutex.
		PrinttoFile (Registro,file_name); // Scrittura su file.
		pthread_mutex_unlock(&print_lock); // Sblocca il mutex.
		
	}

//FASE 3 - Operazioni da svolgere.

// OPERAZIONE REG. Questa procedura si occupa di registrare nella lista un nuovo esame inviato dal client. Ritorna la testa della lista.
RecordingsList Registra (int channel,Info coming){

char buff[100];
int flag = 0;
Recordings info = (Recordings)malloc(sizeof(Recordings)); // Allocazione nuovo nodo Recordings.
/*Inizializzazione nodo con informazioni provenienti dal client.*/
info->matr = coming->matr; 
strcpy(info->exam,coming->exam);
info->vote = coming->vote;
info->year = coming->year;
RecordingsList temp=Registro; // Puntatore per scorrere la lista.	

	if(info->vote < 18 || info->vote >30) // Se il voto non è corretto.
		flag = 1; // Setta flag a 1.
	while(temp != NULL && flag == 0){ // Scorri la lista per controllare che l'esame che si vuole registrare non sia già stato inserito.
		if(temp->r->matr == coming->matr && strcmp(coming->exam,temp->r->exam) == 0 ){ // Esame già registrato.
			flag = 1; // Setta flag a 1.
			 }
		else				
		  temp = temp->next; // Altrimenti continua a scorrere la lista.
  	}
    if(flag == 0){ // Esame può essere registrato.
    Registro = appendNodeListR(Registro,info); // Aggiungi nodo alla lista.
		write(channel,OPERAZ_OK,strlen(OPERAZ_OK)); 
    }
    else write(channel,ERR_REG,strlen(ERR_REG)); // Esame già presente, operazione fallita.

return Registro; 
}

// OPERAZIONE SVA. 

/*Questa funzione si occupa di inizializzare un nuovo nodo della lista. Prende in input le informazioni necessarie e restituisce una nuova
occorrenza della lista.*/
VoteSumList initNodeListS(VoteSum i) {
    VoteSumList L = (VoteSumList)malloc(sizeof(struct TVoteSumList));
    L->i = i;
    L->next = NULL;
    return L;}



/*Questa funzione si occupa di appendere un nodo direttamente alla fine della lista;
Ritorna la nuova lista. */
VoteSumList appendNodeListS (VoteSumList L, VoteSum i) {
    if (L != NULL) {
        L->next = appendNodeListS(L->next,i);
    } else {
        L = initNodeListS(i);
    }
    return L;
}

/*Questa procedura libera tutta la memoria occupata dalla lista*/
void freeListS (VoteSumList L) {
    if (L != NULL) {
        freeListS(L->next);
        free(L);
    }
}


/*Questa funzione si occupa di sommare tutti i voti dello stesso anno relativi ad una matricola,salvandoli localmente. Alla prossima chiamata della funzione con la stessa matricola, la nuova somma sarà sommata alla precedente calcolata.*/
VoteSumList SommaVotiAnno ( VoteSumList L , RecordingsList R , uint16_t matr , uint16_t year ){

RecordingsList Rtemp=R; // Puntatore per scorrere la lista.
uint16_t sommavoti=0; // Somma.
VoteSum somma=malloc(sizeof(VoteSum)); // Allocazione nodo.

	while (Rtemp != NULL ){ // Finchè ci sono elementi.
		if ( Rtemp->r->matr == matr && Rtemp->r->year == year ) // Se l'esame è della matricola cercata e dell'anno richiesto.
			sommavoti=sommavoti+ Rtemp->r->vote; // Somma.
		Rtemp=Rtemp->next; // Vai al successivo.
			       }

	/*Inizializzazione nodo.*/	
	somma->matr=matr;
	somma->sum=sommavoti;
	L=ScorriLista ( L, somma); // Si occupa di inserire i dati nella lista.
return L;
}


/*Questa funzione si occupa di cercare se la matricola è già presente nella lista. In tal caso addiziona la somma precedente con quella attuale. Altrimenti inserisce il nodo in coda alla lista. Ritorna la testa della lista.*/
VoteSumList ScorriLista ( VoteSumList L ,VoteSum somma ){
	if( L != NULL){
		if ( L->i->matr == somma->matr ) 
			L->i->sum = L->i->sum + somma->sum; 
		else 
			L->next=ScorriLista(L->next,somma); }
 	else 
		L=appendNodeListS(L,somma);
return L;
}

// OPERAZIONE IMV. Questa funzione si occupa di inviare al client la somma dei voti calcolata in precedenza per una data matricola.

void InviaSommaVoti ( int channel , VoteSumList L , uint16_t matr){
VoteSumList Ltemp=L; // Puntatore per scorrere la lista.
int flag = 0; 
uint16_t risultato = 0; 
char buffer [50];

	while ( Ltemp != NULL && flag == 0) { // Finchè ci sono elementi oppure non hai ancora trovato la matricola.
		if ( Ltemp->i->matr == matr ) { // Matricola trovata.
			flag=1; // Setta flag a 1.
			risultato = Ltemp->i->sum; // Salva la somma da inviare. 
					      }
		else 
			Ltemp=Ltemp->next;  // Altrimenti continua a scorrere.
					     }
	
 	
	sprintf(buffer, "[SERVER] Matr:%d. Somma:%d\n\n" ,matr, risultato); 
	buffer[49]='\0';
	
	
	write (channel,buffer,strlen(buffer)); // Invio al client.



}
	
//OPERAZIONE CIV. Questa funzione di occupa di sommare ed inviare al client tutti i voti per una data matricola.

void CalcolaeInviaSommaVoti ( int channel , RecordingsList R , uint16_t matr ){

RecordingsList Rtemp=R; // Puntatore per scorrere la lista.
uint16_t sommavoti=0; 
char buffer[50];

	while (Rtemp != NULL ){ // Finchè ci sono elementi.
		if ( Rtemp->r->matr == matr ) // Se la matricola è quella cercata.
			sommavoti=sommavoti+ Rtemp->r->vote; // Aggiorna la somma.
		Rtemp=Rtemp->next; // Contiua a scorrere.
			       }
	sprintf(buffer, "[SERVER] Matr:%d. Somma:%d\n\n" ,matr, sommavoti); 
	buffer[49]='\0';
	
	
	write (channel,buffer,strlen(buffer)); // Invio al client.

}

//OPERAZIONE CLO.

int Close( int channel ){

	close(channel);
}	

//OPERAZIONE SLE.

void Sleep ( int channel, uint16_t time ){

	sleep(time); 
	write(channel,OPERAZ_OK,strlen(OPERAZ_OK));
}

//FASE 4 - Scrittura su file.
/*Questa funzione si occupa di scrivere su file tutti gli esami presenti sulla lista.*/
void PrinttoFile ( RecordingsList R , char file_name[]){

RecordingsList R_temp=R; // Puntatore per scorrere la lista.
char temp[20];
int file_descriptor;

	/* Apertura file */
	file_descriptor = open(file_name,O_WRONLY|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO); 
	if( file_descriptor == -1 ) PrintErrClose(ERR_FILE);

	while(R_temp != NULL){ // Finchè ci sono elementi.

	sprintf(temp,"%d:%s:%d:%d\n",R_temp->r->matr,R_temp->r->exam,R_temp->r->year,R_temp->r->vote);
	temp[15]='\0';
	write(file_descriptor,temp,strlen(temp)); // Scrivi su file.
	R_temp=R_temp->next; //Scorri al successivo.
			     }

	close(file_descriptor); //Chiudi il file.
							}




