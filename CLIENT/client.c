/* BAUBET */

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

#include "service.h"
#include "client_orchestre.h"
#include "client_service.h"

#include "client_arret.h"
#include "client_somme.h"
#include "client_sigma.h"
#include "client_compression.h"

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <num_service> ...\n", exeName);
    fprintf(stderr, "        <num_service> : entre -1 et %d\n", SERVICE_NB - 1);
    fprintf(stderr, "                        -1 signifie l'arrêt de l'orchestre\n");
    fprintf(stderr, "        ...           : les paramètres propres au service\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------
// récupération d'un sémaphore -> il doit déjà exister (création par l'orchestre)
// voir TP7 - exo8_worker.c
static int my_semget()	
{
    key_t key;
    int semaID;

	// récupération de la clé
    key = ftok(FICHIER_CO, ID_CO);
    myassert(key != -1, "client.c ERREUR : ftok in mysemget() \n");
    
    semaID = semget(key, 1, 0);
    myassert(semaID != -1, "client.c ERREUR : semget in mysemget() \n");
    // pas d'initialisation

    return semaID;
}


int main(int argc, char * argv[])
{
    if (argc < 2)
        usage(argv[0], "nombre paramètres incorrect");

    int numService = io_strToInt(argv[1]);
    if (numService < -1 || numService >= SERVICE_NB)
        usage(argv[0], "numéro service incorrect");

    // appeler la fonction de vérification des arguments
    //     une fct par service selon numService
    //            . client_arret_verifArgs
    //         ou . client_somme_verifArgs
    //         ou . client_compression_verifArgs
    //         ou . client_sigma_verifArgs
    
    if(numService == SERVICE_ARRET){ //== -1
        client_arret_verifArgs(argc, argv);
    }
    else if(numService == SERVICE_SOMME){ //== 0
        client_somme_verifArgs(argc, argv);
    }
    else if(numService == SERVICE_COMPRESSION){ // == 1
        client_compression_verifArgs(argc, argv);
    }
    else if(numService == SERVICE_SIGMA){ // == 2
        client_sigma_verifArgs(argc, argv);
    }else{
    	return EXIT_FAILURE;
    }

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
    // initialisations diverses s'il y a lieu
    // var sema
    int semaID_CO;
    semaID_CO = my_semget(); 	// récupération du sémaphore créer par l'orchestre
    
    // var tubes
    int fdCO_read, fdCO_write; 	// tubes communication entre Orchestre et Client
    int fdS2C_read, fdC2S_write;// tubes communication entre Service et Client
    char* pipe_C2S = NULL;
    char* pipe_S2C = NULL;
    
    // Autre
    int ret; 		// pour les assert
    int state_serv;   	// récupération état du service (par l'orchestre)
    int serv_adr;	// récupération adc du service 
    int passwordByO;  	// récupération du password pour accès service
    int len_char_ret; 

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
    // entrée en section critique pour communiquer avec l'orchestre
    entrerSC(semaID_CO);
    
    // ouverture des tubes avec l'orchestre
    		// --> il faut bien que les deux "open" soit inverser par rapport à ceux de l'orchestre (attente mutuelle)
    		//  TP6 - exo_10 - worker.c
    fdCO_write = open(PIPE_C2O, O_WRONLY);
    myassert(fdCO_write != -1, "Erreur client.c, open write fdCO_write\n");
    fdCO_read = open(PIPE_O2C, O_RDONLY);
    myassert(fdCO_read != -1, "Erreur client.c, open read fdCO_read\n");
    
    // envoi à l'orchestre du numéro du service	
    		// TP6 - exo_8 - ecrivain.c
    mywrite_int(fdCO_write, numService);
    
    // attente code de retour
    		// TP6 - exo_8 - lecteur.c
    state_serv = myread_int(fdCO_read);
    
    // si code d'erreur
    //     afficher un message erreur
    if(state_serv == NOT_STARTED_SERVICE){ //Service non lancé
    	printf("client.c State service (-2) : service not start.\n"); 
    } 
    else if(state_serv == ALREADY_USE_SERVICE){ // Service déjà en cours d'exécution
    	printf("client.c State service (-3) : service already working.\n");  
    }
    // sinon si demande d'arrêt (i.e. numService == -1)
    //     afficher un message
    else if(state_serv == SERVICE_ARRET){
    	printf("client.c - SERVICE_ARRET(-1) : l'orchestre va s'arrêter.\n"); 
    }
    // sinon
    //     récupération du mot de passe et des noms des 2 tubes
    else{ // state_serv == ACCEPTATION
    	// lecture mot de passe service
    	passwordByO = myread_int(fdCO_read);
    	
    	//lecture des len nom + nom des pipes (avec allocation)
    	len_char_ret = myread_int(fdCO_read);
	myread_string(fdCO_read, len_char_ret, &pipe_C2S);
	//
    	len_char_ret = myread_int(fdCO_read);
	myread_string(fdCO_read, len_char_ret, &pipe_S2C);
    }
    // finsi
    
 /*-------------------------------------------------------------------------------------------------------------------------------------------*/   
    // envoi d'un accusé de réception à l'orchestre
    mywrite_int(fdCO_write, ADR);
    
    // fermeture des tubes avec l'orchestre
    ret = close(fdCO_write);
    myassert(ret != -1, "Erreur client.c, close fdCO_write");
    ret = close(fdCO_read);
    myassert(ret != -1, "Erreur client.c, close fdCO_read");
    
    // on prévient l'orchestre qu'on a fini la communication (cf. orchestre.c)

    // sortie de la section critique + débloque orchestre en attente avec opération {0, 0, 0} 
        sortirSC(semaID_CO);
	
    ////////////// FIN DES COMMUNICATIONS AVEC L'ORCHESTRE //////////////
/*-------------------------------------------------------------------------------------------------------------------------------------------*/

    // si pas d'erreur et service normal
    if(state_serv == ACCEPTATION){ 
    	//     ouverture des tubes avec le service
  
	fdC2S_write = open(pipe_C2S, O_WRONLY);
	myassert(fdC2S_write != -1, "Erreur client.c, open write fdC2S_write\n");
	fdS2C_read = open(pipe_S2C, O_RDONLY);
	myassert(fdS2C_read != -1, "Erreur client.c, open read fdS2C_read\n");
	    
	//     envoi du mot de passe au service
	mywrite_int(fdC2S_write, passwordByO);
	//     attente de l'accusé de réception du service
   	serv_adr = myread_int(fdS2C_read);

	//     si mot de passe non accepté
	if(serv_adr == WRONG_PASSWORD){
		//         message d'erreur
		printf("Erreur client.c, mot de passe non acepter par le service");
	}
	else{ 
		if(serv_adr == ACCEPTATION){ //code d'acceptation (cf service.c)
			//         appel de la fonction de communication avec le service :
			//             une fct par service selon numService :
			//                    . client_somme
			if(numService == SERVICE_SOMME){
				client_somme(fdC2S_write, fdS2C_read, argc, argv);
			}
			//                 ou . client_compression
			else if(numService == SERVICE_COMPRESSION){
				client_compression(fdC2S_write, fdS2C_read, argc, argv);
			}
			//                 ou . client_sigma
			else if(numService == SERVICE_SIGMA){
				//client_sigma(fdC2S_write, fdS2C_read, argc, argv);
			}
		}
		//         envoi d'un accusé de réception au service
		mywrite_int(fdC2S_write, 0);
	}//     finsi
	
	//     fermeture des tubes avec le service
	ret = close(fdC2S_write);
	myassert(ret != -1, "Erreur client.c, close fdC2S_write");
	ret = close(fdS2C_read);
	myassert(ret != -1, "Erreur client.c, close fdS2C_read");
	
	MY_FREE(pipe_C2S);	// cf. myread_string use in l150-160
	MY_FREE(pipe_S2C);	// cf. "                         "
    }
    // finsi ////////////// FIN DES COMMUNICATIONS AVEC LE SERVICE //////////////

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
    // libération éventuelle de ressources
    // cf. l 234/235
    
    return EXIT_SUCCESS;
}
