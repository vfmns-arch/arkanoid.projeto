#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Incluído para usar o fabsf()

const int LARGURA_TELA = 800; 
const int TELA_ALTURA = 600;
const int TIRA_LINHAS = 5;
const int TIRA_COLS = 10;
const int TIRA_LARGURA = 60;
const int TIRA_ALTURA = 20;
const int TIRA_MEIO = 10;
const int TIRA_STARTY = 80; 
const int TIRA_STARTX = 50;

#define MAX_BOLAS 3

typedef enum { 
    BOSS_INATIVO = 0, 
    BOSS_CUTSCENE,    
    BOSS_ATIVO,       
    BOSS_DERROTADO    
} EstadoBoss;

#define BOSS_LINHAS 6
#define BOSS_COLUNAS 8
#define BLCO_TAM 30 

#define MAX_PROJETEIS 15
struct projetil {
    Vector2 posicao;
    Vector2 velocidade;
    bool ativo;
};

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

typedef enum { BUFF_NENHUM = 0, BUFF_DUPLICAR, BUFF_EXPLOSAO } TipoBuff;

struct nivel {
    int n;
    int s;
    struct nivel *next;
};

struct barra {
    Vector2 position;
    Vector2 tamanho;
    float vel;
    Color cor; 
};

struct tira {
    Vector2 position;
    bool ativo;
    Color color;
};

struct bola {
    Vector2 position;
    Vector2 vel;
    float raio;
    bool ativa;       
    bool explosive;   
};

struct buffItem {
    Vector2 position;
    Vector2 tamanho;
    float velY;
    bool ativo;
    TipoBuff tipo;
};

int arte_boss[BOSS_LINHAS][BOSS_COLUNAS] = {
    {0, 1, 2, 2, 2, 2, 1, 0},
    {1, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 3, 2, 2, 3, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 1},
    {0, 1, 2, 1, 1, 2, 1, 0},
    {0, 0, 1, 2, 2, 1, 0, 0}
};

