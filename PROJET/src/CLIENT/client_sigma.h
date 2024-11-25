#ifndef CLIENT_SIGMA
#define CLIENT_SIGMA

// on ne déclare ici que les fonctions appelables par le main

// vérifie les arguments : arrêt du programme en cas d'erreur
// - argc doit valoir 4 ou plus
// - argv[1] est le numéro du service
// - argv[2] est le nombre de threads que le service doit utiliser
// - argv[3] à argv[argc-1] : les float à envoyer au service
void client_sigma_verifArgs(int argc, char * argv[]);


// fonction pour gérer la communication avec le service
void client_sigma(/* fd des deux tubes avec le service, */ int argc, char * argv[]);

#endif
