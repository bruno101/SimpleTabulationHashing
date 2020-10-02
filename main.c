#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "estruturas.h"

//esse eh o arquivo que iremos escrever
//o deixamos como variavel global
FILE *file_out;

//também deixaremos as tabelas aleatórias como variáveis globais
uint64_t tabelasAleatorias[8][0x100];

struct pos_tabela {
  uint64_t chave;
  //'0' se posição for vazia, '1' se for liberada, '2' se for ocupada
  unsigned int flag : 2;
};

struct hashtable {
  //consiste em um vetor de inteiros
  posTabela *tabela;

  //o número de elementos suportado pela tabela
  uint64_t tamanho;

  //precisaremos desse valor para implementarmos a operacao de "limpar"
  uint64_t numeroDeCodigosDeRemocao;
};

//hashtable_dinamica implementa table doubling/halving
//ela também contém um ponteiro para as tabelas aleatórias
struct hashtable_dinamica {
  uint64_t tamanho;
  uint64_t numeroElementos;

  HashTable *hashtable;
};




//Funções da tabela de dispersão normal

HashTable *criaHashTable (uint64_t tamanho) {
  //'tamanho' é sempre uma potência de 2, o que enforçamos nas funções de HashTableDinamica 

  HashTable *H = malloc(sizeof(HashTable));

  H->tabela = malloc(tamanho*sizeof(posTabela));
  for (uint64_t i = 0; i < tamanho; i++) {
    (H->tabela)[i].chave = 0;
    (H->tabela)[i].flag = 0;
  }

  H->tamanho = tamanho;

  return H;
}

//a nossa função de dispersão
uint64_t funcHashTable (uint64_t chave) {

  uint64_t valor = 0;

  for (uint64_t i = 0; i < 8; i++) {
    //a cada iteração fazemos um 'XOR' entre o valor anterior e o valor a que a tabela mapeia um determinado grupo de dígitos de 'chave'
    valor ^= (tabelasAleatorias[i][chave%(0x100)]);
    chave >>= 8;
  }
  
  return valor;
}

//retorna '1' em caso de sucesso e '0' caso contrário
int buscaHashTable (HashTable *H, uint64_t chave) {

  uint64_t funcChave = funcHashTable(chave);
  uint64_t posChave = funcChave%(H->tamanho);
  uint64_t posChaveInicial = posChave;

  while (posChave < H->tamanho) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %d\n", funcChave, -1);

      return 0;
    }
    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave) ) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %"PRIu64"\n", funcChave, posChave);

      return 1;
    }
    posChave += 1;
  }

  //se chegamos no fim da tabela e ainda nao encontramos a resposta voltamos para o inicio
  posChave = 0;
  
  while (posChave < posChaveInicial) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %d\n", funcChave, -1);

      return 0;
    }
    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave) ) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %"PRIu64"\n", funcChave, posChave);

      return 1;
    }
    posChave += 1;
  }

  //adicionando ao arquivo de texto
  fprintf(file_out, "%"PRIu64" %d\n", funcChave, -1);

  return 0;
}

//retorna '1' em caso de sucesso e '0' caso contrário
//'imprimir' indica se devemos escrever algo no arquivo de texto
int insereHashTable (HashTable *H, uint64_t chave, int imprimir) {

  uint64_t funcChave = funcHashTable(chave);
  uint64_t posChave = funcChave%(H->tamanho);
  uint64_t posChaveInicial = posChave;

  while (posChave < H->tamanho) {
    //flag diferente de '2' significa que a posicao é vazia ou liberada
    if ( H->tabela[posChave].flag != 2 ) {
      H->tabela[posChave].chave = chave;
      H->tabela[posChave].flag = 2;

      if (imprimir) {
        //adicionando ao arquivo de texto
        fprintf(file_out, "%"PRIu64" %"PRIu64"\n", funcChave, posChave);
      }

      return 1;
    }
    posChave +=1;
  }

  //chegamos ao final do vetor 'tabela' e ainda nao encontramos posicao vazia, então passamos a buscar do começo do vetor
  posChave = 0;

  while (posChave < posChaveInicial) {
    //flag diferente de '2' significa que a posicao é vazia ou liberada
    if ( H->tabela[posChave].flag != 2 ) {
      H->tabela[posChave].chave = chave;
      H->tabela[posChave].flag = 2;

      if (imprimir) {
        //adicionando ao arquivo de texto
        fprintf(file_out, "%"PRIu64" %"PRIu64"\n", funcChave, posChave);
      }

      return 1;
    }
    posChave += 1;
  }

  //essa linha em teoria nao deve ser executada
  if (imprimir) {
    //adicionando ao arquivo de texto
    fprintf(file_out, "%"PRIu64" %d\n", funcChave, -1);
  }

  return 0;
}

