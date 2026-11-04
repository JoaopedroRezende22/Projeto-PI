#include "grafo.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void testar_dag(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));
    assert(grafo_adicionar_aresta(&g, "A", "B"));
    assert(grafo_adicionar_aresta(&g, "A", "C"));
    assert(grafo_adicionar_aresta(&g, "B", "D"));
    assert(grafo_adicionar_aresta(&g, "C", "D"));

    size_t *ordem = NULL, tamanho = 0;
    assert(grafo_bfs(&g, "A", &ordem, &tamanho));
    assert(tamanho == 4);
    free(ordem);

    ordem = NULL; tamanho = 0;
    assert(grafo_dfs(&g, "A", &ordem, &tamanho));
    assert(tamanho == 4);
    free(ordem);

    ordem = NULL; tamanho = 0;
    assert(grafo_ordenacao_topologica_kahn(&g, &ordem, &tamanho));
    assert(tamanho == 4);
    free(ordem);

    ordem = NULL; tamanho = 0;
    assert(grafo_ordenacao_topologica_dfs(&g, &ordem, &tamanho));
    assert(tamanho == 4);
    free(ordem);

    assert(!grafo_possui_ciclo(&g));
    assert(!digrafo_fortemente_conexo(&g));
    grafo_liberar(&g);
}

static void testar_ciclo(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));
    assert(grafo_adicionar_aresta(&g, "A", "B"));
    assert(grafo_adicionar_aresta(&g, "B", "C"));
    assert(grafo_adicionar_aresta(&g, "C", "A"));
    assert(grafo_possui_ciclo(&g));

    size_t *ordem = NULL, tamanho = 0;
    assert(!grafo_ordenacao_topologica_kahn(&g, &ordem, &tamanho));
    assert(ordem == NULL);
    assert(!grafo_ordenacao_topologica_dfs(&g, &ordem, &tamanho));
    grafo_liberar(&g);
}

static void testar_nao_direcionado(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 0));
    assert(grafo_adicionar_aresta(&g, "A", "B"));
    assert(grafo_adicionar_aresta(&g, "B", "C"));
    assert(grafo_adicionar_aresta(&g, "B", "D"));
    assert(grafo_conexo_nao_direcionado(&g));

    int *articulacoes = NULL;
    assert(grafo_vertices_articulacao(&g, &articulacoes));
    int b = grafo_indice_vertice(&g, "B");
    assert(b >= 0 && articulacoes[b] == 1);
    free(articulacoes);

    int *pontes = NULL;
    assert(grafo_pontes(&g, &pontes));
    int a = grafo_indice_vertice(&g, "A");
    int c = grafo_indice_vertice(&g, "C");
    int d = grafo_indice_vertice(&g, "D");
    assert(pontes[(size_t)a * g.quantidade_vertices + (size_t)b]);
    assert(pontes[(size_t)b * g.quantidade_vertices + (size_t)c]);
    assert(pontes[(size_t)b * g.quantidade_vertices + (size_t)d]);
    free(pontes);

    long long faces = 0;
    assert(grafo_verifica_formula_euler(&g, &faces));
    assert(faces == 1);

    int *cores = NULL, qtd_cores = 0;
    assert(grafo_coloracao_gulosa(&g, &cores, &qtd_cores));
    assert(qtd_cores >= 2);
    assert(cores[a] != cores[b]);
    assert(cores[b] != cores[c]);
    assert(cores[b] != cores[d]);
    free(cores);

    grafo_liberar(&g);
}

static void testar_conectividade_forte(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));
    assert(grafo_adicionar_aresta(&g, "A", "B"));
    assert(grafo_adicionar_aresta(&g, "B", "C"));
    assert(grafo_adicionar_aresta(&g, "C", "A"));
    assert(digrafo_fortemente_conexo(&g));
    grafo_liberar(&g);
}


