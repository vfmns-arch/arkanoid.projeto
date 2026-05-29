#ifndef BOSS_H
#define BOSS_H

#include "comum.h"
#include "bola.h"
#include "jogador.h"

// Estrutura que representa o chefe (boss) do Arkanoid
struct boss_Arkanoid {
    Vector2 posicao;         // posição do boss na tela
    Vector2 tamanho_total;   // tamanho total do boss
    int vida_maxima;         // vida máxima
    int vida_atual;          // vida atual
    EstadoBoss estado;       // estado atual (IA)
    float tempo_ultimo_tiro; // controle de tiros
    float tempo_laser;       // controle do laser
    bool carregando_laser;   // está carregando laser?
    bool disparando_laser;   // está disparando laser?
    float laser_x;           // posição do laser
};

// Inicializa o boss e seus projéteis
void InicializarBoss(struct boss_Arkanoid* boss, struct projetil tiros[]);
// Atualiza IA, colisão e ataques do boss
void AtualizarBoss(struct boss_Arkanoid* boss, struct bola bolas[], struct projetil tiros[], struct barra* barra, int* score, bool* gameover, bool* gamewin, float* velX);
// Desenha o boss e seus projéteis
void DesenharBoss(struct boss_Arkanoid* boss, struct projetil tiros[]);

#endif