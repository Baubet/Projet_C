#!/bin/bash

#------
# choix du compilateur
cc="gcc"

#------------
# liste des répertoires où chercher les .h
includeDirectories="-I. \
                    -I../UTILS \
                    -I../CONFIG \
                    -I../SERVICE \
                    -I../CLIENT_ORCHESTRE \
                    -I../ORCHESTRE_SERVICE"

#------------
# options classiques de compilation
cflags="-Wall -Wextra -pedantic -std=c99 -fPIC"

#------------
# options supplémentaires (les deux dernières uniquement lorsqu'il reste des fonctions non implémentées)
#cextraflags="-O3"
cextraflags="-g"
#cextraflags="-g -Wno-unused-parameter"
#cextraflags="-g -Wno-unused-parameter -Wno-unused-variable"

#------------
# source communes à plusieurs programmes
commun="../UTILS/myassert.c \
        ../UTILS/io.c \
        ../CONFIG/config.c \
        ../CLIENT_ORCHESTRE/client_orchestre.c \
        ../ORCHESTRE_SERVICE/orchestre_service.c"

#------------
# l'exécutable
exe="orchestre"

#------------
# la compilation proprement dite
$cc $includeDirectories $cflags $cextraflags -o $exe $exe.c $commun $clients

