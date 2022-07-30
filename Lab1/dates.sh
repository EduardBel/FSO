#!/bin/bash
# Script que comprova si el fitxer 'dates.txt' compleix per cada línia el format
# mmm dd hh:mm:ss

if [ $1 = '-h' ] || [ ! -f $1 ]	# en cas que el que se'ns passi per parametre no sigui un fitxer o l'usuari entri -h
then
	echo "./dates.sh dates.txt
Aquest script requereix que se li passi per paràmetre el fitxer dates.txt que conté les dates a evaluar"
	exit 1
fi 

cont=1	# variable que ens servirà per identificar el nombre de línia en cas que es produeixi un error
IFS=$'\n'	# separem les linies
echo "Comença comprovació de dates"	# missatge inici script
for l in $(cat $1 | tr -s ' ')	# iterem a través de totes les linies del fitxer
do
	mes=$(echo $l | cut -f1 -d' ')	# separem els tres camps principals de la data
	dia=$(echo $l | cut -f2 -d' ')
	temps=$(echo $l | cut -f3 -d' ')
	if [[ $mes == [A-Z][a-z][a-z] ]]	# si el mes compleix l'estructura Maj-min-min continuem
	then
		if [ $dia -gt 0 -a $dia -le 31 ]	# si el nombre de dia és entre 1 i 31 continuem
		then
			hora=$(echo $temps | cut -f1 -d':')	# separem en hores, minuts i segons
			minuts=$(echo $temps | cut -f2 -d':')
			segons=$(echo $temps | cut -f3 -d':')
			if [ $hora -ge 0 -a $hora -le 23 ]	# si l'hora està entre 0 i 23
			then
				if [ $minuts -ge 0 -a $minuts -le 59 ]	# igual amb minuts i segons (0 fins 60)
				then
					if [ $segons -lt 0 -o $segons -gt 59 ]
					then
						echo "línia $cont: '$l' error als segons"	# missatges d'error per pantalla però script continua
					fi
				else
					echo "línia $cont: '$l' error als minuts"
				fi
			else
				echo "línia $cont: '$l' error a la hora"
			fi
		else
			echo "línia $cont: '$l' error al dia"
		fi
	else
		echo "línia $cont: '$l' error al mes"	
	fi
	let cont=$cont+1	# incrementem el comptador
done

echo "Totes les dates comprovades"	# missatge final script
exit 0
