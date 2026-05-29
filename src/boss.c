
// Funções para controlar o chefe (boss) do Arkanoid.
#include "boss.h"
#include <math.h>

// Inicializa o boss na posição inicial, define vida e zera projéteis.
void InicializarBoss(struct boss_Arkanoid* boss, struct projetil tiros[]) {
    boss->estado = BOSS_INATIVO;
    boss->tamanho_total = (Vector2){ BOSS_COLUNAS * BLCO_TAM, BOSS_LINHAS * BLCO_TAM }; 
    boss->posicao = (Vector2){ TELA_LARGURA / 2 - boss->tamanho_total.x / 2, -200 }; 
    boss->vida_maxima = 24; 
    boss->vida_atual = boss->vida_maxima;
    boss->tempo_ultimo_tiro = 0.0f; boss->tempo_laser = 0.0f;
    boss->carregando_laser = false; boss->disparando_laser = false; boss->laser_x = 0;
    for (int i = 0; i < MAX_PROJETEIS; i++) tiros[i].ativo = false;
}

// Atualiza o estado do boss, verifica colisão com bolas e controla vida.
// Se o boss for atingido, perde vida e pode ser derrotado.
void AtualizarBoss(struct boss_Arkanoid* boss, struct bola bolas[], struct projetil tiros[], struct barra* barra, int* score, bool* gameover, bool* gamewin, float* velX) {
    if (boss->estado == BOSS_INATIVO) return;

    // Só faz colisão se o boss está ativo (em combate)
    if (boss->estado == BOSS_ATIVO) {
        Rectangle bossRec = { boss->posicao.x, boss->posicao.y, boss->tamanho_total.x, boss->tamanho_total.y };
        for (int b = 0; b < MAX_BOLAS; b++) {
            if (!bolas[b].ativa) continue;
            // Se a bola bate no boss, calcula de onde veio e rebate
            if (CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, bossRec)) {
                float doTopo = (bolas[b].position.y) - boss->posicao.y;
                float daBase = (boss->posicao.y + boss->tamanho_total.y) - bolas[b].position.y;
                float daEsquerda = (bolas[b].position.x) - boss->posicao.x;
                float daDireita = (boss->posicao.x + boss->tamanho_total.x) - bolas[b].position.x;

                // Rebote inteligente: depende do lado que bateu
                if (doTopo < daBase && doTopo < daEsquerda && doTopo < daDireita) {
                    bolas[b].vel.y = -fabsf(bolas[b].vel.y); bolas[b].position.y = boss->posicao.y - bolas[b].raio - 1;
                } else if (daBase < doTopo && daBase < daEsquerda && daBase < daDireita) {
                    bolas[b].vel.y = fabsf(bolas[b].vel.y); bolas[b].position.y = boss->posicao.y + boss->tamanho_total.y + bolas[b].raio + 1;
                } else if (daEsquerda < daDireita && daEsquerda < doTopo && daEsquerda < daBase) {
                    bolas[b].vel.x = -fabsf(bolas[b].vel.x); bolas[b].position.x = boss->posicao.x - bolas[b].raio - 1;
                } else {
                    bolas[b].vel.x = fabsf(bolas[b].vel.x); bolas[b].position.x = boss->posicao.x + boss->tamanho_total.x + bolas[b].raio + 1;
                }
                // Diminui vida do boss e aumenta score
                boss->vida_atual--; *score += 40;
                // Se vida zerar, marca vitória
                if (boss->vida_atual <= 0) { boss->estado = BOSS_DERROTADO; *gamewin = true; }
            }
        }
    }

    // Máquina de Estados da IA
    if (boss->estado == BOSS_CUTSCENE) {
        if (boss->posicao.y < 80) boss->posicao.y += 2.0f; 
        else { boss->estado = BOSS_ATIVO; boss->tempo_ultimo_tiro = GetTime(); boss->tempo_laser = GetTime(); }
    }
    
    if (boss->estado == BOSS_ATIVO) {
        bool furia = (boss->vida_atual <= (boss->vida_maxima / 2));
        if (!boss->carregando_laser && !boss->disparando_laser) {
            boss->posicao.x += *velX;
            if (boss->posicao.x <= 30 || boss->posicao.x + boss->tamanho_total.x >= TELA_LARGURA - 30) *velX *= -1;
        }
        
        float tempoAtual = GetTime();
        float intervaloTiro = furia ? 1.1f : 1.7f; 
        if (tempoAtual - boss->tempo_ultimo_tiro >= intervaloTiro && !boss->disparando_laser) {
            int criados = 0;
            for (int i = 0; i < MAX_PROJETEIS && criados < 2; i++) {
                if (!tiros[i].ativo) {
                    float posX = (criados == 0) ? boss->posicao.x : boss->posicao.x + boss->tamanho_total.x;
                    tiros[i].posicao = (Vector2){ posX, boss->posicao.y + boss->tamanho_total.y - 20 };
                    tiros[i].velocidade = (Vector2){ (float)GetRandomValue(-1, 1), 4.0f };
                    tiros[i].ativo = true; criados++;
                }
            }
            boss->tempo_ultimo_tiro = tempoAtual;
        }
        
        if (furia) {
            if (!boss->carregando_laser && !boss->disparando_laser && (tempoAtual - boss->tempo_laser >= 4.0f)) {
                boss->carregando_laser = true; boss->tempo_laser = tempoAtual; boss->laser_x = boss->posicao.x + boss->tamanho_total.x / 2;
            }
            if (boss->carregando_laser && (tempoAtual - boss->tempo_laser >= 1.2f)) {
                boss->carregando_laser = false; boss->disparando_laser = true; boss->tempo_laser = tempoAtual;
            }
            if (boss->disparando_laser) {
                if (tempoAtual - boss->tempo_laser >= 1.5f) { boss->disparando_laser = false; boss->tempo_laser = tempoAtual; }
                if (CheckCollisionRecs((Rectangle){ boss->laser_x - 25, boss->posicao.y + boss->tamanho_total.y, 50, TELA_ALTURA }, (Rectangle){ barra->position.x, barra->position.y, barra->tamanho.x, barra->tamanho.y })) {
                    *gameover = true;
                }
            }
        }
    }

    // Atualiza Projéteis do Boss
    for (int i = 0; i < MAX_PROJETEIS; i++) {
        if (tiros[i].ativo) {
            tiros[i].posicao.x += tiros[i].velocidade.x; tiros[i].posicao.y += tiros[i].velocidade.y;
            if (tiros[i].posicao.y > TELA_ALTURA) tiros[i].ativo = false;
            if (CheckCollisionCircleRec(tiros[i].posicao, 6.0f, (Rectangle){ barra->position.x, barra->position.y, barra->tamanho.x, barra->tamanho.y })) *gameover = true;
        }
    }
}

