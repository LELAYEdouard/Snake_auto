#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80	
#define HAUTEUR_PLATEAU 40	 
// position initiale de la tête du serpent
#define COORD_DEP_X_1 40
#define COORD_DEP_Y_1 13

#define COORD_DEP_X_2 40
#define COORD_DEP_Y_2 27
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define TEMPO 200000
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE_1 '1'
#define TETE_2 '2'

// touches de direction ou d'arrêt du jeu
#define HAUT 1
#define BAS 2
#define GAUCHE 3
#define DROITE 4
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
#define NB_PAVES 6
#define TBLOC 5

// distance + moitié de ce qu'on parcoure qu'il 
// faut avoir avec le portail pour avoir le moins de mouvements possibles
#define DISTANCE_PORTAIL 22
char posa=DROITE;
char posa2=GAUCHE;

// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];

int lesPommesX[NB_POMMES] = {40, 75, 78, 2, 9, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {20, 38, 2, 2, 5, 38, 32, 38, 32, 2};

int lesPavesX[NB_PAVES] = { 4, 73, 4, 73, 38, 38};
int lesPavesY[NB_PAVES] = { 4, 4, 33, 33, 14, 22};


void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int nbPommes);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent1(int lesX[], int lesY[]);
void dessinerSerpent2(int lesX2[], int lesY2[]);
void progresser1(int lesX[], int lesY[], int *direction, tPlateau plateau, bool *collision1, bool *pomme, int *nbPommes, int lesX2[], int lesY2[], int *pas1);
void progresser2(int lesX2[], int lesY2[], int *direction, tPlateau plateau, bool *collision2, bool *pomme, int *nbPommes, int lesX[], int lesY[], int *pas2);
void gererPomme(int lesX[], int lesY[], int lesX2[], int lesY2[], tPlateau plateau, bool *pomme, int *nbPommes, int *nbpommeMangeeS1, int *nbpommeMangeeS2);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();
bool estPositionValide1(int x, int y, int lesX[], int lesY[], int taille, tPlateau plateau, int lesX2[], int lesY2[]);
bool estPositionValide2(int x, int y, int lesX2[], int lesY2[], int taille, tPlateau plateau, int lesX[], int lesY[]);
void dessiner1Bloc(int x,int y);
void dessinerBlocs(int lesX[],int lesY[]);

