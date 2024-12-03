#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
/*include rajouter*/
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
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
static int my_semget(const char *fd, int proj_ID)
{
    key_t key;
    int semaID;
    int ret;

    key = ftok(fd, proj_ID);
    myassert(key != -1, "orchestre.c - ERREUR : ftok in mysemget() n");
    semaID = semget(key, 1, IPC_CREAT | IPC_EXCL | 0641);	// MODE_CO 0641
    myassert(semaID != -1, "orchestre.c - ERREUR : semget in mysemget() \n");
    
    // 2e param : numéro sémaphore ; 4e param : valeur du sémaphore (nbr de workers en simultannés)
    ret = semctl(semaID, 0, SETVAL, 1);
    myassert(ret != -1, "orchestre.c - ERREUR : semctl / SETVAL  in mysemget() \n");

    return semaID;
}

//-----------------------------------------------------------------
// création et initialisation semaphore + récupération clé
static int my_semgetWithGetKey(const char *fd, int proj_ID, key_t *key)
{
    int semaID;
    int ret;

    *key = ftok(fd, proj_ID);
    myassert(*key != -1, "orchestre.c - ERREUR : ftok in my_semgetWithGetKey() n");
    
    semaID = semget(*key, 1, IPC_CREAT | IPC_EXCL | 0641);	// MODE_SO 0641
    myassert(semaID != -1, "orchestre.c - ERREUR : semget in my_semgetWithGetKey() \n");
    
    // 2e param : numéro sémaphore ; 4e param : valeur du sémaphore (nbr de workers en simultannés)
    ret = semctl(semaID, 0, SETVAL, 1);
    myassert(ret != -1, "orchestre.c - ERREUR : semctl / SETVAL  in my_semgetWithGetKey() \n");

    return semaID;
}

//-----------------------------------------------------------------
// attente du sémaphore
static void my_semwait(int semaID)
{
    int ret;
    
    // Récupérer la valeur actuelle du sémaphore
    ret = semctl(semaID, 0, GETVAL);
    myassert(ret != -1, "orchestre.c - ERREUR : my_semwait() - semctl with GETVAL.\n");
    
    if(ret == 0){
	    struct sembuf operationAttente = {0, 0, 0};
	    ret = semop(semaID, &operationAttente, 1);
	    myassert(ret != -1, "orchestre.c - ERREUR : my_semwait().\n");
    }    
	/*else {
        printf("Le sémaphore n'est pas utilisé ret =%d, pas d'attente.\n", ret);
    }*/
}

//-----------------------------------------------------------------
// oppération d'attente sur les semaphores des services
static void sem_Wait_S(int semaID_SUM, int semaID_COMP, int semaID_SIGMA){
	my_semwait(semaID_SUM);
	my_semwait(semaID_COMP);
	my_semwait(semaID_SIGMA);
}

//-----------------------------------------------------------------
// écriture tube anonymes, pour les 3 services
static void write_To_S(int fd_A_SUM, int fd_A_COMP, int fd_A_SIGMA, int code){
	mywrite_int(fd_A_SUM, code);
	mywrite_int(fd_A_COMP, code);
	mywrite_int(fd_A_SIGMA, code);
}

//-----------------------------------------------------------------
// attente fin d'un processus créer en fork()
static void my_forkwait(){
    	int ret;
    	ret = wait(NULL);
    	myassert(ret != -1, "orchestre.c, ERREUR : attente fin exec processus services");
}

//-----------------------------------------------------------------
// attente fin des processus services créer en fork()
static void fork_Wait_allS(){
    	my_forkwait();
    	my_forkwait();
    	my_forkwait();
}

