#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "io.h"
#include "memory.h"
#include "myassert.h"

#include "client_service.h"
#include "client_somme.h"


/*----------------------------------------------*
 * usage pour le client somme
 *----------------------------------------------*/

static void usage(const char *exeName, const char *numService, const char *message)
{
    fprintf(stderr, "Client somme de deux nombres\n");
    fprintf(stderr, "usage : %s %s <n1> <n2> <prefixe>\n", exeName, numService);
    fprintf(stderr, "        %s         : numéro du service\n", numService);
    fprintf(stderr, "        <n1>      : premier nombre à sommer\n");
    fprintf(stderr, "        <n2>      : deuxième nombre à sommer\n");
    fprintf(stderr, "        <prefixe> : chaîne à afficher avant le résultat\n");
    fprintf(stderr, "exemple d'appel :\n");
    fprintf(stderr, "    %s %s 22 33 \"le résultat est : \"\n", exeName, numService);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

/*----------------------------------------------*
 * fonction de vérification des paramètres
 *----------------------------------------------*/

void client_somme_verifArgs(int argc, char * argv[])
{
    if (argc != 5)
        usage(argv[0], argv[1], "nombre d'arguments");
    // éventuellement d'autres tests
}


/*----------------------------------------------*
 * fonctions de communication avec le service
 *----------------------------------------------*/

//-----------------------------------------------------------------
// écriture float dans un tube
static void mywrite_float(int fdpipe, float x)
{
    int ret;
    
    ret = write(fdpipe, &x, sizeof(float));	
    myassert(ret != -1, "client_somme.c ERREUR ecriture pipe client service SUM (float).\n");
    myassert(ret == sizeof(float), "client_somme.c ERREUR ecriture pipe client service SUM (size(float)).\n");
}

//-----------------------------------------------------------------
// écriture float dans un tube
static void myread_float(int fdpipe, float *x)
{
    int ret;
    
    ret = read(fdpipe, x, sizeof(float));	
    myassert(ret != -1, "client_somme.c ERREUR lecture pipe client service SUM (float).\n");
    myassert(ret == sizeof(float), "client_somme.c ERREUR lecture pipe client service SUM (size(float)).\n");
}

// ---------------------------------------------
// fonction d'envoi des données du client au service
// Les paramètres sont
// - le file descriptor du tube de communication vers le service
// - les deux float dont on veut la somme
static void sendData(int fd_write, float f1,  float f2)
{
    // envoi des deux nombres
    mywrite_float(fd_write, f1);
    mywrite_float(fd_write, f2);
}

// ---------------------------------------------
// fonction de réception des résultats en provenance du service et affichage
// Les paramètres sont
// - le file descriptor du tube de communication en provenance du service
// - le prefixe
// - autre chose si nécessaire
static void receiveResult(int fdRead, char * prefixe)
{
    // récupération de la somme
    float sum;
    myread_float(fdRead, &sum);
    
    // affichage du préfixe et du résultat
    printf("%s %f\n", prefixe, sum);
}


// ---------------------------------------------
// Fonction appelée par le main pour gérer la communications avec le service
// Les paramètres sont
// - les deux file descriptors des tubes nommés avec le service
// - argc et argv fournis en ligne de commande
// Cette fonction analyse argv et en déduit les données à envoyer
//    - argv[2] : premier nombre
//    - argv[3] : deuxième nombre
//    - argv[4] : chaîne à afficher avant le résultat
void client_somme(int fdCS_write, int fdCS_read, int argc, char * argv[])
{
    myassert(argc == 5, "erreur client_somme.c argc != 5");
    
    sendData(fdCS_write, io_strToFloat(argv[2]), io_strToFloat(argv[3]));
    receiveResult(fdCS_read, argv[4]);
}

