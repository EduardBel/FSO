#!/usr/bin/env python3
import sys
# -*- coding: utf-8 -*-


def comprobar_fecha(m, d, hora, minuto, sec):

	#Aquesta llista indica si hi ha algun problema en la data
	llistaErrors = ['', '', '', '', '', '']
	#Array que almacenara los dias que tiene cada mes (si el ano es bisiesto, sumaremos +1 al febrero)
	dias_mes = [31, 28, 31, 30,31, 30, 31, 31, 30, 31, 30, 31]
	#Comprobar si la hora, minutos y segundos son aceptables
	if(minuto < 0 or minuto >= 60):
		llistaErrors[0] = 'Error en els minuts'
	if(sec < 0 or sec >= 60):
		llistaErrors[1] = 'Error en els segons'
	if(hora < 0 or hora >= 24):
		llistaErrors[2] = 'Error en la hora'
	#Comprobar que el mes sea valido
	if(len(m) != 3):
		llistaErrors[3] = 'Error en el mes'
     
	if(d <= 0 or d >= 31):
		llistaErrors[3] = 'Error en el dia'
 
	return llistaErrors
 



fitxer=open(sys.argv[1], "r")
llistaErrors = ['', '', '', '', '', '']
for i in fitxer:
    llista=" ".join(i.split())
    llista=llista.split(" ")
    mes=llista[0]
    dia=int(llista[1])
    temps=(llista[2]).split(":")
    hora=int(temps[0])
    minuts=int(temps[1])
    segons=int(temps[2])
    llistaErrors = comprobar_fecha(mes, dia, hora, minuts, segons)
    for j in llistaErrors:
        if(j != ''):
            print(i)
            print(j)
            sys.exit()
