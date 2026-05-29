// src/bola.h
#ifndef BOLA_H
#define BOLA_H

#include "comum.h"
#include "jogador.h"

struct bola {
    Vector2 position;
    Vector2 vel;
    float raio;
    bool ativa;
    bool explosive;
};

void InicializarBolas(struct bola bolas[]);
void AtualizarBolas(struct bola bolas[], struct barra* barra, struct tira tiras[TIRA_LINHAS][TIRA_COLS], int* score, int* piscarFrames, struct buffItem* poder, EstadoBoss estadoBoss);
void DesenharBolas(struct bola bolas[]);
void AtualizarE_DesenharBuff(struct buffItem* poder, struct barra* barra, struct bola bolas[]);

#endif