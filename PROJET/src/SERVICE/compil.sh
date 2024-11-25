#!/bin/bash

#------
# choix du compilateur
cc="gcc"

#------------
# liste des répertoires où chercher les .h
includeDirectories="-I. \
                    -I../UTILS \
                    -I../CONFIG \
                    -I../ORCHESTRE_SERVICE \
                    -I../CLIENT_SERVICE"

#------------
# options classiques de compilation
cflags="-Wall -Wextra -pedantic -std=c99 -fPIC"

#------------
# bibliothèques
ldflags="-lpthread"

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
        ../ORCHESTRE_SERVICE/orchestre_service.c \
        ../CLIENT_SERVICE/client_service.c"

#------------
# liste des codes spécifiques à chaque client
services="./service_somme.c \
          ./service_compression.c \
          ./service_sigma.c"

#------------
# l'exécutable
exe="service"

#------------
# la compilation proprement dite
$cc $includeDirectories $cflags $cextraflags -o $exe $exe.c $commun $services $ldflags

