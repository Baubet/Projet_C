#!/bin/bash

#------
# choix du compilateur
cc="gcc"

#------------
# liste des répertoires où chercher les .h
includeDirectories="-I. \
                    -I../UTILS \
                    -I../SERVICE \
                    -I../CLIENT_ORCHESTRE \
                    -I../CLIENT_SERVICE"

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
        ../CLIENT_ORCHESTRE/client_orchestre.c \
        ../CLIENT_SERVICE/client_service.c"

#------------
# liste des codes spécifiques à chaque client
clients="./client_arret.c \
         ./client_somme.c \
         ./client_compression.c \
         ./client_sigma.c"

#------------
# l'exécutable
exe="client"

#------------
# la compilation proprement dite
$cc $includeDirectories $cflags $cextraflags -o $exe $exe.c $commun $clients

