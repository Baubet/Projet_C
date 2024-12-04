/* Xavier BAUBET */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
/*include rajouter*/
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "io.h"
#include "memory.h"
#include "myassert.h"
 /* --- */
#include "orchestre_service.h"
#include "client_service.h"
#include "service.h"
#include "service_somme.h"
#include "service_compression.h"
#include "service_sigma.h"


static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <num_service> <clé_sémaphore> <fd_tube_anonyme> "
            "<nom_tube_service_vers_client> <nom_tube_client_vers_service>\n",
            exeName);
    fprintf(stderr, "        <num_service>     : entre 0 et %d\n", SERVICE_NB - 1);
    fprintf(stderr, "        <clé_sémaphore>   : entre ce service et l'orchestre (clé au sens ftok)\n");
    fprintf(stderr, "        <fd_tube_anonyme> : entre ce service et l'orchestre\n");
    fprintf(stderr, "        <nom_tube_...>    : noms des deux tubes nommés reliés à ce service\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------
// récupération d'un sémaphore avec la clé
static int my_semget(key_t key)	
{
    int semaID;
    
    semaID = semget(key, 1, 0);
    myassert(semaID != -1, "service.c ERREUR : semget in mysemget() \n");
    // pas d'initialisation

    return semaID;
}

/*----------------------------------------------*
 * fonction main
 *----------------------------------------------*/
int main(int argc, char * argv[])
{
    if (argc != 6)
        usage(argv[0], "nombre paramètres incorrect");

    // initialisations diverses : analyse de argv
    // Recap params (argv) : 0 = programme, 1 = numService (en string), 2 = semaID du service (en string), 3 = fd tube anonyme read [0] (en string), 4/5 = tube nommés service/client (cf. client_service.h), 6 = NULL (fin tableau)
    int ret;
    int numService = io_strToInt(argv[1]);	
    key_t key = io_strToInt(argv[2]);		// semaphore Service Orchestre
    int semaID_SO = my_semget(key);		// recup sémaphore
    int pipe_A_SO_read = io_strToInt(argv[3]);	// pipe anonyme Service Orchestre
    char* pipe_C2S = argv[4];			// nom tubes nommés service client
    char* pipe_S2C = argv[5];			// "
    int fdS2C_write, fdC2S_read;
    int code_orchestre;
    int passwordByO;
    int passwordByC;
    bool fin = false;
   	
    
    

    while (!fin)
    {
        // attente d'un code de l'orchestre (via tube anonyme)
        code_orchestre = myread_int(pipe_A_SO_read);
        
        // si code de fin
        if(code_orchestre == SERVICE_ARRET){
           	printf(" Service.c, code SERVICE_ARRET -> Arret service %d\n", numService);
           	//    sortie de la boucle
           	fin = true;
        }// sinon
        else{
        	//on préviens l'orchestre qu'on est plus disponible
    		entrerSC(semaID_SO);
    		
        	//    réception du mot de passe de l'orchestre
        	passwordByO = myread_int(pipe_A_SO_read);
		//    ouverture des deux tubes nommés avec le client
		fdC2S_read = open(pipe_C2S, O_RDONLY);
		myassert(fdC2S_read != -1, "Erreur service.c, open read fdC2S\n");
		fdS2C_write = open(pipe_S2C, O_WRONLY);
		myassert(fdS2C_write != -1, "Erreur service.c, open write fdS2C\n");
		//    attente du mot de passe du client
   		passwordByC = myread_int(fdC2S_read);
   		
		//    si mot de passe incorrect
		if(passwordByO != passwordByC){
			//        envoi au client d'un code d'erreur
			mywrite_int(fdS2C_write, WRONG_PASSWORD);
		}//    sinon
		else{
			//        envoi au client d'un code d'acceptation
			mywrite_int(fdS2C_write, ACCEPTATION);
			//        appel de la fonction de communication avec le client :
			//            une fct par service selon numService (cf. argv[1]) :
			//                   . service_somme
			if(numService == SERVICE_SOMME){
				service_somme(fdC2S_read, fdS2C_write);
			}
			//                ou . service_compression
			if(numService == SERVICE_COMPRESSION){
				service_compression(fdC2S_read, fdS2C_write);
			}
			//                ou . service_sigma
			if(numService == SERVICE_SIGMA){
				//service_sigma(fdC2S_read, fdS2C_write);
				printf("service.c : Service non fonctionnel\n");
			}
			//        attente de l'accusé de réception du client
			ret = myread_int(fdC2S_read);
        		myassert(ret == 0, "Erreur service.c, wrong ADR (!= 0)\n");
		
		}//    finsi
		
		//    fermeture ici des deux tubes nommés avec le client
		ret = close(fdC2S_read);
		myassert(ret != -1, "Erreur service.c, close fdC2S_read");
		ret = close(fdS2C_write);
		myassert(ret != -1, "Erreur service.c, close fdS2C_write");
		
		
		//    modification du sémaphore pour prévenir l'orchestre de la fin
		sortirSC(semaID_SO);
	}
        // finsi
        
    }
    // libération éventuelle de ressources
    close(pipe_A_SO_read);
    
    return EXIT_SUCCESS;
}
