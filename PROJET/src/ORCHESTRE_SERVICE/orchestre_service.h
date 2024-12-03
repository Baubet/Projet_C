#ifndef ORCHESTRE_SERVICE_H
#define ORCHESTRE_SERVICE_H

// Ici toutes les communications entre l'orchestre et les services :
// - le tube anonyme pour que l'orchestre envoie des données au service
// - le sémaphore pour que  le service indique à l'orchestre la fin
//   d'un traitement

#define FICHIER_SO "ORCHESTRE_SERVICE/orchestre_service.c"
#define ID_SO_SUM 6 
#define ID_SO_COMP 7
#define ID_SO_SIGMA 8

#define MODE_SO 0641 

#endif
