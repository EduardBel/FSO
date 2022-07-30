/*****************************************************************************/
/*									     */
/*  Autors: Eduard Bel, Antonio Torres i Cristina Izquierdo                  */
/*				     oponent3.c				     */
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
#include <stdint.h>   /* definició de intptr_t per màquines de 64 bits */
#include "memoria.h" /*Funcions de manipulació de memòria compartida */
#include "semafor.h"		/* incloure definicions de funcions propies */
typedef struct {		/* per un tron (usuari o oponent) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
} tron;

typedef struct {		/* per una entrada de la taula de posicio */
	int f;
	int c;
} pos;
int varia;		/* valor de variabilitat dels oponents [0..9] */
int n_opo = 1;
int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */
pos *p_opo;			/* l'oponent */
tron opo;
FILE *fitxer;           /* descriptor de fitxer on escriurem les longituts */
int retard;
char dia[9];
char hora[9];

int *p_win, id_win;
int sem_globals, sem_pantalla;
int ind, pid, fila, col;
int *fi1, *oponents_abatuts, final1, oponents_ab, final2, *fi2, num_oponents;

/* funcio per moure un oponent una posicio; retorna 1 si l'oponent xoca */
/* contra alguna cosa, 0 altrament					*/
/*parametre ind = enter que indica l'ordre de creacio del tron (0-9) --> index a la taula de trons*/


void esborrar_posicions(char car_tron, pos p_pos[], int n_pos)
{
  int i;
  //fprintf(fitxer, "%s %s tron acabat %c: %d\n",dia,hora,car_tron,n_pos);
  for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
  {
    waitS(sem_pantalla);
    win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);	/* esborra una pos. */
    signalS(sem_pantalla);
    win_retard(5);		/* un petit retard per simular el joc real */
  }
}


void * mou_oponent(void * ind)
{
  char cars;
  tron seg;
  int k, vk, nd, vd[3];
  int canvi;
  int indx = (intptr_t) ind;
  int oponent_mort = 0;
  indx--;
  int opo_abatut;
  do{
    canvi = 0;
    win_retard(retard);
    seg.f = opo.f + df[opo.d];	/* calcular seguent posicio */
    seg.c = opo.c + dc[opo.d];
	  waitS(sem_pantalla);
    cars = win_quincar(seg.f,seg.c);	/* calcula caracter seguent posicio */
    signalS(sem_pantalla);

    if (cars != ' ')			/* si seguent posicio ocupada */
        canvi = 1;		/* anotar que s'ha de produir un canvi de direccio */
    else
      if (varia > 0)	/* si hi ha variabilitat */
      { k = rand() % 10;		/* prova un numero aleatori del 0 al 9 */
        if (k < varia) canvi = 1;	/* possible canvi de direccio */
      }

    if (canvi)		/* si s'ha de canviar de direccio */
    {
      nd = 0;
      for (k=-1; k<=1; k++)	/* provar direccio actual i dir. veines */
      {
        vk = (opo.d + k) % 4;		/* nova direccio */
        if (vk < 0) vk += 4;		/* corregeix negatius */
        seg.f = opo.f + df[vk];		/* calcular posicio en la nova dir.*/
        seg.c = opo.c + dc[vk];
        cars = win_quincar(seg.f,seg.c);/* calcula caracter seguent posicio */
        if (cars == ' ')
        { vd[nd] = vk;			/* memoritza com a direccio possible */
          nd++;				/* anota una direccio possible mes */
        }
      }
      waitS(sem_globals);
      if (nd == 0){			/* si no pot continuar, */
      	*oponents_abatuts++;		/* xoc: ha perdut l'oponent! */
	      oponent_mort = 1;
      }
      else
      { 
        if (nd == 1)			/* si nomes pot en una direccio */
        {
          opo.d = vd[0];			/* li assigna aquesta */
        } 
        else				/* altrament */
        {
          opo.d = vd[rand() % nd];	/* segueix una dir. aleatoria */
        }
      }
    }

    signalS(sem_globals);
    if (oponent_mort == 0)		/* si no ha col.lisionat amb res */
    {
      waitS(sem_globals);
      opo.f = opo.f + df[opo.d];			/* actualitza posicio */
      opo.c = opo.c + dc[opo.d];
      p_opo[n_opo].f = opo.f;			/* memoritza posicio actual */
      p_opo[n_opo].c = opo.c;
      n_opo++;
      signalS(sem_globals);

      waitS(sem_pantalla);
      win_escricar(opo.f,opo.c,'1'+indx,INVERS);	/* dibuixa bloc oponent */

      signalS(sem_pantalla);
    }
    else
    {
    	esborrar_posicions(indx, p_opo, n_opo);
    } 
  } while (!(*fi1) && !oponent_mort);

  waitS(sem_globals);
  opo_abatut=*oponents_abatuts;
  *fi2 = (num_oponents == opo_abatut);
  signalS(sem_globals);
  return(NULL);
}


int main(int n_args, char *ll_args[])
{

  ind = atoi(ll_args[1]) + 1;
  pid = atoi(ll_args[2]);
  fila  = atoi(ll_args[3]);
  col = atoi(ll_args[4]);
  id_win  = atoi(ll_args[5]);
  sem_globals= atoi(ll_args[6]);
  sem_pantalla= atoi(ll_args[7]);
  final1= atoi(ll_args[8]);
  oponents_ab= atoi(ll_args[9]);
  final2= atoi(ll_args[10]);
  num_oponents= atoi(ll_args[11]);
  retard= atoi(ll_args[12]);
  varia = atoi(ll_args[13]);


  p_win = map_mem(id_win);	/* obtenir adres. de mem. compartida */
  fi1 = map_mem(final1);	/* obtenir adres. de mem. compartida */
  oponents_abatuts = map_mem(oponents_ab);	/* obtenir adres. de mem. compartida */
  fi2 = map_mem(final2);	/* obtenir adres. de mem. compartida */

  win_set(p_win, fila, col);
  p_opo = calloc(fila*col/2, sizeof(pos));	/* per a les posicions ant. */

  if (!p_opo)	/* si no hi ha prou memoria per als vectors de pos. */
  { 
    win_fi();				/* tanca les curses */
    if (p_opo) free(p_opo);	   /* allibera el que hagi pogut obtenir */
    fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
    exit(3);
  }

  int n_fil =fila+ind + num_oponents;
  opo.f = ind*2 + num_oponents; /* Els trons oponents mantenen la columna pero es reparteixen de forma equidistant per les files */
  opo.c = (col*3)/4;		/* fixa posicio i direccio inicial oponent */
  opo.d = (rand() % 4); /* direccio inicial aleatoria (0-3) --> rand() % N+1 = aleatori [0..N]*/
  win_escricar(opo.f,opo.c,'0'+ind,INVERS);	/* escriu la primer posicio oponent */
  p_opo[0].f = opo.f;		/* memoritza posicio inicial */
  p_opo[0].c = opo.c;
  
	mou_oponent((void *)(intptr_t) ind);
  free(p_opo);	  	 /* allibera la memoria dinamica obtinguda */
}