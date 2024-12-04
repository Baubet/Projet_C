#ifndef CLIENT_ARRET
#define CLIENT_ARRET

// on ne déclare ici que les fonctions appelables par le main

// vérifie les arguments : arrêt du programme en cas d'erreur
// - argc doit valoir 2
// - argv[1] est le numéro du service, à savoir -1
void client_arret_verifArgs(int argc, char * argv[]);

#endif
