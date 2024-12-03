#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#include <stddef.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "myassert.h"
#include "memory.h"
#include "io.h"


/*===================================================================*
 * manipulations de chaînes
 *===================================================================*/

int io_strToInt(const char *s)
{
    myassert(s != NULL, "Erreur chaîne NULL");
    myassert(strlen(s) > 0, "Erreur chaîne vide");

    long int tmp;
    char *end;

    errno = 0;     // voir man strtol
    tmp = strtol(s, &end, 10);
    myassert(errno == 0, "Erreur récupération int");
    myassert(*end == '\0', "Erreur chaîne partiellement lue");
    myassert(tmp >= INT_MIN && tmp <= INT_MAX, "Erreur overflow int");
    
    return (int) tmp;
}

char * io_intToStr(int i)
{
    return io_intToStrFormat("%d", i);
}

char * io_intToStrFormat(const char *format, int i)
{
    myassert(format != NULL, "chaîne inexistante");
    myassert(strstr(format, "%d") != NULL, " la chaîne \"%d\" n'est pas présente");

    char *s = NULL;
    int l = snprintf(NULL, 0, format, i);
    s = malloc((l+1) * sizeof(char));
    myassert(s != NULL, "Erreur allocation");
    sprintf(s, format, i);
    return s;
}

float io_strToFloat(const char *s)
{
    myassert(s != NULL, "Erreur chaîne NULL");
    myassert(strlen(s) > 0, "Erreur chaîne vide");

    float val;
    char *end;

    val = strtof(s, &end);
    //myassert(end != s, "aucun caractère n'a été consommé");
    myassert(! (val ==  HUGE_VALF && errno == ERANGE), "overflow positif");
    myassert(! (val == -HUGE_VALF && errno == ERANGE), "overflow négatif");
    myassert(! (val == 0.f && errno == ERANGE), "underflow");
    myassert(*end == '\0', "Erreur chaîne partiellement lue");
    
    return val;
}

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
    myassert(ret != -1, "ERREUR ecriture pipe client orchestre (int).\n");
    myassert(ret == sizeof(int), "ERREUR ecriture pipe client orchestre (int - size (int)).\n");
}

//-----------------------------------------------------------------
// Lecture entier dans un tube et renvoie
int myread_int(int fdpipe)
{
    int ret_read;
    int res;
   
    ret_read = read(fdpipe, &res, sizeof(int));
    myassert(ret_read != -1, "ERREUR lecture pipe client orchestre (int)\n");
    myassert(ret_read == sizeof(int), "ERREUR lecture pipe client orchestre (int - size(int))\n");

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
    myassert(ret != -1, "ERREUR mywrite_str écriture chaîne.\n");
    myassert(ret == len, "ERREUR mywrite_str écriture chaîne (len).\n");
}

//-----------------------------------------------------------------
// Lecture string 
void myread_string(int fdpipe, int len, char **pipe_name)
{
    int ret;

    MY_MALLOC(*pipe_name, char, len);

    ret = read(fdpipe, *pipe_name, sizeof(char) * len);
    myassert(ret != -1, "Erreur client.c, lecture pipe (string)"); 
    myassert(ret == len, "Erreur client.c, lecture pipe nbre octet (string)");
}


// uncomment to test
//#define IO_TESTING
#ifdef IO_TESTING

int main()
{
    int i;
    const char *s;

    s = "14";
    i = io_strToInt(s);
    printf("%d (%s)\n", i, s);
    s = "-14";
    i = io_strToInt(s);
    printf("%d (%s)\n", i, s);
    s = "2147483647";
    i = io_strToInt(s);
    printf("%d (%s)\n", i, s);
    s = "-2147483648";
    i = io_strToInt(s);
    printf("%d (%s)\n", i, s);
    
    //s = NULL;
    //i = io_strToInt(s);
    //printf("%d (%s)\n", i, s);
    
    //s = "";
    //i = io_strToInt(s);
    //printf("%d (%s)\n", i, s);

    //s = "14a";
    //i = io_strToInt(s);
    //printf("%d (%s)\n", i, s);

    //s = "2147483648";
    //i = io_strToInt(s);
    //printf("%d (%s)\n", i, s);

    //s = "9223372036854775808";
    //i = io_strToInt(s);
    //printf("%d (%s)\n", i, s);
    
    
    char *r;
    //r = io_intToStrFormat(NULL, 33);
    //r = io_intToStrFormat("====", 33);
    r = io_intToStrFormat("==%d==", 33);
    printf("chaîne : >>%s<<\n", r);
    free(r);
    r = io_intToStr(33);
    printf("chaîne : >>%s<<\n", r);
    free(r);
    
    printf("\n");
    float f;
    s = "3.14";
    f = io_strToFloat(s);
    printf("%g (%s)\n", f, s);
    s = "10.2e4";
    f = io_strToFloat(s);
    printf("%g (%s)\n", f, s);
    //s = NULL;         // chaine NULL
    //s = "";           // chaîne vide
    //s = "3.14aa";     // chaîne avec des caractère en trop
    //s = "1e+1000";    // overflow positif
    //s = "-1e+1000";   // overflow négatif
    //s = "1e-1000";    // underflow
    //f = io_strToFloat(s);
    //printf("%g (%s)\n", f, s);

    return EXIT_SUCCESS;
}


#endif
