#include "placar.h"
#include <stdio.h>
#include <stdlib.h>

// Cria a lista encadeada de progresso dos níveis
struct nivel* InicializarPlacar() {
    struct nivel *head = (struct nivel *)malloc(sizeof(struct nivel));
    head->n = 1;      // nível inicial
    head->s = 0;      // score inicial
    head->next = NULL;
    return head;
}

// Salva o progresso do jogador em um novo nó da lista
void SalvarProgressoNivel(struct nivel** atual, int n, int score) {
    struct nivel *aux = (struct nivel *)malloc(sizeof(struct nivel));
    (*atual)->next = aux;
    aux->next = NULL;
    (*atual)->s = score; // salva score no nó atual
    (*atual)->n = n;     // salva nível no nó atual
    *atual = aux;        // avança ponteiro
}

// Limpa a lista encadeada, mantendo apenas o nó cabeça
void ResetarListaEncadeada(struct nivel* head) {
    head->s = 0;
    struct nivel* a = head->next;
    while(a != NULL) {
        struct nivel* aux = a;
        a = a->next;
        free(aux); // libera memória
    }
    head->next = NULL;
}

// Grava o progresso do jogador em arquivo texto
void GravarArquivoLog(struct nivel* head) {
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
}