#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

const int TELA_ALTURA = 600;
const int TELA_LARGURA = 800;
const int TIRA_LINHAS = 5;
const int TIRA_COLS = 10;
const int TIRA_LARGURA = 60;
const int TIRA_ALTURA = 20;
const int TIRA_MEIO = 10;
const int TIRA_STARTY = 50;
const int TIRA_STARTX = 50;

struct nivel{
    int n;
    int s;
    struct *nivel;
};
struct barra{
    Vector2 position;
    Vector2 tamanho;
    float vel;
};
struct tira{
    Vector2 position;
    bool ativo;
    Color color;
};
struct bola{
    Vector2 position;
    Vector2 vel;
    float raio;
};
int main(){
  int nivel_atual = 1;
  struct nivel *head = (struct nivel)malloc(sizeof(struct nivel));
  head->n = 1;
  struct nivel *a = head;
  struct nivel *aux = head;
  InitWindow(TELA_LARGURA, TELA_ALTURA, "Arkanoid");
  SetTargetFPS(60);
  struct barra barra;
  barra.position.x = TELA_LARGURA / 2 - 50;
  barra.position.y = TELA_ALTURA - 40;
  barra.tamanho.x = 100;
  barra.tamanho.y = 20;
  barra.vel = 8;
  struct bola bola;
  bola.position.x = TELA_LARGURA / 2;
  bola.position.y = TELA_ALTURA / 2;
  bola.vel.x = 0;
  bola.vel.y = -4;
  bola.raio = 8;
  struct tira tiras[TIRA_LINHAS][TIRA_COLS];
  for(int i = 0; i < TIRA_LINHAS; i += 1){
    for(int j = 0; j < TIRA_COLS; j += 1){
      tiras[i][j].position.x = TIRA_STARTX + j * (TIRA_LARGURA + TIRA_MEIO);
      tiras[i][j].position.y = TIRA_STARTY + i * (TIRA_ALTURA + TIRA_MEIO);
      tiras[i][j].ativo = true;
      tiras[i][j].color = GREEN;
    }
  }
  int score = 0;
  bool gameover = false;
  bool gamewin = false;
  while(!WindowShouldClose()){
    if(!gameover && !gamewin){
      if(IsKeyDown(KEY_LEFT) && barra.position.x > 0){
        barra.position.x -= barra.vel;
      }
      if(IsKeyDown(KEY_RIGHT) && barra.position.x + barra.tamanho.x < TELA_LARGURA){
        barra.position.x += barra.vel;
      }
      bola.position.x += bola.vel.x;
      bola.position.y += bola.vel.y;
      if(bola.position.x - bola.raio <= 0 || bola.position.x + bola.raio >= TELA_LARGURA){
        bola.vel.x *= -1;
      }
      if(bola.position.y - bola.raio <= 0){
        bola.vel.y *= -1;
      }
      if(bola.position.y + bola.raio >= TELA_ALTURA){
        gameover = true;
      }
      struct Rectangle barraRe;
      barraRe.x = barra.position.x;
      barraRe.y = barra.position.y;
      barraRe.width = barra.tamanho.x;
      barraRe.height = barra.tamanho.y;
      if(CheckCollisionCircleRec(bola.position, bola.raio, barraRe)){
        bola.vel.y *= -1;
        float hit = ((bola.position.x - (barra.position.x + barra.tamanho.x / 2)) / (barra.tamanho.x / 2));
        bola.vel.x = 4 * hit;
      }
      bool nfalta = true;
      bool colidiu = false;
      for(int i = 0; i < TIRA_LINHAS && !colidiu; i += 1){
        for(int j = 0; j < TIRA_COLS && !colidiu; j += 1){
          if(tiras[i][j].ativo){
            nfalta = false;
            struct Rectangle tiraRe;
            tiraRe.x = tiras[i][j].position.x;
            tiraRe.y = tiras[i][j].position.y;
            tiraRe.width = (float)TIRA_LARGURA;
            tiraRe.height = (float)TIRA_ALTURA;
            if(CheckCollisionCircleRec(bola.position, bola.raio, tiraRe)){
              tiras[i][j].ativo = false;
              bola.vel.y *= -1;
              score += 1;
              colidiu = true;
            }
          }
        }
      }
      if(nfalta){
        gamewin = true;
      }
    } else {
      if(gameover == true){
        nivel_atual = 1;
        head->s = 0;
        a = head->next;
        while(a != NULL){
          aux = a;
          a = a->next;
          free(aux);
        } else {
          aux = (struct nivel)malloc(sizeof(struct nivel));
          a->next = aux;
          a->s = score;
          nivel_atual += 1;
          a->n = nivel_atual;
          a = a->next;
        }
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
        score = 0;
        gameover = false;
        gamewin = false;
      }
    }
    BeginDrawing();
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
  return 0;
}
