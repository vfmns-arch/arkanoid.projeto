#include "jogador.h"

// Inicializa a barra do jogador na posição e tamanho padrão
void InicializarJogador(struct barra* b) {
    b->position.x = TELA_LARGURA / 2 - 50; // centraliza horizontalmente
    b->position.y = TELA_ALTURA - 40;      // posiciona próximo da base
    b->tamanho.x = 100;                    // largura da barra
    b->tamanho.y = 20;                     // altura da barra
    b->vel = 8;                           // velocidade de movimento
    b->cor = GREEN;                       // cor inicial
}

// Atualiza a barra conforme input do jogador e efeito de piscar
void AtualizarJogador(struct barra* b, int* piscarFrames) {
    // Pisca a barra se necessário
    if (*piscarFrames > 0) {
        b->cor = WHITE;
        (*piscarFrames)--;
    } else {
        b->cor = GREEN;
    }

    // Move para a esquerda se possível
    if(IsKeyDown(KEY_LEFT) && b->position.x > 0) b->position.x -= b->vel;
    // Move para a direita se possível
    if(IsKeyDown(KEY_RIGHT) && b->position.x + b->tamanho.x < TELA_LARGURA) b->position.x += b->vel;
}

// Desenha a barra do jogador na tela
void DesenharJogador(struct barra* b) {
    DrawRectangleRec((Rectangle){ b->position.x, b->position.y, b->tamanho.x, b->tamanho.y }, b->cor);
}