#!/bin/bash

CPUsLimite=8

# Solicitar al usuario el nombre del fichero de ejemplo que desea simular y el numero maximo de CPUs que se desean simular
while [[ true ]]; do
	echo ""
	echo "Especifique el fichero de ejemplo que desea simular: "
	read fichEjemplo
	if [[ -f $fichEjemplo ]]; then
		echo "	Fichero valido."
		break
	else
		echo "	El fichero especificado ($fichEjemplo) no es un fichero valido."
	fi
done

while [[ true ]]; do
	echo ""
	echo "Especifique ahora el numero maximo de CPUs: "
	read maxCPUs
	if [[ maxCPUs -gt $CPUsLimite ]]; then # Si el numero de CPUs > 8 entonces pide de nuevo el numero maximo de CPUs
		echo "	El numero maximo de CPUs debe ser menor o igual que $CPUsLimite"
	else
		echo "	El numero de CPUs seleccionadas ($maxCPUs) es un numero valido."
		break
	fi
done

# Simular ejemplo para todos los planificadores todos los números de CPUs posibles (hasta el máximo indicado)
# Para cada uno, se generarán las gráficas correspondientes
if [[ -e ./resultados && -d ./resultados ]]; then
	rm -r resultados
fi

mkdir resultados

declare -a planificadores=("FCFS" "RR" "PRIO" "SJF")

for plan in "${planificadores[@]}"; do

	echo ""
	echo "Planificador: $plan con una grafica para cada una de las $maxCPUs CPUs"

	# Aqui empezamos en 1 y acabamos en <= pues para generar los .log necesitamos poner en la consola -n 1
	for (( cpu = 1; cpu <= maxCPUs; ++cpu )); do

		echo "		CPU $cpu"
		./schedsim -i $fichEjemplo -s $plan -n $cpu

		for (( j = 0; j < $cpu; ++j )); do
			# El comando mv nos evita que haya repetidos pues es un ctrl+x & ctrl+v
			mv CPU_$j.log ./resultados/$plan-CPU-$j.log
		done

		cd ../gantt-gplot

		for (( k = 0; k < $cpu; ++k )); do
			./generate_gantt_chart ../schedsim/resultados/$plan-CPU-$k.log
		done

		cd ../schedsim

	done

done

echo ""
echo ""
echo "Proceso finalizado con exito."
