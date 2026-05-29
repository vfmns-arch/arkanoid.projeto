#ifndef BOSS_H
#define BOSS_H

#include "comum.h"
#include "bola.h"
#include "jogador.h"

struct boss_Arkanoid {
    Vector2 posicao;
    Vector2 tamanho_total;
    int vida_maxima;
    int vida_atual;
    EstadoBoss estado;
    float tempo_ultimo_tiro;
    float tempo_laser;
    bool carregando_laser;
    bool disparando_laser;
    float laser_x;
};

void InicializarBoss(struct boss_Arkanoid* boss, struct projetil tiros[]);
void AtualizarBoss(struct boss_Arkanoid* boss, struct bola bolas[], struct projetil tiros[], struct barra* barra, int* score, bool* gameover, bool* gamewin, float* velX);
void DesenharBoss(struct boss_Arkanoid* boss, struct projetil tiros[]);

#endif