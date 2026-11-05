#include "grafo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int garantir_capacidade_vertices(Grafo *g) {
    if (g->quantidade_vertices < g->capacidade_vertices) return 1;

    size_t nova_capacidade = (g->capacidade_vertices == 0) ? 16 : g->capacidade_vertices * 2;
    Vertice *novo = (Vertice *)realloc(g->vertices, nova_capacidade * sizeof(Vertice));
    if (!novo) return 0;

    g->vertices = novo;
    g->capacidade_vertices = nova_capacidade;
    return 1;
}

static int redimensionar_matriz(Grafo *g, size_t novo_n) {
    int *nova = (int *)calloc(novo_n * novo_n, sizeof(int));
    if (!nova) return 0;

    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        for (size_t j = 0; j < g->quantidade_vertices; ++j) {
            nova[i * novo_n + j] = g->matriz[i * g->quantidade_vertices + j];
        }
    }

    free(g->matriz);
    g->matriz = nova;
    return 1;
}

static Adjacente *criar_adjacente(size_t destino) {
    Adjacente *novo = (Adjacente *)malloc(sizeof(Adjacente));
    if (!novo) return NULL;
    novo->destino = destino;
    novo->proximo = NULL;
    return novo;
}

static int lista_contem(const Vertice *v, size_t destino) {
    for (Adjacente *a = v->lista; a; a = a->proximo) {
        if (a->destino == destino) return 1;
    }
    return 0;
}

static int adicionar_na_lista(Vertice *v, size_t destino) {
    if (lista_contem(v, destino)) return 1;
    Adjacente *novo = criar_adjacente(destino);
    if (!novo) return 0;
    novo->proximo = v->lista;
    v->lista = novo;
    return 1;
}

int grafo_inicializar(Grafo *g, int direcionado) {
    if (!g) return 0;
    g->vertices = NULL;
    g->quantidade_vertices = 0;
    g->capacidade_vertices = 0;
    g->quantidade_arestas = 0;
    g->direcionado = direcionado ? 1 : 0;
    g->matriz = NULL;
    return 1;
}

void grafo_liberar(Grafo *g) {
    if (!g) return;
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        Adjacente *a = g->vertices[i].lista;
        while (a) {
            Adjacente *prox = a->proximo;
            free(a);
            a = prox;
        }
    }
    free(g->vertices);
    free(g->matriz);
    grafo_inicializar(g, 0);
}

int grafo_indice_vertice(const Grafo *g, const char *rotulo) {
    if (!g || !rotulo) return -1;
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        if (strcmp(g->vertices[i].rotulo, rotulo) == 0) return (int)i;
    }
    return -1;
}

int grafo_adicionar_vertice(Grafo *g, const char *rotulo) {
    if (!g || !rotulo || rotulo[0] == '\0') return 0;
    if (grafo_indice_vertice(g, rotulo) >= 0) return 1;
    if (strlen(rotulo) > ROTULO_MAX) return 0;
    if (!garantir_capacidade_vertices(g)) return 0;

    size_t antigo_n = g->quantidade_vertices;
    if (antigo_n == 0) {
        g->matriz = (int *)calloc(1, sizeof(int));
        if (!g->matriz) return 0;
    } else {
        if (!redimensionar_matriz(g, antigo_n + 1)) return 0;
    }

    Vertice *v = &g->vertices[antigo_n];
    strncpy(v->rotulo, rotulo, ROTULO_MAX);
    v->rotulo[ROTULO_MAX] = '\0';
    v->duracao = 0;
    v->lista = NULL;
    g->quantidade_vertices++;
    return 1;
}

int grafo_adicionar_aresta(Grafo *g, const char *origem, const char *destino) {
    if (!g || !origem || !destino) return 0;
    if (!grafo_adicionar_vertice(g, origem)) return 0;
    if (!grafo_adicionar_vertice(g, destino)) return 0;

    int u = grafo_indice_vertice(g, origem);
    int v = grafo_indice_vertice(g, destino);
    if (u < 0 || v < 0) return 0;

    if (g->matriz[(size_t)u * g->quantidade_vertices + (size_t)v]) return 1;

    if (!adicionar_na_lista(&g->vertices[u], (size_t)v)) return 0;
    g->matriz[(size_t)u * g->quantidade_vertices + (size_t)v] = 1;

    if (!g->direcionado && u != v) {
        if (!adicionar_na_lista(&g->vertices[v], (size_t)u)) return 0;
        g->matriz[(size_t)v * g->quantidade_vertices + (size_t)u] = 1;
    }

    g->quantidade_arestas++;
    return 1;
}

