#ifndef BOLA_H
#define BOLA_H

#include "comum.h"
#include "jogador.h"

// Estrutura que representa uma bola do Arkanoid
struct bola {
    Vector2 position; // posição da bola
    Vector2 vel;      // velocidade (x, y)
    float raio;       // raio da bola
    bool ativa;       // se está ativa na tela
    bool explosive;   // se está com efeito explosivo
};

// Inicializa todas as bolas no início do jogo
void InicializarBolas(struct bola bolas[]);
// Atualiza física, colisão e efeitos das bolas
void AtualizarBolas(struct bola bolas[], struct barra* barra, struct tira tiras[TIRA_LINHAS][TIRA_COLS], int* score, int* piscarFrames, struct buffItem* poder, EstadoBoss estadoBoss);
// Desenha todas as bolas na tela
void DesenharBolas(struct bola bolas[]);
// Atualiza e desenha o power-up (buff) na tela
void AtualizarE_DesenharBuff(struct buffItem* poder, struct barra* barra, struct bola bolas[]);

#endif