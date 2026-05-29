#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Incluído para usar o fabsf() na física de rebote inteligente do Boss

//constantes com os tamanhos da tela e das tiras
//quando for testar o jogo e os sprites estiverem muito grandes ou muito pequenos, ajuste essas constantes
const int TELA_LARGURA = 800; 
const int TELA_ALTURA = 600;
const int TIRA_LINHAS = 5;
const int TIRA_COLS = 10;
const int TIRA_LARGURA = 60;
const int TIRA_ALTURA = 20;
const int TIRA_MEIO = 10;
const int TIRA_STARTY = 80; 
const int TIRA_STARTX = 50;

#define MAX_BOLAS 3

// Estados do Boss na Fase 2
typedef enum { 
    BOSS_INATIVO = 0, 
    BOSS_CUTSCENE,    // Animação de entrada
    BOSS_ATIVO,       // Estado de combate ativo
    BOSS_DERROTADO    
} EstadoBoss;

// Dimensões do Grid de Pixel Art do Boss
#define BOSS_LINHAS 6
#define BOSS_COLUNAS 8
#define BLCO_TAM 30 // Tamanho de cada pixel/bloco do boss

#define MAX_PROJETEIS 15
struct projetil {
    Vector2 posicao;
    Vector2 velocidade;
    bool ativo;
};

// Estrutura para gerenciar o Boss como entidade única
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

//struct nivel armazena o nivel, score e o endereço de memoria do proximo nivel
struct nivel {
    int n;
    int s;
    struct nivel *next;
};

//struct barra armazena o x e y da posição (da esquera pra direita) e o tamanho, alem da velocidade
struct barra {
    //Vector2 é uma struct contendo float Vector2.x e float Vector2.y
    Vector2 position;
    Vector2 tamanho;
    float vel;
    Color cor; // Nova propriedade para cor dinâmica (efeito visual de impacto)
};

struct tira {
    Vector2 position;
    bool ativo;
    Color color;
};

//bola.vel armazena a velocidade x e y da bola
struct bola {
    Vector2 position;
    Vector2 vel;
    float raio;
    bool ativa;       // Se a bola está em jogo
    bool explosive;   // Se o power-up de explosão está ativo nesta bola
};

struct buffItem {
    Vector2 position;
    Vector2 tamanho;
    float velY;
    bool ativo;
    TipoBuff tipo;
};

// Matriz de Design do Boss (0 = Vazio, 1 = Contorno/Detalhe, 2 = Base, 3 = Olhos)
int arte_boss[BOSS_LINHAS][BOSS_COLUNAS] = {
    {0, 1, 2, 2, 2, 2, 1, 0},
    {1, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 3, 2, 2, 3, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 1},
    {0, 1, 2, 1, 1, 2, 1, 0},
    {0, 0, 1, 2, 2, 1, 0, 0}
};

