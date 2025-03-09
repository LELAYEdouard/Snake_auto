
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#define TAILLE_SERPENT_MAX 100 ///< Taille maximale autorisée pour le serpent.
#define COORD_DEP_X 40         ///< Coordonnée X initiale de départ du serpent.
#define COORD_DEP_Y 20         ///< Coordonnée Y initiale de départ du serpent.
#define LIGNES 40              ///< Nombre total de lignes sur le plateau de jeu.
#define COLONNES 80            ///< Nombre total de colonnes sur le plateau de jeu.
#define MINLONG 2              ///< Longueur minimale d'un pavé.
#define MINLARG 2              ///< Largeur minimale d'un pavé.
#define NB_POMMES 10             ///< Nombre de pommes sur le plateau.

#define TOUCHE_HAUT 'z'        ///< Touche pour aller vers le haut.
#define TOUCHE_GAUCHE 'q'      ///< Touche pour aller vers la gauche.
#define TOUCHE_BAS 's'         ///< Touche pour aller vers le bas.
#define TOUCHE_DROITE 'd'      ///< Touche pour aller vers la droite.
#define TOUCHE_QUITTER 'a'     ///< Touche pour quitter le jeu.

#define TETE 'O'               ///< Caractère représentant la tête du serpent.
#define ANNEAUX 'X'            ///< Caractère représentant les anneaux du serpent.
#define CAR_VIDE ' '           ///< Caractère représentant une case vide.
#define BORDURURE_PAVES '#'    ///< Caractère représentant les bordures des pavés.
#define CRA_POMME '6'          ///< Caractère représentant une pomme.

int tailleSerpentActuelle = 10;

// Position des pommes sur le plateau
int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = { 8, 39, 2, 2, 5, 39, 33, 38, 35, 2};

void afficher(int x, int y, char c);
void effacer(int x, int y);
void gotoXY(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, bool *collision, bool *pommeMangee, int posX[], int posY[]);
void initPlateau(char grille[LIGNES][COLONNES]);
void dessinerPlateau(char grille[LIGNES][COLONNES]);
void ajouterPomme(int posX[], int posY[], int coX[], int coY[]);
void disableEcho();
void enableEcho();
int kbhit();
int distance(int x1, int y1, int x2, int y2);
int positionOccupee(int lesX[], int lesY[], int taille, int x, int y);
char trouverPomme(int lesX[], int lesY[], int taille, int pomX, int pomY, char directionActuelle);

/*****************************************************
 *                 Implementation                    *
 *****************************************************/

int main() {
    srand(time(NULL)); /**< Initialisation de la graine aléatoire pour la génération des blocs et pommes. */

    // Variables principales
    char input = ' '; /**< Dernière touche appuyée par le joueur. */
    int coX[TAILLE_SERPENT_MAX], coY[TAILLE_SERPENT_MAX]; /**< Coordonnées du serpent. */
    char direction = TOUCHE_DROITE; /**< Direction initiale du serpent. */
    char GRID[LIGNES][COLONNES]; /**< Grille de jeu. */
    bool collision = false; /**< Indicateur de collision (fin de partie). */
    int posX[NB_POMMES], posY[NB_POMMES]; /**< Coordonnées des pommes. */
    int vitesse = 200000; /**< Vitesse initiale du jeu (en microsecondes). */
    int compteur = 0; /**< Compteur de pommes mangées. */
    bool pommeMangee = false; /**< Indicateur si une pomme est mangée. */
    int pas = 0;

    // Initialisation du jeu
    system("clear"); /**< Efface l'écran avant de démarrer le jeu. */
    disableEcho(); /**< Désactive l'écho du terminal pour une meilleure interaction. */

    // Position initiale du serpent
    for (int i = 0; i < TAILLE_SERPENT_MAX; i++) {
        coX[i] = COORD_DEP_X - i; /**< Décalage horizontal pour la queue du serpent. */
        coY[i] = COORD_DEP_Y; /**< Position verticale fixe. */
    }

    // Placement initial des obstacles et des pommes
    ajouterPomme(posX, posY, coX, coY);

    clock_t begin = clock(); // Démarage du timer afin de calculer le temps CPU 
    // Boucle principale du jeu
    while (input != TOUCHE_QUITTER && !collision && compteur != 10) {
        system("clear"); /**< Efface l'écran à chaque itération pour redessiner le plateau. */
        initPlateau(GRID); /**< Réinitialise la grille à chaque itération. */
        dessinerPlateau(GRID); /**< Affiche la grille. */


        // Dessine les pommes sur le plateau
        for (int k = 0; k < NB_POMMES; k++) {
            gotoXY(posX[k], posY[k]); /**< Positionne le curseur sur la pomme. */
            printf("%c", CRA_POMME); /**< Affiche une pomme. */
        }

        // Dessine le serpent
        dessinerSerpent(coX, coY); /**< Affiche le serpent à ses coordonnées actuelles. */

        // Met à jour la position et gère les interactions
        progresser(coX, coY, direction, &collision, &pommeMangee, posX, posY);
        pas++;
        // Gestion des pommes mangées
        if (pommeMangee) {
            ajouterPomme(posX, posY, coX, coY); 
            compteur++;
        }

        fflush(stdout); /**< Vide le buffer d'affichage pour éviter les retards. */
        usleep(vitesse); /**< Pause pour ralentir le jeu. */
        direction = trouverPomme(coX, coY, TAILLE_SERPENT_MAX, posX[0], posY[0], direction);
        if (kbhit()) {
            input = getchar();
        }
    }
    clock_t end = clock(); // Fin du timer CPU

    // Fin du jeu
    system("clear"); /**< Efface l'écran avant de quitter. */
    enableEcho(); /**< Réactive l'écho du terminal. */
    // Affichage des données (temps CPU et nb de déplacements)
    double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;
    printf("Temps CPU = %.3f secondes\n", tmpsCPU);
    printf("Nombres de déplacements : %d", pas);
    return EXIT_SUCCESS;
}

