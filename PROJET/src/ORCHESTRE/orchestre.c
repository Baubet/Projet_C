#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
/*include rajouter*/
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h> 

#include "io.h"
#include "memory.h"
#include "myassert.h"


#include "../CLIENT_SERVICE/client_service.h" // char pipe client service
 /* --- */

#include "config.h"
#include "client_orchestre.h"
#include "orchestre_service.h"
#include "service.h"

//--> define dans client_orchestre.h
// FICHIER_CO "CLIENT_ORCHESTRE/client_orchestre.h" 	
// ID_CO 5 					
// MODE_CO 0641

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <fichier config>\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------
// création et initialisation semaphore
static int my_semget(int proj_ID)
{
    key_t key;
    int semaID;
    int ret;

    key = ftok(FICHIER_CO, proj_ID);
    myassert(key != -1, "orchestre.c - ERREUR : my_semget() - ftok\n");
    
    semaID = semget(key, 1, IPC_CREAT | IPC_EXCL | MODE_CO);	// MODE_CO 0641
    myassert(semaID != -1, "orchestre.c - ERREUR : my_semget() - semget\n");
    
    // 2e param : numéro sémaphore ; 4e param : valeur du sémaphore (nbr de workers en simultannés)
    ret = semctl(semaID, 0, SETVAL, 1);
    myassert(ret != -1, "orchestre.c - ERREUR : my_semget() - semctl / SETVAL\n");

    return semaID;
}


//-----------------------------------------------------------------
// attente du sémaphore
static void my_semwait(int semaID)
{
    int ret;
    struct sembuf operationAttente = {0, 0, 0};
    ret = semop(semaID, &operationAttente, 1);
    myassert(ret != -1, "orchestre.c - ERREUR : my_semwait().\n");
}

//-----------------------------------------------------------------
// Etat d'un service, avec GETVAL du sémaphore (-1 pas de sc = service non utilisé)
static bool isEndService(int semaID)
{
    int ret = semctl(semaID, 1, GETVAL);
    myassert(ret != -1, "orchestre.c - ERREUR isEndService on semctl GETVAL.");

    return (ret < 0);
    
}
//
// Enregistre l'état de tout les services dans les booleans associés
static void isEndServices(bool *isEndS_SUM, int semaID_SUM, bool *isEndS_COMP, int semaID_COMP, bool *isEndS_SIGMA, int semaID_SIGMA){

    *isEndS_SUM = isEndService(semaID_SUM);
    
    *isEndS_COMP = isEndService(semaID_COMP);
    
    *isEndS_SIGMA = isEndService(semaID_SIGMA);
}

//-----------------------------------------------------------------
// création tube nommé
static void my_mkfifo(const char *pathname, mode_t mode){
    int ret;
    ret = mkfifo(pathname, mode);
    myassert(ret == 0, "Erreur orchestre.c, creation tube nommé.\n");
}

//-----------------------------------------------------------------
// génération password
static void randPassword(int *x){
    *x = rand() % 96 + 4; //(entre 4 et 99) valeurs supérieurs numService
}

//-----------------------------------------------------------------
// création tube anonyme
static void open_tubeA_and_close(int fd_A[2])
{   
    int ret;
    
    ret = pipe(fd_A);
    myassert(ret == 0, "ERREUR open_tubeA_and_close open tube anonyme\n");
    
    ret = close(fd_A[1]);
    myassert(ret == 0, "ERREUR open_tubeA_and_close close tube anonyme (partie inutile)\n");
    
}

//-----------------------------------------------------------------
// lancement service
static void my_forkexecv(int *semaID_S, int ID_SO, int fd_A_SO[2], char *fdS2C, char* fdC2S, int numService){
	
	int retFork;
	
	*semaID_S = my_semget(ID_SO); 	// creation et initialisation semaphore pour Orchestre et Client
	open_tubeA_and_close(fd_A_SO); 	// création tube anonyme entre orchestre et service
					// p-e pas utile de fermer				
	char *prog2exec = "SERVICE/service";
	
	// params : 0 = programme, 1 = numService (en string), 2 = semaID du service (en string), 3 = fd tube anonyme (en string), 4/5 = tube nommés service/client (cf. client_service.h), 6 = NULL (fin tableau)
	char *const params[] = {prog2exec, io_intToStr(numService), io_intToStr(*semaID_S), io_intToStr(fd_A_SO[0]), fdS2C, fdC2S, NULL};
	
	retFork = fork();
  	myassert(retFork != -1, "Erreur startServices fork()\n");
  	if (retFork == 0)     // in fork
	{
		execv(prog2exec, params);
	}
}

