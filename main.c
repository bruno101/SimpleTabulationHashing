#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "estruturas.h"


//esse eh o arquivo que iremos escrever
//o deixamos como variavel global
FILE *file_out;



struct hashtable {
  //consiste em um vetor de inteiros
  //conterá '0' se uma posição for vazia e '2^64-1' (-1) se ela for liberada
  unsigned long long *tabela;

  //o número de elementos suportado pela tabela
  unsigned long long tamanho;

  //precisaremos desse valor para implementarmos a operacao de "limpar"
  unsigned long long numeroDeCodigosDeRemocao;

  unsigned long long tabelasAleatorias[8][0x100];
};

//hashtable_dinamica implementa table doubling/halving
struct hashtable_dinamica {
  unsigned long long tamanho;
  unsigned long long numeroElementos;

  HashTable *hashtable;
};



//Funções da tabela de dispersão normal

//Variavel global para passarmos como 'seed' e incrementarmos cada vez que usamos a função 'rand()'
unsigned long long num = 1;

//Preenche os valores da tabela aleatoria a partir do numero 'valoresPossiveis', que eh o número de valores em que cada chave pode ser mapeada (o tamanho da nossa tabela de dispersao)
void preencheTabelaAleatoria (unsigned long long tabelaAleatoria[0x100], unsigned long long valoresPossiveis) {

  srand(num);
  num++;

  //a tabelaAleatoria tem 2^8 posicoes
  for (unsigned long long i = 0; i < 0x100; i++) {
    tabelaAleatoria[i] = (rand())%valoresPossiveis;
  }

}

HashTable *criaHashTable (unsigned long long tamanho) {
  //'tamanho' é sempre uma potência de 2, o que enforçamos nas funções de HashTableDinamica 

  HashTable *H = malloc(sizeof(HashTable));

  H->tabela = malloc(tamanho*sizeof(unsigned long long));
  for (unsigned long long i = 0; i < tamanho; i++) {
    (H->tabela)[i] = 0;
  }

  H->tamanho = tamanho;

  for (unsigned long long i = 0; i < 8; i++) {
    preencheTabelaAleatoria (H->tabelasAleatorias[i], H->tamanho);
  }

  return H;
}

//a nossa função de dispersão
unsigned long long funcHashTable (HashTable *H, unsigned long long chave) {

  unsigned long long valor = 0;

  for (unsigned long long i = 0; i < 8; i++) {
    //a cada iteração fazemos um 'XOR' entre o valor anterior e o valor a que a tabela mapeia um determinado grupo de dígitos de 'chave'
    valor = valor^(H->tabelasAleatorias[i][chave%(0x100)]);
    chave >>= 8;
  }
  
  return valor;
}

//retorna a posicao onde o elemento foi encontrado ou '-1' se ele nao for
unsigned long long buscaHashTable (HashTable *H, unsigned long long chave) {

  unsigned long long posChave = funcHashTable(H, chave);
  unsigned long long posChaveInicial = posChave;

  while (posChave < H->tamanho) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave] == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %d\n", posChaveInicial, -1);

      return -1;
    }
    if (H->tabela[posChave] == chave) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %lld\n", posChaveInicial, posChave);

      return posChave;
    }
    posChave += 1;
  }

  //se chegamos no fim da tabela e ainda nao encontramos a resposta voltamos para o inicio
  posChave = 0;
  
  while (posChave < posChaveInicial) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave] == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %d\n", posChaveInicial, -1);

      return -1;
    }
    if (H->tabela[posChave] == chave) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %lld\n", posChaveInicial, posChave);

      return posChave;
    }
    posChave += 1;
  }

  //adicionando ao arquivo de texto
  fprintf(file_out, "%lld %d\n", posChaveInicial, -1);

  return -1;
}

//retorna a posicao onde o elemento foi inserido na tabela
unsigned long long insereHashTable (HashTable *H, unsigned long long chave) {

  unsigned long long posChave = funcHashTable(H, chave);
  unsigned long long posChaveInicial = posChave;

  while (posChave < H->tamanho) {
    //'0' indica que o espaço é vazio e '-1' (2^64-1) que é liberado
    if ( (H->tabela[posChave] == 0) || (H->tabela[posChave] == -1) ) {
      H->tabela[posChave] = chave;

      return posChave;
    }
    posChave +=1;
  }

  //chegamos ao final do vetor 'tabela' e ainda nao encontramos posicao vazia, então passamos a buscar do começo do vetor
  posChave = 0;

  while (posChave < posChaveInicial) {
    //'0' indica que o espaço é vazio e '-1' (2^64-1) que é liberado
    if ( (H->tabela[posChave] == 0) || (H->tabela[posChave] == -1) ) {
      H->tabela[posChave] = chave;

      return posChave;
    }
    posChave += 1;
  }

  //essa linha em teoria nao deve ser executada
  printf("Oh no!");
  return -1;
}

//limpamos os codigos de remocao simplesmente reconstruindo a tabela
void limpaCodigosRemocao (HashTable *H) {

  unsigned long long* tabelaAuxiliar = malloc(sizeof(unsigned long long)*H->tamanho); 

  unsigned long long cont = 0;
  for (unsigned long long i = 0; i < H->tamanho; i++) {

    if ( (H->tabela[i] != 0) && (H->tabela[i] != -1) ) {
      //Adicionando todas as chaves a tabela auxiliar
      tabelaAuxiliar[cont] = H->tabela[i];
      cont++;
    }
    H->tabela[i] = 0;

  }

  for (unsigned long long i = 0; i < cont; i++) {
    //Reinserimos todas as chaves
    insereHashTable(H, tabelaAuxiliar[i]);
  }

  H->numeroDeCodigosDeRemocao = 0;

  free(tabelaAuxiliar);

  //adicionando ao arquivo de texto
  fprintf(file_out, "\n\nLIMPAR\n");

}

