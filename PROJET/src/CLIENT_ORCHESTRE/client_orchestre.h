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

// ACCUSE DE RECEPTION (ADR)
#define ADR 8


#endif
