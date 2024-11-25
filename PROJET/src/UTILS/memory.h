/*****************************************************************************
 * module :  entete les macros d'allocation/desallocation                    *
 *                                                                           *
 * author : Gilles Subrenat                                                  *
 *****************************************************************************/

#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>

/*
 * Commentaires pédagogiques :
 * pourquoi faire des macros plutot que des fonctions ?
 * - parce qu'on utilise les macros predefinies __LINE__ et __FILE__
 * - parce qu'on passe des types en parametre
 * A quoi sert le "while(0)" ?
 * - a pouvoir appeler la macro avec un ";" a la fin sans que cela
 *   ne compte pour deux instructions
 */

/*
 * Les quatre macros MY_MALLOC, MY_REALLOC, MY_CALLOC et MY_FREE appellent
 * les fonctions C de memes noms (sans le MY_ et en minuscules) et stoppent
 * le programme, avec des explications détaillées, si la demande echoue.
 */


#define MY_MALLOC(pt, type, nbElt) \
    do { \
        (pt) = (type *) malloc((nbElt) * sizeof(type)); \
        if ((pt) == NULL) { \
            fprintf(stderr, \
                    "Erreur allocation dans le fichier %s a la ligne %d " \
                    "dans la fonction %s :\n", \
                    __FILE__, __LINE__, __func__); \
            fprintf(stderr, \
                    "    on voulait allouer %s elements de type %s\n", \
                    #nbElt, #type); \
            abort(); \
        } \
    } while(0)


#define MY_REALLOC(pt, ptInit, type, nbElt) \
    do { \
        (pt) = (type *) realloc((ptInit), (nbElt) * sizeof(type)); \
        if ((pt) == NULL) { \
            fprintf(stderr, \
                    "Erreur reallocation dans le fichier %s a la ligne %d " \
                    "dans la fonction %s :\n", \
                    __FILE__, __LINE__, __func__); \
            fprintf(stderr, \
                    "    on voulait allouer %s elements de type %s\n", \
                    #nbElt, #type); \
            abort(); \
        } \
    } while(0)


#define MY_CALLOC(pt, type, nbElt) \
    do { \
        (pt) = (type *) calloc((nbElt), sizeof(type)); \
        if ((pt) == NULL) { \
            fprintf(stderr, \
                    "Erreur allocation dans le fichier %s a la ligne %d " \
                    "dans la fonction %s :\n", \
                    __FILE__, __LINE__, __func__); \
            fprintf(stderr, \
                    "    on voulait allouer %s elements de type %s\n", \
                    #nbElt, #type); \
            abort(); \
        } \
    } while(0)


/*
 * MY_FREE teste si le pointeur est NULL avant l'appel a free, bien que
 * ce ne soit plus/pas nécessaire (on peut faire un free sur NULL est
 * autorisé). Le pointeur liberé est mis a NULL.
 */
#define MY_FREE(pt) \
    do { \
        if ((pt) != NULL) { \
            free(pt); \
            (pt) = NULL; \
        } \
    } while (0)

#endif