int grafo_definir_duracao(Grafo *g, const char *tarefa, int dias) {
    if (!g || !tarefa || dias < 0) return 0;
    if (!grafo_adicionar_vertice(g, tarefa)) return 0;
    int i = grafo_indice_vertice(g, tarefa);
    if (i < 0) return 0;
    g->vertices[i].duracao = dias;
    return 1;
}

void grafo_imprimir_lista(const Grafo *g) {
    if (!g) return;
    printf("\nLista de adjacencia:\n");
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        printf("  %s: ", g->vertices[i].rotulo);
        Adjacente *a = g->vertices[i].lista;
        int primeiro = 1;
        while (a) {
            if (!primeiro) printf(", ");
            printf("%s", g->vertices[a->destino].rotulo);
            primeiro = 0;
            a = a->proximo;
        }
        printf("\n");
    }
}

void grafo_imprimir_matriz(const Grafo *g) {
    if (!g) return;
    printf("\nMatriz de adjacencia:\n    ");
    for (size_t i = 0; i < g->quantidade_vertices; ++i) printf("%-4s", g->vertices[i].rotulo);
    printf("\n");
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        printf("%-4s", g->vertices[i].rotulo);
        for (size_t j = 0; j < g->quantidade_vertices; ++j) {
            printf("%-4d", g->matriz[i * g->quantidade_vertices + j]);
        }
        printf("\n");
    }
}

static int *alocar_visitados(size_t n) {
    return (int *)calloc(n, sizeof(int));
}

int grafo_bfs(const Grafo *g, const char *inicio, size_t **ordem, size_t *tamanho) {
    if (!g || !inicio || !ordem || !tamanho) return 0;
    *ordem = NULL;
    *tamanho = 0;
    int inicio_i = grafo_indice_vertice(g, inicio);
    if (inicio_i < 0) return 0;

    int *visitado = alocar_visitados(g->quantidade_vertices);
    size_t *fila = (size_t *)malloc(g->quantidade_vertices * sizeof(size_t));
    size_t *saida = (size_t *)malloc(g->quantidade_vertices * sizeof(size_t));
    if ((!visitado && g->quantidade_vertices) || (!fila && g->quantidade_vertices) || (!saida && g->quantidade_vertices)) {
        free(visitado); free(fila); free(saida); return 0;
    }

    size_t ini = 0, fim = 0, qtd = 0;
    fila[fim++] = (size_t)inicio_i;
    visitado[inicio_i] = 1;

    while (ini < fim) {
        size_t u = fila[ini++];
        saida[qtd++] = u;
        for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
            if (!visitado[a->destino]) {
                visitado[a->destino] = 1;
                fila[fim++] = a->destino;
            }
        }
    }

    free(visitado); free(fila);
    *ordem = saida; *tamanho = qtd;
    return 1;
}

static void dfs_rec(const Grafo *g, size_t u, int *visitado, size_t *saida, size_t *qtd) {
    visitado[u] = 1;
    saida[(*qtd)++] = u;
    for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
        if (!visitado[a->destino]) dfs_rec(g, a->destino, visitado, saida, qtd);
    }
}

int grafo_dfs(const Grafo *g, const char *inicio, size_t **ordem, size_t *tamanho) {
    if (!g || !inicio || !ordem || !tamanho) return 0;
    *ordem = NULL; *tamanho = 0;
    int inicio_i = grafo_indice_vertice(g, inicio);
    if (inicio_i < 0) return 0;

    int *visitado = alocar_visitados(g->quantidade_vertices);
    size_t *saida = (size_t *)malloc(g->quantidade_vertices * sizeof(size_t));
    if ((!visitado && g->quantidade_vertices) || (!saida && g->quantidade_vertices)) {
        free(visitado); free(saida); return 0;
    }
    size_t qtd = 0;
    dfs_rec(g, (size_t)inicio_i, visitado, saida, &qtd);
    free(visitado);
    *ordem = saida; *tamanho = qtd;
    return 1;
}

