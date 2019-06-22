#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	int iterator = 0; //Number of bytes copied
	while(iterator < nBytes && feof(origin) == 0){
		int lectura = getc(origin);
		if(lectura != EOF){
			int resultadoEscritura = putc(lectura, destination);
			if(resultadoEscritura == EOF) return -1;
			++iterator;
		}
	}
	return iterator;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{
	unsigned char lectura = '1';
	int longitudString = 0;
	while (lectura != '\0' && feof(file) == 0){
		lectura = getc(file);
		if(lectura == EOF) return NULL;
		++longitudString;
	}

	char *p = malloc(longitudString + 1);
	if(fseek(file, - longitudString, SEEK_CUR) == -1){
		return NULL;
	}
	for(int i = 0; i < longitudString; ++i){
		lectura = getc(file);
		if(lectura == EOF){
			return NULL;
		}
		p[i] = lectura;
	}
	p[longitudString] = '\0';
	return p;
}

/**
* Sirve para leer el tamaño de cada stHeaderEntry
*/
unsigned int*
loadint(FILE * tarFile)
{
	unsigned int * lectura = malloc(sizeof(unsigned int));
	fread(lectura, sizeof(unsigned int), 1, tarFile);
	return lectura;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	//Primero leemos los 4 bytes del entero que indica el numero de archivos almacenados en el tarball
	fread(nFiles, sizeof(unsigned int), 1, tarFile);

	//Ahora que conocemos el numero de ficheros que va a contener el archivo tarball, procedemos a leer los pares
	stHeaderEntry * ptrArrayPares = malloc((*nFiles) * sizeof(stHeaderEntry));
	for(int i = 0; i < *nFiles; ++i){
		//leemos los pares y los almacenamos en orden partiendo de la direccion de memoria
		char * p1 = loadstr(tarFile);
		if(p1 == NULL) return NULL;
		ptrArrayPares[i].name = p1;

		unsigned int * p2 = loadint(tarFile);
		if(p2 == NULL) return NULL;
		ptrArrayPares[i].size = *p2;
	}
	return ptrArrayPares;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	
	//Abrimos el fichero mtar
	FILE * file = fopen(tarName, "w");
	if(file == NULL) return EXIT_FAILURE;
	
	//Reservamos memoria para el array de headers
	stHeaderEntry * arrayHeader = malloc(nFiles * sizeof(stHeaderEntry));

	
	//Movemos el puntero de escritura en el fichero a la posicion donde comenzaremos a escribir
	// el contenido de los ficheros

	int charCount = 0;
	for(int i = 0; i < nFiles; ++i){ //Contamos cuantos byte ocupan los titulos
		int counter = 0;
		char lectura = 'l';
		while(lectura != '\0'){
			lectura = *(fileNames[i]  + counter);
			++counter;
			++charCount;
		}
	}

	long offset = sizeof(int) + nFiles * sizeof(unsigned int) + charCount;
	if(fseek(file, offset, SEEK_SET) != 0) return EXIT_FAILURE;

	//Copiamos el contenido de los ficheros 1 a 1, y rellenamos el array en memoria de stHeaderEntry
	for(int i = 0; i < nFiles; ++i){
		FILE * fOrigin = fopen(fileNames[i], "r");
		if(fOrigin == NULL) return EXIT_FAILURE;
		int numB = copynFile(fOrigin, file, INT_MAX);
		if(numB == -1) return EXIT_FAILURE;
		if(fclose(fOrigin) != 0) return EXIT_FAILURE;

		arrayHeader[i].name = fileNames[i];
		arrayHeader[i].size = numB;
	}

	//Llevamos el puntero del fichero de nuevo al inicio, y escribimos los headers en el fichero
	if(fseek(file, 0, SEEK_SET) != 0) return EXIT_FAILURE;

	fwrite(&nFiles, sizeof(unsigned int), 1, file);

	for(int i = 0; i < nFiles; ++i){
		
		for(int j = 0; j < strlen(arrayHeader[i].name) + 1; ++j){
			if(putc(arrayHeader[i].name[j], file) == EOF) return EXIT_FAILURE;
		}
		fwrite(&arrayHeader[i].size, sizeof(unsigned int), 1, file);		
	}

	//Liberamos la memoria dinamica utilizada

	free(arrayHeader);

	//Cerramos el archivo mtar
	if(fclose(file) != 0) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[]){

	//Abrimos el archivo mtar, que contiene los archivos a extraer
	FILE * file = fopen(tarName, "r");
	if (file == NULL) return EXIT_FAILURE;

	//Cargamos el header en memoria, para tener la informacion de cada archivo
	int * nFiles = malloc(sizeof(int));
	stHeaderEntry * arrayHeader = readHeader(file, nFiles);

	//Copiamos la informacion de cada uno de los archivos dentro de ellos
	for(int i = 0; i < *nFiles; ++i){
		//Creamos un archivo para copiar el contenido
		FILE * tmp = fopen(arrayHeader[i].name, "w");
		if (tmp == NULL) return EXIT_FAILURE;
		
		//Copiamos el contenido
		if(copynFile(file, tmp, arrayHeader[i].size) == -1) return EXIT_FAILURE;
		
		//Cerramos el archivo temporal
		if(fclose(tmp) != 0) return EXIT_FAILURE;
	}

	free(arrayHeader);

	if(fclose(file) != 0) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


//Version funcional