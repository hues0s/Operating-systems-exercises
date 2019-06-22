#!/bin/bash

SF="./mount-point"
copiasTemporales="./temp"
virtualDisk="virtual-disk"
auditor="./my-fsck-static"
tamBlock=4096

originalFile1="./src/fuseLib.c"
copyFile1="./mount-point/fuseLib.c"

originalFile2="./src/myFS.h"
copyFile2="./mount-point/myFS.h"

originalFile3="./tercero.txt"
copyFile3="./mount-point/tercero.txt"


if [[ -d ./temp && -e ./temp ]]; then
	rm -r ./temp
fi

mkdir ./temp

# a)
cp $originalFile1 $copiasTemporales/
cp $originalFile1 $SF/
cp $originalFile2 $copiasTemporales/
cp $originalFile2 $SF/

# b)
echo ""
$auditor $virtualDisk
if [[ $(diff $originalFile1 $copyFile1)="" ]]; then
	echo "LOS FICHEROS $originalFile1 Y $copyFile1 SON IGUALES."
else
	echo "LOS FICHEROS $originalFile1 Y $copyFile1 SON DIFERENTES!!!"
fi
if [[ $(diff $originalFile2 $copyFile2)="" ]]; then
	echo "LOS FICHEROS $originalFile2 Y $copyFile2 SON IGUALES."
else
	echo "LOS FICHEROS $originalFile2 Y $copyFile2 SON DIFERENTES!!!"
fi
TAM_FILE1=$(stat -c%s $originalFile1)
DIF=$(($TAM_FILE1 - $tamBlock))
truncate --size=$DIF $copyFile1
truncate --size=$DIF ./temp/fuseLib.c ## truncate --size=DIF $copiasTemporale/fuseLib.c

# c)
read -p "PULSE ENTER PARA CONTINUAR..."
echo ""
$auditor $virtualDisk
if [[ $(diff $originalFile1 $copyFile1)="" ]]; then ## También valdría "diff $originalFile1 ./temp/fuseLib.c" pues fuseLib.c esta truncado en temp y  en nuestro SF
	echo "TRAS TRUNCARLOS, LOS FICHEROS $originalFile1 Y $copyFile1 SON IGUALES."
else
	echo "TRAS TRUNCARLOS, LOS FICHEROS $originalFile1 Y $copyFile1 SON DIFERENTES!!!"
fi

# d)
echo "Esto es el fichero numero 3." > $originalFile3
cp $originalFile3 $copyFile3

# e)
read -p "PULSE ENTER PARA CONTINUAR..."
echo ""
$auditor $virtualDisk
if [[ $(diff $originalFile3 $copyFile3)="" ]]; then
	echo "LOS FICHEROS $originalFile3 Y $copyFile3 SON IGUALES."
else
	echo "LOS FICHEROS $originalFile3 Y $copyFile3 SON DIFERENTES!!!"
fi

# f)
TAM_FILE2=$(stat -c%s $originalFile2)
SUM=$(($TAM_FILE2 + $tamBlock))
truncate --size=$SUM $copyFile2
truncate --size=$SUM ./temp/myFS.h

# g)
read -p "PULSE ENTER PARA CONTINUAR..."
echo ""
$auditor $virtualDisk
if [[ $(diff $originalFile2 $copyFile2)="" ]]; then
	echo "TRAS TRUNCARLOS, LOS FICHEROS $originalFile2 Y $copyFile2 SON IGUALES."
else
	echo "TRAS TRUNCARLOS, LOS FICHEROS $originalFile2 Y $copyFile2 SON DIFERENTES!!!"
fi
echo ""
read -p "PULSE ENTER PARA FINALIZAR..."