//limpamos os codigos de remocao simplesmente reconstruindo a tabela
void limpaCodigosRemocao (HashTable *H) {

  uint64_t* tabelaAuxiliar = malloc(sizeof(uint64_t)*H->tamanho); 

  uint64_t cont = 0;
  for (uint64_t i = 0; i < H->tamanho; i++) {

    if (H->tabela[i].flag == 2) {
      //Adicionando todas as chaves a tabela auxiliar
      tabelaAuxiliar[cont] = H->tabela[i].chave;
      cont++;
    }
    H->tabela[i].chave = 0;
    H->tabela[i].flag = 0;

  }

  for (uint64_t i = 0; i < cont; i++) {
    //Reinserimos todas as chaves
    insereHashTable(H, tabelaAuxiliar[i], 0);
  }

  H->numeroDeCodigosDeRemocao = 0;

  free(tabelaAuxiliar);

  //adicionando ao arquivo de texto
  fprintf(file_out, "\n\nLIMPAR\n");

}

//retorna '1' em caso de sucesso e '0' caso contrário
uint64_t removeHashTable (HashTable *H, uint64_t chave) {
  
  uint64_t funcChave = funcHashTable(chave);
  uint64_t posChave = funcChave%(H->tamanho);
  uint64_t posChaveInicial = posChave;

  while (posChave < H->tamanho) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %d\n", funcChave, -1);

      return 0;
    }
    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave)) {
      //liberamos o espaco
      H->tabela[posChave].flag = 1;
      H->tabela[posChave].chave = 0;

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %"PRIu64"\n", funcChave, posChave);

      H->numeroDeCodigosDeRemocao += 1;
      if (H->numeroDeCodigosDeRemocao >= H->tamanho/4) {
        limpaCodigosRemocao(H);
      }

      return 1;
    }
    posChave += 1;
  }

  //chegamos ao final do vetor 'tabela' e ainda nem encontramos a chave nem alcançamos uma posição vazia, entao passamos a buscar do começo do vetor
  posChave = 0;

  while (posChave < posChaveInicial) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %d\n", funcChave, -1);

      return 0;
    }
    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave)) {
      //liberamos o espaco
      H->tabela[posChave].flag = 1;
      H->tabela[posChave].chave = 0;

      //adicionando ao arquivo de texto
      fprintf(file_out, "%"PRIu64" %"PRIu64"\n", funcChave, posChave);

      H->numeroDeCodigosDeRemocao += 1;
      if (H->numeroDeCodigosDeRemocao >= H->tamanho/4) {
        limpaCodigosRemocao(H);
      }

      return 1;
    }
    posChave += 1;
  }

  return 0;
}

void deletaHashTable (HashTable *H) {
  
  free(H->tabela);
  free(H);
}



//Funções da HashTable hashtableDinamica

uint64_t geraNumeroAleatorio () {

  uint64_t randomInt = 0;

  int logRandMax = log2(RAND_MAX)+1;
  int cont = 64;
  while (cont >= logRandMax) {
    cont -= logRandMax;
    randomInt = (randomInt<<logRandMax) + rand();
  }
  randomInt = (randomInt<<cont) + rand()%((int) pow(2,cont));

  return randomInt;
}

//Preenche os valores da tabela aleatoria a partir do numero 'valoresPossiveis', que eh o número de valores em que cada chave pode ser mapeada (o tamanho da nossa tabela de dispersao)

void preencheTabelaAleatoria (uint64_t tabelaAleatoria[0x100]) {

  //a tabelaAleatoria tem 2^8 posicoes
  for (uint64_t i = 0; i < 0x100; i++) {
    tabelaAleatoria[i] = geraNumeroAleatorio();
  }

}