//-----------------------------------------------------------------
// Etat d'un service, avec GETVAL du sémaphore 
static bool isUseService(int semaID)
{
    int ret = semctl(semaID, 0, GETVAL);
    myassert(ret != -1, "orchestre.c - ERREUR isUseService on semctl GETVAL.");

    return (ret == 0); // true si utilisé
    
}
//-----------------------------------------------------------------
// Enregistre l'état de tout les services dans les booleans associés
static void isUseServices(bool *isUse_S_SUM, int semaID_SUM, bool *isUse_S_COMP, int semaID_COMP, bool *isUse_S_SIGMA, int semaID_SIGMA)
{

    *isUse_S_SUM = isUseService(semaID_SUM);
    *isUse_S_COMP = isUseService(semaID_COMP);
    *isUse_S_SIGMA = isUseService(semaID_SIGMA);
}

//-----------------------------------------------------------------
// création tube nommé
static void my_mkfifo(const char *pathname, mode_t mode)
{
    int ret;
    ret = mkfifo(pathname, mode);
    myassert(ret != -1, "Erreur orchestre.c, creation tube nommé.\n");
}

//-----------------------------------------------------------------
// suppression tube nommé
static void my_unlink(const char *pathname)
{
    int ret;
    ret = unlink(pathname);
    myassert(ret != -1, "Erreur orchestre.c, suppression tube nommé.\n");
}

//-----------------------------------------------------------------
// suppression de tout les tubes nommés
static void unlink_allNamedTube()
{
	my_unlink(PIPE_O2C);
	my_unlink(PIPE_C2O);
	my_unlink(PIPE_S2C_SUM);
	my_unlink(PIPE_C2S_SUM);
	my_unlink(PIPE_S2C_COMP);
	my_unlink(PIPE_C2S_COMP);
	my_unlink(PIPE_S2C_SIGMA);
	my_unlink(PIPE_C2S_SIGMA);
}

//-----------------------------------------------------------------
// génération password
static void randPassword(int *x){
    *x = rand() % 96 + 4; //(entre 4 et 99) valeurs supérieurs numService
}

//-----------------------------------------------------------------
// ouverture tube anonyme
static void open_tubeA(int fd_A[2]) 
{   
    int ret;
    
    ret = pipe(fd_A);
    myassert(ret == 0, "ERREUR orchestre.c open_tubeA\n"); 
}

//-----------------------------------------------------------------
// fermeture tube anonyme inutilisé
static void close_tubeA(int fd_A) 
{   
   int ret;
    
   ret = close(fd_A); 
   myassert(ret == 0, "ERREUR orchestre.c close tube anonyme\n");
}

//-----------------------------------------------------------------
// fermeture des tubes anonymes des services
static void close_tube_A_allS(int fd_A_SUM, int fd_A_COMP, int fd_A_SIGMA){
	close_tubeA(fd_A_SUM);
	close_tubeA(fd_A_COMP);
	close_tubeA(fd_A_SIGMA);
}
//-----------------------------------------------------------------
// lancement service
static void my_forkexecv(int *semaID_S, const char* fichier_SO, int ID_SO, int fd_A_SO[2], char *fdS2C, char* fdC2S, int numService)
{
	
	int retFork;
	key_t key;
	
	*semaID_S = my_semgetWithGetKey(fichier_SO, ID_SO, &key); 	// creation et initialisation semaphore pour Orchestre et Client
	open_tubeA(fd_A_SO); 	// création tube anonyme entre orchestre et service
	my_mkfifo(fdS2C, MODE_SC);	// création tubes nommés service client
	my_mkfifo(fdC2S, MODE_SC);	// MODE_SC = 0644 (cf client_service.h)
				
	retFork = fork();
  	myassert(retFork != -1, "orchestre.c, ERREUR startServices fork()\n");
  	if (retFork == 0)     // in fork
	{
		char *prog2exec = "SERVICE/service";
	
   	        // params : 0 = programme, 1 = numService (en string), 2 = clé du sema SO (en string), 3 = fd tube anonyme read[0] (en string), 4/5 = tube nommés service/client (cf. client_service.h), 6 = NULL (fin tableau)
   	        char *const params[] = {prog2exec, io_intToStr(numService), io_intToStr(key), io_intToStr(fd_A_SO[0]), fdC2S, fdS2C, NULL};
		close_tubeA(fd_A_SO[1]);	//close écriture pour le service
		execv(prog2exec, params);
		
		myassert(false, "Erreur orchestre.c execv service"); 	// normalement inutile
	}
	else
	{
	   close_tubeA(fd_A_SO[0]);	//close lecture pour l'orchestre
	}
}