int grafo_conexo_nao_direcionado(const Grafo *g) {
    if (!g || g->direcionado) return 0;
    if (g->quantidade_vertices == 0) return 1;
    size_t *ordem = NULL, tamanho = 0;
    int ok = grafo_bfs(g, g->vertices[0].rotulo, &ordem, &tamanho);
    free(ordem);
    return ok && tamanho == g->quantidade_vertices;
}

static int dfs_matriz(const Grafo *g, size_t u, int *visitado, int transposta) {
    visitado[u] = 1;
    for (size_t v = 0; v < g->quantidade_vertices; ++v) {
        int existe = transposta
            ? g->matriz[v * g->quantidade_vertices + u]
            : g->matriz[u * g->quantidade_vertices + v];
        if (existe && !visitado[v]) dfs_matriz(g, v, visitado, transposta);
    }
    return 1;
}

static int todos_visitados(const int *visitado, size_t n) {
    for (size_t i = 0; i < n; ++i) if (!visitado[i]) return 0;
    return 1;
}

int digrafo_fortemente_conexo(const Grafo *g) {
    if (!g || !g->direcionado) return 0;
    if (g->quantidade_vertices == 0) return 1;

    int *visitado = alocar_visitados(g->quantidade_vertices);
    if (!visitado) return 0;
    dfs_matriz(g, 0, visitado, 0);
    int ida = todos_visitados(visitado, g->quantidade_vertices);
    memset(visitado, 0, g->quantidade_vertices * sizeof(int));
    dfs_matriz(g, 0, visitado, 1);
    int volta = todos_visitados(visitado, g->quantidade_vertices);
    free(visitado);
    return ida && volta;
}

int grafo_ordenacao_topologica_kahn(const Grafo *g, size_t **ordem, size_t *tamanho) {
    if (!g || !ordem || !tamanho) return 0;
    *ordem = NULL; *tamanho = 0;
    if (!g->direcionado) return 0;

    int *grau = (int *)calloc(g->quantidade_vertices, sizeof(int));
    size_t *fila = (size_t *)malloc(g->quantidade_vertices * sizeof(size_t));
    size_t *saida = (size_t *)malloc(g->quantidade_vertices * sizeof(size_t));
    if ((!grau && g->quantidade_vertices) || (!fila && g->quantidade_vertices) || (!saida && g->quantidade_vertices)) {
        free(grau); free(fila); free(saida); return 0;
    }

    for (size_t u = 0; u < g->quantidade_vertices; ++u)
        for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) grau[a->destino]++;

    size_t ini = 0, fim = 0, qtd = 0;
    for (size_t i = 0; i < g->quantidade_vertices; ++i) if (grau[i] == 0) fila[fim++] = i;

    while (ini < fim) {
        size_t u = fila[ini++];
        saida[qtd++] = u;
        for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
            if (--grau[a->destino] == 0) fila[fim++] = a->destino;
        }
    }

    free(grau); free(fila);
    if (qtd != g->quantidade_vertices) {
        free(saida);
        return 0;
    }
    *ordem = saida; *tamanho = qtd;
    return 1;
}

static int dfs_topo(const Grafo *g, size_t u, int *estado, size_t *saida, size_t *qtd) {
    estado[u] = 1;
    for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
        if (estado[a->destino] == 1) return 0;
        if (estado[a->destino] == 0 && !dfs_topo(g, a->destino, estado, saida, qtd)) return 0;
    }
    estado[u] = 2;
    saida[(*qtd)++] = u;
    return 1;
}

