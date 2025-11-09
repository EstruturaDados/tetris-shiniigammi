#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// ---------------------------- DEFINIÇÕES ----------------------------

#define TAM_FILA 5   // Tamanho máximo da fila de peças
#define TAM_PILHA 3  // Tamanho máximo da pilha de reserva

// ---------------------------- STRUCTS ----------------------------

// Estrutura que representa uma peça do Tetris
typedef struct {
    char nome; // Tipo da peça: I, O, T, L, J, S, Z
    int id;    // Identificador único da peça
} Peca;

// Estrutura que representa uma fila circular de peças
typedef struct {
    Peca fila[TAM_FILA]; // Vetor de peças
    int inicio;          // Índice do primeiro elemento da fila
    int fim;             // Índice do próximo espaço livre
    int tamanho;         // Quantidade de peças na fila
    int contadorID;      // Contador para gerar IDs únicos
} Fila;

// Estrutura que representa uma pilha de peças
typedef struct {
    Peca pilha[TAM_PILHA]; // Vetor de peças da pilha
    int topo;              // Índice do topo da pilha (-1 se vazia)
} Pilha;

// Estrutura que guarda o estado da fila e pilha para desfazer jogadas
typedef struct {
    Peca fila[TAM_FILA];
    int inicio;
    int fim;
    int tamanho;
    Pilha pilha;
} Estado;

// Vetor com os tipos de peças disponíveis
char tiposPecas[] = {'I', 'O', 'T', 'L', 'J', 'S', 'Z'};

// ---------------------------- FUNÇÕES ----------------------------

/**
 * Gera uma nova peça aleatória e atribui um ID único.
 */
Peca gerarPeca(int* contadorID) {
    Peca nova;
    nova.nome = tiposPecas[rand() % 7]; // Escolhe aleatoriamente entre I, O, T, L, J, S, Z
    nova.id = (*contadorID)++;          // Incrementa o ID da peça
    return nova;
}

/**
 * Inicializa a fila com peças aleatórias.
 * 
 * Observação sobre fila circular:
 * - 'inicio' aponta para a primeira peça a ser jogada.
 * - 'fim' aponta para a posição livre onde a próxima peça será inserida.
 * - Ao avançar 'inicio' ou 'fim', usamos módulo TAM_FILA para "circular" o vetor.
 */
void inicializarFila(Fila* f) {
    f->inicio = 0;
    f->fim = 0;
    f->tamanho = 0;
    f->contadorID = 1;

    for(int i=0; i<TAM_FILA; i++){
        f->fila[i] = gerarPeca(&(f->contadorID));
        f->fim = (f->fim + 1) % TAM_FILA; // Avança circularmente
        f->tamanho++;
    }
}

/**
 * Inicializa a pilha vazia.
 * - topo = -1 indica que a pilha está vazia.
 */
void inicializarPilha(Pilha* p){
    p->topo = -1;
}

/**
 * Mostra as peças da fila na tela.
 */
void mostrarFila(Fila* f){
    printf("Fila: ");
    int i = f->inicio;
    for(int c=0; c<f->tamanho; c++){
        printf("[%c%d] ", f->fila[i].nome, f->fila[i].id);
        i = (i + 1) % TAM_FILA; // Move circularmente pelo vetor
    }
    printf("\n");
}

/**
 * Retira a peça da frente da fila (jogada) e insere uma nova peça no fim.
 * Observação:
 * - A fila circular permite que 'inicio' e 'fim' avancem sem deslocar elementos.
 * - Sempre que jogamos uma peça, o 'inicio' avança, e uma nova peça é inserida no 'fim'.
 */
Peca jogarPeca(Fila* f){
    if(f->tamanho==0){
        printf("Fila vazia!\n");
        return (Peca){'-',0};
    }
    Peca p = f->fila[f->inicio];          // Pega a peça da frente
    f->inicio = (f->inicio+1)%TAM_FILA;   // Avança início circularmente
    f->tamanho--;                         // Remove da contagem da fila

    // Inserir nova peça automaticamente no fim
    f->fila[f->fim] = gerarPeca(&(f->contadorID));
    f->fim = (f->fim+1)%TAM_FILA;         // Avança fim circularmente
    f->tamanho++;

    return p;
}

/**
 * Mostra as peças da pilha na tela.
 * - A pilha funciona no modelo LIFO (Last In First Out).
 * - topo = índice do último elemento inserido.
 */
void mostrarPilha(Pilha* p){
    printf("Pilha: ");
    if(p->topo==-1) printf("(vazia)");
    for(int i=0;i<=p->topo;i++){
        printf("[%c%d] ", p->pilha[i].nome, p->pilha[i].id);
    }
    printf("\n");
}

/**
 * Adiciona uma peça no topo da pilha (reserva).
 */
int push(Pilha* p, Peca pc){
    if(p->topo >= TAM_PILHA-1){
        printf("Pilha cheia!\n");
        return 0;
    }
    p->topo++;
    p->pilha[p->topo] = pc;
    return 1;
}

/**
 * Remove e retorna a peça do topo da pilha (usar peça reservada).
 */
