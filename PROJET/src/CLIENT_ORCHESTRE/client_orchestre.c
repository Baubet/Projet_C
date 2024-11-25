#include "myassert.h"

#include "client_orchestre.h"
/* include rajouter */
#include <stddef.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>
/* ---------------- */

//-----------------------------------------------------------------
// On entre en SC et on est bloqué s'il y a trop de monde
void entrerSC(int semaID)
{
    int ret;
    // paramètres : num sépamhore, opération, flags
    struct sembuf operationMoins = {0, -1, 0};
    ret = semop(semaID, &operationMoins, 1);           // bloquant si sem == 0
    myassert(ret != -1, "ERREUR : entrerSC -> semop (orchestre/client)\n");
}

//-----------------------------------------------------------------
// On sort de la  SC
void sortirSC(int semaID)
{
    int ret;
    // paramètres : num sépamhore, opération, flags
    struct sembuf operationPlus = {0, 1, 0};
    ret = semop(semaID, &operationPlus, 1);
    myassert(ret != -1, "ERREUR : sortirSC -> semop (orchestre/client)\n");
}

//-----------------------------------------------------------------
// destruction du sémaphore
void my_destroy(int semaID)
{
    int ret;
    
    ret = semctl(semaID, -1, IPC_RMID);
    myassert(ret != -1, "orchestre.c - ERREUR : my_destroy().\n");
}

//-----------------------------------------------------------------
// écriture entier dans un tube
void mywrite_int(int fdpipe, int x)
{
    int ret;
    
    ret = write(fdpipe, &x, sizeof(int));	
    myassert(ret == sizeof(int), "ERREUR ecriture pipe client orchestre (int).\n");
}

//-----------------------------------------------------------------
// Lecture entier dans un tube et renvoie
int myread_int(int fdpipe)
{
    int ret_read;
    int res;
   
    ret_read = read(fdpipe, &res, sizeof(int));
    myassert(ret_read == sizeof(int), "ERREUR lecture pipe client orchestre (int)\n");

    return res;
}

//-----------------------------------------------------------------
// écriture len + string dans un tube
void mywrite_str(int fdpipe, const char *str)
{
    int ret;
    int len = strlen(str);  // Longueur de la chaîne à envoyer
    
    // Ecriture longueur de la chaîne dans le tube
    mywrite_int(fdpipe, len);

    // Ecriture de la chaîne elle-même
    ret = write(fdpipe, str, len);
    myassert(ret == len, "ERREUR mywrite_str écriture chaîne.\n");
}

