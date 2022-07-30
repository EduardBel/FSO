#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Practica 1 de FSO-2019/20
# Autors:
# Data:
# Versio:

# imports tipics/generics
import os   # os.path, os.stat, os.remove ...
from sys import stderr
from stat import filemode
from os import listdir
from os.path import isfile, isdir, join
# imports pel GUI
from tkinter import *   # GUI
from tkinter import filedialog	# per a demanar fitxers
from tkinter import messagebox	# per a mostrar missatges a l’usuari
# https://docs.python.org/3.9/library/tkinter.messagebox.html

# imports específics d'aquesta practica
from datetime import datetime
import sqlite3
import gzip
global conn
global cur
import chardet
# imports auxilliars/secondaris
from time import strftime   # nom de la BD variable
import urllib.parse         # guardar events a la DB amb cars especials
import gzip                 # per si el log esta comprimit
global mostra
#### BD SQLITE
# https://docs.python.org/3/library/sqlite3.html


def tracta_exepcio_sql(error, sql):
    exception_type = type(error).__name__
    print("\n",exception_type, file=sys.stderr)
    print(error, file=sys.stderr)
    print(sql,"\n", file=sys.stderr)

def crea_connexio(database):
    """ crea la connexio i un cursor
        retorna: la connexio i el cursor
        """
    con = None;
    try:
        con = sqlite3.connect(database)
        cur = con.cursor()
    except sqlite3.Error as error:
        tracta_exepcio_sql(err, sqlcmd)
    return con, cur

def crea_taula(cur, taula_sql):
    try:
        cur.execute(taula_sql)
    except sqlite3.Error as error:
        tracta_exepcio_sql(error, taula_sql)
    

# definicio de la taula amb els camps dels logs
sql_crea_taula = """CREATE TABLE events (mes text, dia integer, hora text, maquina text, dem_proc text, pid integer, missatge text);"""
                                        

#### IMPORTAR FITXER DE LOG a BD
# >>>>>>>>>> CODI ALUMNES <<<<<<<<<<

#### QUERIES SQL a la BD


# Les funcions de busqueda fan una 
# seleccio de la columna indicada

def buscaMes():
	mostra=cur.execute("SELECT * FROM events WHERE mes = '%s'" % entrada.get())
	j=0
	for i in mostra:
		lbox_events.insert(0, i)
		j=j+1
	status.set("Elements filtrats: "+str(j))
	pass
def buscaData():
	mostra=cur.execute("SELECT * FROM events WHERE dia = '%s'" % entrada.get())
	j=0
	for i in mostra:
		lbox_events.insert(0, i)
		j=j+1
	status.set("Elements filtrats: "+str(j))
	pass
def buscaMaquina():
	mostra=cur.execute("SELECT * FROM events WHERE maquina = '%s'" % entrada.get())
	j=0
	for i in mostra:
		lbox_events.insert(0, i)
		j=j+1
	status.set("Elements filtrats: "+str(j))
	pass
def buscaProces():
	mostra=cur.execute("SELECT * FROM events WHERE dem_proc = '%s'" % entrada.get())
	j=0
	for i in mostra:
		lbox_events.insert(0, i)
		j=j+1
	status.set("Elements filtrats: "+str(j))
	pass
def neteja():
	lbox_events.delete('0', 'end')
	status.set("Elements filtrats: "+"0")
	pass
def buscaPID():
	mostra=cur.execute("SELECT * FROM events WHERE pid = '[%s]:'" % entrada.get())
	j=0
	for i in mostra:
		lbox_events.insert(0, i)
		j=j+1
	status.set("Elements filtrats: "+str(j))
	pass
def buscaTots():
	mostra=cur.execute("SELECT * FROM events")
	j=0
	for i in mostra:
		lbox_events.insert(0, i)
		j=j+1
	status.set("Elements filtrats: "+str(j))
	pass
def acabar():
	tancaGUI()
	pass
