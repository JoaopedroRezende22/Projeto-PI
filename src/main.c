#include "grafo.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double milissegundos(clock_t inicio, clock_t fim) {
    return 1000.0 * (double)(fim - inicio) / (double)CLOCKS_PER_SEC;
}

static void liberar_ordem(size_t *ordem) {
    free(ordem);
}

static void imprimir_vertices_marcados(const Grafo *g, const int *marcados, const char *titulo) {
    printf("\n%s\n", titulo);
    int encontrou = 0;
    for (size_t i = 0; i < g->quantidade_vertices; ++i) {
        if (marcados[i]) {
            printf("  %s\n", g->vertices[i].rotulo);
            encontrou = 1;
        }
    }
    if (!encontrou) printf("  nenhum\n");
}

static void imprimir_pontes(const Grafo *g, const int *pontes) {
    printf("\nPontes:\n");
    int encontrou = 0;
    for (size_t u = 0; u < g->quantidade_vertices; ++u) {
        for (size_t v = u + 1; v < g->quantidade_vertices; ++v) {
            if (pontes[u * g->quantidade_vertices + v]) {
                printf("  %s -- %s\n", g->vertices[u].rotulo, g->vertices[v].rotulo);
                encontrou = 1;
            }
        }
    }
    if (!encontrou) printf("  nenhuma\n");
}

