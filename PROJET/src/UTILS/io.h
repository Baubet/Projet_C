#ifndef IO_H
#define IO_H

/*********************************************************************
 * manipulations générales sur les entrées/sorties :
 * - file descriptors
 * - conversions diverses
 * - ...
 *********************************************************************/
 
//Etat du service demandé
#define ACCEPTATION 1
#define NOT_STARTED_SERVICE -2
#define ALREADY_USE_SERVICE -3
#define WRONG_PASSWORD -8

/*===================================================================*
 * manipulations de chaînes
 *===================================================================*/

int io_strToInt(const char *s);
// la chaîne renvoyée est allouée dynamiquement
char * io_intToStr(int i);
// la chaine format doit contenir "%d" à l'endroit où doit être inséré i
// la chaîne renvoyée est allouée dynamiquement
char * io_intToStrFormat(const char *format, int i);

float io_strToFloat(const char *s);

void entrerSC(int semId);
void sortirSC(int semId);
void my_destroy(int semId); 	// détruit semaphore
//void my_semwait(int semaID);  	// attente semaphore

void mywrite_int(int fdpipe, int x);	// écrit un entier par tube
int myread_int(int fdpipe); 		// lecture entier par tube
void mywrite_str(int fdpipe, const char *str); // écriture len + string par tube
void myread_string(int fdpipe, int len, char **pipe_name); // lecture string par tube

#endif