int grafo_tempos_mais_cedo(
    const Grafo *g,
    int *inicio_mais_cedo,
    int *termino_mais_cedo,
    int *duracao_projeto
) {
    if (!g || !inicio_mais_cedo || !termino_mais_cedo || !duracao_projeto) return 0;
    if (!g->direcionado || g->quantidade_vertices == 0) return 0;

    size_t *ordem = NULL;
    size_t tamanho = 0;

    /* A ordenacao topologica tambem confirma que o grafo nao tem ciclos. */
    if (!grafo_ordenacao_topologica_kahn(g, &ordem, &tamanho)) return 0;

    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        if (g->vertices[i].duracao < 0) {
            free(ordem);
            return 0;
        }
        inicio_mais_cedo[i] = 0;
        termino_mais_cedo[i] = 0;
    }

    int maior_termino = 0;

    for (size_t i = 0; i < tamanho; ++i) {
        size_t u = ordem[i];
        int duracao = g->vertices[u].duracao;

        if (duracao > INT_MAX - inicio_mais_cedo[u]) {
            free(ordem);
            return 0;
        }

        termino_mais_cedo[u] = inicio_mais_cedo[u] + duracao;
        if (termino_mais_cedo[u] > maior_termino) {
            maior_termino = termino_mais_cedo[u];
        }

        for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
            size_t v = a->destino;
            if (termino_mais_cedo[u] > inicio_mais_cedo[v]) {
                inicio_mais_cedo[v] = termino_mais_cedo[u];
            }
        }
    }

    free(ordem);
    *duracao_projeto = maior_termino;
    return 1;
}


int grafo_tempos_mais_tarde(
    const Grafo *g,
    int *inicio_mais_tarde,
    int *termino_mais_tarde,
    int duracao_projeto
) {
    if (!g || !inicio_mais_tarde || !termino_mais_tarde) return 0;
    if (!g->direcionado || g->quantidade_vertices == 0) return 0;
    if (duracao_projeto < 0) return 0;

    size_t *ordem = NULL;
    size_t tamanho = 0;

    /* A ordenacao topologica valida que o grafo nao possui ciclos. */
    if (!grafo_ordenacao_topologica_kahn(g, &ordem, &tamanho)) {
        return 0;
    }

    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        if (g->vertices[i].duracao < 0) {
            free(ordem);
            return 0;
        }

        inicio_mais_tarde[i] = 0;
        termino_mais_tarde[i] = 0;
    }

    /* Inicialmente, todas as tarefas podem terminar no fim do projeto. */
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        termino_mais_tarde[i] = duracao_projeto;
        inicio_mais_tarde[i] =
            termino_mais_tarde[i] - g->vertices[i].duracao;
    }

    /* Percorre as tarefas na ordem topologica inversa. */
    for (size_t i = tamanho; i > 0; --i) {
        size_t u = ordem[i - 1];
        int menor_inicio_sucessoras = duracao_projeto;
        int possui_sucessora = 0;

        for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
            size_t v = a->destino;
            possui_sucessora = 1;

            if (inicio_mais_tarde[v] < menor_inicio_sucessoras) {
                menor_inicio_sucessoras = inicio_mais_tarde[v];
            }
        }

        if (possui_sucessora) {
            termino_mais_tarde[u] = menor_inicio_sucessoras;
            inicio_mais_tarde[u] =
                termino_mais_tarde[u] - g->vertices[u].duracao;
        }
    }

    free(ordem);
    return 1;
}


int grafo_calcular_folgas(
    const Grafo *g,
    const int *inicio_mais_cedo,
    const int *inicio_mais_tarde,
    int **folgas
) {
    if (!g || !inicio_mais_cedo || !inicio_mais_tarde || !folgas) {
        return 0;
    }

    if (g->quantidade_vertices == 0) {
        return 0;
    }

    size_t n = g->quantidade_vertices;
    int *resultado = malloc(n * sizeof(int));

    if (!resultado) {
        return 0;
    }

    for (size_t i = 0; i < n; ++i) {
        resultado[i] = inicio_mais_tarde[i] - inicio_mais_cedo[i];

        if (resultado[i] < 0) {
            free(resultado);
            return 0;
        }
    }

    *folgas = resultado;
    return 1;
}

int grafo_identificar_caminho_critico(
    const Grafo *g,
    const int *folgas,
    int **eh_critica
) {
    if (!g || !folgas || !eh_critica) {
        return 0;
    }

    if (g->quantidade_vertices == 0) {
        return 0;
    }

    size_t n = g->quantidade_vertices;

    int *resultado = malloc(n * sizeof(int));

    if (!resultado) {
        return 0;
    }

    for (size_t i = 0; i < n; ++i) {
        if (folgas[i] < 0) {
            free(resultado);
            return 0;
        }

        resultado[i] = (folgas[i] == 0) ? 1 : 0;
    }

    *eh_critica = resultado;

    return 1;
}

