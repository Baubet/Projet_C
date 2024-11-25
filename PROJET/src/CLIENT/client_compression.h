#ifndef CLIENT_COMPRESSION
#define CLIENT_COMPRESSION

// on ne déclare ici que les fonctions appelables par le main

// vérifie les arguments : arrêt du programme en cas d'erreur
// - argc doit valoir 3
// - argv[1] est le numéro du service
// - argv[2] est la chaîne à compresser
void client_compression_verifArgs(int argc, char * argv[]);


// fonction pour gérer la communication avec le service
void client_compression(/* fd des deux tubes avec le service, */ int argc, char * argv[]);

#endif
