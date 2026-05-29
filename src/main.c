#include "comum.h"
#include "placar.h"
#include "jogador.h"
#include "bola.h"
#include "boss.h"

// Reinicializa todos os elementos do jogo para o início
void ResetarJogoCompleto(struct barra* barra, struct bola bolas[], struct buffItem* poder, struct projetil tiros[], struct boss_Arkanoid* boss, struct tira tiras[TIRA_LINHAS][TIRA_COLS], int* nivel_atual, int* score, bool* gameover, bool* gamewin, struct nivel* head) {
    InicializarJogador(barra);
    InicializarBolas(bolas);
    InicializarBoss(boss, tiros);
    poder->ativo = false;
    *score = 0; *gameover = false; *gamewin = false;
    *nivel_atual = 1;

    // Inicializa blocos coloridos
    Color coresLinhas[5] = { RED, ORANGE, YELLOW, GREEN, BLUE };
    for(int i = 0; i < TIRA_LINHAS; i++) {
        for(int j = 0; j < TIRA_COLS; j++) {
            tiras[i][j].position.x = TIRA_STARTX + j * (TIRA_LARGURA + TIRA_MEIO);
            tiras[i][j].position.y = TIRA_STARTY + i * (TIRA_ALTURA + TIRA_MEIO);
            tiras[i][j].ativo = true;
            tiras[i][j].color = coresLinhas[i % 5];
        }
    }
    ResetarListaEncadeada(head);
}

int main() {
    // Variáveis principais do jogo
    int nivel_atual = 1;
    struct nivel *head = InicializarPlacar();
    struct nivel *a = head;

    InitWindow(TELA_LARGURA, TELA_ALTURA, "Arkanoid - Beat it up!");
    SetTargetFPS(60);
    
    struct barra barra;
    InicializarJogador(&barra);

    struct bola bolas[MAX_BOLAS];
    InicializarBolas(bolas);
    
    struct boss_Arkanoid boss;
    struct projetil tiros[MAX_PROJETEIS];
    InicializarBoss(&boss, tiros);

    struct buffItem poder;
    poder.ativo = false; poder.tamanho = (Vector2){ 15, 15 }; poder.velY = 3.0f;

    struct tira tiras[TIRA_LINHAS][TIRA_COLS];
    int score = 0; bool gameover = false; bool gamewin = false;
    int piscarFrames = 0; float velBossX = 3.5f;

    // Inicialização forçada das tiras no início
    ResetarJogoCompleto(&barra, bolas, &poder, tiros, &boss, tiras, &nivel_atual, &score, &gameover, &gamewin, head);

    // Loop principal do jogo
    while(!WindowShouldClose()) {
        if(!gameover && !gamewin) {
            AtualizarJogador(&barra, &piscarFrames); // move barra
            AtualizarBolas(bolas, &barra, tiras, &score, &piscarFrames, &poder, boss.estado); // física das bolas
            
            if (nivel_atual == 2) {
                AtualizarBoss(&boss, bolas, tiros, &barra, &score, &gameover, &gamewin, &velBossX); // IA do boss
            }

            // Verifica Game Over por falta de bolas
            bool algumaBola = false;
            for (int b = 0; b < MAX_BOLAS; b++) if (bolas[b].ativa) algumaBola = true;
            if (!algumaBola) gameover = true;

            // Transição para Fase do Boss
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
                    bolas[0].position = (Vector2){ TELA_LARGURA / 2, TELA_ALTURA / 2 + 60 };
                    bolas[0].vel = (Vector2){ 3, -5 }; bolas[0].ativa = true;
                    for(int b = 1; b < MAX_BOLAS; b++) bolas[b].ativa = false;
                }
            }
        } 
        else {
            if(IsKeyPressed(KEY_SPACE)) {
                if(gameover || gamewin) {
                    ResetarJogoCompleto(&barra, bolas, &poder, tiros, &boss, tiras, &nivel_atual, &score, &gameover, &gamewin, head);
                    a = head;
                } else if (nivel_atual == 1) {
                    SalvarProgressoNivel(&a, 2, score);
                    nivel_atual = 2;
                    boss.estado = BOSS_CUTSCENE;
                }
            }
        }

        // ---- RENDERIZAÇÃO ----
        BeginDrawing();
        ClearBackground(BLACK);
        
        DesenharJogador(&barra);
        DesenharBolas(bolas);
        
        if (boss.estado == BOSS_INATIVO) {
            for(int i = 0; i < TIRA_LINHAS; i++) {
                for(int j = 0; j < TIRA_COLS; j++) {
                    if(tiras[i][j].ativo) DrawRectangleRec((Rectangle){ tiras[i][j].position.x, tiras[i][j].position.y, TIRA_LARGURA, TIRA_ALTURA }, tiras[i][j].color);
                }
            }
        }
        
        if (nivel_atual == 2) DesenharBoss(&boss, tiros);
        AtualizarE_DesenharBuff(&poder, &barra, bolas);

        DrawText(TextFormat("SCORE: %04i", score), 10, 10, 20, GREEN);
        DrawText(TextFormat("NÍVEL: %d", nivel_atual), TELA_LARGURA - 120, 10, 20, RAYWHITE);
        
        if(gameover) {
            DrawText("GAME OVER", TELA_LARGURA / 2 - 100, TELA_ALTURA / 2 - 20, 40, RED);
            DrawText("PRESS SPACE TO RESTART", TELA_LARGURA / 2 - 140, TELA_ALTURA / 2 + 30, 20, RAYWHITE);
        }
        if(gamewin) {
            DrawText("VITÓRIA COMPLETA!!!", TELA_LARGURA / 2 - 190, TELA_ALTURA / 2 - 20, 40, GOLD);
            DrawText("PRESS SPACE TO RESTART", TELA_LARGURA / 2 - 140, TELA_ALTURA / 2 + 30, 20, RAYWHITE);
        }
        EndDrawing();
    }
    
    CloseWindow();
    GravarArquivoLog(head);
    return 0;
}