int main(void) {
    Grafo projeto;
    if (!grafo_inicializar(&projeto, 1)) {
        fprintf(stderr, "Erro ao inicializar o grafo.\n");
        return 1;
    }

    if (!grafo_carregar_dataset("data/projeto_exemplo.txt", &projeto)) {
        fprintf(stderr, "Erro ao carregar data/projeto_exemplo.txt\n");
        grafo_liberar(&projeto);
        return 1;
    }

    printf("==============================================\n");
    printf(" FASE I - GESTAO DE PROJETOS INDUSTRIAIS\n");
    printf("==============================================\n");
    printf("Vertices: %zu\n", projeto.quantidade_vertices);
    printf("Arestas:  %zu\n", projeto.quantidade_arestas);
    printf("Direcionado: sim (DAG)\n");

    grafo_imprimir_lista(&projeto);
    grafo_imprimir_matriz(&projeto);

    size_t *ordem = NULL, tamanho = 0;
    clock_t inicio = clock();
    if (grafo_bfs(&projeto, "A", &ordem, &tamanho)) {
        clock_t fim = clock();
        printf("\nBFS a partir de A: ");
        grafo_imprimir_ordem(&projeto, ordem, tamanho);
        printf("Tempo BFS: %.3f ms\n", milissegundos(inicio, fim));
    }
    liberar_ordem(ordem);

    ordem = NULL; tamanho = 0;
    inicio = clock();
    if (grafo_dfs(&projeto, "A", &ordem, &tamanho)) {
        clock_t fim = clock();
        printf("DFS a partir de A: ");
        grafo_imprimir_ordem(&projeto, ordem, tamanho);
        printf("Tempo DFS: %.3f ms\n", milissegundos(inicio, fim));
    }
    liberar_ordem(ordem);

    ordem = NULL; tamanho = 0;
    inicio = clock();
    int topo_kahn = grafo_ordenacao_topologica_kahn(&projeto, &ordem, &tamanho);
    clock_t fim = clock();
    printf("\nOrdenacao topologica - Kahn: ");
    if (topo_kahn) grafo_imprimir_ordem(&projeto, ordem, tamanho);
    else printf("nao existe (o grafo possui ciclo ou houve erro)\n");
    printf("Tempo Kahn: %.3f ms\n", milissegundos(inicio, fim));
    liberar_ordem(ordem);

    ordem = NULL; tamanho = 0;
    inicio = clock();
    int topo_dfs = grafo_ordenacao_topologica_dfs(&projeto, &ordem, &tamanho);
    fim = clock();
    printf("Ordenacao topologica - DFS: ");
    if (topo_dfs) grafo_imprimir_ordem(&projeto, ordem, tamanho);
    else printf("nao existe (o grafo possui ciclo ou houve erro)\n");
    printf("Tempo Topo-DFS: %.3f ms\n", milissegundos(inicio, fim));
    liberar_ordem(ordem);

    printf("Possui ciclo? %s\n", grafo_possui_ciclo(&projeto) ? "sim" : "nao");
    printf("O DAG e fortemente conexo? %s\n", digrafo_fortemente_conexo(&projeto) ? "sim" : "nao");

    printf("\nDuracoes das tarefas (dias):\n");
    for (size_t i = 0; i < projeto.quantidade_vertices; ++i) {
        printf("  %s: %d\n", projeto.vertices[i].rotulo, projeto.vertices[i].duracao);
    }

    /* Fase II - Calculo dos tempos no PERT/CPM */
    size_t n = projeto.quantidade_vertices;

    int *inicio_mais_cedo = malloc(n * sizeof(int));
    int *termino_mais_cedo = malloc(n * sizeof(int));
    int *inicio_mais_tarde = malloc(n * sizeof(int));
    int *termino_mais_tarde = malloc(n * sizeof(int));
    int duracao_projeto = 0;

    printf("\n--- FASE II: PERT/CPM ---\n");

    if (!inicio_mais_cedo || !termino_mais_cedo ||
        !inicio_mais_tarde || !termino_mais_tarde) {
        printf("Erro ao alocar memoria para o PERT/CPM.\n");
    } else if (grafo_tempos_mais_cedo(
                   &projeto,
                   inicio_mais_cedo,
                   termino_mais_cedo,
                   &duracao_projeto)) {

        printf("\nTempos mais cedo das tarefas (dias):\n");

        for (size_t i = 0; i < n; ++i) {
            printf("  Tarefa %s: inicio = %d, termino = %d\n",
                   projeto.vertices[i].rotulo,
                   inicio_mais_cedo[i],
                   termino_mais_cedo[i]);
        }

        printf("\nDuracao minima do projeto: %d dias\n",
               duracao_projeto);

        if (grafo_tempos_mais_tarde(
                &projeto,
                inicio_mais_tarde,
                termino_mais_tarde,
                duracao_projeto)) {

            printf("\nTempos mais tarde das tarefas (dias):\n");

            for (size_t i = 0; i < n; ++i) {
                printf("  Tarefa %s: inicio = %d, termino = %d\n",
                       projeto.vertices[i].rotulo,
                       inicio_mais_tarde[i],
                       termino_mais_tarde[i]);
            }

            int *folgas = NULL;

            if (grafo_calcular_folgas(
                    &projeto,
                    inicio_mais_cedo,
                    inicio_mais_tarde,
                    &folgas)) {

                printf("\nFolgas das tarefas (dias):\n");

                for (size_t i = 0; i < n; ++i) {
                    printf("  Tarefa %s: folga = %d dia(s)\n",
                           projeto.vertices[i].rotulo,
                           folgas[i]);
                }
                int *eh_critica = NULL;

                if (grafo_identificar_caminho_critico(
                        &projeto,
                        folgas,
                        &eh_critica)) {

                    printf("\nTarefas do caminho critico:\n");

                    for (size_t i = 0; i < n; ++i) {
                        if (eh_critica[i]) {
                            printf("  Tarefa %s\n",
                                   projeto.vertices[i].rotulo);
                        }
                    }

                printf("\nSequencia dos caminhos criticos:\n");

                if (!grafo_imprimir_caminhos_criticos(
                        &projeto,
                        inicio_mais_cedo,
                        termino_mais_cedo,
                        eh_critica,
                        duracao_projeto)) {

                    printf("Nao foi possivel reconstruir os caminhos criticos.\n");
                }

                free(eh_critica);

                } else {
                    printf("Nao foi possivel identificar o caminho critico.\n");
                }

                free(folgas);

            } else {
                printf("Nao foi possivel calcular as folgas.\n");
            }
        } else {
            printf("Nao foi possivel calcular os tempos mais tarde.\n");
        }

    } else {
        printf("Nao foi possivel calcular os tempos mais cedo. "
               "Verifique se o grafo e direcionado, possui tarefas "
               "e nao contem ciclos.\n");
    }

    free(inicio_mais_cedo);
    free(termino_mais_cedo);
    free(inicio_mais_tarde);
    free(termino_mais_tarde);

    /* Para articulacao, pontes, Euler e coloracao, usamos o grafo subjacente
       nao direcionado das dependencias do projeto. */
    Grafo estrutura;
    if (!grafo_inicializar(&estrutura, 0)) {
        grafo_liberar(&projeto);
        return 1;
    }
    for (size_t i = 0; i < projeto.quantidade_vertices; ++i) {
        grafo_adicionar_vertice(&estrutura, projeto.vertices[i].rotulo);
        grafo_definir_duracao(&estrutura, projeto.vertices[i].rotulo, projeto.vertices[i].duracao);
    }
    for (size_t u = 0; u < projeto.quantidade_vertices; ++u) {
        for (Adjacente *a = projeto.vertices[u].lista; a; a = a->proximo) {
            grafo_adicionar_aresta(&estrutura, projeto.vertices[u].rotulo, projeto.vertices[a->destino].rotulo);
        }
    }

    printf("\n--- Analise do grafo subjacente nao direcionado ---\n");
    printf("Conexo? %s\n", grafo_conexo_nao_direcionado(&estrutura) ? "sim" : "nao");

    int *articulacoes = NULL;
    if (grafo_vertices_articulacao(&estrutura, &articulacoes)) {
        imprimir_vertices_marcados(&estrutura, articulacoes, "Vertices de articulacao:");
        free(articulacoes);
    }

    int *pontes = NULL;
    if (grafo_pontes(&estrutura, &pontes)) {
        imprimir_pontes(&estrutura, pontes);
        free(pontes);
    }

    long long faces = 0;
    int euler_ok = grafo_verifica_formula_euler(&estrutura, &faces);
    printf("\nVerificacao baseada em Euler: %s\n", euler_ok ? "condicoes satisfeitas" : "condicoes nao satisfeitas");
    if (euler_ok) {
        printf("  V = %zu, E = %zu, F = %lld\n", estrutura.quantidade_vertices, estrutura.quantidade_arestas, faces);
        printf("  V - E + F = %lld\n", (long long)estrutura.quantidade_vertices - (long long)estrutura.quantidade_arestas + faces);
    }
    printf("  Observacao: isso verifica condicoes necessarias; nao e um algoritmo completo de planaridade.\n");

    int *cores = NULL, quantidade_cores = 0;
    inicio = clock();
    if (grafo_coloracao_gulosa(&estrutura, &cores, &quantidade_cores)) {
        fim = clock();
        printf("\nColoracao gulosa (%d cores):\n", quantidade_cores);
        grafo_imprimir_cores(&estrutura, cores);
        printf("Tempo coloracao: %.3f ms\n", milissegundos(inicio, fim));
        free(cores);
    }

    printf("\nObservacao: a implementacao da Fase II esta em andamento.\n");

    grafo_liberar(&estrutura);
    grafo_liberar(&projeto);
    return 0;
}
