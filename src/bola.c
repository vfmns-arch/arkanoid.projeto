
// Funções principais para manipular as bolas do Arkanoid
#include "bola.h"
#include <stdlib.h>

// Inicializa as bolas na posição e estado padrão do início do jogo
void InicializarBolas(struct bola bolas[]) {
    bolas[0].position = (Vector2){ TELA_LARGURA / 2, TELA_ALTURA / 2 }; // centro da tela
    bolas[0].vel = (Vector2){ 3, -5 }; // velocidade inicial
    bolas[0].raio = 8;
    bolas[0].ativa = true;
    bolas[0].explosive = false;
    for (int i = 1; i < MAX_BOLAS; i++) bolas[i].ativa = false; // só a primeira ativa
}

// Atualiza física, colisão e efeitos das bolas
void AtualizarBolas(struct bola bolas[], struct barra* barra, struct tira tiras[TIRA_LINHAS][TIRA_COLS], int* score, int* piscarFrames, struct buffItem* poder, EstadoBoss estadoBoss) {
    for (int b = 0; b < MAX_BOLAS; b++) {
        if (!bolas[b].ativa) continue;

        // Movimento da bola
        bolas[b].position.x += bolas[b].vel.x;
        bolas[b].position.y += bolas[b].vel.y;

        // Rebote nas paredes laterais
        if(bolas[b].position.x - bolas[b].raio <= 0 || bolas[b].position.x + bolas[b].raio >= TELA_LARGURA)
            bolas[b].vel.x *= -1;
        // Rebote no teto
        if(bolas[b].position.y - bolas[b].raio <= 0)
            bolas[b].vel.y *= -1;

        // Se cair no fundo, desativa a bola
        if(bolas[b].position.y + bolas[b].raio >= TELA_ALTURA) {
            bolas[b].ativa = false;
            continue;
        }

        // Colisão com a barra do jogador
        Rectangle barraRe = { barra->position.x, barra->position.y, barra->tamanho.x, barra->tamanho.y };
        if(CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, barraRe)) {
            bolas[b].vel.y *= -1;
            // Calcula o quanto a bola desviou do centro da barra para variar o ângulo
            float hit = ((bolas[b].position.x - (barra->position.x + barra->tamanho.x / 2)) / (barra->tamanho.x / 2));
            bolas[b].vel.x = 6 * hit;
            *piscarFrames = 6; // Ativa efeito visual na barra
        }

        // Só verifica colisão com blocos se o boss não estiver ativo
        if (estadoBoss == BOSS_INATIVO) {
            bool colidiu = false;
            for(int i = 0; i < TIRA_LINHAS && !colidiu; i++) {
                for(int j = 0; j < TIRA_COLS && !colidiu; j++) {
                    if(tiras[i][j].ativo) {
                        Rectangle tiraRe = { tiras[i][j].position.x, tiras[i][j].position.y, (float)TIRA_LARGURA, (float)TIRA_ALTURA };
                        if(CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, tiraRe)) {
                            tiras[i][j].ativo = false;
                            bolas[b].vel.y *= -1;
                            *score += 10; // pontua ao destruir bloco

                            // Efeito explosivo: destrói blocos ao redor
                            if (bolas[b].explosive) {
                                bolas[b].explosive = false; 
                                if (i > 0 && tiras[i-1][j].ativo) { tiras[i-1][j].ativo = false; *score += 10; }
                                if (i < TIRA_LINHAS-1 && tiras[i+1][j].ativo) { tiras[i+1][j].ativo = false; *score += 10; }
                                if (j > 0 && tiras[i][j-1].ativo) { tiras[i][j-1].ativo = false; *score += 10; }
                                if (j < TIRA_COLS-1 && tiras[i][j+1].ativo) { tiras[i][j+1].ativo = false; *score += 10; }
                            }

                            // Sorteia se dropa um power-up
                            if (!poder->ativo && GetRandomValue(1, 4) == 1) {
                                poder->ativo = true;
                                poder->position.x = tiras[i][j].position.x + TIRA_LARGURA / 2 - 7;
                                poder->position.y = tiras[i][j].position.y;
                                poder->tipo = (GetRandomValue(1, 2) == 1) ? BUFF_DUPLICAR : BUFF_EXPLOSAO;
                            }
                            colidiu = true;
                        }
                    }
                }
            }
        }
    }
}

void DesenharBolas(struct bola bolas[]) {
    for (int b = 0; b < MAX_BOLAS; b++) {
        if (bolas[b].ativa) DrawCircleV(bolas[b].position, bolas[b].raio, bolas[b].explosive ? RED : WHITE);
    }
}

void AtivarDuplicacao(struct bola bolas[]) {
    int ref = -1;
    for (int b = 0; b < MAX_BOLAS; b++) { if (bolas[b].ativa) { ref = b; break; } }
    if (ref != -1) {
        for (int b = 0; b < MAX_BOLAS; b++) {
            if (!bolas[b].ativa) { 
                bolas[b].position = bolas[ref].position;
                bolas[b].vel = (Vector2){ -bolas[ref].vel.x, bolas[ref].vel.y };
                bolas[b].raio = bolas[ref].raio;
                bolas[b].explosive = bolas[ref].explosive;
                bolas[b].ativa = true;
                break; 
            }
        }
    }
}

void AtualizarE_DesenharBuff(struct buffItem* poder, struct barra* barra, struct bola bolas[]) {
    if (!poder->ativo) return;

    poder->position.y += poder->velY; 
    if (poder->position.y > TELA_ALTURA) poder->ativo = false;

    DrawRectangleRec((Rectangle){poder->position.x, poder->position.y, poder->tamanho.x, poder->tamanho.y}, (poder->tipo == BUFF_DUPLICAR) ? BLUE : RED);

    if (CheckCollisionRecs((Rectangle){ poder->position.x, poder->position.y, poder->tamanho.x, poder->tamanho.y }, (Rectangle){ barra->position.x, barra->position.y, barra->tamanho.x, barra->tamanho.y })) {
        poder->ativo = false;
        if (poder->tipo == BUFF_DUPLICAR) {
            AtivarDuplicacao(bolas);
        } else if (poder->tipo == BUFF_EXPLOSAO) {
            for (int b = 0; b < MAX_BOLAS; b++) if (bolas[b].ativa) bolas[b].explosive = true;
        }
    }
}