def exportaFiltrats():
	per_exportar = lbox_events.get('0', 'end')
	fitxer_base=open(fitxer_bd, "a")
	for i in per_exportar:
		fitxer_base.write(str(i))
	fitxer_base.close()
	messagebox.showinfo(title="Atenció", message="Logs exportats correctament al fitxer de DB")
	pass

#### tkinter GUI
# No cal mes informacio, pero si teniu curiositat,
# sense entar en molts detalls tcl/tk:
# https://likegeeks.com/es/ejemplos-de-la-gui-de-python/

guiroot= Tk()
guiroot.title("Filtar entrades de LOG a la BD")
guiroot.minsize(600,400)
cerca= StringVar()  # variable usada en els QUERIES


 #funció que comprova que les línies del fitxer de logs 
 # seleccionat compleixin la sintaxi correcta. 
 # Les línies correctes són introduides a una llista per posterior ús
 
def comprovaLogs(fitxer_logs, cur): 
	if fitxer_logs.endswith('.gz'):
		f=gzip.open(fitxer_logs,'rb')
	else:
		f=open(fitxer_logs, "rb" )

	for i in f:
		correcte=True
		tupus_codif=chardet.detect(i)   # detectem tipus de codificació
		encod=tupus_codif['encoding']   
		i=i.decode(encod, 'ignore') #decodifiquem segons el tipus de codificació
		llista=" ".join(i.split())
		llista=llista.split(" ")
		mes=llista[0]
		dia=int(llista[1])
		temps=(llista[2]).split(":")
		hora=int(temps[0])
		minuts=int(temps[1])
		segons=int(temps[2])
		correcte = comprova_data(mes, dia, hora, minuts, segons)
		if correcte:    #si el format dia/mes/hora és correcte continuem la comprovació
			nom_maq=llista[3]
			dem_proc=llista[4]
			te_pid=dem_proc.find("[")
			if te_pid>=0:  #si hi entra, és el format de 7 elements
				j=5
				dem_proc=dem_proc.split("[")
				pid="["+dem_proc[1]
				dem_proc=dem_proc[0]
			else:
				dem_proc=dem_proc[:-1]	#perquè sino no troba els processos(es desaven amb : finals)
				j=4
				pid=""  #pid buit perquè no hi és en aquest format
			miss=""
			while j<len(llista):    #conformem el missatge
				miss=miss+" "+llista[j]
				j=j+1
			sql_insert = ''' INSERT INTO events (mes, dia, hora, maquina, dem_proc, pid, missatge) VALUES(?,?,?,?,?,?,?) '''
			llista_mom=[mes, dia, llista[2], nom_maq, dem_proc, pid, miss]
			cur.execute(sql_insert, llista_mom)
		else:   # en cas que no sigui correcte prontem per stderr
			print("Error de format: "+i+"\n", file=sys.stderr)
	f.close()

def comprova_data(m, d, hora, minuto, sec):
	correcte=True
	#Aquesta llista indica si hi ha algun problema en la data
	mesos = ['Gen', 'Ene', 'Jan', 'Feb', 'Mar', 'Abr', 'Apr', 'Mai', 'May', 'Jun', 'Jul', 'Ago', 'Aug', 'Set', 'Sep', 'Oct', 'Nov', 'Dec', 'Dic', 'gen', 'ene', 'jan', 'feb', 'mar', 'abr', 'apr', 'mai', 'may', 'jun', 'jul', 'ago', 'aug', 'set', 'sep', 'oct', 'nov', 'dec', 'dic']

	if(minuto < 0 or minuto >= 60):
		return False
	if(sec < 0 or sec >= 60):
		return False
	if(hora < 0 or hora >= 24):
		return False
	#Comprobar que el mes sea valido
	if(len(m) != 3):
		return False
	if(d <= 0 or d >= 31):
		return False 
	for i in mesos:
		if m == i:
			return correcte

	return False
 


def tancaGUI():
    guiroot.quit()

# layout del GUI: 4 marcs apilats
frameCerca= Frame(guiroot)
frameBotons= Frame(guiroot)
frameEvents= Frame(guiroot)
frameStatus= Frame(guiroot)