//-----------------------------------------------------------------
// communication accès service
static void sendAccept(int fdCO_write, int *randPwd, int fd_A_SERVICE_WRITE, const char *pipe_s2c, const char *pipe_c2s){
	
	//     envoi au client d'un code d'acceptation (via le tube nommé)
	mywrite_int(fdCO_write, ACCEPTATION);   
		
	//     génération d'un mot de passe
	randPassword(randPwd);
	
	//     envoi d'un code de travail au service (via le tube anonyme)
	mywrite_int(fd_A_SERVICE_WRITE, 1);
	//     envoi du mot de passe au service (via le tube anonyme)
	mywrite_int(fd_A_SERVICE_WRITE, *randPwd);
	
	//     envoi du mot de passe au client (via le tube nommé)
	mywrite_int(fdCO_write, *randPwd);
	//     envoi des noms des tubes nommés au client (via le tube nommé)
	mywrite_str(fdCO_write, pipe_c2s);
	mywrite_str(fdCO_write, pipe_s2c);

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
    bool isUse_S_SUM, isUse_S_COMP, isUse_S_SIGMA;// bool état service (getval on semaphore)
    
    // var tubes
    int fdCO_read, fdCO_write; 	// tubes nommés entre Orchestre et Client
    //int fdSC_read, fdSC_write; 	// tubes nommés entre Service et Client
    
    int fd_A_SUM[2];    	//
    int fd_A_COMP[2];     	// tubes anonyme entre Orchestre et Service
    int fd_A_SIGMA[2];; 	//
	// [0] = read  | [1] = write
    
    // Autre
    int ret; // pour les assert
    int numService; 	// recupération numéro service envoyé par le client
    int randPassword;

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
    // Pour la communication avec les clients
    // - création de 2 tubes nommés pour converser avec les clients
    // - création d'un sémaphore pour que deux clients ne
    //   ne communiquent pas en même temps avec l'orchestre 
    
    // création tubes nommés vers les clients
    my_mkfifo(PIPE_O2C, MODE_CO);
    my_mkfifo(PIPE_C2O, MODE_CO);
    
    semaID_CO = my_semget(FICHIER_CO, ID_CO); 	// creation + init du semaphore Orchestre Client
    
    
/*-------------------------------------------------------------------------------------------------------------------------------------------*/    
    // lancement des services, avec pour chaque service :
    		// --> voir TP6 - exo8
    		
    // - création d'un tube anonyme pour converser (orchestre vers service)
    // - un sémaphore pour que le service préviene l'orchestre de la
    //   fin d'un traitement
    // - création de deux tubes nommés (pour chaque service) pour les
    //   communications entre les clients et les services
    
    // SERVICE_SOMME
    my_forkexecv(&semaID_SUM, FICHIER_SO, ID_SO_SUM, fd_A_SUM, PIPE_S2C_SUM, PIPE_C2S_SUM, SERVICE_SOMME);
    
    // SERVICE_COMPRESSION
    my_forkexecv(&semaID_COMP, FICHIER_SO, ID_SO_COMP, fd_A_COMP, PIPE_S2C_COMP, PIPE_C2S_COMP, SERVICE_COMPRESSION);
    
    // SERVICE_SIGMA
    my_forkexecv(&semaID_SIGMA, FICHIER_SO, ID_SO_SIGMA, fd_A_SIGMA, PIPE_S2C_SIGMA, PIPE_C2S_SIGMA, SERVICE_SIGMA);


/*-------------------------------------------------------------------------------------------------------------------------------------------*/ 
    while (! fin)
    {
        // ouverture ici des tubes nommés avec un client
        fdCO_read = open(PIPE_C2O, O_RDONLY);
        myassert(fdCO_read != -1, "Erreur orchestre.c, open write fdCO_write\n");
        fdCO_write = open(PIPE_O2C, O_WRONLY);
        myassert(fdCO_write != -1, "Erreur orchestre.c, open read fdCO_read\n");
        
        // attente d'une demande de service du client
        numService = myread_int(fdCO_read);
        printf("+1 requête client \n");

        // détecter la fin des traitements lancés précédemment via
        // les sémaphores dédiés (attention on n'attend pas la
        // fin des traitement, on note juste ceux qui sont finis)
        isUseServices(&isUse_S_SUM, semaID_SUM, &isUse_S_COMP, semaID_COMP, &isUse_S_SIGMA, semaID_SIGMA); 

        // analyse de la demande du client  
        // si ordre de fin
        //     envoi au client d'un code d'acceptation (via le tube nommé)
        //     marquer le booléen de fin de la boucle
        if(numService == SERVICE_ARRET){
        	mywrite_int(fdCO_write, SERVICE_ARRET);

            	fin = true; // arrêt boucle => fin service/client après la boucle
        }
	        
        // sinon si service non ouvert
        //     envoi au client d'un code d'erreur (via le tube nommé)
        /*else if(config_isServiceOpen(numService)){
        	mywrite_int(fdCO_write, NOT_STARTED_SERVICE);
        }*/
        // sinon si service déjà en cours de traitement
        //     envoi au client d'un code d'erreur (via le tube nommé)
        else if(numService == SERVICE_SOMME){
        	if(isUse_S_SUM){
        		mywrite_int(fdCO_write, ALREADY_USE_SERVICE); // déjà en cours de traitement
        	}else{
        		sendAccept(fdCO_write, &randPassword, fd_A_SUM[1], PIPE_S2C_SUM, PIPE_C2S_SUM); // acceptation
        	}
        }
        else if(numService == SERVICE_COMPRESSION){
        	if(isUse_S_COMP){
        		mywrite_int(fdCO_write, ALREADY_USE_SERVICE); // déjà en cours de traitement
        	}else{
        		sendAccept(fdCO_write, &randPassword, fd_A_COMP[1], PIPE_S2C_COMP, PIPE_C2S_COMP); // acceptation
        	}
        }
        else if(numService == SERVICE_SIGMA){
        	if(isUse_S_SIGMA){
        		mywrite_int(fdCO_write, ALREADY_USE_SERVICE); // déjà en cours de traitement
        	}else{
        		sendAccept(fdCO_write, &randPassword, fd_A_SIGMA[1], PIPE_S2C_SIGMA, PIPE_C2S_SIGMA); // acceptation
        	}
        }
        // sinon
        else{
        	//erreur pas de retour, on ne doit pas arriver ici
        	myassert(false, "ERREUR orchestre.c, aucun retour client sur service.");
        }
	// finsi

        // attente d'un accusé de réception du client
        ret = myread_int(fdCO_read);
        myassert(ret == ADR, "Erreur orchestre.c, read ADR\n");
        
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
    //isUseServices(&isUse_S_SUM, semaID_SUM, &isUse_S_COMP, semaID_COMP, &isUse_S_SIGMA, semaID_SIGMA); 	
	/*	    	ret = semctl(semaID_SIGMA, 0, GETVAL);
    	printf("service.c Valeur du sémaphore (semaID=%d) : %d\n", semaID_SIGMA, ret); */
    sem_Wait_S(semaID_SUM, semaID_COMP, semaID_SIGMA);

    // envoi à chaque service d'un code de fin
    write_To_S(fd_A_SUM[1], fd_A_COMP[1], fd_A_SIGMA[1], SERVICE_ARRET);
    

    // attente de la terminaison des processus services
    fork_Wait_allS();	

    // libération des ressources
    close_tube_A_allS(fd_A_SUM[1], fd_A_COMP[1], fd_A_SIGMA[1]);
    unlink_allNamedTube();
    my_destroy(semaID_CO);
    my_destroy(semaID_SUM);
    my_destroy(semaID_COMP);
    my_destroy(semaID_SIGMA);
    
    return EXIT_SUCCESS;
}