static void testar_multiplos_caminhos_criticos(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));

    /* Criacao das atividades e suas duracoes */
    const char *atividades[] = {"A", "B", "C", "D", "E"};
    const int duracoes[] = {2, 3, 3, 2, 1};

    for (size_t i = 0; i < 5; ++i) {
        assert(grafo_adicionar_vertice(&g, atividades[i]));
        assert(grafo_definir_duracao(&g, atividades[i], duracoes[i]));
    }

    /* Criacao das dependencias */
    assert(grafo_adicionar_aresta(&g, "A", "B"));
    assert(grafo_adicionar_aresta(&g, "A", "C"));
    assert(grafo_adicionar_aresta(&g, "B", "D"));
    assert(grafo_adicionar_aresta(&g, "C", "D"));
    assert(grafo_adicionar_aresta(&g, "D", "E"));

    size_t n = g.quantidade_vertices;

    int *inicio_cedo = malloc(n * sizeof(int));
    int *termino_cedo = malloc(n * sizeof(int));
    int *inicio_tarde = malloc(n * sizeof(int));
    int *termino_tarde = malloc(n * sizeof(int));
    int *folgas = NULL;
    int *eh_critica = NULL;

    assert(inicio_cedo && termino_cedo &&
           inicio_tarde && termino_tarde);

    int duracao = 0;

    assert(grafo_tempos_mais_cedo(
        &g, inicio_cedo, termino_cedo, &duracao));

    assert(duracao == 8);

    assert(grafo_tempos_mais_tarde(
        &g, inicio_tarde, termino_tarde, duracao));

    assert(grafo_calcular_folgas(
        &g, inicio_cedo, inicio_tarde, &folgas));

    assert(grafo_identificar_caminho_critico(
        &g, folgas, &eh_critica));

    /* Verifica se todas as cinco atividades sao criticas */
    for (size_t i = 0; i < n; ++i) {
        assert(eh_critica[i] == 1);
    }

    printf("\nTeste de multiplos caminhos criticos:\n");

    assert(grafo_imprimir_caminhos_criticos(
        &g,
        inicio_cedo,
        termino_cedo,
        eh_critica,
        duracao));

    free(inicio_cedo);
    free(termino_cedo);
    free(inicio_tarde);
    free(termino_tarde);
    free(folgas);
    free(eh_critica);

    grafo_liberar(&g);
}

static void testar_atividades_independentes(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));

    const char *atividades[] = {"A", "B", "C"};
    const int duracoes[] = {2, 5, 3};

    for (size_t i = 0; i < 3; ++i) {
        assert(grafo_adicionar_vertice(&g, atividades[i]));
        assert(grafo_definir_duracao(&g, atividades[i], duracoes[i]));
    }

    size_t n = g.quantidade_vertices;

    int *inicio_cedo = malloc(n * sizeof(int));
    int *termino_cedo = malloc(n * sizeof(int));
    int *inicio_tarde = malloc(n * sizeof(int));
    int *termino_tarde = malloc(n * sizeof(int));
    int *folgas = NULL;
    int *eh_critica = NULL;

    assert(inicio_cedo && termino_cedo &&
           inicio_tarde && termino_tarde);

    int duracao_projeto = 0;

    assert(grafo_tempos_mais_cedo(
        &g, inicio_cedo, termino_cedo, &duracao_projeto));

    assert(duracao_projeto == 5);

    assert(grafo_tempos_mais_tarde(
        &g, inicio_tarde, termino_tarde, duracao_projeto));

    assert(grafo_calcular_folgas(
        &g, inicio_cedo, inicio_tarde, &folgas));

    assert(grafo_identificar_caminho_critico(
        &g, folgas, &eh_critica));

    int a = grafo_indice_vertice(&g, "A");
    int b = grafo_indice_vertice(&g, "B");
    int c = grafo_indice_vertice(&g, "C");

    assert(a >= 0 && b >= 0 && c >= 0);

    assert(folgas[a] == 3);
    assert(folgas[b] == 0);
    assert(folgas[c] == 2);

    assert(eh_critica[a] == 0);
    assert(eh_critica[b] == 1);
    assert(eh_critica[c] == 0);

    printf("\nTeste de atividades independentes:\n");
    printf("Duracao do projeto: %d dias\n", duracao_projeto);

    assert(grafo_imprimir_caminhos_criticos(
        &g, inicio_cedo, termino_cedo,
        eh_critica, duracao_projeto));

    free(inicio_cedo);
    free(termino_cedo);
    free(inicio_tarde);
    free(termino_tarde);
    free(folgas);
    free(eh_critica);

    grafo_liberar(&g);
}