static void reconstruir_caminhos_criticos(
    const Grafo *g,
    size_t atual,
    const int *inicio_mais_cedo,
    const int *termino_mais_cedo,
    const int *eh_critica,
    int duracao_projeto,
    size_t *caminho,
    size_t profundidade,
    size_t *quantidade_caminhos
) {
    caminho[profundidade] = atual;

    int possui_sucessora_critica = 0;

    for (Adjacente *a = g->vertices[atual].lista;
         a;
         a = a->proximo) {

        size_t sucessora = a->destino;

        if (eh_critica[sucessora] &&
            inicio_mais_cedo[sucessora] == termino_mais_cedo[atual]) {

            possui_sucessora_critica = 1;

            reconstruir_caminhos_criticos(
                g,
                sucessora,
                inicio_mais_cedo,
                termino_mais_cedo,
                eh_critica,
                duracao_projeto,
                caminho,
                profundidade + 1,
                quantidade_caminhos
            );
        }
    }

    if (!possui_sucessora_critica &&
        termino_mais_cedo[atual] == duracao_projeto) {

        printf("Caminho critico %zu: ", *quantidade_caminhos + 1);

        for (size_t i = 0; i <= profundidade; ++i) {
            printf("%s", g->vertices[caminho[i]].rotulo);

            if (i < profundidade) {
                printf(" -> ");
            }
        }

        printf("\n");
        (*quantidade_caminhos)++;
    }
}


int grafo_imprimir_caminhos_criticos(
    const Grafo *g,
    const int *inicio_mais_cedo,
    const int *termino_mais_cedo,
    const int *eh_critica,
    int duracao_projeto
) {
    if (!g || !inicio_mais_cedo || !termino_mais_cedo ||
        !eh_critica || !g->direcionado ||
        g->quantidade_vertices == 0 || duracao_projeto < 0) {
        return 0;
    }

    size_t n = g->quantidade_vertices;
    size_t *caminho = malloc(n * sizeof(size_t));

    if (!caminho) {
        return 0;
    }

    size_t quantidade_caminhos = 0;

    for (size_t i = 0; i < n; ++i) {
        if (!eh_critica[i] || inicio_mais_cedo[i] != 0) {
            continue;
        }

        int possui_predecessora_critica = 0;

for (size_t u = 0; u < n; ++u) {
    for (Adjacente *a = g->vertices[u].lista;
         a;
         a = a->proximo) {

        if (a->destino == i &&
            eh_critica[u] &&
            termino_mais_cedo[u] == inicio_mais_cedo[i]) {

            possui_predecessora_critica = 1;
            break;
        }
    }

    if (possui_predecessora_critica) {
        break;
    }
}

if (!possui_predecessora_critica &&
    inicio_mais_cedo[i] == 0) {
            reconstruir_caminhos_criticos(
                g,
                i,
                inicio_mais_cedo,
                termino_mais_cedo,
                eh_critica,
                duracao_projeto,
                caminho,
                0,
                &quantidade_caminhos
            );
        }
    }

    free(caminho);

    if (quantidade_caminhos == 0) {
        printf("Nenhum caminho critico completo foi encontrado.\n");
        return 0;
    }

    return 1;
}

int grafo_ordenacao_topologica_dfs(const Grafo *g, size_t **ordem, size_t *tamanho) {
    if (!g || !ordem || !tamanho) return 0;
    *ordem = NULL; *tamanho = 0;
    if (!g->direcionado) return 0;

    int *estado = (int *)calloc(g->quantidade_vertices, sizeof(int));
    size_t *saida = (size_t *)malloc(g->quantidade_vertices * sizeof(size_t));
    if ((!estado && g->quantidade_vertices) || (!saida && g->quantidade_vertices)) {
        free(estado); free(saida); return 0;
    }

    size_t qtd = 0;
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        if (estado[i] == 0 && !dfs_topo(g, i, estado, saida, &qtd)) {
            free(estado); free(saida); return 0;
        }
    }
    for (size_t i = 0; i < qtd / 2; ++i) {
        size_t tmp = saida[i]; saida[i] = saida[qtd - 1 - i]; saida[qtd - 1 - i] = tmp;
    }
    free(estado);
    *ordem = saida; *tamanho = qtd;
    return 1;
}

