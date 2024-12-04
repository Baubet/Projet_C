#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "io.h"
#include "memory.h"
#include "myassert.h"

#include "orchestre_service.h"
#include "client_service.h"

#include "service_compression.h"

// définition éventuelle de types pour stocker les données
struct stringCOMP{
    int len1;
    char* original;
    int len2;
    char* compressed;
};

typedef struct stringCOMP * PstringCOMP;


/*----------------------------------------------*
 * fonctions appelables par le service
 *----------------------------------------------*/

//-----------------------------------------------------------------
// lecture int dans un tube et attribution valeur driect
static void myread_intV2(int fdpipe, int *x)
{
    int ret;
    
    ret = read(fdpipe, x, sizeof(int));	
    myassert(ret != -1, "service_compression.c ERREUR ecriture pipe client service COMP (int).\n");
    myassert(ret == sizeof(int), "service_compression.c ERREUR ecriture pipe client service COMP (size(int)).\n");
}

// fonction de réception des données
static void receiveData(int fdCS_read, PstringCOMP data)
{
    // récupération de la longueur de la chaîne
    myread_intV2(fdCS_read, &data->len1);
    
    // récupération de la chaîne compressée
    myread_string(fdCS_read, data->len1, &data->original);	// ALLOCATION de data->original
}

// fonction de traitement des données
static void computeResult(PstringCOMP data)
{
    // verif reçu
    myassert(data->original != NULL, "erreur service_compression.c void original DATA");
    
    char *compressed = (char *)malloc(2 * data->len1 * sizeof(char));  // on alloue pour le pire cas (len1*2)
    myassert(compressed != NULL, "Erreur allocation dans le fichier service_compression.c ");

    int count = 1;  // Compteur nombre d'occurrences
    int idc = 0;    // Indice pour la chaîne compressée
    int len_occ;
    char *original = data->original;
    
    // Parcours de la chaîne 'original' à compresser.
    for (int i = 1; i <= data->len1; i++) {
        // Si on arrive à la fin ou que le caractère suivant est différent.
        if (i == data->len1 || original[i] != original[i-1]) {
        
            // Ajout nombre d'occurrences
            char *count_str = io_intToStrFormat("%d", count); 
            len_occ = strlen(count_str);
            for(int j = 0; j < len_occ; j++) {
                compressed[idc] = count_str[j];
                idc++;
            }
            
            // Ajout du caractère
            compressed[idc] = original[i-1]; 
            idc++;
            count = 1;
        } else {
            count++;
        }
    }
    
    // Mettre à jour les données compressées dans le structure
    data->compressed = compressed;
    data->len2 = idc;

}

// fonction d'envoi du résultat
static void sendResult(int fdCS_write, PstringCOMP data)
{
    // si les données reçus sont null
    myassert(data->compressed != NULL, "erreur service_compression.c void original DATA");
    
    // envoi de la chaîne compresser (et sa longueur pour la lecture)
    mywrite_str(fdCS_write, data->compressed); 
}


/*----------------------------------------------*
 * fonction appelable par le main
 *----------------------------------------------*/
void service_compression(int fdCS_read, int fdCS_write)
{
    // initialisations diverses
    PstringCOMP dataCOMP;
    dataCOMP = (PstringCOMP) malloc(sizeof(struct stringCOMP)); 
    myassert(dataCOMP != NULL, "Erreur allocation dans le fichier service_compression.c \n");
    dataCOMP->original = NULL;
    dataCOMP->compressed = NULL;
    
    receiveData(fdCS_read, dataCOMP);
    computeResult(dataCOMP);
    sendResult(fdCS_write, dataCOMP);

    // libération éventuelle de ressources
    MY_FREE(dataCOMP->original);
    MY_FREE(dataCOMP->compressed);
    MY_FREE(dataCOMP);
}