/*****************************************************
 *           5. Procédures et fonctions              *
 *****************************************************/

void afficher(int x, int y, char c) {
    gotoXY(x, y);
    printf("%c", c);
    gotoXY(1, 1); // Remet le curseur au coin supérieur gauche
}

void effacer(int x, int y) {
    gotoXY(x, y);
    printf(" ");
}

void dessinerSerpent(int lesX[], int lesY[]) {
    afficher(lesX[0], lesY[0], TETE);
    for (int i = 1; i < tailleSerpentActuelle; i++) {
        afficher(lesX[i], lesY[i], ANNEAUX);
    }
}

void progresser(int lesX[], int lesY[], char direction, bool *collision, bool *pommeMangee, int posX[], int posY[]) {
    *pommeMangee = false; // Initialisation : aucune pomme n'est mangée au début de l'appel.

    // Empêcher de faire demi-tour (directions opposées)
    if ((direction == TOUCHE_DROITE && direction != TOUCHE_GAUCHE) ||
        (direction == TOUCHE_GAUCHE && direction != TOUCHE_DROITE) ||
        (direction == TOUCHE_HAUT && direction != TOUCHE_BAS) ||
        (direction == TOUCHE_BAS && direction != TOUCHE_HAUT)) {
        // Ne rien faire si la direction est opposée
        direction = direction;
    }

    // Gestion des collisions avec les bords
    if ((direction == TOUCHE_DROITE && lesX[0] == COLONNES - 1 && lesY[0] == (LIGNES / 2) + 1) ||
        (direction == TOUCHE_GAUCHE && lesX[0] == MINLARG && lesY[0] == (LIGNES / 2) + 1)) {
        lesX[0] = (direction == TOUCHE_DROITE) ? MINLARG : COLONNES - 1;
    } else if ((direction == TOUCHE_HAUT && lesY[0] == MINLONG && lesX[0] == (COLONNES / 2) + 1) ||
               (direction == TOUCHE_BAS && lesY[0] == LIGNES - 1 && lesX[0] == (COLONNES / 2) + 1)) {
        lesY[0] = (direction == TOUCHE_HAUT) ? LIGNES - 1 : MINLONG;
    } else if ((direction==TOUCHE_DROITE && lesX[0]==COLONNES-1 )|| 
                (direction==TOUCHE_GAUCHE && lesX[0]==MINLARG) || 
                (direction==TOUCHE_HAUT && lesY[0]==MINLONG) || 
                (direction==TOUCHE_BAS && lesY[0]==LIGNES-1)) {
        *collision = true;
    }

    // Vérifie si le serpent se mord lui-même
    for (int i = 1; i < tailleSerpentActuelle; i++) {
        if (lesX[0] == lesX[i] && lesY[0] == lesY[i]) {
            *collision = true;
        }
    }

    // Mise à jour des coordonnées du corps du serpent
    for (int i = tailleSerpentActuelle - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    // Mise à jour de la tête du serpent
    if (direction == TOUCHE_HAUT) lesY[0]--;
    else if (direction == TOUCHE_GAUCHE) lesX[0]--;
    else if (direction == TOUCHE_BAS) lesY[0]++;
    else if (direction == TOUCHE_DROITE) lesX[0]++;

    // Vérifie si la tête du serpent est sur une pomme
    for (int i = 0; i < NB_POMMES; i++) {
        if (lesX[0] == posX[i] && lesY[0] == posY[i]) {
            *pommeMangee = true; // Une pomme est mangée
            break;
        }
    }
}


void initPlateau(char grille[LIGNES][COLONNES]) {
    for (int i = 0; i < LIGNES; i++) {
        for (int j = 0; j < COLONNES; j++) {
            // Initialisation des bordures
            if (i == 0 || i == LIGNES - 1 || j == 0 || j == COLONNES - 1) {
                // Ajouter des espaces au centre des bordures
                if ((i == 0 || i == LIGNES - 1) && j == COLONNES / 2) {
                    grille[i][j] = CAR_VIDE;
                } else if ((j == 0 || j == COLONNES - 1) && i == LIGNES / 2) {
                    grille[i][j] = CAR_VIDE;
                } else {
                    grille[i][j] = BORDURURE_PAVES;
                }
            } else {
                // Intérieur vide
                grille[i][j] = CAR_VIDE;
            }
        }
    }
}


void dessinerPlateau(char grille[LIGNES][COLONNES]) {
    for (int i = 0; i < LIGNES; i++) {
        for (int j = 0; j < COLONNES; j++) {
            printf("%c", grille[i][j]);
        }
        printf("\n");
    }
}

void ajouterPomme(int posX[], int posY[], int coX[], int coY[]) {
    static int indexPomme = 0;  // Index pour suivre quelle pomme afficher
    posX[0] = lesPommesX[indexPomme];
    posY[0] = lesPommesY[indexPomme];

    indexPomme = (indexPomme + 1) % NB_POMMES;  // Passer à la pomme suivante, revenir au début si nécessaire
}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void gotoXY(int x, int y) {
    printf("\033[%d;%df", y, x);
}

void disableEcho() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void enableEcho() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}


