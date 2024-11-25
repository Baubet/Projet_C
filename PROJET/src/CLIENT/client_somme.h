#ifndef CLIENT_SOMME
#define CLIENT_SOMME

// on ne déclare ici que les fonctions appelables par le main

// vérifie les arguments : arrêt du programme en cas d'erreur
// - argc doit valoir 5
// - argv[1] est le numéro du service
// - argv[2] est le premier nombre à sommer
// - argv[3] est le deuxième nombre à sommer
// - argv[4] est une chaîne à afficher avec le résultat
void client_somme_verifArgs(int argc, char * argv[]);


// fonction pour gérer la communication avec le service
void client_somme(/* fd des deux tubes avec le service, */ int argc, char * argv[]);

#endif