int grafo_possui_ciclo(const Grafo *g) {
    if (!g || !g->direcionado) return 0;
    size_t *ordem = NULL, tamanho = 0;
    int ok = grafo_ordenacao_topologica_kahn(g, &ordem, &tamanho);
    free(ordem);
    return !ok;
}

static void tarjan_ap(const Grafo *g, size_t u, int *visitado, int *disc, int *low, int *pai, int *eh_ap, int *tempo) {
    visitado[u] = 1;
    disc[u] = low[u] = ++(*tempo);
    int filhos = 0;
    for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
        size_t v = a->destino;
        if (!visitado[v]) {
            pai[v] = (int)u;
            filhos++;
            tarjan_ap(g, v, visitado, disc, low, pai, eh_ap, tempo);
            if (low[v] < low[u]) low[u] = low[v];
            if (pai[u] == -1 && filhos > 1) eh_ap[u] = 1;
            if (pai[u] != -1 && low[v] >= disc[u]) eh_ap[u] = 1;
        } else if ((int)v != pai[u] && disc[v] < low[u]) {
            low[u] = disc[v];
        }
    }
}

int grafo_vertices_articulacao(const Grafo *g, int **eh_articulacao) {
    if (!g || !eh_articulacao || g->direcionado) return 0;
    *eh_articulacao = (int *)calloc(g->quantidade_vertices, sizeof(int));
    if (!*eh_articulacao && g->quantidade_vertices) return 0;

    int *visitado = (int *)calloc(g->quantidade_vertices, sizeof(int));
    int *disc = (int *)calloc(g->quantidade_vertices, sizeof(int));
    int *low = (int *)calloc(g->quantidade_vertices, sizeof(int));
    int *pai = (int *)malloc(g->quantidade_vertices * sizeof(int));
    if ((!visitado && g->quantidade_vertices) || (!disc && g->quantidade_vertices) || (!low && g->quantidade_vertices) || (!pai && g->quantidade_vertices)) {
        free(*eh_articulacao); free(visitado); free(disc); free(low); free(pai); *eh_articulacao = NULL; return 0;
    }
    for (size_t i = 0; i < g->quantidade_vertices; ++i) pai[i] = -1;
    int tempo = 0;
    for (size_t i = 0; i < g->quantidade_vertices; ++i) if (!visitado[i]) tarjan_ap(g, i, visitado, disc, low, pai, *eh_articulacao, &tempo);
    free(visitado); free(disc); free(low); free(pai);
    return 1;
}

static void tarjan_bridge(const Grafo *g, size_t u, int *visitado, int *disc, int *low, int *pai, int *eh_ponte, int *tempo) {
    visitado[u] = 1;
    disc[u] = low[u] = ++(*tempo);
    for (Adjacente *a = g->vertices[u].lista; a; a = a->proximo) {
        size_t v = a->destino;
        if (!visitado[v]) {
            pai[v] = (int)u;
            tarjan_bridge(g, v, visitado, disc, low, pai, eh_ponte, tempo);
            if (low[v] < low[u]) low[u] = low[v];
            if (low[v] > disc[u]) {
                eh_ponte[u * g->quantidade_vertices + v] = 1;
                eh_ponte[v * g->quantidade_vertices + u] = 1;
            }
        } else if ((int)v != pai[u] && disc[v] < low[u]) {
            low[u] = disc[v];
        }
    }
}