int distance(int x1, int y1, int x2, int y2) {
    return abs(x2 - x1) + abs(y2 - y1);
}

// Fonction pour vérifier si une position est occupée par le corps du serpent
int positionOccupee(int lesX[], int lesY[], int taille, int x, int y) {
    for (int i = 0; i < taille; i++) {
        if (lesX[i] == x && lesY[i] == y) {
            return 1; // La position est occupée
        }
    }
    return 0; // La position est libre
}


/*
1. Calcul de la Distance Directe
La première étape consiste à calculer la distance directe entre
 la tête du serpent et la pomme. Cette distance est calculée en utilisant 
 la distance de Manhattan, qui est la somme des différences absolues des 
 coordonnées X et Y. Cette méthode est simple et efficace pour déterminer 
 la proximité de la pomme par rapport à la tête du serpent.

2. Calcul de la Distance en Utilisant les Trous
Le plateau de jeu peut contenir des trous qui permettent au serpent 
de traverser d'un côté à l'autre. La fonction vérifie si le serpent est
 à une position de trou et calcule la distance en utilisant ces trous. 
 Cela permet au serpent de prendre des raccourcis en utilisant les bords 
 du plateau pour atteindre la pomme plus rapidement.

3. Décision de la Direction
La fonction décide de la direction à prendre en fonction de la distance la 
plus courte. Si la distance en utilisant les trous est plus courte, le serpent
 utilise les trous pour atteindre la pomme. Sinon, il prend le chemin direct. 
 Cette décision est basée sur la position actuelle du serpent et la position 
 de la pomme, en tenant compte des obstacles potentiels.

4. Vérification des Demi-Tours
Pour éviter que le serpent ne se mordre lui-même, la fonction vérifie si le 
demi-tour est sûr. Si le demi-tour est sûr, il est autorisé. Sinon, le serpent 
essaie une autre direction. Cette vérification est cruciale pour éviter les collisions 
fatales avec le corps du serpent.

5. Essayer une Autre Direction si le Demi-Tour n'est pas Sûr
Si le demi-tour n'est pas sûr, le serpent essaie une autre direction. Par exemple, 
si le serpent se déplace vers la droite ou la gauche et tente de faire demi-tour, 
il vérifie s'il peut se déplacer vers le haut ou le bas. Cette logique permet au serpent 
de continuer à avancer vers la pomme sans se bloquer.

6. Garder la Direction Actuelle si Aucune Direction n'est Trouvée
Si aucune nouvelle direction n'est trouvée, le serpent garde la direction actuelle 
pour éviter de se bloquer. Cette mesure de sécurité assure que le serpent continue à 
avancer même si aucune direction alternative n'est immédiatement disponible.

7. Retourner la Nouvelle Direction
Enfin, la fonction retourne la nouvelle direction calculée. Cette direction est 
utilisée pour mettre à jour la position du serpent et le faire avancer vers la pomme.*/

