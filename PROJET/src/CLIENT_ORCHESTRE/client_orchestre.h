#ifndef CLIENT_ORCHESTRE_H
#define CLIENT_ORCHESTRE_H

// Ici toutes les communications entre l'orchestre et les clients :
// - le sémaphore pour que 2 clients ne conversent pas en même
//   temps avec l'orchestre
// - les deux tubes nommés pour la communication bidirectionnelle

//Information semaphore (utilisation ftok(FICHIER_CO, ID))
#define FICHIER_CO "CLIENT_ORCHESTRE/client_orchestre.h"
#define ID_CO 5
#define MODE_CO 0641


// tubes nommés
#define PIPE_O2C "pipe_o2c"  	// Orchestre vers Client
#define PIPE_C2O "pipe_c2o"	// Client vers Orchestre

void entrerSC(int semId);
void sortirSC(int semId);
void my_destroy(int semId); 	// détruit semaphore

void mywrite_int(int fdpipe, int x);	// écrit un entier par tube
int myread_int(int fdpipe); 		// lit un entier par tube
void mywrite_str(int fdpipe, const char *str); // écriture len + string par tube


#endif
