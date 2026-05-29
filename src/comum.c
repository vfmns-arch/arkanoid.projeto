#include "comum.h"

// Constantes de configuração da tela e blocos
const int TELA_LARGURA = 800; 
const int TELA_ALTURA = 600;
const int TIRA_LINHAS = 5;
const int TIRA_COLS = 10;
const int TIRA_LARGURA = 60;
const int TIRA_ALTURA = 20;
const int TIRA_MEIO = 10;
const int TIRA_STARTY = 80; 
const int TIRA_STARTX = 50;

// Matriz que define o "desenho" do boss na tela
int arte_boss[BOSS_LINHAS][BOSS_COLUNAS] = {
    {0, 1, 2, 2, 2, 2, 1, 0},
    {1, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 3, 2, 2, 3, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 1},
    {0, 1, 2, 1, 1, 2, 1, 0},
    {0, 0, 1, 2, 2, 1, 0, 0}
};