int main(){
	// 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX[TAILLE];
	int lesY[TAILLE];
    int lesX2[TAILLE];
	int lesY2[TAILLE];

	// représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
	char touche;
	int direction = DROITE;
	int direction2 = GAUCHE;
	// le plateau de jeu
	tPlateau lePlateau;

	bool collision=false;
	bool gagne = false;
	bool pommeMangee = false;

	// compteur de pommes mangées pour chaque serpent
	int nbPommes = 0;
    int pas1 = 0, pas2 = 0;
    int nbpommeMangeeS1 = 0, nbpommeMangeeS2 = 0;

    // initialisation de la position du serpent 1 : positionnement de la
    // tête en (COORD_DEP_X_1, COORD_DEP_Y_1), puis des anneaux à sa gauche
    for(int i=0 ; i<TAILLE ; i++){
        lesX[i] = COORD_DEP_X_1-i;
        lesY[i] = COORD_DEP_Y_1;
    }

    // initialisation de la position du serpent 2 : positionnement de la
    // tête en (COORD_DEP_X_2, COORD_DEP_Y_2), puis des anneaux à sa droite
    for(int i=0 ; i<TAILLE ; i++){
        lesX2[i] = COORD_DEP_X_2+i;
        lesY2[i] = COORD_DEP_Y_2;
    }

    // mise en place du plateau
    initPlateau(lePlateau);
    system("clear");
    dessinerPlateau(lePlateau);
    dessinerBlocs(lesPavesX, lesPavesY);

    // initialisation du générateur de nombres aléatoires
    srand(time(NULL));
    // ajout de la première pomme sur le plateau
    ajouterPomme(lePlateau, nbPommes);

    // initialisation : le serpent se dirige vers la DROITE
    dessinerSerpent1(lesX, lesY);
    dessinerSerpent2(lesX2, lesY2);
    // désactivation de l'echo pour la saisie utilisateur
    disable_echo();
    // initialisation de la touche à une valeur par défaut
    touche = ' ';

	clock_t begin = clock(); // Démarage du timer afin de calculer le temps CPU 
	// boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
	// si toutes les pommes sont mangées
	do {
        // Avance le serpent 1 et vérifie les collisions et les pommes mangées
        progresser1(lesX, lesY, &direction, lePlateau, &collision, &pommeMangee, &nbPommes, lesX2, lesY2, &pas1);
        
        // Avance le serpent 2 et vérifie les collisions et les pommes mangées
        progresser2(lesX2, lesY2, &direction2, lePlateau, &collision, &pommeMangee, &nbPommes, lesX, lesY, &pas2);
        
        // Gère la consommation des pommes par les serpents
        gererPomme(lesX, lesY, lesX2, lesY2, lePlateau, &pommeMangee, &nbPommes, &nbpommeMangeeS1, &nbpommeMangeeS2); 
		if (pommeMangee) {
			nbPommes++;
			gagne = (nbPommes == NB_POMMES);
			if (!gagne) {
				ajouterPomme(lePlateau, nbPommes);
				pommeMangee = false;
			}
		}
		if (!gagne){
			if (!collision){
				usleep(TEMPO);
				if (kbhit()==1){
					touche = getchar();
				}
			}
		}
	} while (touche != STOP && !collision && !gagne);

	gotoxy(1, HAUTEUR_PLATEAU+1);

	clock_t end = clock(); // Fin du timer CPU
    double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;
    // Affiche le temps CPU utilisé
    printf("Temps CPU : %d", tmpsCPU);
    // Affiche le nombre de déplacements et de pommes mangées par le serpent 1
    printf("Serpent 1 : %d déplacements et %d pommes mangées \n", pas1, nbpommeMangeeS1);
    // Affiche le nombre de déplacements et de pommes mangées par le serpent 2
    printf("Serpent 2 : %d déplacements et %d pommes mangées", pas2, nbpommeMangeeS2);
	enable_echo();
	return EXIT_SUCCESS;
}
/************************************************/
/*		FONCTIONS ET PROCEDURES DU JEU 			*/
/************************************************/
void initPlateau(tPlateau plateau){
    // initialisation du plateau avec des espaces
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            plateau[i][j] = VIDE;
        }
    }
    // Mise en place la bordure autour du plateau
    // première ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        if(i==LARGEUR_PLATEAU/2){
            plateau[i][1] = VIDE; // Ouverture au milieu de la première ligne
        }
        else{
            plateau[i][1] = BORDURE;
        }
    }
    // lignes intermédiaires
    for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
        if(j==HAUTEUR_PLATEAU/2){
            plateau[1][j] = VIDE; // Ouverture au milieu de la première colonne
            plateau[LARGEUR_PLATEAU][j] = VIDE; // Ouverture au milieu de la dernière colonne
        }
        else{
            plateau[1][j] = BORDURE;
            plateau[LARGEUR_PLATEAU][j] = BORDURE;
        }
    }
    // dernière ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        if(i==LARGEUR_PLATEAU/2){
            plateau[i][HAUTEUR_PLATEAU] = VIDE; // Ouverture au milieu de la dernière ligne
        }
        else{
            plateau[i][HAUTEUR_PLATEAU] = BORDURE;
        }
    }
}

void dessinerPlateau(tPlateau plateau){
	// affiche eà l'écran le contenu du tableau 2D représentant le plateau
	for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
		for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
			afficher(i, j, plateau[i][j]);
		}
	}
}

void ajouterPomme(tPlateau plateau , int nbPommes){
	// vérifie que ça correspond à une case vide
	// du plateau puis l'ajoute au plateau et l'affiche
    plateau[lesPommesX[nbPommes]][lesPommesY[nbPommes]] = POMME;
    afficher(lesPommesX[nbPommes], lesPommesY[nbPommes], POMME);
}

void afficher(int x, int y, char car){
	gotoxy(x, y);
	printf("%c", car);
	gotoxy(1,1);
}

void effacer(int x, int y){
	gotoxy(x, y);
	printf(" ");
	gotoxy(1,1);
}

void dessinerSerpent1(int lesX[], int lesY[]){
    // Affiche les anneaux du serpent 1
    for(int i=1 ; i<TAILLE ; i++){
        afficher(lesX[i], lesY[i], CORPS);
    }
    // Affiche la tête du serpent 1
    afficher(lesX[0], lesY[0], TETE_1);
}

void dessinerSerpent2(int lesX[], int lesY[]){
    // Affiche les anneaux du serpent 2
    for(int i=1 ; i<TAILLE ; i++){
        afficher(lesX[i], lesY[i], CORPS);
    }
    // Affiche la tête du serpent 2
    afficher(lesX[0], lesY[0], TETE_2);
}


// La fonction "progresser1" met à jour la position du serpent à chaque itération.
// Stratégie de déplacement :
// - Calcule la distance actuelle entre la tête du serpent et la pomme.
// - Compare les distances pour chaque direction (haut, bas, gauche, droite).
// - Privilégie les mouvements vers la pomme ou vers les portes si elles permettent de réduire la distance totale.
// - Empêche le serpent de faire demi-tour sur lui-même.
// - Vérifie si la nouvelle position est valide (pas de collision avec le serpent ou les obstacles).
// - Gère la transition par les portes lorsque la position correspond à une issue.

