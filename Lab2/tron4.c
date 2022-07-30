/*****************************************************************************/
/*									     */
/*  Autors: Eduard Bel, Antonio Torres i Cristina Izquierdo                  */
/*				     tron4.c				     */
/*									     */
/*  Programa inicial d'exemple per a les practiques 2 i 3 de FSO.	     */
/*     Es tracta del joc del tron: sobre un camp de joc rectangular, es      */
/*     mouen uns objectes que anomenarem 'trons' (amb o tancada). En aquesta */
/*     primera versio del joc, nomes hi ha un tron que controla l'usuari, i  */
/*     que representarem amb un '0', i un tron que controla l'ordinador, el  */
/*     qual es representara amb un '1'. Els trons son una especis de 'motos' */
/*     que quan es mouen deixen rastre (el caracter corresponent). L'usuari  */
/*     pot canviar la direccio de moviment del seu tron amb les tecles:      */
/*     'w' (adalt), 's' (abaix), 'd' (dreta) i 'a' (esquerra). El tron que   */
/*     controla l'ordinador es moura aleatoriament, canviant de direccio     */
/*     aleatoriament segons un parametre del programa (veure Arguments).     */
/*     El joc consisteix en que un tron intentara 'tancar' a l'altre tron.   */
/*     El primer tron que xoca contra un obstacle (sigui rastre seu o de     */
/*     l'altre tron), esborrara tot el seu rastre i perdra la partida.       */
/*									     */
/*  Arguments del programa:						     */
/*     per controlar la variabilitat del canvi de direccio, s'ha de propor-  */
/*     cionar com a primer argument un numero del 0 al 3, el qual indicara   */
/*     si els canvis s'han de produir molt frequentment (3 es el maxim) o    */
/*     poc frequentment, on 0 indica que nomes canviara per esquivar les     */
/*     parets.								     */
/*     Hi haura un segon parametre que sera el nom d'un fitxer on guardarem  */
/*     les llargaries dels diferents trons.				     */
/*     A mes, es podra afegir un tercer argument opcional per indicar el     */
/*     retard de moviment dels diferents trons (en ms);                      */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									     */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides a "winsuport.c", les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				     */
/*									     */
/*	   $ gcc -c winsuport.c -o winsuport.o				     */
/*	   $ gcc tron0.c winsuport.o -o tron0 -lcurses			     */
/*	   $ ./tron0 variabilitat fitxer [retard]			     */
/*									     */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*	3  ==>  no hi ha prou memoria per crear les estructures dinamiques   */
/*									     */
/*****************************************************************************/


#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include <pthread.h>    /*llibreria per la gestio de fils*/
#include <stdint.h>   /* definició de intptr_t per màquines de 64 bits */
#include "semafor.h"		/* incloure definicions de funcions propies */
#include "memoria.h" /*Funcions de manipulació de memòria compartida */
#include <sys/wait.h>
#include "missatge.h"  /*Funcions de gestions de bústies*/

#define MAX_THREADS 10
				/* definir estructures d'informacio */
typedef struct {		/* per un tron (usuari o oponent) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
} tron;

typedef struct {		/* per una entrada de la taula de posicio */
	int f;
	int c;
} pos;


/* variables globals */
int n_fil, n_col;		/* dimensions del camp de joc */
int id_oponents_abatuts = 0;
int *p_oponents_abatuts;
tron usu;   	   		/* informacio de l'usuari */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int num_oponents; /* numero d'oponents --> n_threads oponents */
int varia;		/* valor de variabilitat dels oponents [0..9] */
FILE *fitxer;           /* descriptor de fitxer on escriurem les longituts */
int retard;		/* valor del retard de moviment, en mil.lisegons */

pos *p_usu;			/* taula de posicions que van recorrent */
int n_usu = 0, n_opo = 0;	/* numero d'entrades en les taules de pos. */
char dia[9];
char hora[9];

int fi1, fi2; /*indiquen condicio final del joc: usuari mort, oponent mort o tecla RETURN*/
pthread_t tid[MAX_THREADS];/* taula d'identificadors dels threads */
int *p_fi1;
int *p_fi2;
int sem_pantalla;
int sem_globals;
int id_bustia;
void *p_win;


/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(char car_tron, pos p_pos[], int n_pos)
{
  int i;
  fprintf(fitxer, "%s %s tron acabat %c: %d\n",dia,hora,car_tron,n_pos);
  for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
  {
    //waitS(sem_pantalla);
    win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);	/* esborra una pos. */
    //signalS(sem_pantalla);
    win_retard(5);		/* un petit retard per simular el joc real */
  }
}


