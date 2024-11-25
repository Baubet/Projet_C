#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#include "myassert.h"

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
