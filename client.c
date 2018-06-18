#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "AuxFunctions.h"


/*Parametri*/
int num_oper = 0;

List ReadToFile(List L,char file_name[]);  //Legge da file.
List OperationReg(int file_descriptor,List L,char operation[]);  //Gestisce l'operazione "REG".
List OperationSva(int file_descriptor,List L,char operation[]);  //Gestisce l'operazione "SVA".
List OperationImvAndCiv(int file_descriptor,List L,char operation[]); //Gestisce le operazioni "IMV" e "CIV".
List OperationSle(int file_descriptor,List L,char operation[]); //Gestisce l'operazione "SLE".
List OperationClo(int file_descriptor,List L,char operation[]); //Gestisce l'operazione "CLO".
void PrintBuffer ( int indice , Info i );
int main(int argc,char* argv[]){

	List L = NULL;  //Lista in cui andranno salvate informazioni raccolte dal file.
	char buff_server[100];	
	char temp[5];
	int i = 1;

	struct sockaddr_in mio_ind;

         /* Controlli sui parametri in ingresso */
        if(argc<4)  PrintErrClose(ERR_NUM); //Numero parametri in ingresso;
        if( inet_aton(argv[1], NULL)==0 ) PrintErrClose(ERR_IP);  //Indirizzo IP;
        if ( !IsUInteger(argv[2]) ) PrintErrClose(ERR_PORT);   //Numero porta;
        
        /* Lettura da file */
	L = ReadToFile(L,argv[3]);  
	
	/*Creazione del socket*/
	mio_ind.sin_family=AF_INET;
	mio_ind.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1],&mio_ind.sin_addr);

	int sockcl=socket(AF_INET,SOCK_STREAM,0);
	if (sockcl<0) PrintErrClose(ERR_SOCKET);
	
	CLEARSCREEN;  
	/*Connessione al server*/
	int conn=connect(sockcl,(struct sockaddr *) &mio_ind, sizeof(mio_ind)); 
	if (conn<0) PrintErrClose(ERR_CONNECT);
        write(1,CONNECT_OK,strlen(CONNECT_OK)*sizeof(char));
	sleep(2);
	
	
	/*Invio al server*/
	sprintf(temp,"%d",num_oper);
		temp[2] = '\0';
		write(sockcl,temp,2*sizeof(char));
	while(L != NULL) {
		write(sockcl,L->i->operation,3*sizeof(char));
		sprintf(temp,"%d",L->i->matr);
		temp[4] = '\0';
		write(sockcl,temp,4*sizeof(char));
		write(sockcl,L->i->exam,4*sizeof(char));
		sprintf(temp,"%d",L->i->year);
		temp[1] = '\0';
		write(sockcl,temp,1*sizeof(char));
		sprintf(temp,"%d",L->i->vote);
		temp[2] = '\0';
		write(sockcl,temp,2*sizeof(char));
		sprintf(temp,"%d",L->i->time);
		temp[2] = '\0';
		write(sockcl,temp,1*sizeof(char));
		PrintBuffer(i,L->i);
		read(sockcl,buff_server,31*sizeof(char));
		sleep(2);
		write(1,buff_server,strlen(buff_server));
		
		L = L->next;
		i++;
		
	}
	
	//Terminare (chiude il socket)
	close(sockcl);
        
	freeList(L);	
   
return 0;
}



