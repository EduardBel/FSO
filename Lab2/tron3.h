#define MAX_THREADS 10

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

int oponents_abatuts = 0;


int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int num_oponents; /* numero d'oponents --> n_threads oponents */
int varia;		/* valor de variabilitat dels oponents [0..9] */
FILE *fitxer;           /* descriptor de fitxer on escriurem les longituts */
int retard;		/* valor del retard de moviment, en mil.lisegons */

int n_usu = 0, n_opo = 0;	/* numero d'entrades en les taules de pos. */
char dia[9];
char hora[9];

int fi1, fi2; /*indiquen condicio final del joc: usuari mort, oponent mort o tecla RETURN*/
pthread_t tid[MAX_THREADS];/* taula d'identificadors dels threads */
int sem_pantalla;
int sem_globals;

