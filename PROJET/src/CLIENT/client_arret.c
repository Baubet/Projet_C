#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "client_arret.h"


/*----------------------------------------------*
 * usage pour le client arrêt
 *----------------------------------------------*/

static void usage(const char *exeName, const char *numService, const char *message)
{
    fprintf(stderr, "Client : arrêt de l'orchestre\n");
    fprintf(stderr, "usage : %s %s\n", exeName, numService);
    fprintf(stderr, "seul appel possible :\n");
    fprintf(stderr, "    %s %s\n", exeName, numService);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

/*----------------------------------------------*
 * fonction de vérification des paramètres
 *----------------------------------------------*/

void client_arret_verifArgs(int argc, char * argv[])
{
    if (argc != 2)
        usage(argv[0], argv[1], "nombre d'arguments");
}
