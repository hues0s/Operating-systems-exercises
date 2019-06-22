#/bin/bash

# 1) Comprobar que el mytar está en el directorio actual y que es ejecutable. Si no está, mostrar un mensaje informando del error y terminará.
# 2) Comprobar si existe un directorio tmp dentro del directorio actual. Si existe lo borrará , incluyendo todo lo que tenga dentro
# 3) Crear un nuevo directorio tmp dentro del directorio actual y cambiar a este nuevo directorio.
# 4) Crear 3 ficheros: MIRAR EN EL ENUNCIADO
# 5) Invocar al programa mytar para crear un fichero filetar.mtar con el contenido de los tres ficheros anteriores.
# 6) Crear un directorio out y ejecutar el programa mytar para extraer el contenido del tarball
# 7) Usar el programa diff para comprobar los ficheros extraídos con los originales, que estarán en el directorio anterior (..)
# 8) Si los tres ficheros extraídos son iguales que los originales, volverá al directorio original (../..), mostrará el mensaje “Correct” por pantalla y
#	 devolverá 0. Si hay algún error, volverá al directorio original, mostrará un mensaje descriptivo por pantalla y devolverá 1.

directory=$(pwd)

if [[ -e $directory/mytar && -x $directory/mytar && -f $directory/mytar ]]; then

	if [[ -d $directory/tmp && -e $directory/tmp ]]; then
		rm -r /tmp
	fi

	mkdir tmp
	cd tmp/

	echo "Hello World!" > file1.txt
	head -n 10 /etc/passwd > file2.txt
	head -c 1024 /dev/urandom > file3.dat

	# Aqui es para mostrar mensaje de carga de creacion de mtar (si existe en el directorio y es ejecutable)
	if [[ -e $directory/load.sh && -x $directory/load.sh && -f $directory/load.sh ]]; then
		../load.sh
	fi

	../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat

	directory=$(pwd)
	mkdir $directory/out
	cp filetar.mtar $directory/out

	cd $directory/out
	../../mytar -x -f filetar.mtar

	if  diff ../file1.txt file1.txt  && diff ../file2.txt file2.txt && diff ../file3.dat file3.dat; then
		echo "Correct: all the files have been extracted properly"
	else
		echo "Incorrect: any of the files differ from the original ones"
		exit 1
	fi

	cd ..

else
	echo "El programa mytar no esta en el directorio actual y/o no tiene permisos de ejecucion"
	exit 1
fi

exit 0

## La diferencia entre "exit" y "return" esque con el "exit" terminamos la ejecucion del proceso (este script) y con "return" retornamos desde la funcion actual