/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  char strin[45];

  usu.f = (n_fil-1)/2;
  usu.c = (n_col)/4;		/* fixa posicio i direccio inicial usuari */
  usu.d = 3;
  win_escricar(usu.f,usu.c,'0',INVERS);	/* escriu la primer posicio usuari */
  p_usu[n_usu].f = usu.f;		/* memoritza posicio inicial */
  p_usu[n_usu].c = usu.c;
  n_usu++;

  sprintf(strin,"Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  win_escristr(strin);
}



/* funcio per moure l'usuari una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si ha xocat     */
/* contra alguna cosa, i 0 altrament */
/*parametre nulo no conte cap informacio*/
void * mou_usuari(void * nulo)
{
  char cars;
  tron seg;
  int tecla;
    
  do
  {
    win_retard(retard);
  	waitS(sem_pantalla);
    tecla = win_gettec();
  	signalS(sem_pantalla);
    
    if (tecla != 0)
    switch (tecla)	/* modificar direccio usuari segons tecla */
    {
      case TEC_AMUNT:	usu.d = 0; break;
      case TEC_ESQUER:	usu.d = 1; break;
      case TEC_AVALL:	usu.d = 2; break;
      case TEC_DRETA:	usu.d = 3; break;
      case TEC_RETURN:	*p_fi1 = -1; break;
    }

    seg.f = usu.f + df[usu.d];	/* calcular seguent posicio */
    seg.c = usu.c + dc[usu.d];
	  waitS(sem_pantalla);
    cars = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
    if (cars == ' ')			/* si seguent posicio lliure */
    {
      usu.f = seg.f; usu.c = seg.c;		/* actualitza posicio */
      win_escricar(usu.f,usu.c,'0',INVERS);	/* dibuixa bloc usuari */
	    signalS(sem_pantalla);
      p_usu[n_usu].f = usu.f;		/* memoritza posicio actual */
      p_usu[n_usu].c = usu.c;
      n_usu++;
    }
    else
    { 
      esborrar_posicions('0', p_usu, n_usu);
	    signalS(sem_pantalla);
      *p_fi1 = 1;
    }

  }while (!*p_fi1 && !*p_fi2);
  return(NULL);
}


