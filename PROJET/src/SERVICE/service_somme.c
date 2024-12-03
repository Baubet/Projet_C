#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "io.h"
#include "memory.h"
#include "myassert.h"

#include "orchestre_service.h"
#include "client_service.h"

#include "service_somme.h"

// définition éventuelle de types pour stocker les données
struct FloatSum{
    float f1;
    float f2;
    float sum;
};

typedef struct FloatSum * FloatSumP;

/*----------------------------------------------*
 * fonctions appelables par le service
 *----------------------------------------------*/
//-----------------------------------------------------------------
// écriture float dans un tube
static void mywrite_float(int fdpipe, float x)
{
    int ret;
    
    ret = write(fdpipe, &x, sizeof(float));	
    myassert(ret != -1, "service_somme.c ERREUR ecriture pipe client service SUM (float).\n");
    myassert(ret == sizeof(float), "service_somme.c ERREUR ecriture pipe client service SUM (size(float)).\n");
}

//-----------------------------------------------------------------
// lecture float dans un tube
static void myread_float(int fdpipe, float *x)
{
    int ret;
    
    ret = read(fdpipe, x, sizeof(float));	
    myassert(ret != -1, "service_somme.c ERREUR ecriture pipe client service SUM (float).\n");
    myassert(ret == sizeof(float), "service_somme.c ERREUR ecriture pipe client service SUM (size(float)).\n");
}

// fonction de réception des données
static void receiveData(int fdCS_read, FloatSumP dataFSum)
{
	myread_float(fdCS_read, &dataFSum->f1);
	myread_float(fdCS_read, &dataFSum->f2);
}

// fonction de traitement des données
static void computeResult(FloatSumP dataFSum)
{
	dataFSum->sum = dataFSum->f1 + dataFSum->f2;
}

// fonction d'envoi du résultat
static void sendResult(int fdCS_write, FloatSumP dataFSum)
{
	mywrite_float(fdCS_write, dataFSum->sum);
}


/*----------------------------------------------*
 * fonction appelable par le main
 *----------------------------------------------*/
void service_somme(int fdCS_read, int fdCS_write)
{
    // initialisations diverses
    FloatSumP dataFSum;
    //MY_MALLOC(dataFSum, FloatSum, 1);
    dataFSum = (FloatSumP) malloc(sizeof(struct FloatSum)); 
    myassert(dataFSum != NULL, "Erreur allocation dans le fichier service_somme.c a la ligne 84 \n");
    
    receiveData(fdCS_read, dataFSum);
    computeResult(dataFSum);
    sendResult(fdCS_write, dataFSum);

    // libération éventuelle de ressources
    MY_FREE(dataFSum);
}