void progresser1(int lesX[], int lesY[], int *direction, tPlateau plateau, bool *collision1, bool *pomme, int *nbPommes, int lesX2[], int lesY2[], int *pas1) {
    // Efface le dernier élément avant d'actualiser la position de tous les éléments du serpent avant de le redessiner et détecte une collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

    for (int i = TAILLE - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    int distance_haut = 0, distance_bas=0, distance_gauche=0, distance_droite=0;

    // Calcul des distances pour chaque direction (vers la pomme et les portes)
    // Utilisation de sqrt pour évaluer la distance euclidienne
    int distance_serpent_pomme_ent_X = sqrt(pow(lesPommesX[*nbPommes] - lesX[0], 2));
    int distance_pomme_serpent_enY = sqrt(pow(lesPommesY[*nbPommes] - lesY[0], 2));

    // Évaluation des portes si elles permettent de réduire la distance totale
    if (distance_serpent_pomme_ent_X > LARGEUR_PLATEAU / 2 + DISTANCE_PORTAIL) {
        // issue de droite
        if (lesX[0] > LARGEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] - 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] + 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY[0], 2));
        }
        // issue de gauche
        else if (lesX[0] < LARGEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(1 - (lesY[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(1 - (lesY[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] - 1), 2) + pow(1 - lesY[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] + 1), 2) + pow(1 - lesY[0], 2));
        }
    } else if (distance_pomme_serpent_enY > HAUTEUR_PLATEAU / 2) {
        // issue du haut
        if (lesY[0] > HAUTEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] - 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] + 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY[0], 2));
        }
        // issue du bas
        else if (lesY[0] < HAUTEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(1 - (lesY[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX[0], 2) + pow(1 - (lesY[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] - 1), 2) + pow(1 - lesY[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX[0] + 1), 2) + pow(1 - lesY[0], 2));
        }
    } else {
        distance_haut = sqrt(pow(lesPommesX[*nbPommes] - lesX[0], 2) + pow(lesPommesY[*nbPommes] - (lesY[0] - 1), 2));
        distance_bas = sqrt(pow(lesPommesX[*nbPommes] - lesX[0], 2) + pow(lesPommesY[*nbPommes] - (lesY[0] + 1), 2));
        distance_gauche = sqrt(pow(lesPommesX[*nbPommes] - (lesX[0] - 1), 2) + pow(lesPommesY[*nbPommes] - lesY[0], 2));
        distance_droite = sqrt(pow(lesPommesX[*nbPommes] - (lesX[0] + 1), 2) + pow(lesPommesY[*nbPommes] - lesY[0], 2));
    }

    int plusCourtCheminX=0, plusCourtCheminY=0;
    int plusCourtX=0, plusCourtY=0;

    // Determine la plus courte distance pour aller vers la pomme en Y
    if (distance_bas < distance_haut) {
        if (*direction == HAUT) {  // Condition pour qu'il ne fasse pas demi tour sur lui meme
            plusCourtCheminY = distance_droite;
            plusCourtY = DROITE;
        } else {
            plusCourtCheminY = distance_bas;
            plusCourtY = BAS;
        }
    } else {
        if (*direction == BAS) {
            plusCourtCheminY = distance_droite;
            plusCourtY = DROITE;
        } else {
            plusCourtCheminY = distance_haut;
            plusCourtY = HAUT;
        }
    }

    // Determine la plus courte distance pour aller vers la pomme en X
    if (distance_gauche < distance_droite) {
        if (*direction == DROITE && plusCourtCheminX != GAUCHE) {
            plusCourtCheminX = distance_bas;
            plusCourtX = BAS;
        } else if (*direction != DROITE) {
            plusCourtCheminX = distance_gauche;
            plusCourtX = GAUCHE;
        }
    } else {
        if (*direction == GAUCHE && plusCourtCheminX != DROITE) {
            plusCourtCheminX = distance_bas;
            plusCourtX = BAS;
        } else if (*direction != GAUCHE) {
            plusCourtCheminX = distance_droite;
            plusCourtX = DROITE;
        }
    }


    // Determine la plus courte distance pour aller vers la pomme
    if (plusCourtCheminX < plusCourtCheminY) {
        *direction = plusCourtX;
    } else {
        *direction = plusCourtY;
    }

    if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == 1) { // Porte en haut
        lesX[0] = LARGEUR_PLATEAU / 2;
        lesY[0] = HAUTEUR_PLATEAU - 1;
    } else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == HAUTEUR_PLATEAU) { // Porte en bas
        lesX[0] = LARGEUR_PLATEAU / 2;
        lesY[0] = 2;
    } else if (lesY[0] == HAUTEUR_PLATEAU / 2 && lesX[0] == 1) { // Porte à gauche
        lesX[0] = LARGEUR_PLATEAU - 1;
        lesY[0] = HAUTEUR_PLATEAU / 2;
    } else if (lesY[0] == HAUTEUR_PLATEAU / 2 && lesX[0] == LARGEUR_PLATEAU) { // Porte à droite
        lesX[0] = 2;
        lesY[0] = HAUTEUR_PLATEAU / 2;
    }



    // Faire progresser1 la tête dans la nouvelle direction
    int nextX = lesX[0];
    int nextY = lesY[0];

    switch (*direction) {
        case HAUT:    nextY--; break;
        case BAS:     nextY++; break;
        case GAUCHE:  nextX--; break;
        case DROITE:  nextX++; break;
    }

    // Vérifiez si la prochaine position est valide

    if (!estPositionValide1(nextX, nextY, lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
        // Essayez une autre direction
        if (*direction != HAUT && estPositionValide1(lesX[0], lesY[0] + 1, lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = BAS; // Priorité au bas
        } else if (*direction != BAS && estPositionValide1(lesX[0], lesY[0] - 1, lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = HAUT;
        } else if (*direction != DROITE && estPositionValide1(lesX[0] - 1, lesY[0], lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = GAUCHE;
        } else if (*direction != GAUCHE && estPositionValide1(lesX[0] + 1, lesY[0], lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = DROITE;
        } else {
            *collision1 = true; // Aucune direction valide
            return;
        }
    }

// Vérifiez si la prochaine position entre en collision avec l'autre serpent
for (int i = 0; i < TAILLE; i++) {
    if (nextX == lesX[i] && nextY == lesY[i]) {
        // Essayez une autre direction
        if (*direction != HAUT && estPositionValide1(lesX[0], lesY[0] + 1, lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = BAS; // Priorité au bas
        } else if (*direction != BAS && estPositionValide1(lesX[0], lesY[0] - 1, lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = HAUT;
        } else if (*direction != DROITE && estPositionValide1(lesX[0] - 1, lesY[0], lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = GAUCHE;
        } else if (*direction != GAUCHE && estPositionValide1(lesX[0] + 1, lesY[0], lesX, lesY, TAILLE, plateau, lesX2, lesY2)) {
            *direction = DROITE;
        } else {
            *collision1 = true; // Aucune direction valide
            return;
        }
        break;
    }
}

// Décide de choisir le meilleur chemin en fonction de où se situe le bloc par rapport à lui
// Si il est positionné avant la moitié du bloc, il va soit à gauche soit en bas selon d'où il vient
// Si il est positionné après la moitié du bloc, il va soit à droite soit en haut selon d'où il vient
for (int i = 0; i < NB_PAVES; i++) {
    // Si le bloc est en haut du serpent
    if (*direction == HAUT && lesX[0] >= lesPavesX[i] && lesX[0] < lesPavesX[i] + TBLOC && lesY[0] == lesPavesY[i] + TBLOC) {
        // S'il vient de la droite, il continue à gauche
        if (posa == GAUCHE) {
            *direction = GAUCHE;
        }
        // S'il vient de la gauche, il continue à droite
        else if (posa == DROITE) {
            *direction = DROITE;
        }
        // Sinon, il va soit à droite ou à gauche en fonction de ce qui est le plus proche
        else {
            if (lesX[0] < lesPavesX[i] + TBLOC / 2) {
                *direction = GAUCHE;
            } else {
                *direction = DROITE;
            }
        }
    }
    // Si le bloc est en bas
    else if (*direction == BAS && lesX[0] >= lesPavesX[i] && lesX[0] < lesPavesX[i] + TBLOC && lesY[0] == lesPavesY[i] - 1) {
        // S'il vient de la droite, il continue à gauche
        if (posa == GAUCHE) {
            *direction = GAUCHE;
        }
        // S'il vient de la gauche, il continue à droite
        else if (posa == DROITE) {
            *direction = DROITE;
        }
        // Sinon, il va soit à droite ou à gauche en fonction de ce qui est le plus proche
        else {
            if (lesX[0] < lesPavesX[i] + TBLOC / 2) {
                *direction = GAUCHE;
            } else {
                *direction = DROITE;
            }
        }
    }
    // Si le bloc est à gauche
    else if (*direction == GAUCHE && lesY[0] >= lesPavesY[i] && lesY[0] < lesPavesY[i] + TBLOC && lesX[0] == lesPavesX[i] - 1) {
        // S'il vient du bas, il va en haut
        if (posa == HAUT) {
            *direction = HAUT;
        }
        // S'il vient du haut, il va en bas
        else if (posa == BAS) {
            *direction = BAS;
        }
        // Sinon, il va soit en bas ou en haut en fonction de ce qui est le plus proche
        else {
            if (lesY[0] < lesPavesY[i] + TBLOC / 2) {
                *direction = HAUT;
            } else {
                *direction = BAS;
            }
        }
    }
    // Si le bloc est à droite
    else if (*direction == DROITE && lesY[0] >= lesPavesY[i] && lesY[0] < lesPavesY[i] + TBLOC && lesX[0] == lesPavesX[i] + TBLOC) {
        // S'il vient du bas, il va en haut
        if (posa == HAUT) {
            *direction = HAUT;
        }
        // S'il vient du haut, il va en bas
        else if (posa == BAS) {
            *direction = BAS;
        }
        // Sinon, il va soit en bas ou en haut en fonction de ce qui est le plus proche
        else {
            if (lesY[0] < lesPavesY[i] + TBLOC / 2) {
                *direction = HAUT;
            } else {
                *direction = BAS;
            }
        }
    }
}
posa = *direction;

// Mettre à jour la direction finale
switch (*direction) {
    case HAUT:    lesY[0]--; break;
    case BAS:     lesY[0]++; break;
    case GAUCHE:  lesX[0]--; break;
    case DROITE:  lesX[0]++; break;
}
(*pas1)++;

dessinerSerpent1(lesX, lesY);

// Vérifiez si la tête du serpent entre en collision avec une bordure
if (plateau[lesX[0]][lesY[0]] == BORDURE) {
    *collision1 = true;
}

// Vérifiez si la tête du serpent entre en collision avec son propre corps
for (int i = TAILLE - 1; i > 0; i--) {
    if (lesX[0] == lesX[i] && lesY[0] == lesY[i]) {
        *collision1 = true;
    }
}

dessinerSerpent1(lesX, lesY);



// La fonction "progresser2" met à jour la position du serpent à chaque itération.
// Stratégie de déplacement :
// - Calcule la distance actuelle entre la tête du serpent et la pomme.
// - Compare les distances pour chaque direction (haut, bas, gauche, droite).
// - Privilégie les mouvements vers la pomme ou vers les portes si elles permettent de réduire la distance totale.
// - Empêche le serpent de faire demi-tour sur lui-même.
// - Vérifie si la nouvelle position est valide (pas de collision avec le serpent ou les obstacles).
// - Gère la transition par les portes lorsque la position correspond à une issue.

void progresser2(int lesX2[], int lesY2[], int *direction, tPlateau plateau, bool *collision2, bool *pomme, int *nbPommes, int lesX[], int lesY[], int *pas2) {
    // Efface le dernier élément avant d'actualiser la position de tous les éléments du serpent avant de le redessiner et détecte une collision avec une pomme ou avec une bordure
    effacer(lesX2[TAILLE - 1], lesY2[TAILLE - 1]);

    for (int i = TAILLE - 1; i > 0; i--) {
        lesX2[i] = lesX2[i - 1];
        lesY2[i] = lesY2[i - 1];
    }

    int distance_haut=0, distance_bas=0, distance_gauche=0, distance_droite=0;

    // Calcul des distances pour chaque direction (vers la pomme et les portes)
    // Utilisation de sqrt pour évaluer la distance euclidienne
    int distance_serpent_pomme_ent_X = abs(lesPommesX[*nbPommes] - lesX2[0]);
    int distance_pomme_serpent_enY = abs(lesPommesY[*nbPommes] - lesY2[0]);

    // Évaluation des portes si elles permettent de réduire la distance totale
    if (distance_serpent_pomme_ent_X > LARGEUR_PLATEAU / 2 + DISTANCE_PORTAIL) {
        // issue de droite
        if (lesX2[0] > LARGEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY2[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY2[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] - 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY2[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] + 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY2[0], 2));
        }
        // issue de gauche
        else if (lesX2[0] < LARGEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(1 - (lesY2[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(1 - (lesY2[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] - 1), 2) + pow(1 - lesY2[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] + 1), 2) + pow(1 - lesY2[0], 2));
        }
    } else if (distance_pomme_serpent_enY > HAUTEUR_PLATEAU / 2) {
        // issue du haut
        if (lesY2[0] > HAUTEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY2[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(HAUTEUR_PLATEAU / 2 - (lesY2[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] - 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY2[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] + 1), 2) + pow(HAUTEUR_PLATEAU / 2 - lesY2[0], 2));
        }
        // issue du bas
        else if (lesY2[0] < HAUTEUR_PLATEAU / 2) {
            distance_haut = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(1 - (lesY2[0] - 1), 2));
            distance_bas = sqrt(pow(LARGEUR_PLATEAU - lesX2[0], 2) + pow(1 - (lesY2[0] + 1), 2));
            distance_gauche = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] - 1), 2) + pow(1 - lesY2[0], 2));
            distance_droite = sqrt(pow(LARGEUR_PLATEAU - (lesX2[0] + 1), 2) + pow(1 - lesY2[0], 2));
        }
    } else {
        distance_haut = sqrt(pow(lesPommesX[*nbPommes] - lesX2[0], 2) + pow(lesPommesY[*nbPommes] - (lesY2[0] - 1), 2));
        distance_bas = sqrt(pow(lesPommesX[*nbPommes] - lesX2[0], 2) + pow(lesPommesY[*nbPommes] - (lesY2[0] + 1), 2));
        distance_gauche = sqrt(pow(lesPommesX[*nbPommes] - (lesX2[0] - 1), 2) + pow(lesPommesY[*nbPommes] - lesY2[0], 2));
        distance_droite = sqrt(pow(lesPommesX[*nbPommes] - (lesX2[0] + 1), 2) + pow(lesPommesY[*nbPommes] - lesY2[0], 2));
    }

    int plusCourtCheminX=0, plusCourtCheminY=0;
    int plusCourtX=0, plusCourtY=0;

    // Determine la plus courte distance pour aller vers la pomme en Y
    if (distance_bas < distance_haut) {
        if (*direction == HAUT) {  // Condition pour qu'il ne fasse pas demi tour sur lui meme
            plusCourtCheminY = distance_droite;
            plusCourtY = DROITE;
        } else {
            plusCourtCheminY = distance_bas;
            plusCourtY = BAS;
        }
    } else {
        if (*direction == BAS) {
            plusCourtCheminY = distance_droite;
            plusCourtY = DROITE;
        } else {
            plusCourtCheminY = distance_haut;
            plusCourtY = HAUT;
        }
    }

    // Determine la plus courte distance pour aller vers la pomme en X
    if (distance_gauche < distance_droite) {
        if (*direction == DROITE && plusCourtCheminX != GAUCHE) {
            plusCourtCheminX = distance_bas;
            plusCourtX = BAS;
        } else if (*direction != DROITE) {
            plusCourtCheminX = distance_gauche;
            plusCourtX = GAUCHE;
        }
    } else {
        if (*direction == GAUCHE && plusCourtCheminX != DROITE) {
            plusCourtCheminX = distance_bas;
            plusCourtX = BAS;
        } else if (*direction != GAUCHE) {
            plusCourtCheminX = distance_droite;
            plusCourtX = DROITE;
        }
    }


    // Determine la plus courte distance pour aller vers la pomme
    if (plusCourtCheminX < plusCourtCheminY) {
        *direction = plusCourtX;
    } else {
        *direction = plusCourtY;
    }

    if (lesX2[0] == LARGEUR_PLATEAU / 2 && lesY2[0] == 1) { // Porte en haut
        lesX2[0] = LARGEUR_PLATEAU / 2;
        lesY2[0] = HAUTEUR_PLATEAU - 1;
    } else if (lesX2[0] == LARGEUR_PLATEAU / 2 && lesY2[0] == HAUTEUR_PLATEAU) { // Porte en bas
        lesX2[0] = LARGEUR_PLATEAU / 2;
        lesY2[0] = 2;
    } else if (lesY2[0] == HAUTEUR_PLATEAU / 2 && lesX2[0] == 1) { // Porte à gauche
        lesX2[0] = LARGEUR_PLATEAU - 1;
        lesY2[0] = HAUTEUR_PLATEAU / 2;
    } else if (lesY2[0] == HAUTEUR_PLATEAU / 2 && lesX2[0] == LARGEUR_PLATEAU) { // Porte à droite
        lesX2[0] = 2;
        lesY2[0] = HAUTEUR_PLATEAU / 2;
    }



    // Faire progresser1 la tête dans la nouvelle direction
    int nextX = lesX2[0];
    int nextY = lesY2[0];

    switch (*direction) {
        case HAUT:    nextY--; break;
        case BAS:     nextY++; break;
        case GAUCHE:  nextX--; break;
        case DROITE:  nextX++; break;
    }

    
    // Vérifiez si la prochaine position est valide
    if (!estPositionValide2(nextX, nextY, lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
        // Essayez une autre direction
        if (*direction != HAUT && estPositionValide2(lesX2[0], lesY2[0] + 1, lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
            *direction = BAS; // Priorité au bas
        } else if (*direction != BAS && estPositionValide2(lesX2[0], lesY2[0] - 1, lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
            *direction = HAUT;
        } else if (*direction != DROITE && estPositionValide2(lesX2[0] - 1, lesY2[0], lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
            *direction = GAUCHE;
        } else if (*direction != GAUCHE && estPositionValide2(lesX2[0] + 1, lesY2[0], lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
            *direction = DROITE;
        } else {
            *collision2 = true; // Aucune direction valide
            return;
        }
    }

    // Vérifiez si la prochaine position entre en collision avec l'autre serpent
    for (int i = 0; i < TAILLE; i++) {
        if (nextX == lesX[i] && nextY == lesY[i]) {
            // Essayez une autre direction
            if (*direction != HAUT && estPositionValide2(lesX2[0], lesY2[0] + 2, lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
                *direction = BAS; // Priorité au bas
            } else if (*direction != BAS && estPositionValide2(lesX2[0], lesY2[0] - 2, lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
                *direction = HAUT;
            } else if (*direction != DROITE && estPositionValide2(lesX2[0] - 2, lesY2[0], lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
                *direction = GAUCHE;
            } else if (*direction != GAUCHE && estPositionValide2(lesX2[0] + 2, lesY2[0], lesX2, lesY2, TAILLE, plateau, lesX, lesY)) {
                *direction = DROITE;
            } else {
                *collision2 = true; // Aucune direction valide
                return;
            }
            break;
        }
    }

    //decide de choisir le meilleur chemin en fonction de ou se situe le bloc par rapport a lui 
    //si il est positionné avant la moitié du bloc il va sois a gauche soit en bas selon d'ou il vient
    //si il est positionné apres la moitié du bloc il va sois a droite soit en haut selon d'ou il vient
    for (int i = 0; i < NB_PAVES; i++) {
        // Si le bloc est en haut du serpent
        if (*direction == HAUT && lesX2[0] >= lesPavesX[i] && lesX2[0] < lesPavesX[i] + TBLOC && lesY2[0] == lesPavesY[i] + TBLOC) {
            // S'il vient de la droite, il continue à gauche
            if (posa2 == GAUCHE) {
                *direction = GAUCHE;
            }
            // S'il vient de la gauche, il continue à droite
            else if (posa2 == DROITE) {
                *direction = DROITE;
            }
            // Sinon, il va soit à droite ou à gauche en fonction de ce qui est le plus proche
            else {
                if (lesX2[0] < lesPavesX[i] + TBLOC / 2) {
                    *direction = GAUCHE;
                } else {
                    *direction = DROITE;
                }
            }
        }
        // Si le bloc est en bas
        else if (*direction == BAS && lesX2[0] >= lesPavesX[i] && lesX2[0] < lesPavesX[i] + TBLOC && lesY2[0] == lesPavesY[i] - 1) {
            // S'il vient de la droite, il continue à gauche
            if (posa2 == GAUCHE) {
                *direction = GAUCHE;
            }
            // S'il vient de la gauche, il continue à droite
            else if (posa2 == DROITE) {
                *direction = DROITE;
            }
            // Sinon, il va soit à droite ou à gauche en fonction de ce qui est le plus proche
            else {
                if (lesX2[0] < lesPavesX[i] + TBLOC / 2) {
                    *direction = GAUCHE;
                } else {
                    *direction = DROITE;
                }
            }
        }
        // Si le bloc est à gauche
        else if (*direction == GAUCHE && lesY2[0] >= lesPavesY[i] && lesY2[0] < lesPavesY[i] + TBLOC && lesX2[0] == lesPavesX[i] - 1) {
            // S'il vient du bas, il va en haut
            if (posa2 == HAUT) {
                *direction = HAUT;
            }
            // S'il vient du haut, il va en bas
            else if (posa2 == BAS) {
                *direction = BAS;
            }
            // Sinon, il va soit en bas ou en haut en fonction de ce qui est le plus proche
            else {
                if (lesY2[0] < lesPavesY[i] + TBLOC / 2) {
                    *direction = HAUT;
                } else {
                    *direction = BAS;
                }
            }
        }
        // Si le bloc est à droite
        else if (*direction == DROITE && lesY2[0] >= lesPavesY[i] && lesY2[0] < lesPavesY[i] + TBLOC && lesX2[0] == lesPavesX[i] + TBLOC) {
            // S'il vient du bas, il va en haut
            if (posa2 == HAUT) {
                *direction = HAUT;
            }
            // S'il vient du haut, il va en bas
            else if (posa2 == BAS) {
                *direction = BAS;
            }
            // Sinon, il va soit en bas ou en haut en fonction de ce qui est le plus proche
            else {
                if (lesY2[0] < lesPavesY[i] + TBLOC / 2) {
                    *direction = HAUT;
                } else {
                    *direction = BAS;
                }
            }
        }
    }
    posa2 = *direction;


    // Mettre à jour la direction finale
    switch (*direction) {
        case HAUT:    lesY2[0]--; break;
        case BAS:     lesY2[0]++; break;
        case GAUCHE:  lesX2[0]--; break;
        case DROITE:  lesX2[0]++; break;
    }

    (*pas2)++;
    dessinerSerpent2(lesX2, lesY2);

    if (plateau[lesX2[0]][lesY2[0]] == BORDURE) {
        *collision2 = true;
    }

    for (int i = TAILLE; i > 0; i--) {
        if (lesX2[0] == lesX2[i] && lesY2[0] == lesY2[i]) {
            *collision2 = true;
        }
    }

    dessinerSerpent2(lesX2, lesY2);
}


void gererPomme(int lesX[], int lesY[], int lesX2[], int lesY2[], tPlateau plateau, bool *pomme, int *nbPommes, int *nbpommeMangeeS1, int *nbpommeMangeeS2) {
    *pomme = false;
    // Détection d'une "collision" avec une pomme pour le serpent 1
    if (plateau[lesX[0]][lesY[0]] == POMME) {
        *pomme = true;
        // La pomme disparaît du plateau
        plateau[lesX[0]][lesY[0]] = VIDE;
        *nbpommeMangeeS1 += 1;
    }

    // Détection d'une "collision" avec une pomme pour le serpent 2
    if (plateau[lesX2[0]][lesY2[0]] == POMME) {
        *pomme = true;
        // La pomme disparaît du plateau
        plateau[lesX2[0]][lesY2[0]] = VIDE;
        *nbpommeMangeeS2 += 1;
    }
}


/******************************-****************/
/*				 FONCTIONS UTILITAIRES 			*/
/************************************************/
void gotoxy(int x, int y) { 
    printf("\033[%d;%df", y, x);
}

int kbhit(){
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}

// Fonction pour désactiver l'echo
void disable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Désactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour réactiver l'echo
void enable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Réactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour évaluer la validité de la prochaine position du serpent 1
bool estPositionValide1(int x, int y, int lesX[], int lesY[], int taille, tPlateau plateau, int lesX2[], int lesY2[]) {
    // Vérifie si la position est occupée par le corps du serpent 1
    for (int i = 0; i < taille; i++) {
        if (lesX[i] == x && lesY[i] == y) {
            return false;
        }
        // Vérifie si la position est occupée par le corps du serpent 2
        if (lesX2[i] == x && lesY2[i] == y) {
            return false;
        }
    }
    
    // Vérifie si la position est en dehors des limites du plateau ou sur une bordure
    if (x < 1 || x > LARGEUR_PLATEAU || y < 1 || y > HAUTEUR_PLATEAU || plateau[x][y] == BORDURE) {
        return false;
    }

    // Vérifie si la position est occupée par le corps ou la tête d'un serpent
    if (plateau[x][y] == CORPS || plateau[x][y] == TETE_1 || plateau[x][y] == TETE_2) {
        return false;
    }

    // Vérifie si la position est occupée par un bloc
    for (int i = 0; i < NB_PAVES; i++) {
        if (x >= lesPavesX[i] && x < lesPavesX[i] + TBLOC && y >= lesPavesY[i] && y < lesPavesY[i] + TBLOC) {
            return false;
        }
    }
    return true;
}

// Fonction pour évaluer la validité de la prochaine position du serpent 2
bool estPositionValide2(int x, int y, int lesX2[], int lesY2[], int taille, tPlateau plateau, int lesX[], int lesY[]) {
    // Vérifie si la position est occupée par le corps du serpent 2
    for (int i = 0; i < taille; i++) {
        if (lesX2[i] == x && lesY2[i] == y) {
            return false;
        }
        // Vérifie si la position est occupée par le corps du serpent 1
        if (lesX[i] == x && lesY[i] == y) {
            return false;
        }
    }
    
    // Vérifie si la position est en dehors des limites du plateau ou sur une bordure
    if (x < 1 || x > LARGEUR_PLATEAU || y < 1 || y > HAUTEUR_PLATEAU || plateau[x][y] == BORDURE) {
        return false;
    }

    // Vérifie si la position est occupée par le corps ou la tête d'un serpent
    if (plateau[x][y] == CORPS || plateau[x][y] == TETE_1 || plateau[x][y] == TETE_2) {
        return false;
    }

    // Vérifie si la position est occupée par un bloc
    for (int i = 0; i < NB_PAVES; i++) {
        if (x >= lesPavesX[i] && x < lesPavesX[i] + TBLOC && y >= lesPavesY[i] && y < lesPavesY[i] + TBLOC) {
            return false;
        }
    }

    return true;
}

//dessine un bloc au coordonnées x y 
void dessiner1Bloc(int x,int y){
	for(int i=0;i<TBLOC;i++){
		for(int j=0;j<TBLOC;j++){
			gotoxy(x+i,y+j);
			printf("%c",BORDURE);
		}
	}
}

//dessine tout les blocs
void dessinerBlocs(int lesX[],int lesY[]){
    for(int k=0;k<NB_PAVES;k++){
		dessiner1Bloc(lesX[k],lesY[k]);
	}
}