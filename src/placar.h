#ifndef PLACAR_H
#define PLACAR_H

// Estrutura para armazenar o progresso do jogador em cada nível
struct nivel {
    int n;                // número do nível
    int s;                // score do nível
    struct nivel *next;   // ponteiro para o próximo nível
};

// Inicializa a lista de progresso
struct nivel* InicializarPlacar();
// Salva score e nível na lista
void SalvarProgressoNivel(struct nivel** atual, int n, int score);
// Limpa a lista de progresso
void ResetarListaEncadeada(struct nivel* head);
// Grava progresso em arquivo texto
void GravarArquivoLog(struct nivel* head);

#endif