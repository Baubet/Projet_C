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


/*===================================================================*
 * manipulations de semaphores
 *===================================================================*/
void entrerSC(int semId);	// prendre semaphore
void sortirSC(int semId);	// vendre semaphore
void my_destroy(int semId); 	// détruit semaphore
void my_semwait(int semaID); 	// attente semaphore
void sem_Wait_S(int semaID_SUM, int semaID_COMP, int semaID_SIGMA); // pour tout les services
bool isUseService(int semaID); // bool sur l'état du service
void isUseServices(bool *isUse_S_SUM, int semaID_SUM, bool *isUse_S_COMP, int semaID_COMP, bool *isUse_S_SIGMA, int semaID_SIGMA); // enregistrement état service


/*===================================================================*
 * manipulations des tubes
 *===================================================================*/
void my_mkfifo(const char *pathname, mode_t mode); // création de tube nommé
void my_unlink(const char *pathname); 	//suppression de tube nommé
void unlink_allNamedTube();	//suppression de tout les tubes nommés

void open_tubeA(int fd_A[2]);	//ouverture tube anonyme
void close_tubeA(int fd_A);	//fermeture d'une extréminité tube anonyme
void close_tube_A_allS(int fd_A_SUM, int fd_A_COMP, int fd_A_SIGMA); // fermeture de tout les tubes anonyme

void mywrite_int(int fdpipe, int x);	// écrit un entier par tube
int myread_int(int fdpipe); 		// lecture entier par tube

void mywrite_str(int fdpipe, const char *str); // écriture len + string par tube
void write_To_S(int fd_A_SUM, int fd_A_COMP, int fd_A_SIGMA, int code); // pour tout les services

void myread_string(int fdpipe, int len, char **pipe_name); // lecture string par tube

#endif