HashTableDinamica *criaHashTableDinamica () {
  HashTableDinamica *T = (HashTableDinamica *)malloc(sizeof(HashTableDinamica));

  T->tamanho = 4;
  T->numeroElementos = 0;
  T->hashtable = criaHashTable(4);

  for (uint64_t i = 0; i < 8; i++) {
    preencheTabelaAleatoria (tabelasAleatorias[i]);
  }

  //adicionando ao arquivo de texto
  fprintf(file_out, "TAM:4\n\n\n");

  return T;
}

//retorna '1' em caso de sucesso e '0' caso contrário
int buscaHashTableDinamica (HashTableDinamica *T, uint64_t chave) {

  return buscaHashTable (T->hashtable, chave);

}

//retorna '1' em caso de sucesso e '0' caso contrário
int long insereHashTableDinamica (HashTableDinamica *T, uint64_t chave) {

  int res = insereHashTable(T->hashtable, chave, 1);
  T->numeroElementos +=1;

  //O 'tamanho' sempre será pelo menos o dobro do número de elementos (epsilon = 1, o que significa que o tempo por operação deve ser O(1))
  if (T->tamanho == 2*T->numeroElementos) {

    HashTable *H = criaHashTable(2*(T->tamanho));
    for (uint64_t i = 0; i < (T->tamanho); i++) {
      if ( T->hashtable->tabela[i].flag == 2 ) {
        insereHashTable(H, T->hashtable->tabela[i].chave, 0);
      }
    }
    deletaHashTable(T->hashtable);
    T->hashtable = H;
    T->tamanho = 2*(T->tamanho);

    //adicionando ao arquivo de texto
    fprintf(file_out, "\n\nDOBRAR TAM:%"PRIu64"\n", T->tamanho);

  }

  return res;
}

//retorna '1' em caso de sucesso e '0' caso contrário
int removeHashTableDinamica (HashTableDinamica *T, uint64_t chave) {

  int res = removeHashTable(T->hashtable, chave);

  if (res) {
    T->numeroElementos -= 1;

    if (T->tamanho == 8*T->numeroElementos) {

      HashTable *H = criaHashTable((T->tamanho)/2);
      for (uint64_t i = 0; i < (T->tamanho); i++) {
        if ( T->hashtable->tabela[i].flag == 2 ) {
          insereHashTable(H, T->hashtable->tabela[i].chave, 0);
        }
      }

      deletaHashTable(T->hashtable);
      T->hashtable = H;
      T->tamanho = (T->tamanho)/2;
      
      //adicionando ao arquivo de texto
      fprintf(file_out, "\n\nMETADE TAM:%"PRIu64"\n", T->tamanho);

    }

  }

  return res;
}




int main(int argc, char **argv) {

  if ( (!argv[1]) || (!argv[2]) ) {
    printf("Por favor passe o caminho do arquivo a ser lido e o do arquivo a ser criado como parâmetros para o programa.\n");
    exit(1);
  }

  FILE *file_in = fopen(argv[1], "r");
  if ( !file_in ) {
    printf("Erro abrindo o arquivo. Cheque se o caminho do arquivo a ser lido passado para o programa está correto.");
    exit(1);
  }
  file_out = fopen(argv[2], "w");
  if ( !file_out ) {
    printf("Erro criando o arquivo. Cheque se o caminho do arquivo a ser criado passado para o programa está correto.");
    exit(1);
  }

  HashTableDinamica *T = criaHashTableDinamica();

  char line[256];
  uint64_t chave;
  while (fgets(line, sizeof(line), file_in)) {

    fprintf(file_out, "%s", line);
    if (line[strlen(line)-1] != '\n') {
      fprintf(file_out, "%s", "\n");
    } 

    chave = atoi(line+4);

    switch (line[0]) {
      case 'B': 
        buscaHashTableDinamica(T, chave);
        break;
      case 'I': 
        insereHashTableDinamica(T, chave);
        break;
      case 'R': 
        removeHashTableDinamica(T, chave);
        break;
    }

    fprintf(file_out, "\n\n");
  }

  fclose(file_in);
  fclose(file_out);

  return 0;
}