# Frame CERCA
etiq= Label (frameCerca, text="Cerca")
etiq.pack(side=LEFT)
entrada= Entry(frameCerca, textvariable=cerca, width=80, bd=5)
entrada.pack(side=LEFT)
frameCerca.pack(side=TOP, expand=FALSE,fill=X,padx=2)

# Frame Botons
# Les funcions/commands les han d'implementar els alumnes
Button (frameBotons, text='Tots', command=buscaTots).pack(anchor=W,side=LEFT)
Button (frameBotons, text='Mes', command=buscaMes).pack(anchor=W,side=LEFT)
Button (frameBotons, text='Data', command=buscaData).pack(anchor=W,side=LEFT)
Button (frameBotons, text='Maq', command=buscaMaquina).pack(anchor=W,side=LEFT)
Button (frameBotons, text='Proc', command=buscaProces).pack(anchor=W,side=LEFT)
Button (frameBotons, text='PID', command=buscaPID).pack(anchor=W,side=LEFT)
Button (frameBotons, text='Sortir',command=acabar).pack(anchor=E, side=RIGHT)
Button (frameBotons, text='Exporta', command=exportaFiltrats).pack(anchor='e',side=RIGHT)
Button (frameBotons, text='Neteja',command=neteja).pack(anchor=E, side=RIGHT)
frameBotons.pack(expand=FALSE,fill=X,padx=40)

# Frame Events
Label (frameEvents,text='Events Filtrats:').pack(anchor=W,side=TOP)
scrollist = Scrollbar(frameEvents,orient=VERTICAL)
# llista on es quarden els events filtrats
lbox_events = Listbox(frameEvents,yscrollcommand=scrollist.set,selectmode=SINGLE,exportselection=False)
scrollist.config(command=lbox_events.yview)
scrollist.pack(side=RIGHT,fill=Y)
lbox_events.pack(side=LEFT,expand=True,fill=BOTH)
frameEvents.pack(side=TOP, expand=True,fill=BOTH,padx=2)

# Frame estat
status= StringVar()
status.set("Elements filtrats: 0")
etqStatus= Label (frameStatus, textvariable= status)
etqStatus.pack(side=RIGHT, anchor=E)
frameStatus.pack(side=RIGHT,expand=False,padx=2)

#### MAIN
bd_prog="bd_prog.db"
nova= messagebox.askyesno("Nova DB o vella", "Vols crear una DB nova ?") # retorna True o False
# >>>>>>>>>> CODI ALUMNES <<<<<<<<<<
fitxer_bd = ""
fitxer_logs=filedialog.askopenfilename()    #demanem quin fitxer de logs es vol utilitzar

if nova == True:    # si l'usuari vol crear una BD nova
	data=datetime.today().strftime('%d%m')
	any=datetime.today().strftime('%Y')
	data=data+any[2:]
	fitxer_bd = "/home/milax/Escriptori/logs2db_"+data+".db"
	fit=open(fitxer_bd, "w")#per poder llegir els permisos
	fit.close()
else:   # si vol utilitzar una BD ja existent
    fitxer_bd=filedialog.askopenfilename(filetypes=[("DataBase", "*.db")])    #demanem quin fitxer de logs es vol utilitzar
con,cur=crea_connexio(bd_prog)
crea_taula(cur, sql_crea_taula)

nom_logs=fitxer_logs.split("/")
nom_logs=nom_logs[-1]
print("Fitxer de logs: "+fitxer_logs+"	"+nom_logs)
bashCommand = "echo 'Permisos: ';stat -c %A "+fitxer_logs
permisos=os.system(bashCommand)
print("\n")
nom_logs=fitxer_bd.split("/")
nom_logs=nom_logs[-1]
print("Fitxer de BD: "+fitxer_bd+"	"+nom_logs)
bashCommand = "echo 'Permisos: ';stat -c %A "+fitxer_bd
permisos=os.system(bashCommand)
comprovaLogs(fitxer_logs, cur)

# al final del fitxer: bucle d'espera d'events asincrons de l'usuari
guiroot.mainloop()