char trouverPomme(int lesX[], int lesY[], int taille, int pomX, int pomY, char directionActuelle) {
    // Calculer la distance entre la tête du serpent et la pomme
    int distDirecte = distance(lesX[0], lesY[0], pomX, pomY);

    // Distance en utilisant les trous
    int distAvecTrou = INT_MAX;

    // Vérifier si le serpent doit utiliser un trou
    if (lesX[0] == COLONNES - 1 && lesY[0] == LIGNES / 2) {  // Trou droit
        distAvecTrou = distance(0, lesY[0], pomX, pomY);
    } else if (lesX[0] == 0 && lesY[0] == LIGNES / 2) {  // Trou gauche
        distAvecTrou = distance(COLONNES - 1, lesY[0], pomX, pomY);
    } else if (lesY[0] == 0 && lesX[0] == COLONNES / 2) {  // Trou haut
        distAvecTrou = distance(lesX[0], LIGNES - 1, pomX, pomY);
    } else if (lesY[0] == LIGNES - 1 && lesX[0] == COLONNES / 2) {  // Trou bas
        distAvecTrou = distance(lesX[0], 0, pomX, pomY);
    }

    // Décider de la direction en fonction de la distance la plus courte
    char newDirection = '\0'; // Variable pour stocker la nouvelle direction

    if (distAvecTrou < distDirecte) {
        // Utiliser un trou pour atteindre la pomme
        if (lesX[0] > pomX && !positionOccupee(lesX, lesY, taille, lesX[0] - 1, lesY[0])) newDirection = TOUCHE_GAUCHE;
        else if (lesX[0] < pomX && !positionOccupee(lesX, lesY, taille, lesX[0] + 1, lesY[0])) newDirection = TOUCHE_DROITE;
        else if (lesY[0] > pomY && !positionOccupee(lesX, lesY, taille, lesX[0], lesY[0] - 1)) newDirection = TOUCHE_HAUT;
        else if (lesY[0] < pomY && !positionOccupee(lesX, lesY, taille, lesX[0], lesY[0] + 1)) newDirection = TOUCHE_BAS;
    } else {
        // Ne pas utiliser de trou, prendre le chemin direct
        if (lesX[0] > pomX && !positionOccupee(lesX, lesY, taille, lesX[0] - 1, lesY[0])) newDirection = TOUCHE_GAUCHE;
        else if (lesX[0] < pomX && !positionOccupee(lesX, lesY, taille, lesX[0] + 1, lesY[0])) newDirection = TOUCHE_DROITE;
        else if (lesY[0] > pomY && !positionOccupee(lesX, lesY, taille, lesX[0], lesY[0] - 1)) newDirection = TOUCHE_HAUT;
        else if (lesY[0] < pomY && !positionOccupee(lesX, lesY, taille, lesX[0], lesY[0] + 1)) newDirection = TOUCHE_BAS;
    }

    // Vérifier si le demi-tour est sûr
    if ((directionActuelle == TOUCHE_DROITE && newDirection == TOUCHE_GAUCHE) ||
        (directionActuelle == TOUCHE_GAUCHE && newDirection == TOUCHE_DROITE) ||
        (directionActuelle == TOUCHE_HAUT && newDirection == TOUCHE_BAS) ||
        (directionActuelle == TOUCHE_BAS && newDirection == TOUCHE_HAUT)) {
        // Vérifier si le demi-tour est sûr
        if (!positionOccupee(lesX, lesY, taille, lesX[0] - (directionActuelle == TOUCHE_DROITE ? 1 : -1), lesY[0]) &&
            !positionOccupee(lesX, lesY, taille, lesX[0], lesY[0] - (directionActuelle == TOUCHE_BAS ? 1 : -1))) {
            return newDirection;
        }
    }

    // Si le demi-tour n'est pas sûr, essayer une autre direction
    if (directionActuelle == TOUCHE_DROITE || directionActuelle == TOUCHE_GAUCHE) {
        if (lesY[0] > pomY && !positionOccupee(lesX, lesY, taille, lesX[0], lesY[0] - 1)) newDirection = TOUCHE_HAUT;
        else if (lesY[0] < pomY && !positionOccupee(lesX, lesY, taille, lesX[0], lesY[0] + 1)) newDirection = TOUCHE_BAS;
    } else {
        if (lesX[0] > pomX && !positionOccupee(lesX, lesY, taille, lesX[0] - 1, lesY[0])) newDirection = TOUCHE_GAUCHE;
        else if (lesX[0] < pomX && !positionOccupee(lesX, lesY, taille, lesX[0] + 1, lesY[0])) newDirection = TOUCHE_DROITE;
    }

    // Si aucune direction n'est trouvée, garder la direction actuelle
    if (newDirection == '\0') {
        newDirection = directionActuelle;
    }

    // Retourner la nouvelle direction
    return newDirection;
}
