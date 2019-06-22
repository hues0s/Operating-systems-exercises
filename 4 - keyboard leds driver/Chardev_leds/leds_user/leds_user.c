#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/input.h>

int numerosBinarios(int fd);
int morse(int fd);
int help();
int haciaDerecha(int fd);
int haciaIzquierda(int fd);
int mostrarMenu();
int detectarMovimientoRaton(int fd);

int main(void) {
	int ledsFD;

	if ((ledsFD = open("/dev/leds", O_WRONLY)) < 0)
		return -1;
	
	int eleccionMenu = mostrarMenu();
	while(eleccionMenu != 0){
		switch(eleccionMenu){
			case 2: numerosBinarios(ledsFD);
					break;
			case 3: morse(ledsFD);
					break;
			case 1: help();
					break;
			case 4: haciaDerecha(ledsFD);
					break;
			case 5: haciaIzquierda(ledsFD);
					break;
			case 6: detectarMovimientoRaton(ledsFD);
					break;
		}
		if(eleccionMenu != 0) eleccionMenu = mostrarMenu();
	}

	close(ledsFD);
}

int numerosBinarios(int fd) {

	// Ahora hacemos la secuencia que corresponda
	int numBinario;
	for (numBinario = 1; numBinario <= 7; ++numBinario){
		switch(numBinario){
			case 1:
				if(write(fd, "3", 1) < 0) return -1;
				break;
			case 2:
				if(write(fd, "2", 1) < 0) return -1;
				break;
			case 3:
				if(write(fd, "23", 2) < 0) return -1;
				break;
			case 4:
				if(write(fd, "1", 1) < 0) return -1;
				break;
			case 5:
				if(write(fd, "13", 2) < 0) return -1;
				break;
			case 6:
				if(write(fd, "12", 2) < 0) return -1;
				break;
			case 7:
				if(write(fd, "123", 3) < 0) return -1;
				break;
		}
		sleep(1);
	}

	if(write(fd, "", 1) < 0) return -1;

	return 0;
}

int morse(int fd) {

	// Luego realizamos la secuencia corespondiente
	// H --> ....
	// O --> ---
	// L --> .-..
	// A --> .-
	char palabraMorse[] = "HOLA";
	int numLetra, i, j;

	
	for (numLetra = 1; numLetra <= strlen(palabraMorse); ++numLetra){
		sleep(1);
		switch(numLetra){
			case 1: // ....
				for (j = 0; j < 4; ++j){
					if(write(fd, "1", 1) < 0)
						return -1;
					usleep(100000);
					if(write(fd, "", 1) < 0) return -1;
					usleep(100000);
				}
				break;
			case 2: // ---
				for (i = 0; i < 3; ++i){
					if(write(fd, "123", 3) < 0)
						return -1;
					usleep(100000);
					if(write(fd, "", 1) < 0) return -1;
					usleep(100000);
				}
				break;
			case 3: // .-..
				if(write(fd, "1", 1) < 0)
						return -1;
				usleep(100000);
				if(write(fd, "", 1) < 0) return -1;
					usleep(100000);
				if(write(fd, "123", 3) < 0)
					return -1;
				usleep(100000);
				if(write(fd, "", 1) < 0) return -1;
					usleep(100000);
				if(write(fd, "1", 1) < 0)
						return -1;
				usleep(100000);
				if(write(fd, "", 1) < 0) return -1;
					usleep(100000);
				if(write(fd, "1", 1) < 0)
						return -1;
				usleep(100000);
				if(write(fd, "", 1) < 0) return -1;
					usleep(100000);
				break;
			case 4: // .-
				if(write(fd, "1", 1) < 0)
						return -1;
				usleep(100000);
				if(write(fd, "", 1) < 0) return -1;
					usleep(100000);
				if(write(fd, "123", 3) < 0)
					return -1;
				usleep(100000);
				break;
		}
	}

	if(write(fd, "", 1) < 0) return -1;

	return 0;
}

int help() {

	char op1[] = "1. Muestra este mensaje";
	char op2[] = "2. Numeros binarios del 1 al 7";
	char op3[] = "3. H(....) O(---) L(.-..) A(.-)";
	char op4[] = "4. Enciende los leds uno a uno de izquierda a derecha";
	char op5[] = "5. Enciende los leds uno a uno de derecha a izquierda";
	char op6[] = "6. Enciende los leds mientras detecte movimiento de raton";

	printf("\n\n%s\n%s\n%s\n%s\n%s\n%s\n\n", op1, op2, op3, op4, op5, op6);

	return 0;
}

int haciaDerecha(int fd) {
	
	// Encendemos 1, 2 y luego 3 durante 3 veces seguidas 
	int vuelta, led;
	
	for (vuelta = 0; vuelta < 10; ++vuelta){
		for (led = 1; led <= 3; ++led){
			char * tmp = malloc(sizeof(char));
			*tmp = led + '0';
			if(write(fd, tmp, 1) < 0) return -1;
			usleep(100000);
		}
	}

	if(write(fd, "", 1) < 0) return -1;

	return 0;
}

