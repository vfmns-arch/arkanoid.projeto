#include "raylib/src/raylib.h"
#include <stdio.h>
#include <stdlib.h>

//constantes com os tamanhos da tela e das tiras
//quando for testar o jogo e os sprites estiverem muito grandes ou muito pequenos, ajuste essas constantes
const int LARGURA_TELA = 800; // Mudado para evitar conflito com TIRA_LARGURA
const int TELA_ALTURA = 600;
const int TIRA_LINHAS = 5;
const int TIRA_COLS = 10;
const int TIRA_LARGURA = 60;
const int TIRA_ALTURA = 20;
const int TIRA_MEIO = 10;
const int TIRA_STARTY = 80; // Abaixado um pouco para não cortar o Score
const int TIRA_STARTX = 50;

// Limite máximo de bolas simultâneas em jogo
#define MAX_BOLAS 3

// Tipos de Buffs
typedef enum { BUFF_NENHUM = 0, BUFF_DUPLICAR, BUFF_EXPLOSAO } TipoBuff;

//struct nivel armazena o nivel, score e o endereço de memoria do proximo nivel 
struct nivel{
    int n;
    int s;
    struct nivel *next;
};
//struct barra armazena o x e y da posição (da esquera pra direita) e o tamanho, alem da velocidade
struct barra{
    //Vector2 é uma struct contendo float Vector2.x e float Vector2.y
    Vector2 position;
    Vector2 tamanho;
    float vel;
    Color cor; // Mantido para o efeito visual de piscar
};
struct tira{
    Vector2 position;
    bool ativo;
    Color color;
};
//bola.vel armazena a velocidade x e y da bola
struct bola{
    Vector2 position;
    Vector2 vel;
    float raio;
    bool ativa;       // Flag para saber se esta bola específica está em jogo
    bool explosiva;   // Flag para o buff de explosão
};
// Estrutura para gerenciar o item de poder que cai pela tela
struct buffItem {
    Vector2 position;
    Vector2 tamanho;
    float velY;
    bool ativo;
    TipoBuff tipo;
};

