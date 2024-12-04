#ifndef SERVICE_H
#define SERVICE_H

// Liste des services
// - la numérotation est commune aux orchestre, client, service, config
//   elle commence obligatoirement à 0 et est contiguë
// - le service -1 est particulier : il indique à l'orchestre de tout arrêter
// - si on rajoute/enlève un service il faut mettre tout le monde à jour

#define SERVICE_ARRET       -1

#define SERVICE_NB 3

#define SERVICE_SOMME        0
#define SERVICE_COMPRESSION  1
#define SERVICE_SIGMA        2


#endif