/*Questa funzione prende in input una lista ed una stringa contenente il nome di un file. Si occupa di aprire il file(se esiste)
in sola lettura. Legge i primi 3 caratteri per riconoscere l'operazione da eseguire e lancia la routine che la riguarda. Termina
alla fine del file,chiudendolo e ritornando la lista al chiamante.*/
List ReadToFile(List L,char file_name[]){

	char operation[4];
	int file_descriptor,read_value;

	/* Apertura file */
	file_descriptor = open(file_name,O_RDONLY|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO); 
	if( file_descriptor == -1 ) PrintErrClose(ERR_FILE); 

        /*Lettura file*/
	read_value = read(file_descriptor,operation,3*sizeof(char));
	operation[3] = '\0';
	
	while(read_value != 0){  
		if(read_value == -1)
		   PrintErrClose(ERR_READ);
	
        if(strcmp(operation,"REG") == 0)  //E' un'operazione "REG".
	   L = OperationReg(file_descriptor,L,operation);
		
	else if(strcmp(operation,"SVA") == 0) //E' un'operazione "SVA".
	   L = OperationSva(file_descriptor,L,operation);

	else if(strcmp(operation,"IMV") == 0) //E' un'operazione "IMV".
	   L = OperationImvAndCiv(file_descriptor,L,operation);

	else if(strcmp(operation,"CIV") == 0) //E' un'operazione "CIV".
           L = OperationImvAndCiv(file_descriptor,L,operation);

	else if(strcmp(operation,"CLO") == 0){ //E' un'operazione "CLO".
	   L = OperationClo(file_descriptor,L,operation); 
	   lseek(file_descriptor, (off_t)1, SEEK_CUR); }  

	else if(strcmp(operation,"SLE") == 0) //E' un'operazione "SLE".
	   L = OperationSle(file_descriptor,L,operation);

        else PrintErrClose(ERR_OPERATION); //Operazione non riconosciuta.Nessuna delle precedenti.
       
	lseek(file_descriptor, (off_t)1, SEEK_CUR);  //Salta il carattere fine riga;
	num_oper++;
        /*Lettura operazione successiva*/
	read_value = read(file_descriptor,operation,3*sizeof(char));
        operation[3] = '\0';   }

	close(file_descriptor); //Chiudi file.
return L;
}


