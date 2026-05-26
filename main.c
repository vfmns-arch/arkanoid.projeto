#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

//constantes com os tamanhos da tela e das tiras
//quando for testar o jogo e os sprites estiverem muito grandes ou muito pequenos, ajuste essas constantes
const int TELA_ALTURA = 600;
const int TELA_LARGURA = 800;
const int TIRA_LINHAS = 5;
const int TIRA_COLS = 10;
const int TIRA_LARGURA = 60;
const int TIRA_ALTURA = 20;
const int TIRA_MEIO = 10;
const int TIRA_STARTY = 50;
const int TIRA_STARTX = 50;

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
};
int main(){
  //inicializa o log inicial de nivel
  int nivel_atual = 1;
  //aloca espaço pro head e preenche seus atributos
  struct nivel *head = (struct nivel *)malloc(sizeof(struct nivel));
  head->n = 1;
  head->next = NULL;
  //declara as structs a (atual) e aux (auxiliar), elas começam apontando pro head
  struct nivel *a = head;
  struct nivel *aux = head;
  //inicializa a tela, com a largura, altura e o nome
  InitWindow(TELA_LARGURA, TELA_ALTURA, "Arkanoid");
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
  //inicializa a bola
  struct bola bola;
  //a posição inicial da bola fica no centro da tela
  bola.position.x = TELA_LARGURA / 2;
  bola.position.y = TELA_ALTURA / 2;
  //assim que iniciar, vai cair em linha reta numa velocidade de 4
  bola.vel.x = 0;
  bola.vel.y = -4;
  bola.raio = 8;
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
      tiras[i][j].color = GREEN;
    }
  }
  //inicializa p jogo
  int score = 0;
  bool gameover = false;
  bool gamewin = false;
  //enquanto a janela estiver aberta
  while(!WindowShouldClose()){
    //atualiza a posição dos objetos a cada frame
    if(!gameover && !gamewin){
      //muda a posição da barra de acordo com a tecla que apertar
      if(IsKeyDown(KEY_LEFT) && barra.position.x > 0){
        //a posição muda de acordo com velocidade, quando testar mudar velocidade caso não esteja boa 
        barra.position.x -= barra.vel;
      }
      if(IsKeyDown(KEY_RIGHT) && barra.position.x + barra.tamanho.x < TELA_LARGURA){
        barra.position.x += barra.vel;
      }
      //atualiza a posição da bola de acordo com sua velocidade atual nos eixos x e y
      bola.position.x += bola.vel.x;
      bola.position.y += bola.vel.y;
      //caso a bola ultrapasse as paredes da tela, inverte sua velocidade no eixo x
      if(bola.position.x - bola.raio <= 0 || bola.position.x + bola.raio >= TELA_LARGURA){
        bola.vel.x *= -1;
      }
      //caso a bola ultrapasse o topo da tela, inverte sua velocidade mo eixo y
      if(bola.position.y - bola.raio <= 0){
        bola.vel.y *= -1;
      }
      //caso a bola ultrapasse o fundo da tela, game over
      if(bola.position.y + bola.raio >= TELA_ALTURA){
        gameover = true;
      }
      //cria um struct Rectangle com os atributos da barra que o player controla pra usar a função CheckCollisionCircleRec
      struct Rectangle barraRe;
      //struct Rectangle é uma struct oferecida pelo raylib
      barraRe.x = barra.position.x;
      barraRe.y = barra.position.y;
      barraRe.width = barra.tamanho.x;
      barraRe.height = barra.tamanho.y;
      //caso a bola colida com a barra, inverte a velocidade da bola no eixo y
      if(CheckCollisionCircleRec(bola.position, bola.raio, barraRe)){
        bola.vel.y *= -1;
        //multiplica a velocidade do eixo x da bola dependendo do quão longe a bola bateu do centro da barra
        float hit = ((bola.position.x - (barra.position.x + barra.tamanho.x / 2)) / (barra.tamanho.x / 2));
        bola.vel.x = 4 * hit;
      }
      bool falta = false;
      bool colidiu = false;
      //checa todas as tiras
      for(int i = 0; i < TIRA_LINHAS && !colidiu; i += 1){
        for(int j = 0; j < TIRA_COLS && !colidiu; j += 1){
          if(tiras[i][j].ativo){
            //caso a tira esteja ativa, checa colisão com a bola
            falta = true;
            struct Rectangle tiraRe;
            tiraRe.x = tiras[i][j].position.x;
            tiraRe.y = tiras[i][j].position.y;
            tiraRe.width = (float)TIRA_LARGURA;
            tiraRe.height = (float)TIRA_ALTURA;
            if(CheckCollisionCircleRec(bola.position, bola.raio, tiraRe)){
              tiras[i][j].ativo = false;
              bola.vel.y *= -1;
              score += 1;
              //colidiu = true evita glitch de dupla colisão
              colidiu = true;
            }
          }
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
        barra.position.x = TELA_LARGURA / 2 - 50;
        barra.position.y = TELA_ALTURA - 40;
        bola.position.x = TELA_LARGURA / 2;
        bola.position.y = TELA_ALTURA / 2;
        bola.vel.x = 0;
        bola.vel.y = -4;
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
          aux = (struct nivel *)malloc(sizeof(struct nivel));
          a->next = aux;
          aux->next = NULL;
          a->s = score;
          nivel_atual += 1;
          a->n = nivel_atual;
          a = a->next;
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
    struct Rectangle drawbarra;
    drawbarra.x = barra.position.x;
    drawbarra.y = barra.position.y;
    drawbarra.width = barra.tamanho.x;
    drawbarra.height = barra.tamanho.y;
    DrawRectangleRec(drawbarra, GREEN);
    DrawCircleV(bola.position, bola.raio, GREEN);
    for(int i = 0; i < TIRA_LINHAS; i += 1){
      for(int j = 0; j < TIRA_COLS; j += 1){
        if(tiras[i][j].ativo){
          struct Rectangle drawtiras;
          drawtiras.x = tiras[i][j].position.x;
          drawtiras.y = tiras[i][j].position.y;
          drawtiras.width = TIRA_LARGURA;
          drawtiras.height = TIRA_ALTURA;
          DrawRectangleRec(drawtiras, tiras[i][j].color);
        }
      }
    }
    //desenha os sprites de texto
    DrawText(TextFormat("SCORE: %04i", score), 10, 10, 20, GREEN);
    if(gameover){
      DrawText("GAME OVER", TELA_LARGURA / 2 - 100, TELA_ALTURA / 2 - 20, 40, GREEN);
      DrawText("PRESS SPACE TO RESTART", TELA_LARGURA / 2 - 140, TELA_ALTURA / 2 + 30, 20, GREEN);
    }
    if(gamewin){
      DrawText("VENCEU!!!", TELA_LARGURA / 2 - 100, TELA_ALTURA / 2 - 20, 40, GREEN);
      DrawText("PRESS SPACE TO RESTART", TELA_LARGURA / 2 - 140, TELA_ALTURA / 2 + 30, 20, GREEN);
    }
    EndDrawing();
  }
  CloseWindow();
  //salva os status de cada nivel em um arquivo txt
  FILE *file = fopen("data.txt", "a");
  fprintf(file, "log jogatina\n");
  while(a != NULL){
    fprintf(file, "nivel: %d\nscore: %d\n", a->n, a->s);
    a = a->next;
  }
  fclose(file);
  
  return 0;
}