int main(){
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
  InitWindow(LARGURA_TELA, TELA_ALTURA, "Arkanoid - Beat this up!");
  //limita o FPS a 60
  SetTargetFPS(60);
  //inicia a barra que o jogador controla
  struct barra barra;
  //a esquerda da barra começa 50 pixels a esquerda do centro
  barra.position.x = LARGURA_TELA / 2 - 50;
  //o fundo da barra começa 40 pixels acima do fundo da tela
  barra.position.y = TELA_ALTURA - 40;
  barra.tamanho.x = 100;
  barra.tamanho.y = 20;
  barra.vel = 8;
  barra.cor = GREEN;
  
  //inicializa o array de bolas baseado no novo limite máximo
  struct bola bolas[MAX_BOLAS];
  //a posição inicial da primeira bola fica no centro da tela
  bolas[0].position.x = LARGURA_TELA / 2;
  bolas[0].position.y = TELA_ALTURA / 2;
  //assim que iniciar, vai cair em linha reta numa velocidade de 4
  bolas[0].vel.x = 0;
  bolas[0].vel.y = -4;
  bolas[0].raio = 8;
  bolas[0].ativa = true;
  bolas[0].explosiva = false;

  // Inicializa as bolas reservas do array como desativadas
  for (int i = 1; i < MAX_BOLAS; i++) {
      bolas[i].ativa = false;
      bolas[i].explosiva = false;
      bolas[i].raio = 8;
  }
  
  // Inicializa o item de poder que cai
  struct buffItem poder;
  poder.ativo = false;
  poder.tamanho = (Vector2){ 15, 15 };
  poder.velY = 3.0f; 

  // Array de cores estilo Atari para as linhas de blocos
  Color coresLinhas[5] = { RED, ORANGE, YELLOW, GREEN, BLUE };
  struct tira tiras[TIRA_LINHAS][TIRA_COLS];
  //loop inicializando as linhas das tiras
  for(int i = 0; i < TIRA_LINHAS; i += 1){
    //loop inicializando as colunas de tiras
    for(int j = 0; j < TIRA_COLS; j += 1){
      //a posição de cada tira vai ser aonde as tiras devem começar na tela + as tiras anteriores + o espaço entre elas
      tiras[i][j].position.x = TIRA_STARTX + j * (TIRA_LARGURA + TIRA_MEIO);
      tiras[i][j].position.y = TIRA_STARTY + i * (TIRA_ALTURA + TIRA_MEIO);
      //caso a bola não tenha destruido a tira, ela fica ativa
      tiras[i][j].ativo = true;
      tiras[i][j].color = coresLinhas[i % 5]; // Define uma cor por linha!
    }
  }
  //inicializa p jogo
  int score = 0;
  bool gameover = false;
  bool gamewin = false;
  int piscarFrames = 0; // Contador para o efeito da barra piscar
  
  //enquanto a janela estiver aberta
  while(!WindowShouldClose()){
    // Lógica do efeito de piscar a barra
    if (piscarFrames > 0) {
        barra.cor = WHITE;
        piscarFrames--;
    } else {
        barra.cor = GREEN;
    }

    //atualiza a posição dos objetos a cada frame
    if(!gameover && !gamewin){
      //muda a posição da barra de acordo com a tecla que apertar
      if(IsKeyDown(KEY_LEFT) && barra.position.x > 0){
        //a posição muda de acordo com velocidade, quando testar mudar velocidade caso não esteja boa 
        barra.position.x -= barra.vel;
      }
      if(IsKeyDown(KEY_RIGHT) && barra.position.x + barra.tamanho.x < LARGURA_TELA){
        barra.position.x += barra.vel;
      }
      
      // Loop para atualizar todas as bolas ativas do jogo
      for (int b = 0; b < MAX_BOLAS; b++) {
        if (!bolas[b].ativa) continue;

        //atualiza a posição da bola de acordo com sua velocidade atual nos eixos x e y
        bolas[b].position.x += bolas[b].vel.x;
        bolas[b].position.y += bolas[b].vel.y;
        
        //caso a bola ultrapasse as paredes da tela, inverte sua velocidade no eixo x
        if(bolas[b].position.x - bolas[b].raio <= 0 || bolas[b].position.x + bolas[b].raio >= LARGURA_TELA){
          bolas[b].vel.x *= -1;
        }
        //caso a bola ultrapasse o topo da tela, inverte sua velocidade mo eixo y
        if(bolas[b].position.y - bolas[b].raio <= 0){
          bolas[b].vel.y *= -1;
        }
        // Se a bola cair no abismo, ela é desativada
        if(bolas[b].position.y + bolas[b].raio >= TELA_ALTURA){
          bolas[b].ativa = false;
        }
        
        //cria um struct Rectangle com os atributos da barra que o player controla pra usar a função CheckCollisionCircleRec
        struct Rectangle barraRe = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
        //caso a bola colida com a barra, inverte a velocidade da bola no eixo y
        if(bolas[b].ativa && CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, barraRe)){
          bolas[b].vel.y *= -1;
          //multiplica a velocidade do eixo x da bola dependendo do quão longe a bola bateu do centro da barra
          float hit = ((bolas[b].position.x - (barra.position.x + barra.tamanho.x / 2)) / (barra.tamanho.x / 2));
          bolas[b].vel.x = 5 * hit;
          piscarFrames = 6; // Faz a barra piscar por 6 frames ao colidir
        }
        
        bool falta = false;
        bool colidiu = false;
        //checa todas as tiras
        for(int i = 0; i < TIRA_LINHAS && !colidiu; i += 1){
          for(int j = 0; j < TIRA_COLS && !colidiu; j += 1){
            if(tiras[i][j].ativo){
              //caso a tira esteja ativa, checa colisão com a bola
              falta = true;
              struct Rectangle tiraRe = { tiras[i][j].position.x, tiras[i][j].position.y, (float)TIRA_LARGURA, (float)TIRA_ALTURA };
              if(CheckCollisionCircleRec(bolas[b].position, bolas[b].raio, tiraRe)){
                tiras[i][j].ativo = false;
                bolas[b].vel.y *= -1;
                score += 10; // Cada bloco agora vale 10 pontos!
                //colidiu = true evita glitch de dupla colisão
                colidiu = true;

                // --- EFEITO DO BUFF DE EXPLOSÃO ---
                if (bolas[b].explosiva) {
                    bolas[b].explosiva = false; // Consome o buff após o impacto
                    
                    // Se não for a primeira linha, destrói o bloco de cima (i-1)
                    if (i > 0 && tiras[i-1][j].ativo) { tiras[i-1][j].ativo = false; score += 10; }
                    // Se não for a última linha, destrói o bloco de baixo (i+1)
                    if (i < TIRA_LINHAS-1 && tiras[i+1][j].ativo) { tiras[i+1][j].ativo = false; score += 10; }
                    // Se não for a primeira coluna, destrói o bloco da esquerda (j-1)
                    if (j > 0 && tiras[i][j-1].ativo) { tiras[i][j-1].ativo = false; score += 10; }
                    // Se não for a última coluna, destrói o bloco da direita (j+1)
                    if (j < TIRA_COLS-1 && tiras[i][j+1].ativo) { tiras[i][j+1].ativo = false; score += 10; }
                }

                // --- CHANCE DE SOLTAR UM BUFF (25% de chance) ---
                if (!poder.ativo && GetRandomValue(1, 4) == 1) {
                    poder.ativo = true;
                    poder.position.x = tiras[i][j].position.x + TIRA_LARGURA / 2 - 7;
                    poder.position.y = tiras[i][j].position.y;
                    poder.tipo = (GetRandomValue(1, 2) == 1) ? BUFF_DUPLICAR : BUFF_EXPLOSAO;
                }
              }
            }
          }
        }
      }

      // Condição de derrota: Varre o array e só dá Game Over se nenhuma bola estiver ativa
      bool algumaBolaAtiva = false;
      for (int b = 0; b < MAX_BOLAS; b++) {
          if (bolas[b].ativa) algumaBolaAtiva = true;
      }
      if (!algumaBolaAtiva) {
          gameover = true;
      }

      // lógica do buff que cai, se estiver ativo, ele cai e checa colisão com a barra
      if (poder.ativo) {
          poder.position.y += poder.velY; 
          
          if (poder.position.y > TELA_ALTURA) poder.ativo = false;

          struct Rectangle barraRe = { barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y };
          struct Rectangle buffRe = { poder.position.x, poder.position.y, poder.tamanho.x, poder.tamanho.y };
          
          if (CheckCollisionRecs(buffRe, barraRe)) {
              poder.ativo = false;
              
              if (poder.tipo == BUFF_DUPLICAR) {
                  int indiceBase = -1;
                  
                  // 1. Encontra a primeira bola ativa para servir de base para o clone
                  for (int b = 0; b < MAX_BOLAS; b++) {
                      if (bolas[b].ativa) {
                          indiceBase = b;
                          break; 
                      }
                  }
                  
                  // 2. Procura a primeira vaga livre no array de 3 elementos para adicionar +1 bola
                  if (indiceBase != -1) {
                      for (int b = 0; b < MAX_BOLAS; b++) {
                          if (!bolas[b].ativa) { 
                              bolas[b].position = bolas[indiceBase].position;
                              bolas[b].vel.x = -bolas[indiceBase].vel.x - 1; 
                              bolas[b].vel.y = -bolas[indiceBase].vel.y;
                              bolas[b].ativa = true;
                              bolas[b].explosiva = false;
                              break; // Interrompe o loop para criar apenas 1 bola por buff coletado
                          }
                      }
                  }
              } 
              else if (poder.tipo == BUFF_EXPLOSAO) {
                  for (int b = 0; b < MAX_BOLAS; b++) {
                      if (bolas[b].ativa) bolas[b].explosiva = true;
                  }
              }
          }
      }

      // Verifica se ainda restam tiras na tela
      bool falta = false;
      for(int i = 0; i < TIRA_LINHAS; i++) {
          for(int j = 0; j < TIRA_COLS; j++) {
              if(tiras[i][j].ativo) falta = true;
          }
      }
      //se não faltar tiras, jogador vence
      if(!falta){
        gamewin = true;
      }
    //se o jogador estiver na tela de menu
    } else {
      //quando o jogador pressionar espaço, o jogo reinicia 
      if(IsKeyPressed(KEY_SPACE)){
        barra.position.x = LARGURA_TELA / 2 - 50;
        barra.position.y = TELA_ALTURA - 40;
        
        bolas[0].position.x = LARGURA_TELA / 2;
        bolas[0].position.y = TELA_ALTURA / 2;
        bolas[0].vel.x = 0;
        bolas[0].vel.y = -4;
        bolas[0].ativa = true;
        bolas[0].explosiva = false;
        
        // Desativa todas as outras bolas adicionais no reset
        for (int b = 1; b < MAX_BOLAS; b++) {
            bolas[b].ativa = false;
            bolas[b].explosiva = false;
        }
        poder.ativo = false;
        
        for(int i = 0; i < TIRA_LINHAS; i += 1){
          for(int j = 0; j < TIRA_COLS; j += 1){
            tiras[i][j].ativo = true;
          }
        }
        //o score é logado nas structs
        if(gameover == true){
          nivel_atual = 1;
          head->s = 0;
          a = head->next;
          while(a != NULL){
            aux = a;
            a = a->next;
            free(aux);
          }
          a = head;
          head->next = NULL;
        } else {
          a->s = score; // Salva o score do nível que acabou de vencer
          aux = (struct nivel *)malloc(sizeof(struct nivel));
          a->next = aux;
          aux->next = NULL;
          nivel_atual += 1;
          aux->n = nivel_atual;
          aux->s = 0;
          a = aux;
        }
        //reinicializa os stats do jogo
        score = 0;
        gameover = false;
        gamewin = false;
        
      }
    }
    //essa parte do codigo desenha os sprites a cada frame
    BeginDrawing();
    //set a cor do fundo pra preto
    ClearBackground(BLACK);
    
    // Desenha a barra com a cor dinâmica (efeito piscar)
    DrawRectangleRec((Rectangle){barra.position.x, barra.position.y, barra.tamanho.x, barra.tamanho.y}, barra.cor);
    
    // Desenha as bolas que estiverem em jogo varrendo o array completo
    for (int b = 0; b < MAX_BOLAS; b++) {
        if (bolas[b].ativa) {
            Color corBola = bolas[b].explosiva ? RED : WHITE; // Bola vermelha se for explosiva
            DrawCircleV(bolas[b].position, bolas[b].raio, corBola);
        }
    }
    
    // Desenha os blocos
    for(int i = 0; i < TIRA_LINHAS; i += 1){
      for(int j = 0; j < TIRA_COLS; j += 1){
        if(tiras[i][j].ativo){
          DrawRectangleRec((Rectangle){tiras[i][j].position.x, tiras[i][j].position.y, TIRA_LARGURA, TIRA_ALTURA}, tiras[i][j].color);
        }
      }
    }
    
    // Desenha o Item de Buff caindo
    if (poder.ativo) {
        Color corItem = (poder.tipo == BUFF_DUPLICAR) ? BLUE : RED;
        DrawRectangleRec((Rectangle){poder.position.x, poder.position.y, poder.tamanho.x, poder.tamanho.y}, corItem);
        DrawRectangleLines(poder.position.x, poder.position.y, poder.tamanho.x, poder.tamanho.y, RAYWHITE);
    }

    //desenha os sprites de texto
    DrawText(TextFormat("SCORE: %04i", score), 20, 20, 20, RAYWHITE);
    DrawText(TextFormat("NÍVEL: %d", nivel_atual), LARGURA_TELA - 120, 20, 20, RAYWHITE);
    
    // Mostra aviso na tela caso alguma bola esteja sob efeito explosivo
    bool algumaExplosiva = false;
    for(int b = 0; b < MAX_BOLAS; b++) {
        if(bolas[b].ativa && bolas[b].explosiva) algumaExplosiva = true;
    }
    if (algumaExplosiva) {
        DrawText("PODER ATIVO: PRÓXIMO HIT EXPLODE EM ÁREA!", LARGURA_TELA / 2 - 190, 20, 16, RED);
    }

    if(gameover){
      DrawText("GAME OVER", LARGURA_TELA / 2 - 110, TELA_ALTURA / 2 - 20, 40, RED);
      DrawText("PRESSIONE ESPAÇO PARA RECOMEÇAR", LARGURA_TELA / 2 - 180, TELA_ALTURA / 2 + 30, 20, RAYWHITE);
    }
    if(gamewin){
      DrawText("VENCEU!!!", LARGURA_TELA / 2 - 100, TELA_ALTURA / 2 - 20, 40, GOLD);
      DrawText("PRESSIONE ESPAÇO PARA O PRÓXIMO NÍVEL", LARGURA_TELA / 2 - 210, TELA_ALTURA / 2 + 30, 20, RAYWHITE);
    }
    EndDrawing();
  }
  CloseWindow();
  
  // Aponta o ponteiro 'a' de volta para o início ('head') para salvar tudo no txt corretamente
  a = head;
  //salva os status de cada nivel em um arquivo txt
  FILE *file = fopen("data.txt", "a");
  if (file != NULL) {
      fprintf(file, "--- Novo Log de Jogatina ---\n");
      while(a != NULL){
        fprintf(file, "Nivel: %d | Score final: %d\n", a->n, a->s);
        struct nivel *temp = a;
        a = a->next;
        free(temp); // Libera a memória alocada no fim do programa
      }
      fclose(file);
  }
  
  return 0;
}