/* Funzione di routine per l'operazione "REG". Si occupa di allocare una struttura in grado di contenere le informazioni,aggiungerle e infine passarle alla lista.*/
List OperationReg(int file_descriptor,List L,char operation[]){

        /*Allocazione struttura*/
     	Info information = (Info)malloc(sizeof(struct TInfo));
        strcpy(information->operation,operation);

	char *string=malloc(dim*sizeof(char)); //Stringa acquisita;

	/*Riempimento struttura*/
	lseek(file_descriptor, (off_t)1, SEEK_CUR); //Salta i ":";
		if(read(file_descriptor,string,4*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci matricola.
	        string[4] = '\0';
                information->matr = atoi(string);   
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
	/*Setta valori non utili per questa operazione*/
        
	information->time = 0;  //Setta time a 0.
                
        L = appendNodeList(L,information); //Inserisci in lista
return L;
}

/* Funzione di routine per l'operazione "SVA". Si occupa di allocare una struttura in grado di contenere le informazioni,aggiungerle e infine passarle alla lista.*/
List OperationSva(int file_descriptor,List L,char operation[]){

	/*Allocazione struttura*/
     	Info information = (Info)malloc(sizeof(struct TInfo));
        strcpy(information->operation,operation);

	char *string=malloc(dim*sizeof(char)); //Stringa acquisita;

	/*Riempimento struttura*/
	lseek(file_descriptor, (off_t)1, SEEK_CUR); //Salta i ":";
		if(read(file_descriptor,string,4*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci matricola.
	        string[4] = '\0';
                information->matr = atoi(string);   
                lseek(file_descriptor, (off_t)1, SEEK_CUR);  //Salta i ":";
		if(read(file_descriptor,string,1*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci anno accademico.
	        string[1] = '\0';
                information->year = atoi(string);
	
        /*Setta valori non utili per questa operazione*/
	information->vote = 0;
	information->time = 0;
	strcpy(information->exam,"nullo");
                
        L = appendNodeList(L,information); //Inserisci in lista
return L;
}


/* Funzione di routine per le operazione "IMV" e "CIV". Si occupa di allocare una struttura in grado di contenere le informazioni,aggiungerle e infine passarle alla lista.*/
List OperationImvAndCiv(int file_descriptor,List L,char operation[]){

	/*Allocazione struttura*/
     	Info information = (Info)malloc(sizeof(struct TInfo));
        strcpy(information->operation,operation);

	char *string=malloc(dim*sizeof(char)); //Stringa acquisita;
	
	/*Riempimento struttura*/
	lseek(file_descriptor, (off_t)1, SEEK_CUR); //Salta i ":";
		if(read(file_descriptor,string,4*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci matricola.
	        string[4] = '\0';
                information->matr = atoi(string);  
	
	/*Setta valori non utili per questa operazione*/ 
        information->time = 0;  //Setta time a 0.
	information->vote = 0;
	information->year = 0;
	strcpy(information->exam,"nullo");
                
        L = appendNodeList(L,information); //Inserisci in lista

return L;
}



/* Funzione di routine per l'operazione "CLO". Si occupa di allocare una struttura in grado di contenere le informazioni,aggiungerle e infine passarle alla lista.*/
List OperationClo(int file_descriptor,List L,char operation[]){

	/*Allocazione struttura*/
     	Info information = (Info)malloc(sizeof(struct TInfo));
        strcpy(information->operation,operation);
	
	/*Setta valori non utili per questa operazione*/
	information->matr = 0;   
        information->time = 0;  //Setta time a 0.
	information->vote = 0;
	information->year = 0;
	strcpy(information->exam,"nullo");

	L = appendNodeList(L,information); //Inserisci in lista
	
}


/* Funzione di routine per l'operazione "SLE". Si occupa di allocare una struttura in grado di contenere le informazioni,aggiungerle e infine passarle alla lista.*/
List OperationSle(int file_descriptor,List L,char operation[]){

	/*Allocazione struttura*/
     	Info information = (Info)malloc(sizeof(struct TInfo));
        strcpy(information->operation,operation);

	char *string=malloc(dim*sizeof(char)); //Stringa acquisita;

	/*Riempimento struttura*/
	lseek(file_descriptor, (off_t)1, SEEK_CUR); //Salta i ":";
		if(read(file_descriptor,string,1*sizeof(char))==-1)  PrintErrClose(ERR_READ); //Acquisisci matricola.
	        string[1] = '\0';
                information->time = atoi(string); 

	/*Setta valori non utili per questa operazione*/ 
        information->matr = 0;
        information->vote = 0;
	information->year = 0;
	strcpy(information->exam,"nullo");
	
	L = appendNodeList(L,information); //Inserisci in lista

return L;
}


/*Questa procedura si occupa di formattare e mandare a video una stringa che contiene l'operazione inviata al server.*/
void PrintBuffer ( int indice , Info i ) {

char buff_client[100];

	if(strcmp(i->operation,"REG") == 0)  //E' un'operazione "REG".
	  sprintf(buff_client,"[CLIENT] Operazione %d: REG:%d:%s:%d:%d. Invio Ok!\n",indice,i->matr,i->exam,i->year,i->vote);
		
	else if(strcmp(i->operation,"SVA") == 0) //E' un'operazione "SVA".
	  sprintf(buff_client,"[CLIENT] Operazione %d: SVA:%d:%d. Invio Ok!\n",indice,i->matr,i->year);

	else if(strcmp(i->operation,"IMV") == 0) //E' un'operazione "IMV".
	  sprintf(buff_client,"[CLIENT] Operazione %d: IMV:%d. Invio Ok!\n",indice,i->matr);

	else if(strcmp(i->operation,"CIV") == 0) //E' un'operazione "CIV".
          sprintf(buff_client,"[CLIENT] Operazione %d: CIV:%d. Invio Ok!\n",indice,i->matr);

	else if(strcmp(i->operation,"CLO") == 0) //E' un'operazione "CLO".
	  sprintf(buff_client,"[CLIENT] Operazione %d: CLO:. Invio Ok!\n",indice);

	else if(strcmp(i->operation,"SLE") == 0) //E' un'operazione "SLE".
	  sprintf(buff_client,"[CLIENT] Operazione %d: SLE:%d. Invio Ok!\n",indice,i->time);

	write(1,buff_client,strlen(buff_client));
}