int main() {
    //inicializa o log inicial de nivel
    int nivel_atual = 1;
    
    //aloca espaço pro head e preenche seus atributos
    struct nivel *head = (struct nivel *)malloc(sizeof(struct nivel));
    head->n = 1;
    head->s = 0; 
    head->next = NULL;
    
    //declara as structs a (atual) e aux (auxiliar), elas começam apontando pro head
    struct nivel *a = head;
    struct nivel *aux = head;
    
    //inicializa a tela, com a largura, altura e o nome
    InitWindow(TELA_LARGURA, TELA_ALTURA, "Arkanoid - Beat it up!");
    //limita o FPS a 60
    SetTargetFPS(60);
    
    //inicia a barra que o jogador controla
    struct barra barra;
    //a esquerda da barra começa 50 pixels a esquerda do centro
    barra.position.x = TELA_LARGURA / 2 - 50;
    //o fundo da barra começa 40 pixels acima do fundo da tela
    barra.position.y = TELA_ALTURA - 40;
    barra.tamanho.x = 100;
    barra.tamanho.y = 20;
    barra.vel = 8;
    barra.cor = GREEN;
    
    //inicializa o array de bolas sustentando multiplicidade
    struct bola bolas[MAX_BOLAS];
    //a posição inicial da bola fica no centro da tela
    bolas[0].position.x = TELA_LARGURA / 2;
    bolas[0].position.y = TELA_ALTURA / 2;
    //assim que iniciar, vai cair em linha reta numa velocidade de -5 no eixo Y
    bolas[0].vel.x = 3;
    bolas[0].vel.y = -5;
    bolas[0].raio = 8;
    bolas[0].ativa = true;
    bolas[0].explosive = false;

    //Inicializa as bolas reservas adicionais como inativas
    for (int i = 1; i < MAX_BOLAS; i++) bolas[i].ativa = false;
    
    // Inicialização estrutural do Boss composto
    struct boss_Arkanoid boss;
    boss.estado = BOSS_INATIVO;
    boss.tamanho_total = (Vector2){ BOSS_COLUNAS * BLCO_TAM, BOSS_LINHAS * BLCO_TAM }; 
    boss.posicao = (Vector2){ TELA_LARGURA / 2 - boss.tamanho_total.x / 2, -200 }; 
    boss.vida_maxima = 24; 
    boss.vida_atual = boss.vida_maxima;
    boss.tempo_ultimo_tiro = 0.0f;
    boss.tempo_laser = 0.0f;
    boss.carregando_laser = false;
    boss.disparando_laser = false;
    boss.laser_x = 0;

    // Inicializa pooling de projéteis ativos/inativos disparados pelo inimigo
    struct projetil tiros[MAX_PROJETEIS];
    for (int i = 0; i < MAX_PROJETEIS; i++) tiros[i].ativo = false;

    struct buffItem poder;
    poder.ativo = false;
    poder.tamanho = (Vector2){ 15, 15 };
    poder.velY = 3.0f; 

    Color coresLinhas[5] = { RED, ORANGE, YELLOW, GREEN, BLUE };
    struct tira tiras[TIRA_LINHAS][TIRA_COLS];
    
    //loop inicializando as linhas das tiras
    for(int i = 0; i < TIRA_LINHAS; i += 1) {
        //loop inicializando as colunas de tiras
        for(int j = 0; j < TIRA_COLS; j += 1) {
            //a posição de cada tira vai ser aonde as tiras devem começar na tela + as tiras anteriores + o espaço entre elas
            tiras[i][j].position.x = TIRA_STARTX + j * (TIRA_LARGURA + TIRA_MEIO);
            tiras[i][j].position.y = TIRA_STARTY + i * (TIRA_ALTURA + TIRA_MEIO);
            //caso a bola não tenha destruido a tira, ela fica ativa
            tiras[i][j].ativo = true;
            tiras[i][j].color = coresLinhas[i % 5]; 
        }
    }
    
    //inicializa o jogo
    int score = 0;
    bool gameover = false;
    bool gamewin = false;
    int piscarFrames = 0; 
    float velBossX = 3.5f;
    
    //enquanto a janela estiver aberta
    while(!WindowShouldClose()) {
        if (piscarFrames > 0) {
            barra.cor = WHITE;
            piscarFrames--;
        } else {
            barra.cor = GREEN;
        }

        //atualiza a posição dos objetos a cada frame
        if(!gameover && !gamewin) {
            //muda a posição da barra de acordo com a tecla que apertar
            if(IsKeyDown(KEY_LEFT) && barra.position.x > 0) {
                barra.position.x -= barra.vel;
            }
            if(IsKeyDown(KEY_RIGHT) && barra.position.x + barra.tamanho.x < TELA_LARGURA) {
                barra.position.x += barra.vel;
            }
            
            // Loop gerenciando a movimentação e física independente de cada bola ativa
            for (int b = 0; b < MAX_BOLAS; b++) {
                if (!bolas[b].ativa) continue;

                //atualiza a posição da bola de acordo com sua velocidade atual nos eixos x e y
                bolas[b].position.x += bolas[b].vel.x;
                bolas[b].position.y += bolas[b].vel.y;
                
                //caso a bola ultrapasse as paredes da tela, inverte sua velocidade no eixo x
                if(bolas[b].position.x - bolas[b].raio <= 0 || bolas[b].position.x + bolas[b].raio >= TELA_LARGURA) {
                    bolas[b].vel.x *= -1;
                }
                //caso a bola ultrapasse o topo da tela, inverte sua velocidade mo eixo y
                if(bolas[b].position.y - bolas[b].raio <= 0) {
                    bolas[b].vel.y *= -1;
                }
                
                //caso a bola ultrapasse o fundo da tela, desativa essa instância específica
                if(bolas[b].position.y + bolas[b].raio >= TELA_ALTURA) {
                    bolas[b].ativa = false;
                    continue;
                }
                
                //cria um struct Rectangle com os atributos da barra que o player controla pra usar a função CheckCollisionCircleRec
                struct Rectangle barraRe = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
                //caso a bola colida com a barra, inverte a velocidade da bola no eixo y
                if(CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, barraRe)) {
                    bolas[b].vel.y *= -1;
                    //multiplica a velocidade do eixo x da bola dependendo do quão longe a bola bateu do centro da barra
                    float hit = ((bolas[b].position.x - (barra.position.x + barra.tamanho.x / 2)) / (barra.tamanho.x / 2));
                    bolas[b].vel.x = 6 * hit;
                    piscarFrames = 6; 
                }
                
                // Colisão Fase 1 (Blocos tradicionais estruturados)
                if (boss.estado == BOSS_INATIVO) {
                    bool colidiu = false;
                    //checa todas as tiras
                    for(int i = 0; i < TIRA_LINHAS && !colidiu; i++) {
                        for(int j = 0; j < TIRA_COLS && !colidiu; j++) {
                            if(tiras[i][j].ativo) {
                                struct Rectangle tiraRe = { tiras[i][j].position.x, tiras[i][j].position.y, (float)TIRA_LARGURA, (float)TIRA_ALTURA };
                                if(CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, tiraRe)) {
                                    tiras[i][j].ativo = false;
                                    bolas[b].vel.y *= -1;
                                    score += 10; 

                                    // Lógica de Destruição do Item de Explosão em Cruz
                                    if (bolas[b].explosive) {
                                        bolas[b].explosive = false; 
                                        if (i > 0 && tiras[i-1][j].ativo) { tiras[i-1][j].ativo = false; score += 10; }
                                        if (i < TIRA_LINHAS-1 && tiras[i+1][j].ativo) { tiras[i+1][j].ativo = false; score += 10; }
                                        if (j > 0 && tiras[i][j-1].ativo) { tiras[i][j-1].ativo = false; score += 10; }
                                        if (j < TIRA_COLS-1 && tiras[i][j+1].ativo) { tiras[i][j+1].ativo = false; score += 10; }
                                    }

                                    // Chance de drop de item (25% de probabilidade aleatória)
                                    if (!poder.ativo && GetRandomValue(1, 4) == 1) {
                                        poder.ativo = true;
                                        poder.position.x = tiras[i][j].position.x + TIRA_LARGURA / 2 - 7;
                                        poder.position.y = tiras[i][j].position.y;
                                        poder.tipo = (GetRandomValue(1, 2) == 1) ? BUFF_DUPLICAR : BUFF_EXPLOSAO;
                                    }
                                    //colidiu = true evita glitch de dupla colisão em um único frame
                                    colidiu = true;
                                }
                            }
                        }
                    }
                }
                
                // Colisão Inteligente com o Bloco Unificado do Boss (Fase 2)
                if (boss.estado == BOSS_ATIVO) {
                    struct Rectangle bossRec = { boss.posicao.x, boss.posicao.y, boss.tamanho_total.x, boss.tamanho_total.y };
                    if (CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, bossRec)) {
                        
                        float doTopo = (bolas[b].position.y) - boss.posicao.y;
                        float daBase = (boss.posicao.y + boss.tamanho_total.y) - bolas[b].position.y;
                        float daEsquerda = (bolas[b].position.x) - boss.posicao.x;
                        float daDireita = (boss.posicao.x + boss.tamanho_total.x) - bolas[b].position.x;

                        if (doTopo < daBase && doTopo < daEsquerda && doTopo < daDireita) {
                            bolas[b].vel.y = -fabsf(bolas[b].vel.y); 
                            bolas[b].position.y = boss.posicao.y - bolas[b].raio - 1;
                        } 
                        else if (daBase < doTopo && daBase < daEsquerda && daBase < daDireita) {
                            bolas[b].vel.y = fabsf(bolas[b].vel.y); 
                            bolas[b].position.y = boss.posicao.y + boss.tamanho_total.y + bolas[b].raio + 1;
                        } 
                        else if (daEsquerda < daDireita && daEsquerda < doTopo && daEsquerda < daBase) {
                            bolas[b].vel.x = -fabsf(bolas[b].vel.x); 
                            bolas[b].position.x = boss.posicao.x - bolas[b].raio - 1;
                        } 
                        else {
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

            // --- INTELIGÊNCIA ARTIFICIAL E MÁQUINA DE ESTADOS DO BOSS ---
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
                        if (boss.posicao.x <= 30 || boss.posicao.x + boss.tamanho_total.x >= TELA_LARGURA - 30) {
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

            //caso a bola ultrapasse o fundo da tela em sua totalidade (nenhuma bola ativa), game over
            bool algumaBola = false;
            for (int b = 0; b < MAX_BOLAS; b++) if (bolas[b].ativa) algumaBola = true;
            if (!algumaBola) gameover = true;

            // Gerenciamento e aplicação de efeitos dos coletáveis
            if (poder.ativo) {
                poder.position.y += poder.velY; 
                if (poder.position.y > TELA_ALTURA) poder.ativo = false;

                struct Rectangle barraRe = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
                struct Rectangle buffRe = { poder.position.x, poder.position.y, poder.tamanho.x, poder.tamanho.y };
                
                // Verifica colisão do item de poder com a barra do jogador
                if (CheckCollisionRecs(buffRe, barraRe)) {
                    poder.ativo = false; // Desativa o item coletado

                    // Lógica do Power-up: Duplicar Bola
                    if (poder.tipo == BUFF_DUPLICAR) {
                        int bolaReferencia = -1;
                        
                        // Encontra a primeira bola que já está em jogo
                        for (int b = 0; b < MAX_BOLAS; b++) {
                            if (bolas[b].ativa) {
                                bolaReferencia = b;
                                break;
                            }
                        }

                        // Se encontrou uma bola ativa, cria uma nova a partir dela
                        if (bolaReferencia != -1) {
                            for (int b = 0; b < MAX_BOLAS; b++) {
                                if (!bolas[b].ativa) { 
                                    bolas[b].position = bolas[bolaReferencia].position;
                                    // Inverte a velocidade X para que as duas sigam direções opostas
                                    bolas[b].vel = (Vector2){ -bolas[bolaReferencia].vel.x, bolas[bolaReferencia].vel.y };
                                    bolas[b].raio = bolas[bolaReferencia].raio;
                                    bolas[b].explosive = bolas[bolaReferencia].explosive;
                                    bolas[b].ativa = true; // Ativa a bola reserva
                                    break; 
                                }
                            }
                        }
                    // Lógica do Power-up: Bola Explosiva
                    } else if (poder.tipo == BUFF_EXPLOSAO) {
                        // Aplica o efeito de explosão em todas as bolas atualmente ativas
                        for (int b = 0; b < MAX_BOLAS; b++) {
                            if (bolas[b].ativa) bolas[b].explosive = true;
                        }
                    }
                }
            }

            // Avaliação de transição de cenários se não houver tiras restantes
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
                    bolas[0].vel = (Vector2){ 3, -5 };
                    bolas[0].ativa = true;
                    for(int b = 1; b < MAX_BOLAS; b++) bolas[b].ativa = false;
                }
            }
        } 
        //se o jogador estiver na tela de menu
        else {
            //quando o jogador pressionar espaço, o jogo reinicia
            if(IsKeyPressed(KEY_SPACE)) {
                barra.position.x = TELA_LARGURA / 2 - 50;
                barra.position.y = TELA_ALTURA - 40;
                bolas[0].position = (Vector2){ TELA_LARGURA / 2, TELA_ALTURA / 2 };
                bolas[0].vel.x = 3;
                bolas[0].vel.y = -5;
                bolas[0].ativa = true; bolas[0].explosive = false;
                for (int b = 1; b < MAX_BOLAS; b++) bolas[b].ativa = false;
                poder.ativo = false;
                for (int i = 0; i < MAX_PROJETEIS; i++) tiros[i].ativo = false;
                boss.carregando_laser = false; boss.disparando_laser = false;

                if(gameover || gamewin) { // Adicionamos 'gamewin' aqui para o reset funcionar
                  nivel_atual = 1;
                  boss.estado = BOSS_INATIVO;
                  boss.posicao.y = -200;
                  boss.vida_atual = boss.vida_maxima; // Reseta a vida do boss

                  // Reseta as tiras da Fase 1
                  for(int i = 0; i < TIRA_LINHAS; i++) {
                      for(int j = 0; j < TIRA_COLS; j++) tiras[i][j].ativo = true;
                  }

                  // Limpa a lista encadeada de níveis
                  head->s = 0;
                  a = head->next;
                  while(a != NULL) {
                      aux = a;
                      a = a->next;
                      free(aux);
                  }
                  a = head;
                  head->next = NULL;
              } else {
                  // Lógica de avanço para o nível 2 (se ainda não estiver no Boss)
                  if (nivel_atual == 1) {
                      aux = (struct nivel *)malloc(sizeof(struct nivel));
                      a->next = aux;
                      aux->next = NULL;
                      a->s = score;
                      nivel_atual = 2; // Garante que vai para o 2
                      a->n = nivel_atual;
                      a = a->next;
                  }
              }
                
                //o score é logado nas structs
                if(gameover == true) {
                    nivel_atual = 1;
                    boss.estado = BOSS_INATIVO;
                    boss.posicao.y = -200;
                    for(int i = 0; i < TIRA_LINHAS; i++) {
                        for(int j = 0; j < TIRA_COLS; j++) tiras[i][j].ativo = true;
                    }
                    head->s = 0;
                    a = head->next;
                    while(a != NULL) {
                        aux = a;
                        a = a->next;
                        free(aux);
                    }
                    a = head;
                    head->next = NULL;
                } else {
                    aux = (struct nivel *)malloc(sizeof(struct nivel));
                    a->next = aux;
                    aux->next = NULL;
                    a->s = score;
                    nivel_atual += 1;
                    a->n = nivel_atual;
                    a = a->next;
                }
                //reinicializa os stats do jogo
                score = 0; gameover = false; gamewin = false;
            }
        }

        //essa parte do codigo desenha os sprites a cada frame
        BeginDrawing();
        ClearBackground(BLACK);
        
        struct Rectangle drawbarra = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
        DrawRectangleRec(drawbarra, barra.cor);
        
        for (int b = 0; b < MAX_BOLAS; b++) {
            if (bolas[b].ativa) DrawCircleV(bolas[b].position, bolas[b].raio, bolas[b].explosive ? RED : WHITE);
        }
        
        if (boss.estado == BOSS_INATIVO) {
            for(int i = 0; i < TIRA_LINHAS; i += 1) {
                for(int j = 0; j < TIRA_COLS; j += 1) {
                    if(tiras[i][j].ativo) {
                        struct Rectangle drawtiras = { tiras[i][j].position.x, tiras[i][j].position.y, TIRA_LARGURA, TIRA_ALTURA };
                        DrawRectangleRec(drawtiras, tiras[i][j].color);
                    }
                }
            }
        }
        
        // RENDERIZAÇÃO MATRICIAL DO BOSS
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
                    DrawText("CUIDADO: CARREGANDO LASER!", TELA_LARGURA / 2 - 120, 20, 16, RED);
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
                    DrawText("FASE 2: MODO FÚRIA ATIVADO!", TELA_LARGURA / 2 - 120, 20, 15, ORANGE);
                }
            }
            
            for (int i = 0; i < MAX_PROJETEIS; i++) {
                if (tiros[i].ativo) DrawCircleV(tiros[i].posicao, 6.0f, fúria ? ORANGE : VIOLET);
            }
        }
        
        if (poder.ativo) {
            DrawRectangleRec((Rectangle){poder.position.x, poder.position.y, poder.tamanho.x, poder.tamanho.y}, (poder.tipo == BUFF_DUPLICAR) ? BLUE : RED);
        }

        //desenha os sprites de texto
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
    
    //salva os status de cada nivel em um arquivo txt
    FILE *file = fopen("data.txt", "a");
    if (file != NULL) {
        fprintf(file, "log jogatina\n");
        struct nivel *temp_save = head; 
        while(temp_save != NULL && temp_save->next != NULL){ 
            fprintf(file, "nivel: %d\nscore: %d\n", temp_save->n, temp_save->s);
            temp_save = temp_save->next;
        }
        fclose(file);
    }
     
    return 0;
}