#ifndef JOGADOR_H
#define JOGADOR_H

#include "comum.h"

struct barra {
    Vector2 position;
    Vector2 tamanho;
    float vel;
    Color cor;
};

void InicializarJogador(struct barra* b);
void AtualizarJogador(struct barra* b, int* piscarFrames);
void DesenharJogador(struct barra* b);

#endif