//-----------------------------------------------------------------
// communication accès service
static void sendAccept(int fdCO_write, int *randP, int fd_A_SERVICE_WRITE, int semaID_SC, const char *pipe_s2c, const char *pipe_c2s){
	
	//     envoi au client d'un code d'acceptation (via le tube nommé)
	mywrite_int(fdCO_write, 1);   // code d'acception == 1 (true)
		
	//     génération d'un mot de passe
	randPassword(randP);
	
	//     envoi d'un code de travail au service (via le tube anonyme)
	mywrite_int(fd_A_SERVICE_WRITE, 1);
	//     envoi du mot de passe au service (via le tube anonyme)
	mywrite_int(fd_A_SERVICE_WRITE, *randP);
	
	//     envoi du mot de passe au client (via le tube nommé)
	mywrite_int(fdCO_write, *randP);
	//     envoi des noms des tubes nommés au client (via le tube nommé)
	mywrite_str(fdCO_write, pipe_s2c);
	mywrite_str(fdCO_write, pipe_c2s);

}

int main(int argc, char * argv[])
{
    if (argc != 2)
        usage(argv[0], "nombre paramètres incorrect");
    
    bool fin = false;

    // lecture du fichier de configuration
    config_init(argv[1]);
    
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
    // init rand number (for password Service Client)
    srand(time(NULL));
    
    // Varibales
    // var sema
    int semaID_CO;     		// sema synchro orchestre et client
    //int semaID_CO_FIN;		// sema synchro orchestre et client (close pipes)
    int semaID_SUM, semaID_COMP, semaID_SIGMA; // semaphore orchestre et serviceS
    bool isEndS_SUM, isEndS_COMP, isEndS_SIGMA;// bool état service (getval on semaphore)
    
    // var tubes
    int fdCO_read, fdCO_write; 	// tubes nommés entre Orchestre et Client
    int fdSO_read, fdSO_write; 	// tubes nommés entre Service et Client
    
    int fd_A_SUM[2];    	//
    int fd_A_COMP[2];     	// tubes anonyme entre Orchestre et Service
    int fd_A_SIGMA[2];; 	//
	// [0] = read  | [1] = write
    
    // Autre
    int ret; // pour les assert
    int numService; 	// recupération numéro service envoyé par le client
    int randP;

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
    // Pour la communication avec les clients
    // - création de 2 tubes nommés pour converser avec les clients
    // - création d'un sémaphore pour que deux clients ne
    //   ne communiquent pas en même temps avec l'orchestre 
    
    // création tubes nommés vers les clients
    my_mkfifo(PIPE_O2C, MODE_CO);
    my_mkfifo(PIPE_C2O, MODE_CO);
    
    semaID_CO = my_semget(ID_CO); 	// creation + init du semaphore Orchestre Client
    
    
/*-------------------------------------------------------------------------------------------------------------------------------------------*/    
    // lancement des services, avec pour chaque service :
    		// --> voir TP6 - exo8
    		
    // - création d'un tube anonyme pour converser (orchestre vers service)
    // - un sémaphore pour que le service préviene l'orchestre de la
    //   fin d'un traitement
    // - création de deux tubes nommés (pour chaque service) pour les
    //   communications entre les clients et les services
    
    // SERVICE_SOMME
    my_forkexecv(&semaID_SUM, ID_SO_SUM, fd_A_SUM, PIPE_S2C_SUM, PIPE_C2S_SUM, SERVICE_SOMME);
    
    // SERVICE_COMPRESSION
    my_forkexecv(&semaID_COMP, ID_SO_COMP, fd_A_COMP, PIPE_S2C_COMP, PIPE_C2S_COMP, SERVICE_COMPRESSION);
    
    // SERVICE_SIGMA
    my_forkexecv(&semaID_SIGMA, ID_SO_SIGMA, fd_A_SIGMA, PIPE_S2C_SIGMA, PIPE_C2S_SIGMA, SERVICE_SIGMA);


/*-------------------------------------------------------------------------------------------------------------------------------------------*/ 
    while (! fin)
    {
        // ouverture ici des tubes nommés avec un client
    		// --> il faut bien que les deux "open" soit inverser par rapport à ceux de l'orchestre (attente mutuelle)
        fdCO_write = open(PIPE_C2O, O_WRONLY);
        myassert(fdCO_write != -1, "Erreur orchestre.c, open write fdCO_write\n");
        fdCO_read = open(PIPE_O2C, O_RDONLY);
        myassert(fdCO_read != -1, "Erreur orchestre.c, open read fdCO_read\n");
        
        // attente d'une demande de service du client
        int numService = myread_int(fdCO_read);

        // détecter la fin des traitements lancés précédemment via
        // les sémaphores dédiés (attention on n'attend pas la
        // fin des traitement, on note juste ceux qui sont finis)
        isEndServices(&isEndS_SUM, semaID_SUM, &isEndS_COMP, semaID_COMP, &isEndS_SIGMA, semaID_SIGMA); // GETVAL on sema + bool on statement

        // analyse de la demande du client  
        numService = myread_int(fdCO_read); 
        
        // si ordre de fin
        //     envoi au client d'un code d'acceptation (via le tube nommé)
        //     marquer le booléen de fin de la boucle
        if(numService == SERVICE_ARRET){
        	mywrite_int(fdCO_write, numService);

            	fin = true; // arrêt boucle => fin service/client après la boucle
        }
	        
        // sinon si service non ouvert
        //     envoi au client d'un code d'erreur (via le tube nommé)
        else if(config_isServiceOpen(numService)){
        	mywrite_int(fdCO_write, -2);
        }
        // sinon si service déjà en cours de traitement
        //     envoi au client d'un code d'erreur (via le tube nommé)
        else if(numService == SERVICE_SOMME){
        	if(!isEndS_SUM){
        		mywrite_int(fdCO_write, -3); // déjà en cours de traitement
        	}else{
        		sendAccept(fdCO_write, &randP, fd_A_SUM[1], semaID_SUM, PIPE_S2C_SUM, PIPE_C2S_SUM);	// acception -> envoie code travail/password,tubes names
        	}
        }
        else if(numService == SERVICE_COMPRESSION){
        	if(!isEndS_COMP){
        		mywrite_int(fdCO_write, -3); // déjà en cours de traitement
        	}else{
        		sendAccept(fdCO_write, &randP, fd_A_COMP[1], semaID_COMP, PIPE_S2C_COMP, PIPE_C2S_COMP); 	// acception -> envoie code travail/password,tubes names
        	}
        }
        else if(numService == SERVICE_SIGMA){
        	if(!isEndS_SIGMA){
        		mywrite_int(fdCO_write, -3); // déjà en cours de traitement
        	}else{
        		sendAccept(fdCO_write, &randP, fd_A_SIGMA[1], semaID_SIGMA, PIPE_S2C_SIGMA, PIPE_C2S_SIGMA);	// acception -> envoie code travail/password,tubes names
        	}
        }
        // sinon
        else{
        	//erreur ?
        	printf("ERREUR orchestre.c, aucun retour client sur service.");
        }
	// finsi

        // attente d'un accusé de réception du client
        	// client send 8
        ret = myread_int(fdCO_read);
        myassert(ret != 8, "Erreur orchestre.c, read ADC (!= 8)\n");
        
        // fermer les tubes vers le client
	ret = close(fdCO_read);
        myassert(ret != -1, "Erreur orchestre.c, close fdCO_read\n");
        ret = close(fdCO_write);
        myassert(ret != -1, "Erreur orchestre.c, close fdCO_write\n");

        // il peut y avoir un problème si l'orchestre revient en haut de la
        // boucle avant que le client ait eu le temps de fermer les tubes
        // il faut attendre avec un sémaphore.
        // attendre avec un sémaphore que le client ait fermé les tubes
        
    	my_semwait(semaID_CO); // attente déblocage du client
    }

    // attente de la fin des traitements en cours (via les sémaphores)

    // envoi à chaque service d'un code de fin

    // attente de la terminaison des processus services

    // libération des ressources
    my_destroy(semaID_CO);
    
    return EXIT_SUCCESS;
}