int main() {
    int nivel_atual = 1;
    
    struct nivel *head = (struct nivel *)malloc(sizeof(struct nivel));
    head->n = 1;
    head->s = 0;
    head->next = NULL;
    
    InitWindow(LARGURA_TELA, TELA_ALTURA, "Arkanoid - Beat this up!");
    SetTargetFPS(60);
    
    struct barra barra;
    barra.position.x = LARGURA_TELA / 2 - 50;
    barra.position.y = TELA_ALTURA - 40;
    barra.tamanho.x = 100;
    barra.tamanho.y = 20;
    barra.vel = 8;
    barra.cor = GREEN;
    
    struct bola bolas[MAX_BOLAS];
    bolas[0].position.x = LARGURA_TELA / 2;
    bolas[0].position.y = TELA_ALTURA / 2;
    bolas[0].vel.x = 3;
    bolas[0].vel.y = -5;
    bolas[0].raio = 8;
    bolas[0].ativa = true;
    bolas[0].explosive = false;

    for (int i = 1; i < MAX_BOLAS; i++) bolas[i].ativa = false;
    
    struct boss_Arkanoid boss;
    boss.estado = BOSS_INATIVO;
    boss.tamanho_total = (Vector2){ BOSS_COLUNAS * BLCO_TAM, BOSS_LINHAS * BLCO_TAM }; 
    boss.posicao = (Vector2){ LARGURA_TELA / 2 - boss.tamanho_total.x / 2, -200 }; 
    boss.vida_maxima = 24; 
    boss.vida_atual = boss.vida_maxima;
    boss.tempo_ultimo_tiro = 0.0f;
    boss.tempo_laser = 0.0f;
    boss.carregando_laser = false;
    boss.disparando_laser = false;
    boss.laser_x = 0;

    struct projetil tiros[MAX_PROJETEIS];
    for (int i = 0; i < MAX_PROJETEIS; i++) tiros[i].ativo = false;

    struct buffItem poder;
    poder.ativo = false;
    poder.tamanho = (Vector2){ 15, 15 };
    poder.velY = 3.0f; 

    Color coresLinhas[5] = { RED, ORANGE, YELLOW, GREEN, BLUE };
    struct tira tiras[TIRA_LINHAS][TIRA_COLS];
    
    for(int i = 0; i < TIRA_LINHAS; i += 1) {
        for(int j = 0; j < TIRA_COLS; j += 1) {
            tiras[i][j].position.x = TIRA_STARTX + j * (TIRA_LARGURA + TIRA_MEIO);
            tiras[i][j].position.y = TIRA_STARTY + i * (TIRA_ALTURA + TIRA_MEIO);
            tiras[i][j].ativo = true;
            tiras[i][j].color = coresLinhas[i % 5]; 
        }
    }
    
    int score = 0;
    bool gameover = false;
    bool gamewin = false;
    int piscarFrames = 0; 
    float velBossX = 3.5f;
    
    while(!WindowShouldClose()) {
        if (piscarFrames > 0) {
            barra.cor = WHITE;
            piscarFrames--;
        } else {
            barra.cor = GREEN;
        }

        if(!gameover && !gamewin) {
            if(IsKeyDown(KEY_LEFT) && barra.position.x > 0) barra.position.x -= barra.vel;
            if(IsKeyDown(KEY_RIGHT) && barra.position.x + barra.tamanho.x < LARGURA_TELA) barra.position.x += barra.vel;
            
            for (int b = 0; b < MAX_BOLAS; b++) {
                if (!bolas[b].ativa) continue;

                bolas[b].position.x += bolas[b].vel.x;
                bolas[b].position.y += bolas[b].vel.y;
                
                if(bolas[b].position.x - bolas[b].raio <= 0 || bolas[b].position.x + bolas[b].raio >= LARGURA_TELA) bolas[b].vel.x *= -1;
                if(bolas[b].position.y - bolas[b].raio <= 0) bolas[b].vel.y *= -1;
                
                if(bolas[b].position.y + bolas[b].raio >= TELA_ALTURA) {
                    bolas[b].ativa = false;
                    continue;
                }
                
                struct Rectangle barraRe = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
                if(CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, barraRe)) {
                    bolas[b].vel.y *= -1;
                    float hit = ((bolas[b].position.x - (barra.position.x + barra.tamanho.x / 2)) / (barra.tamanho.x / 2));
                    bolas[b].vel.x = 6 * hit;
                    piscarFrames = 6; 
                }
                
                if (boss.estado == BOSS_INATIVO) {
                    for(int i = 0; i < TIRA_LINHAS; i++) {
                        for(int j = 0; j < TIRA_COLS; j++) {
                            if(tiras[i][j].ativo) {
                                struct Rectangle tiraRe = { tiras[i][j].position.x, tiras[i][j].position.y, (float)TIRA_LARGURA, (float)TIRA_ALTURA };
                                if(CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, tiraRe)) {
                                    tiras[i][j].ativo = false;
                                    bolas[b].vel.y *= -1;
                                    score += 10; 

                                    if (bolas[b].explosive) {
                                        bolas[b].explosive = false; 
                                        if (i > 0 && tiras[i-1][j].ativo) { tiras[i-1][j].ativo = false; score += 10; }
                                        if (i < TIRA_LINHAS-1 && tiras[i+1][j].ativo) { tiras[i+1][j].ativo = false; score += 10; }
                                        if (j > 0 && tiras[i][j-1].ativo) { tiras[i][j-1].ativo = false; score += 10; }
                                        if (j < TIRA_COLS-1 && tiras[i][j+1].ativo) { tiras[i][j+1].ativo = false; score += 10; }
                                    }

                                    if (!poder.ativo && GetRandomValue(1, 4) == 1) {
                                        poder.ativo = true;
                                        poder.position.x = tiras[i][j].position.x + TIRA_LARGURA / 2 - 7;
                                        poder.position.y = tiras[i][j].position.y;
                                        poder.tipo = (GetRandomValue(1, 2) == 1) ? BUFF_DUPLICAR : BUFF_EXPLOSAO;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                
                // --- COLISÃO DO BOSS CORRIGIDA COM FABSF() ---
                if (boss.estado == BOSS_ATIVO) {
                    struct Rectangle bossRec = { boss.posicao.x, boss.posicao.y, boss.tamanho_total.x, boss.tamanho_total.y };
                    if (CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, bossRec)) {
                        
                        float doTopo = (bolas[b].position.y) - boss.posicao.y;
                        float daBase = (boss.posicao.y + boss.tamanho_total.y) - bolas[b].position.y;
                        float daEsquerda = (bolas[b].position.x) - boss.posicao.x;
                        float daDireita = (boss.posicao.x + boss.tamanho_total.x) - bolas[b].position.x;

                        if (doTopo < daBase && doTopo < daEsquerda && doTopo < daDireita) {
                            // Bateu no TOPO
                            bolas[b].vel.y = -fabsf(bolas[b].vel.y); 
                            bolas[b].position.y = boss.posicao.y - bolas[b].raio - 1;
                        } 
                        else if (daBase < doTopo && daBase < daEsquerda && daBase < daDireita) {
                            // Bateu na BASE
                            bolas[b].vel.y = fabsf(bolas[b].vel.y); 
                            bolas[b].position.y = boss.posicao.y + boss.tamanho_total.y + bolas[b].raio + 1;
                        } 
                        else if (daEsquerda < daDireita && daEsquerda < doTopo && daEsquerda < daBase) {
                            // Bateu na ESQUERDA
                            bolas[b].vel.x = -fabsf(bolas[b].vel.x); 
                            bolas[b].position.x = boss.posicao.x - bolas[b].raio - 1;
                        } 
                        else {
                            // Bateu na DIREITA
                            bolas[b].vel.x = fabsf(bolas[b].vel.x); 
                            bolas[b].position.x = boss.posicao.x + boss.tamanho_total.x + bolas[b].raio + 1;
                        }

                        boss.vida_atual--;
                        score += 40;
                        
                        if (boss.vida_atual <= 0) {
                            boss.estado = BOSS_DERROTADO;
                            gamewin = true; 
                        }
                    }
                }
            }

            if (nivel_atual == 2 && boss.estado != BOSS_INATIVO) {
                if (boss.estado == BOSS_CUTSCENE) {
                    if (boss.posicao.y < 80) boss.posicao.y += 2.0f; 
                    else {
                        boss.estado = BOSS_ATIVO; 
                        boss.tempo_ultimo_tiro = GetTime();
                        boss.tempo_laser = GetTime();
                    }
                }
                
                if (boss.estado == BOSS_ATIVO) {
                    if (!boss.carregando_laser && !boss.disparando_laser) {
                        boss.posicao.x += velBossX;
                        if (boss.posicao.x <= 30 || boss.posicao.x + boss.tamanho_total.x >= LARGURA_TELA - 30) {
                            velBossX *= -1; 
                        }
                    }
                    
                    float tempoAtual = GetTime();
                    bool segundaFase = (boss.vida_atual <= (boss.vida_maxima / 2));
                    
                    float intervaloTiro = segundaFase ? 1.1f : 1.7f; 
                    if (tempoAtual - boss.tempo_ultimo_tiro >= intervaloTiro && !boss.disparando_laser) {
                        int criados = 0;
                        for (int i = 0; i < MAX_PROJETEIS && criados < 2; i++) {
                            if (!tiros[i].ativo) {
                                float posX = (criados == 0) ? boss.posicao.x : boss.posicao.x + boss.tamanho_total.x;
                                tiros[i].posicao = (Vector2){ posX, boss.posicao.y + boss.tamanho_total.y - 20 };
                                tiros[i].velocidade = (Vector2){ (float)GetRandomValue(-1, 1), 4.0f };
                                tiros[i].ativo = true;
                                criados++;
                            }
                        }
                        boss.tempo_ultimo_tiro = tempoAtual;
                    }
                    
                    if (segundaFase) {
                        if (!boss.carregando_laser && !boss.disparando_laser && (tempoAtual - boss.tempo_laser >= 4.0f)) {
                            boss.carregando_laser = true;
                            boss.tempo_laser = tempoAtual;
                            boss.laser_x = boss.posicao.x + boss.tamanho_total.x / 2; 
                        }
                        
                        if (boss.carregando_laser && (tempoAtual - boss.tempo_laser >= 1.2f)) {
                            boss.carregando_laser = false;
                            boss.disparando_laser = true;
                            boss.tempo_laser = tempoAtual;
                        }
                        
                        if (boss.disparando_laser) {
                            if (tempoAtual - boss.tempo_laser >= 1.5f) {
                                boss.disparando_laser = false;
                                boss.tempo_laser = tempoAtual; 
                            }
                            
                            struct Rectangle areaLaser = { boss.laser_x - 25, boss.posicao.y + boss.tamanho_total.y, 50, TELA_ALTURA };
                            struct Rectangle barraRec = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
                            if (CheckCollisionRecs(areaLaser, barraRec)) {
                                gameover = true;
                            }
                        }
                    }
                }
                
                for (int i = 0; i < MAX_PROJETEIS; i++) {
                    if (tiros[i].ativo) {
                        tiros[i].posicao.x += tiros[i].velocidade.x;
                        tiros[i].posicao.y += tiros[i].velocidade.y;
                        
                        if (tiros[i].posicao.y > TELA_ALTURA) tiros[i].ativo = false;
                        
                        struct Rectangle barraRec = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
                        if (CheckCollisionCircleRec(tiros[i].posicao, 6.0f, barraRec)) {
                            gameover = true; 
                        }
                    }
                }
            }

            bool algumaBola = false;
            for (int b = 0; b < MAX_BOLAS; b++) if (bolas[b].ativa) algumaBola = true;
            if (!algumaBola) gameover = true;

            if (poder.ativo) {
                poder.position.y += poder.velY; 
                if (poder.position.y > TELA_ALTURA) poder.ativo = false;

                struct Rectangle barraRe = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
                struct Rectangle buffRe = { poder.position.x, poder.position.y, poder.tamanho.x, poder.tamanho.y };
                
                if (CheckCollisionRecs(buffRe, barraRe)) {
                    poder.ativo = false;
                    if (poder.tipo == BUFF_DUPLICAR) {
                        int bolaReferencia = -1;
                        for (int b = 0; b < MAX_BOLAS; b++) {
                            if (bolas[b].ativa) {
                                bolaReferencia = b;
                                break;
                            }
                        }

                        if (bolaReferencia != -1) {
                            for (int b = 0; b < MAX_BOLAS; b++) {
                                if (!bolas[b].ativa) { 
                                    bolas[b].position = bolas[bolaReferencia].position;
                                    bolas[b].vel = (Vector2){ -bolas[bolaReferencia].vel.x, bolas[bolaReferencia].vel.y };
                                    bolas[b].raio = bolas[bolaReferencia].raio;
                                    bolas[b].explosive = bolas[bolaReferencia].explosive;
                                    bolas[b].ativa = true;
                                    break; 
                                }
                            }
                        }
                    } else if (poder.tipo == BUFF_EXPLOSAO) {
                        for (int b = 0; b < MAX_BOLAS; b++) if (bolas[b].ativa) bolas[b].explosive = true;
                    }
                }
            }

            if (boss.estado == BOSS_INATIVO) {
                bool blocosRestantes = false;
                for(int i = 0; i < TIRA_LINHAS; i++)
                    for(int j = 0; j < TIRA_COLS; j++)
                        if(tiras[i][j].ativo) blocosRestantes = true;
                
                if(!blocosRestantes && nivel_atual == 1) {
                    nivel_atual = 2;
                    boss.estado = BOSS_CUTSCENE;
                    boss.vida_atual = boss.vida_maxima;
                    poder.ativo = false;
                    bolas[0].position = (Vector2){ LARGURA_TELA / 2, TELA_ALTURA / 2 + 60 };
                    bolas[0].vel = (Vector2){ 3, -5 };
                    bolas[0].ativa = true;
                    for(int b = 1; b < MAX_BOLAS; b++) bolas[b].ativa = false;
                }
            }
        } 
        else {
            if(IsKeyPressed(KEY_SPACE)) {
                barra.position.x = LARGURA_TELA / 2 - 50;
                bolas[0].position = (Vector2){ LARGURA_TELA / 2, TELA_ALTURA / 2 };
                bolas[0].vel = (Vector2){ 3, -5 };
                bolas[0].ativa = true; bolas[0].explosive = false;
                for (int b = 1; b < MAX_BOLAS; b++) bolas[b].ativa = false;
                poder.ativo = false;
                for (int i = 0; i < MAX_PROJETEIS; i++) tiros[i].ativo = false;
                boss.carregando_laser = false; boss.disparando_laser = false;
                
                if(gameover) {
                    nivel_atual = 1;
                    boss.estado = BOSS_INATIVO;
                    boss.posicao.y = -200;
                    for(int i = 0; i < TIRA_LINHAS; i++)
                        for(int j = 0; j < TIRA_COLS; j++) tiras[i][j].ativo = true;
                } else {
                    nivel_atual += 1;
                    boss.estado = BOSS_INATIVO;
                }
                score = 0; gameover = false; gamewin = false;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawRectangleRec((Rectangle){barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y}, barra.cor);
        
        for (int b = 0; b < MAX_BOLAS; b++) {
            if (bolas[b].ativa) DrawCircleV(bolas[b].position, bolas[b].raio, bolas[b].explosive ? RED : WHITE);
        }
        
        if (boss.estado == BOSS_INATIVO) {
            for(int i = 0; i < TIRA_LINHAS; i++) {
                for(int j = 0; j < TIRA_COLS; j++) {
                    if(tiras[i][j].ativo) DrawRectangleRec((Rectangle){tiras[i][j].position.x, tiras[i][j].position.y, TIRA_LARGURA, TIRA_ALTURA}, tiras[i][j].color);
                }
            }
        }
        
        if (nivel_atual == 2 && boss.estado != BOSS_INATIVO) {
            bool fúria = (boss.vida_atual <= (boss.vida_maxima / 2));
            
            for (int i = 0; i < BOSS_LINHAS; i++) {
                for (int j = 0; j < BOSS_COLUNAS; j++) {
                    int tipo_bloco = arte_boss[i][j];
                    if (tipo_bloco == 0) continue; 
                    
                    Color corPixel = BLACK;
                    if (tipo_bloco == 1) corPixel = fúria ? MAROON : DARKBLUE;     
                    if (tipo_bloco == 2) corPixel = fúria ? ORANGE : PURPLE;       
                    if (tipo_bloco == 3) corPixel = fúria ? RED : GREEN;           
                    
                    DrawRectangle(boss.posicao.x + (j * BLCO_TAM), boss.posicao.y + (i * BLCO_TAM), BLCO_TAM - 1, BLCO_TAM - 1, corPixel);
                }
            }
            
            if (boss.estado == BOSS_ATIVO && fúria) {
                if (boss.carregando_laser) {
                    if ((int)(GetTime() * 10) % 2 == 0) {
                        DrawRectangle(boss.laser_x - 3, boss.posicao.y + boss.tamanho_total.y, 6, TELA_ALTURA, RED);
                    }
                    DrawText("CUIDADO: CARREGANDO LASER!", LARGURA_TELA / 2 - 120, 20, 16, RED);
                }
                
                if (boss.disparando_laser) {
                    DrawRectangle(boss.laser_x - 25, boss.posicao.y + boss.tamanho_total.y, 50, TELA_ALTURA, RED);
                    DrawRectangle(boss.laser_x - 10, boss.posicao.y + boss.tamanho_total.y, 20, TELA_ALTURA, WHITE);
                }
            }
            
            if (boss.estado == BOSS_ATIVO) {
                float percentual = (float)boss.vida_atual / boss.vida_maxima;
                DrawRectangle(boss.posicao.x, boss.posicao.y - 20, boss.tamanho_total.x, 6, RED);
                DrawRectangle(boss.posicao.x, boss.posicao.y - 20, boss.tamanho_total.x * percentual, 6, fúria ? ORANGE : GREEN);
                
                if (fúria && !boss.carregando_laser && !boss.disparando_laser) {
                    DrawText("FASES DO BOSS: MODO FÚRIA COORDENADO!", LARGURA_TELA / 2 - 170, 20, 15, ORANGE);
                }
            }
            
            for (int i = 0; i < MAX_PROJETEIS; i++) {
                if (tiros[i].ativo) DrawCircleV(tiros[i].posicao, 6.0f, fúria ? ORANGE : VIOLET);
            }
        }
        
        if (poder.ativo) {
            DrawRectangleRec((Rectangle){poder.position.x, poder.position.y, poder.tamanho.x, poder.tamanho.y}, (poder.tipo == BUFF_DUPLICAR) ? BLUE : RED);
        }

        DrawText(TextFormat("SCORE: %04i", score), 20, 40, 20, RAYWHITE);
        DrawText(TextFormat("NÍVEL: %d", nivel_atual), LARGURA_TELA - 120, 40, 20, RAYWHITE);
        
        if(gameover) {
            DrawText("GAME OVER", LARGURA_TELA / 2 - 110, TELA_ALTURA / 2 - 20, 40, RED);
            DrawText("PRESSIONE ESPAÇO PARA RECOMEÇAR", LARGURA_TELA / 2 - 180, TELA_ALTURA / 2 + 30, 20, RAYWHITE);
        }
        if(gamewin) {
            DrawText("VITÓRIA COMPLETA!", LARGURA_TELA / 2 - 190, TELA_ALTURA / 2 - 20, 40, GOLD);
            DrawText("PRESSIONE ESPAÇO PARA CONTINUAR", LARGURA_TELA / 2 - 170, TELA_ALTURA / 2 + 30, 16, RAYWHITE);
        }
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}