/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int retwin, n, i, id_win;

  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);
  strftime(dia,9,"%d/%m/%y",tlocal);
  strftime(hora,9,"%H:%M:%S",tlocal);

  srand(getpid());		/* inicialitza numeros aleatoris */

  if ((n_args <=  4))
  {	fprintf(stderr,"Comanda: ./tron1 num_oponents fitxer variabilitat [retard]\n");
  	fprintf(stderr,"         on \'variabilitat\' indica la frequencia de canvi de direccio\n");
  	fprintf(stderr,"         de l'oponent: de 0 a 3 (0- gens variable, 3- molt variable),\n");
  	fprintf(stderr,"         fitxer es el fitxer on acumularem la llargaria dels drons ),\n");
  	fprintf(stderr,"         i \'retard\' es el numero de mil.lisegons que s'espera entre dos\n");
  	exit(1);
  }

  num_oponents = atoi(ll_args[1]); /*obtenir el num_oponents*/
  if (num_oponents < 1) num_oponents = 1; /* i filtrar el seu valor */
  if (num_oponents > MAX_THREADS-1) num_oponents = MAX_THREADS-1;

  varia = atoi(ll_args[3]);	/* obtenir parametre de variabilitat */
  if (varia < 0) varia = 0;	/* verificar limits */
  if (varia > 3) varia = 3;

  fitxer = fopen(ll_args[2],"a");
  if (n_args == 3)		/* si s'ha especificat parametre de retard */
  {	retard = atoi(ll_args[3]);	/* convertir-lo a enter */
  	if (retard < 10) retard = 10;	/* verificar limits */
  	if (retard > 1000) retard = 1000;
  }
  else retard = 100;		/* altrament, fixar retard per defecte */

  printf("Joc del Tron\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();

  n_fil = 0; n_col = 0;		/* demanarem dimensions de taulell maximes */
  retwin = win_ini(&n_fil,&n_col,'+',INVERS);	/* intenta crear taulell */

  if (retwin < 0)	/* si no pot crear l'entorn de joc amb les curses */
  { fprintf(stderr,"Error en la creacio del taulell de joc:\t");
    switch (retwin)
    {	case -1: fprintf(stderr,"camp de joc ja creat!\n"); break;
	case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n"); break;
	case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n"); break;
	case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n"); break;
     }
     exit(2);
  }

  id_win = ini_mem(retwin);	/* crear zona mem. compartida */
  id_oponents_abatuts = ini_mem(sizeof(int));	/* crear zona mem. compartida */
  fi1=ini_mem(sizeof(int)); /* crear zona mem. compartida */
  fi2=ini_mem(sizeof(int)); /* crear zona mem. compartida */


  p_win = map_mem(id_win);	/* obtenir adres. de mem. compartida */
  p_oponents_abatuts = map_mem(id_oponents_abatuts);	/* obtenir adres. de mem. compartida */
  p_fi1=map_mem(fi1); /* obtenir adres. de mem. compartida */
  p_fi2=map_mem(fi2); /* obtenir adres. de mem. compartida */

  win_set(p_win,n_fil,n_col);		/* crea acces a finestra oberta */

  p_usu = calloc(n_fil*n_col/2, sizeof(pos));	/* demana memoria dinamica */

  if (!p_usu)	/* si no hi ha prou memoria per als vectors de pos. */
  { win_fi();				/* tanca les curses */
    if (p_usu) free(p_usu);
    fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
    exit(3);
  }

			/* Fins aqui tot ha anat be! */
  inicialitza_joc();
  
  n = 0; /*num del proces*/

  /*Creem un thread per a l'usuari*/
  sem_globals=ini_sem(1);
  sem_pantalla=ini_sem(1);
  id_bustia = ini_mis();    /* crear bustia IPC */
  char a0[10], a1[10], a2[10], a3[10], a4[10], a5[10], a6[10], a7[10], a8[10], a9[10], a10[10], a11[10], a12[10], a13[10];
  pthread_create(&tid[0], NULL, mou_usuari, (void *)(intptr_t) 0); 

  sprintf(a1, "%i", n_fil);
  sprintf(a2, "%i", n_col);
  sprintf(a3, "%i", id_win);
  sprintf(a5, "%i", sem_globals);
  sprintf(a6, "%i", sem_pantalla);
  sprintf(a7, "%i", fi1);
  sprintf(a8, "%i",id_oponents_abatuts);
  sprintf(a9, "%i",fi2);
  sprintf(a10, "%i",num_oponents);
  sprintf(a11, "%i",retard);
  sprintf(a12, "%i", varia);
  sprintf(a13, "%i", id_bustia);

      
  /*Creem tants processos com a oponents s'hagin indicat per parametre*/
  for (i = 0; i < num_oponents; i++)
  {
    tid[n] = fork();		/* crea un nou proces */

    if (!tid[n])		/* branca del fill */
    {
      sprintf(a0, "%i", getpid());
      sprintf(a4, "%i", i);

			execlp("./oponent3", "oponent3", a4, a0, a1, a2, a3, a5, a6, a7, a8, a9, a10, a11, a12, a13, (char *)0);
			fprintf(stderr,"error: no puc executar el process fill \'oponent3\'\n");
		  exit(0);
    }
    else if (tid[n] > 0) n++;		/* branca del pare */
  }

  double seconds=0; 
  int minutes=0;
  char strin[45];
  do			/********** bucle principal del joc **********/
  {
    win_retard(retard);
    seconds = seconds + (double) retard / 1000;
    if (seconds>=60) {
        minutes++;
        seconds = seconds - 60;
    }
    sprintf(strin, "Time %d : %.0f", minutes, seconds);

    win_escristr(strin);
		win_update();			/* actualitza visualitzacio CURSES */
    
  } while (!*p_fi1 && !*p_fi2);

  /*esperar a que els processos fills acabin la seva execucio*/
	int status = 0;
  for ( i = 0; i <= n; i++)
  {
   	waitpid(tid[i],&status,0);	/* espera finalitzacio d'un fill */
  }

  win_fi();				/* tanca les curses */

  if (*p_fi1 == -1) printf("S'ha aturat el joc amb tecla RETURN!\n\n");
  else { 
    if (*p_fi1)
    {
      fprintf(fitxer, "%s %s tron guanyat %c: %d\n",dia,hora,'1',n_opo);
       printf("Ha guanyat l'ordinador!                                  \n\n");
    }
    else {

     fprintf(fitxer, "%s %s tron guanyat %c: %d\n",dia,hora,'0',n_usu);
       printf("Ha guanyat l'usuari!                                    \n\n");
    }
  }

  fclose(fitxer);
  free(p_usu);

  elim_mis(id_bustia);/* elimina bustia */
  elim_sem(sem_globals);/* elimina semafor */ 
  elim_sem(sem_pantalla);   
  elim_mem(id_win);/* elimina zona de memoria compartida */
  elim_mem(id_oponents_abatuts);/* elimina zona de memoria compartida */
  elim_mem(fi1);/* elimina zona de memoria compartida */
  elim_mem(fi2);/* elimina zona de memoria compartida */
  
  return(0);
}