int haciaIzquierda(int fd) {

	// Encendemos 1, 2 y luego 3 durante 3 veces seguidas 
	int vuelta, led;
	
	for (vuelta = 0; vuelta < 10; ++vuelta){
		for (led = 3; led >= 1; --led){
			char * tmp = malloc(sizeof(char));
			*tmp = led + '0';
			if(write(fd, tmp, 1) < 0) return -1;
			usleep(100000);
		}
	}

	if(write(fd, "", 1) < 0) return -1;

	return 0;
}

int mostrarMenu() {
	/*
	The getpwuid() function returns a pointer to a structure containing the broken-out fields of  the  record  in  the  password
    database that matches the user ID uid.
    The passwd structure is defined in <pwd.h> as follows:

		   struct passwd {
		       char   *pw_name;       // username
		       char   *pw_passwd;     // user password
		       uid_t   pw_uid;        // user ID
		       gid_t   pw_gid;        // group ID
		       char   *pw_gecos;      // user information
		       char   *pw_dir;        // home directory
		       char   *pw_shell;      // shell program
		   };
	*/
	char * lectura = malloc(10*sizeof(char));
	int opcionEscogida = 1;
	int numCorrectos = 1;
	struct passwd *userInfo = getpwuid(getuid());

	if(userInfo != NULL)
		printf("\n\tBienvenido %s!\n", userInfo->pw_name);


	printf("--------------------------------------------------\n");

	printf("1. Mostrar informacion\n\n");
	printf("2. Numeros binarios\n");
	printf("3. Palabra HOLA en morse\n");
	printf("4. Movimiento --->\n");
	printf("5. Movimiento <---\n\n");
	printf("6. Movimiento raton\n\n");
	printf("0. Salir\n\n");
	printf("\tSeleccionar una opcion: ");

	do{
		if(numCorrectos != 1 || opcionEscogida < 0 || opcionEscogida > 6)
			printf("Opcion escogida erronea.\nIntroduce otra a continuacion: ");
		fgets(lectura,10, stdin);
		numCorrectos = sscanf(lectura,"%d",&opcionEscogida);
	}while(numCorrectos != 1 || opcionEscogida < 0 || opcionEscogida > 6);

	return opcionEscogida;
}

int detectarMovimientoRaton(int fd){

	/*

		!!!!!!!!!!!!!!!!! EXPERIMENTAL !!!!!!!!!!!!!!!!!

	*/

	int raton;
	int MOUSE_WAIT;

	//Abrimos el descriptor del raton en modo READONLY, para que al hacer read se bloquee la ejecucion del programa
	//hasta que se mueva el raton.
	if ((MOUSE_WAIT = open("/dev/input/mice", O_RDONLY)) < 0)
		return -1;

	//Aqui abrimos en modo READONLY Y NONBLOCK, para que si o si lea, aunque el raton no se mueva
	if ((raton = open("/dev/input/mice", O_RDONLY | O_NONBLOCK)) < 0)
		return -1;

	printf("En esta funcion, los led se encenderan mientras el raton este en movimiento.\n");
	printf("Para salir, pulse el BOTON IZQUIERDO del raton.\n");
	printf("***Es posible que se requieran 2 clicks, en algunos casos.\n\n");

	//Struct que contiene los datos del raton (botones, x, y)
	struct input_event * ie = malloc(sizeof(struct input_event));
	
	unsigned char botonGeneral, botonIzq;

	char x = '\0',y = '\0';

	//Guardamos posiciones antiguas de x e y, para poder comparar si cambian o no
	char xOld = '\n',yOld = '\n';

	//Esperamos a que el raton se mueva para continuar con el programa;
	//Mientras tanto, estara bloqueado.
	read(MOUSE_WAIT,NULL,0);
	close(MOUSE_WAIT);

	int pulsado = 0;

	while(read(raton,ie,sizeof(struct input_event))){
		
		unsigned char * ptr = (unsigned char*) ie;

		botonGeneral = ptr[0];

		botonIzq = botonGeneral & 0x1;


		x = (char) ptr[1];
		y = (char) ptr[2];

		//Esta parte de aqui es debida a un bug que hay al hacer read del raton. A veces se inicia el boton izquierdo
		//como pulsado, y hasta que no se pulsa, no se pone a 0.
		if(botonIzq == 0) pulsado = 1;
		if(pulsado == 1 && botonIzq == 1) break;

		usleep(50000);


		if(x == xOld && y == yOld){
			//Si el raton se detiene, apagamos las luces y lo dejamos en espera
			//hasta que se vuelva a mover.
			if(write(fd, "", 1) < 0) return -1;
			
			if ((MOUSE_WAIT = open("/dev/input/mice", O_RDONLY)) < 0)
				return -1;
			read(MOUSE_WAIT,NULL,0);
			close(MOUSE_WAIT);
			
		}

		if(write(fd, "123", 3) < 0) return -1;

		xOld = x;
		yOld = y;

	}

	if(write(fd, "", 1) < 0) return -1;

	free(ie);
	close(raton);
	
	return 0;
}