#ifndef COMUM_H
#define COMUM_H

#include "raylib.h"

// Constantes globais do jogo (tamanho tela, limites, etc)
#define TELA_LARGURA 800
#define TELA_ALTURA 600
#define MAX_BOLAS 10
#define MAX_TIROS 20
#define TIRA_LINHAS 5
#define TIRA_COLS 10
#define TIRA_LARGURA 74
#define TIRA_ALTURA 20

// Estados possíveis do boss (usado na IA)
typedef enum {
    BOSS_INATIVO,   // boss fora da tela
    BOSS_ATIVO,     // boss em combate
    BOSS_DERROTADO  // boss derrotado
} EstadoBoss;

// Tipos de power-up que podem cair
typedef enum {
    BUFF_NENHUM,    // nenhum efeito
    BUFF_DUPLICAR,  // duplica bolas
    BUFF_EXPLOSAO   // bola explosiva
} TipoBuff;

// Estrutura da barra do jogador
struct barra {
    Vector2 position;
    Vector2 tamanho;
};

// Estrutura da bola
struct bola {
    Vector2 position;
    Vector2 vel;
    float raio;
    bool ativa;
    bool explosive; 
};

// Estrutura de um bloco (tira)
struct tira {
    Vector2 position;
    bool ativo;
};

// Estrutura de um projetil disparado pelo boss
struct projetil {
    Vector2 posicao;
    Vector2 vel;
    bool ativo;
};

// Estrutura de um item de power-up
struct buffItem {
    Vector2 position;
    TipoBuff tipo;
    bool ativo;
};

struct boss_Arkanoid {
    Vector2 posicao;
    Vector2 tamanho_total;
    int vida_max;
    int vida_current;
    EstadoBoss estado;
};

#endif