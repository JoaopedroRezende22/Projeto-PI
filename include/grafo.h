#ifndef GRAFO_H
#define GRAFO_H

#include <stddef.h>

#define ROTULO_MAX 63

/* No projeto, os algoritmos de grafos sao implementados manualmente em C.
 * Nao sao utilizadas bibliotecas prontas de grafos. */

typedef struct Adjacente {
    size_t destino;
    struct Adjacente *proximo;
} Adjacente;

typedef struct {
    char rotulo[ROTULO_MAX + 1];
    int duracao;
    Adjacente *lista;
} Vertice;

typedef struct {
    Vertice *vertices;
    size_t quantidade_vertices;
    size_t capacidade_vertices;
    size_t quantidade_arestas;
    int direcionado;
    int *matriz;
} Grafo;

/* Ciclo de vida */
int grafo_inicializar(Grafo *g, int direcionado);
void grafo_liberar(Grafo *g);

/* Modelagem */
int grafo_adicionar_vertice(Grafo *g, const char *rotulo);
int grafo_adicionar_aresta(Grafo *g, const char *origem, const char *destino);
int grafo_definir_duracao(Grafo *g, const char *tarefa, int dias);
int grafo_indice_vertice(const Grafo *g, const char *rotulo);

/* Representacoes */
void grafo_imprimir_lista(const Grafo *g);
void grafo_imprimir_matriz(const Grafo *g);

/* Buscas */
int grafo_bfs(const Grafo *g, const char *inicio, size_t **ordem, size_t *tamanho);
int grafo_dfs(const Grafo *g, const char *inicio, size_t **ordem, size_t *tamanho);

/* Conectividade */
int grafo_conexo_nao_direcionado(const Grafo *g);
int digrafo_fortemente_conexo(const Grafo *g);

/* Propriedades estruturais */
int grafo_possui_ciclo(const Grafo *g);
int grafo_ordenacao_topologica_kahn(const Grafo *g, size_t **ordem, size_t *tamanho);
int grafo_ordenacao_topologica_dfs(const Grafo *g, size_t **ordem, size_t *tamanho);

/* PERT/CPM: tempos mais cedo */
int grafo_tempos_mais_cedo(
    const Grafo *g,
    int *inicio_mais_cedo,
    int *termino_mais_cedo,
    int *duracao_projeto
);

/* PERT/CPM: tempos mais tarde */
int grafo_tempos_mais_tarde(
    const Grafo *g,
    int *inicio_mais_tarde,
    int *termino_mais_tarde,
    int duracao_projeto
);


/* PERT/CPM: calculo das folgas */
int grafo_calcular_folgas(
    const Grafo *g,
    const int *inicio_mais_cedo,
    const int *inicio_mais_tarde,
    int **folgas
);

/* PERT/CPM: identificacao das tarefas do caminho critico */
int grafo_identificar_caminho_critico(
    const Grafo *g,
    const int *folgas,
    int **eh_critica
);

/* PERT/CPM: impressao de todos os caminhos criticos */
int grafo_imprimir_caminhos_criticos(
    const Grafo *g,
    const int *inicio_mais_cedo,
    const int *termino_mais_cedo,
    const int *eh_critica,
    int duracao_projeto
);

int grafo_vertices_articulacao(const Grafo *g, int **eh_articulacao);
int grafo_pontes(const Grafo *g, int **eh_ponte);

/* Planaridade: verifica condicoes necessarias baseadas em Euler.
 * Isso NAO e um teste completo de planaridade. */
int grafo_verifica_formula_euler(const Grafo *g, long long *faces);

/* Coloracao gulosa do grafo subjacente (arestas direcionadas sao tratadas como conflitos). */
int grafo_coloracao_gulosa(const Grafo *g, int **cores, int *quantidade_cores);

/* Dados */
int grafo_carregar_dataset(const char *caminho, Grafo *g);

/* Utilitarios */
void grafo_imprimir_ordem(const Grafo *g, const size_t *ordem, size_t tamanho);
void grafo_imprimir_cores(const Grafo *g, const int *cores);

#endif