Peca pop(Pilha* p){
    if(p->topo==-1){
        printf("Pilha vazia!\n");
        return (Peca){'-',0};
    }
    return p->pilha[p->topo--];
}

/**
 * Troca a peça da frente da fila com o topo da pilha.
 * - Permite trocar uma peça que será jogada com a reservada.
 * - A fila circular mantém 'inicio' intacto.
 * - A pilha mantém seu topo, trocando somente os elementos.
 */
void trocarFilaPilha(Fila* f, Pilha* p){
    if(p->topo==-1){
        printf("Pilha vazia! Nada para trocar.\n");
        return;
    }
    Peca temp = f->fila[f->inicio];
    f->fila[f->inicio] = p->pilha[p->topo];
    p->pilha[p->topo] = temp;
    printf("Troca realizada: [%c%d] <-> [%c%d]\n",
        f->fila[f->inicio].nome,f->fila[f->inicio].id,
        p->pilha[p->topo].nome,p->pilha[p->topo].id);
}

/**
 * Inverte os primeiros elementos da fila com os da pilha (3 elementos).
 * - O topo da pilha vira frente da fila.
 * - A frente da fila vai para o topo da pilha.
 * - Demonstra interação fila (FIFO) <-> pilha (LIFO).
 */
void inverterFilaPilha(Fila* f, Pilha* p) {
    int qtd = 3; // número de elementos a inverter
    if (f->tamanho < qtd) qtd = f->tamanho;
    if (p->topo + 1 < qtd) qtd = p->topo + 1;

    // Guardar os elementos da fila que irão para a pilha
    Peca tempFila[3];
    for (int i = 0; i < qtd; i++) {
        int idx = (f->inicio + i) % TAM_FILA;
        tempFila[i] = f->fila[idx];
    }

    // Colocar elementos da pilha na frente da fila (LIFO -> frente da fila)
    for (int i = 0; i < qtd; i++) {
        int idx = (f->inicio + i) % TAM_FILA;
        f->fila[idx] = p->pilha[p->topo - i]; // topo da pilha vira frente da fila
    }

    // Colocar elementos antigos da fila no topo da pilha (FIFO -> topo da pilha)
    p->topo = qtd - 1;
    for (int i = 0; i < qtd; i++) {
        p->pilha[i] = tempFila[i];
    }

    printf("Invertidos os primeiros %d elementos da fila com a pilha (fila e pilha trocadas)\n", qtd);
}

/**
 * Salva o estado atual de fila e pilha para permitir desfazer jogada.
 */
void salvarEstado(Estado* est, Fila* f, Pilha* p){
    memcpy(est->fila, f->fila, sizeof(f->fila));
    est->inicio = f->inicio;
    est->fim = f->fim;
    est->tamanho = f->tamanho;
    est->pilha = *p;
}

/**
 * Restaura o último estado salvo de fila e pilha.
 */
void restaurarEstado(Estado* est, Fila* f, Pilha* p){
    memcpy(f->fila, est->fila, sizeof(f->fila));
    f->inicio = est->inicio;
    f->fim = est->fim;
    f->tamanho = est->tamanho;
    *p = est->pilha;
    printf("Última jogada desfeita!\n");
}

// ---------------------------- MAIN ----------------------------

int main(){
    srand(time(NULL)); // Inicializa gerador de números aleatórios

    Fila fila;
    Pilha pilha;
    Estado ultimo;
    inicializarFila(&fila);
    inicializarPilha(&pilha);

    int opcao;
    do{
        printf("\n=== TETRIS STACK - NÍVEL MESTRE ===\n");
        mostrarFila(&fila);
        mostrarPilha(&pilha);
        printf("\n1 - Jogar peça\n2 - Reservar peça\n3 - Usar peça reservada\n");
        printf("4 - Trocar peça do topo da pilha com frente da fila\n");
        printf("5 - Desfazer última jogada\n");
        printf("6 - Inverter fila e pilha\n0 - Sair\nEscolha: ");
        scanf("%d",&opcao);
        getchar();

        switch(opcao){
            case 1:
                salvarEstado(&ultimo,&fila,&pilha);
                {
                    Peca p = jogarPeca(&fila);
                    printf("Você jogou [%c%d]\n",p.nome,p.id);
                }
                break;
            case 2:
                salvarEstado(&ultimo,&fila,&pilha);
                {
                    Peca p = fila.fila[fila.inicio];
                    if(push(&pilha,p)){
                        printf("Você reservou [%c%d]\n",p.nome,p.id);
                        jogarPeca(&fila);
                    }
                }
                break;
            case 3:
                salvarEstado(&ultimo,&fila,&pilha);
                {
                    Peca p = pop(&pilha);
                    if(p.id!=0) printf("Você usou [%c%d]\n",p.nome,p.id);
                }
                break;
            case 4:
                salvarEstado(&ultimo,&fila,&pilha);
                trocarFilaPilha(&fila,&pilha);
                break;
            case 5:
                restaurarEstado(&ultimo,&fila,&pilha);
                break;
            case 6:
                salvarEstado(&ultimo,&fila,&pilha);
                inverterFilaPilha(&fila,&pilha);
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }

    }while(opcao!=0);

    return 0;
}