int grafo_pontes(const Grafo *g, int **eh_ponte) {
    if (!g || !eh_ponte || g->direcionado) return 0;
    size_t n = g->quantidade_vertices;
    *eh_ponte = (int *)calloc(n * n, sizeof(int));
    if (!*eh_ponte && n) return 0;
    int *visitado = (int *)calloc(n, sizeof(int));
    int *disc = (int *)calloc(n, sizeof(int));
    int *low = (int *)calloc(n, sizeof(int));
    int *pai = (int *)malloc(n * sizeof(int));
    if ((!visitado && n) || (!disc && n) || (!low && n) || (!pai && n)) {
        free(*eh_ponte); free(visitado); free(disc); free(low); free(pai); *eh_ponte = NULL; return 0;
    }
    for (size_t i = 0; i < n; ++i) pai[i] = -1;
    int tempo = 0;
    for (size_t i = 0; i < n; ++i) if (!visitado[i]) tarjan_bridge(g, i, visitado, disc, low, pai, *eh_ponte, &tempo);
    free(visitado); free(disc); free(low); free(pai);
    return 1;
}

int grafo_verifica_formula_euler(const Grafo *g, long long *faces) {
    if (!g || !faces || g->direcionado || g->quantidade_vertices == 0) return 0;
    if (!grafo_conexo_nao_direcionado(g)) return 0;

    long long v = (long long)g->quantidade_vertices;
    long long e = (long long)g->quantidade_arestas;
    *faces = e - v + 2;

    /* Para grafos simples planares conexos com V >= 3, E <= 3V-6 e F >= 1
       sao condicoes necessarias. Elas nao constituem um algoritmo completo
       de reconhecimento de planaridade. */
    if (v >= 3 && e > 3 * v - 6) return 0;
    return *faces >= 1;
}

int grafo_coloracao_gulosa(const Grafo *g, int **cores, int *quantidade_cores) {
    if (!g || !cores || !quantidade_cores) return 0;

    *cores = NULL;
    *quantidade_cores = 0;

    size_t n = g->quantidade_vertices;

    /* Rejeita grafos com laco. */
    for (size_t i = 0; i < n; ++i) {
        if (g->matriz[i * n + i]) return 0;
    }

    *cores = (int *)malloc(n * sizeof(int));
    if (!*cores && n) return 0;
    for (size_t i = 0; i < n; ++i) (*cores)[i] = -1;

    int maior = -1;
    for (size_t u = 0; u < n; ++u) {
        int *usada = (int *)calloc(n + 1, sizeof(int));
        if (!usada) { free(*cores); *cores = NULL; return 0; }
        for (size_t v = 0; v < n; ++v) {
            int adjacente = g->matriz[u * n + v] || g->matriz[v * n + u];
            if (adjacente && (*cores)[v] >= 0) usada[(*cores)[v]] = 1;
        }
        int cor = 0;
        while (cor <= (int)n && usada[cor]) ++cor;
        (*cores)[u] = cor;
        if (cor > maior) maior = cor;
        free(usada);
    }
    *quantidade_cores = maior + 1;
    return 1;
}

int grafo_carregar_dataset(const char *caminho, Grafo *g) {
    if (!caminho || !g) return 0;
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) return 0;

    char linha[256];
    int sucesso = 1;
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r') continue;
        char tipo[16], a[ROTULO_MAX + 1], b[ROTULO_MAX + 1];
        int duracao = 0;
        int lidos = sscanf(linha, "%15s %63s %63s %d", tipo, a, b, &duracao);
        if (lidos < 2) continue;

        if (strcmp(tipo, "TAREFA") == 0 && lidos >= 3) {
            if (!grafo_definir_duracao(g, a, atoi(b))) sucesso = 0;
        } else if (strcmp(tipo, "ARESTA") == 0 && lidos >= 3) {
            if (!grafo_adicionar_aresta(g, a, b)) sucesso = 0;
        } else {
            sucesso = 0;
        }
    }
    fclose(arquivo);
    return sucesso;
}

void grafo_imprimir_ordem(const Grafo *g, const size_t *ordem, size_t tamanho) {
    if (!g || !ordem) return;
    for (size_t i = 0; i < tamanho; ++i) {
        if (i) printf(" -> ");
        printf("%s", g->vertices[ordem[i]].rotulo);
    }
    printf("\n");
}

void grafo_imprimir_cores(const Grafo *g, const int *cores) {
    if (!g || !cores) return;
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        printf("  %s -> cor %d\n", g->vertices[i].rotulo, cores[i] + 1);
    }
}