//retorna a posicao em que o elemento foi removido ou '-1' se nenhum elemento foi removido
unsigned long long removeHashTable (HashTable *H, unsigned long long chave) {
  
  unsigned long long posChave = funcHashTable(H, chave);
  unsigned long long posChaveInicial = posChave;

  while (posChave < H->tamanho) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave] == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %d\n", posChaveInicial, -1);

      return -1;
    }
    if (H->tabela[posChave] == chave) {
      //liberamos o espaco
      H->tabela[posChave] = -1;

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %lld\n", posChaveInicial, posChave);

      H->numeroDeCodigosDeRemocao += 1;
      if (H->numeroDeCodigosDeRemocao >= H->tamanho/4) {
        limpaCodigosRemocao(H);
      }

      return posChave;
    }
    posChave += 1;
  }

  //chegamos ao final do vetor 'tabela' e ainda nem encontramos a chave nem alcançamos uma posição vazia, entao passamos a buscar do começo do vetor
  posChave = 0;

  while (posChave < posChaveInicial) {
    //'0' indica que espaço é vazio
    if (H->tabela[posChave] == 0) {

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %d\n", posChaveInicial, -1);

      return -1;
    }
    if (H->tabela[posChave] == chave) {
      //liberamos o espaco
      H->tabela[posChave] = -1;

      //adicionando ao arquivo de texto
      fprintf(file_out, "%lld %lld\n", posChaveInicial, posChave);

      H->numeroDeCodigosDeRemocao += 1;
      if (H->numeroDeCodigosDeRemocao >= H->tamanho/4) {
        limpaCodigosRemocao(H);
      }

      return posChave;
    }
    posChave += 1;
  }

  return -1;
}

void deletaHashTable (HashTable *H) {
  
  free(H->tabela);
  free(H);
}



//Funções da HashTable hashtableDinamica

HashTableDinamica *criaHashTableDinamica () {
  HashTableDinamica *T = (HashTableDinamica *)malloc(sizeof(HashTableDinamica));

  T->tamanho = 4;
  T->numeroElementos = 0;
  T->hashtable = criaHashTable(4);

  //adicionando ao arquivo de texto
  fprintf(file_out, "TAM:4\n\n\n");

  return T;
}

//retorna a posicao onde o elemento foi encontrado ou '-1' se ele nao for
unsigned long long buscaHashTableDinamica (HashTableDinamica *T, unsigned long long chave) {
  return buscaHashTable (T->hashtable, chave);
}

//retorna a posicao onde o elemento foi inserido na tabela
unsigned long long insereHashTableDinamica (HashTableDinamica *T, unsigned long long chave) {

  unsigned long long pos = insereHashTable(T->hashtable, chave);
  T->numeroElementos +=1;

  //adicionando ao arquivo de texto
  fprintf(file_out, "%lld %lld\n", funcHashTable(T->hashtable, chave), pos);

  //O 'tamanho' sempre será pelo menos o dobro do número de elementos (epsilon = 1, o que significa que o tempo por operação deve ser O(1))
  if (T->tamanho == 2*T->numeroElementos) {

    HashTable *H = criaHashTable(2*(T->tamanho));
    for (unsigned long long i = 0; i < (T->tamanho); i++) {
      if ( (T->hashtable->tabela[i] != 0) && (T->hashtable->tabela[i] != -1) ) {
        insereHashTable(H, T->hashtable->tabela[i]);
      }
    }
    deletaHashTable(T->hashtable);
    T->hashtable = H;
    T->tamanho = 2*(T->tamanho);

    //adicionando ao arquivo de texto
    fprintf(file_out, "\n\nDOBRAR TAM:%lld\n", T->tamanho);

  }

  return pos;
}

//retorna a posicao em que o elemento foi removido ou '-1' se nenhum elemento foi removido
unsigned long long removeHashTableDinamica (HashTableDinamica *T, unsigned long long chave) {

  unsigned long long pos = removeHashTable(T->hashtable, chave);

  if (pos != -1) {
    T->numeroElementos -= 1;

    if (T->tamanho == 8*T->numeroElementos) {

      HashTable *H = criaHashTable((T->tamanho)/2);
      for (unsigned long long i = 0; i < (T->tamanho); i++) {
        if ( (T->hashtable->tabela[i] != 0) && (T->hashtable->tabela[i] != -1) ) {
          insereHashTable(H, T->hashtable->tabela[i]);
        }
      }

      deletaHashTable(T->hashtable);
      T->hashtable = H;
      T->tamanho = (T->tamanho)/2;
      
      //adicionando ao arquivo de texto
      fprintf(file_out, "\n\nMETADE TAM:%lld\n", T->tamanho);

    }

  }

  return pos;
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
  unsigned long long chave;
  while (fgets(line, sizeof(line), file_in)) {

    fprintf(file_out, "%s", line);
    if (line[strlen(line)-1] != '\n') {
      fprintf(file_out, "%s", "\n");
    } 

    chave = atoi(line+4);
    if ( (chave == 0) || (chave == -1) ) {
      printf("Por favor nao use chaves com valores '0' ou '2^64-1' porque esses valores estao sendo usados como flags.");
      break;
    }

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