void DesenharBoss(struct boss_Arkanoid* boss, struct projetil tiros[]) {
    if (boss->estado == BOSS_INATIVO) return;
    bool furia = (boss->vida_atual <= (boss->vida_maxima / 2));

    for (int i = 0; i < BOSS_LINHAS; i++) {
        for (int j = 0; j < BOSS_COLUNAS; j++) {
            int tipo_bloco = arte_boss[i][j];
            if (tipo_bloco == 0) continue; 
            Color corPixel = BLACK;
            if (tipo_bloco == 1) corPixel = furia ? MAROON : DARKBLUE;     
            if (tipo_bloco == 2) corPixel = furia ? ORANGE : PURPLE;       
            if (tipo_bloco == 3) corPixel = furia ? RED : GREEN;           
            DrawRectangle(boss->posicao.x + (j * BLCO_TAM), boss->posicao.y + (i * BLCO_TAM), BLCO_TAM - 1, BLCO_TAM - 1, corPixel);
        }
    }

    if (boss->estado == BOSS_ATIVO && furia) {
        if (boss->carregando_laser) {
            if ((int)(GetTime() * 10) % 2 == 0) DrawRectangle(boss->laser_x - 3, boss->posicao.y + boss->tamanho_total.y, 6, TELA_ALTURA, RED);
            DrawText("CUIDADO: CARREGANDO LASER!", TELA_LARGURA / 2 - 120, 20, 16, RED);
        }
        if (boss->disparando_laser) {
            DrawRectangle(boss->laser_x - 25, boss->posicao.y + boss->tamanho_total.y, 50, TELA_ALTURA, RED);
            DrawRectangle(boss->laser_x - 10, boss->posicao.y + boss->tamanho_total.y, 20, TELA_ALTURA, WHITE);
        }
    }

    if (boss->estado == BOSS_ATIVO) {
        float percentual = (float)boss->vida_atual / boss->vida_maxima;
        DrawRectangle(boss->posicao.x, boss->posicao.y - 20, boss->tamanho_total.x, 6, RED);
        DrawRectangle(boss->posicao.x, boss->posicao.y - 20, boss->tamanho_total.x * percentual, 6, furia ? ORANGE : GREEN);
        if (furia && !boss->carregando_laser && !boss->disparando_laser) DrawText("FASE 2: MODO FÚRIA ATIVADO!", TELA_LARGURA / 2 - 120, 20, 15, ORANGE);
    }

    for (int i = 0; i < MAX_PROJETEIS; i++) {
        if (tiros[i].ativo) DrawCircleV(tiros[i].posicao, 6.0f, furia ? ORANGE : VIOLET);
    }
}