static void testar_atividades_duracao_zero(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));

    const char *atividades[] = {"A", "B", "C"};
    const int duracoes[] = {2, 0, 3};

    for (size_t i = 0; i < 3; ++i) {
        assert(grafo_adicionar_vertice(&g, atividades[i]));
        assert(grafo_definir_duracao(&g, atividades[i], duracoes[i]));
    }

    assert(grafo_adicionar_aresta(&g, "A", "B"));
    assert(grafo_adicionar_aresta(&g, "B", "C"));

    size_t n = g.quantidade_vertices;

    int *inicio_cedo = malloc(n * sizeof(int));
    int *termino_cedo = malloc(n * sizeof(int));
    int *inicio_tarde = malloc(n * sizeof(int));
    int *termino_tarde = malloc(n * sizeof(int));
    int *folgas = NULL;
    int *eh_critica = NULL;

    assert(inicio_cedo && termino_cedo &&
           inicio_tarde && termino_tarde);

    int duracao_projeto = 0;

    assert(grafo_tempos_mais_cedo(
        &g, inicio_cedo, termino_cedo, &duracao_projeto));

    assert(duracao_projeto == 5);

    assert(grafo_tempos_mais_tarde(
        &g, inicio_tarde, termino_tarde, duracao_projeto));

    assert(grafo_calcular_folgas(
        &g, inicio_cedo, inicio_tarde, &folgas));

    assert(grafo_identificar_caminho_critico(
        &g, folgas, &eh_critica));

    for (size_t i = 0; i < n; ++i) {
        assert(folgas[i] == 0);
        assert(eh_critica[i] == 1);
    }

    int a = grafo_indice_vertice(&g, "A");
    int b = grafo_indice_vertice(&g, "B");
    int c = grafo_indice_vertice(&g, "C");

    assert(a >= 0 && b >= 0 && c >= 0);

    assert(inicio_cedo[a] == 0 && termino_cedo[a] == 2);
    assert(inicio_cedo[b] == 2 && termino_cedo[b] == 2);
    assert(inicio_cedo[c] == 2 && termino_cedo[c] == 5);

    printf("\nTeste de atividades com duracao zero:\n");
    printf("Duracao do projeto: %d dias\n", duracao_projeto);

    assert(grafo_imprimir_caminhos_criticos(
        &g, inicio_cedo, termino_cedo,
        eh_critica, duracao_projeto));

    free(inicio_cedo);
    free(termino_cedo);
    free(inicio_tarde);
    free(termino_tarde);
    free(folgas);
    free(eh_critica);

    grafo_liberar(&g);
}

static void testar_duracao_negativa(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));

    assert(grafo_adicionar_vertice(&g, "A"));

    /* Duracao negativa deve ser rejeitada. */
    assert(!grafo_definir_duracao(&g, "A", -5));

    /* A duracao original deve permanecer inalterada. */
    int a = grafo_indice_vertice(&g, "A");
    assert(a >= 0);
    assert(g.vertices[a].duracao == 0);

    /* Valores validos devem continuar sendo aceitos. */
    assert(grafo_definir_duracao(&g, "A", 5));
    assert(g.vertices[a].duracao == 5);

    printf("\nTeste de duracao negativa aprovado.\n");

    grafo_liberar(&g);
}

static void testar_grafo_sem_atividades(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 0));

    int inicio_mais_cedo[1] = {0};
    int termino_mais_cedo[1] = {0};
    int duracao_projeto = 0;

    /* O calculo deve rejeitar um grafo sem atividades. */
    assert(!grafo_tempos_mais_cedo(
        &g,
        inicio_mais_cedo,
        termino_mais_cedo,
        &duracao_projeto
    ));

    printf("\nTeste de grafo sem atividades aprovado.\n");

    grafo_liberar(&g);
}

static void testar_parametros_nulos(void) {
    Grafo g;
    assert(grafo_inicializar(&g, 1));
    assert(grafo_adicionar_vertice(&g, "A"));

    int inicio[1] = {0};
    int termino[1] = {0};
    int duracao = 0;

    assert(!grafo_tempos_mais_cedo(NULL, inicio, termino, &duracao));
    assert(!grafo_tempos_mais_cedo(&g, NULL, termino, &duracao));
    assert(!grafo_tempos_mais_cedo(&g, inicio, NULL, &duracao));
    assert(!grafo_tempos_mais_cedo(&g, inicio, termino, NULL));

    printf("\nTeste de parametros nulos aprovado.\n");

    grafo_liberar(&g);
}

int main(void) {
    testar_dag();
    testar_ciclo();
    testar_nao_direcionado();
    testar_conectividade_forte();
    testar_multiplos_caminhos_criticos();
    testar_atividades_independentes();
    testar_atividades_duracao_zero();
    testar_duracao_negativa();
    testar_grafo_sem_atividades();
    testar_parametros_nulos();
    printf("Todos os testes passaram.\n");
    return 0;
}
