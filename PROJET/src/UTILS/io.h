#ifndef IO_H
#define IO_H

/*********************************************************************
 * manipulations générales sur les entrées/sorties :
 * - file descriptors
 * - conversions diverses
 * - ...
 *********************************************************************/

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

#endif
