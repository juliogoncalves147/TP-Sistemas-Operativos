#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

//Estrutura de dados para gerir os Filtros Existentes e sua utilização
typedef struct filtros{
    char* filtro_name; // nome do filtro, ex: alto, eco
    char* filtro_exec; // nome do executavel do filtro a aplicar
    int used;          // quantos processos estão a utilizar este filtro
    int max_used;      // numero maximo de processos que podem utilizar este filtro ao mesmo tempo
    int numero_filtros; // numero de filtros que existem diferentes
} *FILTROS;

// Função que divide o comando enviado pelo cliente num array de strings
char **inputDivide(char *input)
{
    char **stdi = (char **)malloc(10 * sizeof(char *)); // aloca memória para um array de apontadores para Strings.
    for (int j = 0; j < 10; j++)
        stdi[j] = NULL; // inicia todos os apontadores a NULL.

    int i = 0;
    while (input)
    {
        char *aux = strsep(&input, "\n \t"); // ;  // divide a String dada segundo os delimitadores dados.
        if (aux && aux[0])
            stdi[i++] = strdup(aux); // aloca espaço e copia a String.
    }

    return stdi; // retorna o Array de apontadores para Strings
}

// Função determina se um certo input é um filtro
int isFilter(char* comando, FILTROS filtros[]){
    int i = 0;
    int val = 0;
    while(i < filtros[0]->numero_filtros && val == 0){
        if(strcmp(comando, filtros[i]->filtro_name)) val = 1;
        i++;
    }

    return val;
}

// Função que determina quantos filtros vão ser aplicados a um determinado input
int numisFilter(char** comandos, FILTROS filtros[]){
    int i = 3;
    int cont = 0;
    while(comandos[i] != NULL){
        if(isFilter(comandos[i], filtros)) cont++;
        i++;
    }
    return cont;
}



// Função que executa a filtragem de uma musica aquando da chamada do comando transform pelo cliente.
// config_filename - diretoria onde esta o ficheiro de configuração de filtros
// filters_folder - diretoria onde se encontram os executaveis dos filtos
// comandos - array de strings que contem os comandos mandados pelo cliente
/*
   Normalmente para esta funcao:
         comandos[0] -> "transform"
         comandos[1] -> "input file name"
         comandos[2] -> "output file name"
         comandos[3] em diante -> nome dos filtros a aplicar
   */

// Função que dado o nome do filtro retorna o nome do executavel
char* filter_diretory(char* filter, FILTROS filtros[]){
    int i = 0;
    int val = 0;

    while(i < filtros[0]->numero_filtros && val == 0){
        if(strcmp(filter, filtros[i]->filtro_name) == 0) val = 1;
        i++;
    }

    if(val == 1) return filtros[i-1]->filtro_exec;
    else return NULL;
}

void atualiza_filtros(char* filter, FILTROS filtros[]){
    int i = 0;
    int val = 0;
    while(i < filtros[0]->numero_filtros && val == 0){
        if(strcmp(filter, filtros[i]->filtro_name) == 0) {
             val = 1;
             filtros[i]->used++; // atualizar isto depois  para impedir que se atualize estando no maximo
        }
        i++;
    }
}

void liberta_filtros(char* comandos[], int numero, FILTROS filtros[]){
    int i = 0;
    int val = 0;
    for(int a = 0; a < numero; a++){
        i = 0;
        val = 0;
        while(i < filtros[0]->numero_filtros && val == 0){
            if(strcmp(comandos[a], filtros[i]->filtro_name) == 0){
                val = 1;
                if(filtros[i]->used > 0) filtros[i]->used--;
            }
            i++;
        }
    }
}



// Função de execução do comando transform
void exec_transform(char* config_filename, char* filters_folder, char** comandos, FILTROS filtros[]){

    int numFilters = numisFilter(comandos, filtros); // numero de filtros a serem aplicados à musica dada
    
    int fi = open(comandos[1], O_RDONLY);   // descritor musica a ser tratada
    if(fi == -1){
        perror("Musica de input não existe!");
        return;
    }
    int fo = open(comandos[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); // descritor da musica final
    if(fo == -1){
        perror("Erro ao criar ficheiro de output");
        return;
    }

    dup2(fi, 0);  // redirecionamentos
    dup2(fo, 1);

    close(fi);
    close(fo);

    int fd[2];
    int i;
    for(i = 0; i < numFilters - 1; i++){  // execução dos vários filtros seguidos
        int pipe_ret = pipe(fd);
        if(pipe_ret == -1){
            perror("Problema ao criar o pipe!");
            return;
        }

        if(fork() == 0){
             dup2(fd[1], 1);
             close(fd[1]);  // fd[0] leitura, fd[1] escrita
             close(fd[0]);
             char* diretoria = malloc(1024);
             strcpy(diretoria, filters_folder);
             strcat(diretoria, "/");
             char* exec_filter = filter_diretory(comandos[3+i], filtros); // vai a estrutura de dados buscar o nome do executavel
             if (exec_filter == NULL){
                 perror("Filtro nao existente");
                 return;
             } 
             strcat(diretoria, exec_filter);          // fazer função para atribuir as diretorias dos executaveis aos respetivos nomes dos filtros
             execl(diretoria, diretoria, NULL);
             _exit(0);
        } else {    
             atualiza_filtros(comandos[3+i], filtros);
             dup2(fd[0], 0);
             close(fd[0]);
             close(fd[1]);
        }    
    }    
    // ultimo caso, redirecionamento para o ficheiro de output
    if(fork() == 0){
        char* diretoria = malloc(1024);
        strcpy(diretoria, filters_folder);
        strcat(diretoria, "/");
        char* exec_filter = filter_diretory(comandos[3+i], filtros);
        if (exec_filter == NULL){
            perror("Filtro nao existente");
            return;
        } 
        strcat(diretoria, exec_filter);
        execl(diretoria, diretoria, NULL);  
        _exit(0);
    } else {
        atualiza_filtros(comandos[3+i], filtros);
    }

}

// Função para contar o numero de filtros definidos no conf com base em \n
int numberFilters(char* filtros){

    int i = 0;
    int cont = 0;
    while(filtros[i]){
        if(filtros[i] == '\n') cont++;
        i++;
    }
    return cont;
}

// Função para inicializar a estrutura de filtros com base no ficheiro de configuração de filtros
FILTROS* initFiltros(char* config_filename){
    
    
    int fd = open(config_filename, O_RDONLY); // descritor para o ficheirod e configuração

    if (fd == -1){
        write(1, "\nCould not open the config file\n", 32); // tratamento de erro
    }
    
    char* buffer = malloc(sizeof(char) * 1024);
    
    read(fd,buffer,1024);                       // le o conteudo do ficheiro de configuração        
    int numFilters = numberFilters(buffer);     // determina o numero de filtros existentes com base nos \n
    
    FILTROS *filtros = malloc(sizeof(FILTROS)* numFilters);

    char* configs[numFilters][3];
   
    for(int j = 0; j < numFilters ; j++){       // faz a respetiva atribuição da divisão da string para uma matriz
        for(int i = 0; i < 3 ; i++)
            configs[j][i] = strdup(strsep(&buffer," \n"));
    }

    int i = 0;
    while(i < numFilters){                                  // através da matriz anterior é inicializada a estrutura de filtros
        filtros[i] = malloc(sizeof(struct filtros));
        filtros[i]->filtro_name = strdup(configs[i][0]);
        filtros[i]->max_used = atoi(configs[i][2]);
        filtros[i]->used = 0;
        filtros[i]->filtro_exec = strdup(configs[i][1]);
        filtros[i]->numero_filtros = numFilters;
        i++;
    }

    close(fd);

    return filtros;
}

void exec_status(FILTROS filtros[]){ // Incompleto, falta meter as tasks em processamento e voltar a enviar isto para o filho.
    int i = 0;

    if ((mkfifo("bin/servertocliente", 0777)) == -1) { // criação do fifo de comunição
		    if(errno != EEXIST){	    
		 	  write(1, "Could Not Creat The FIFO\n", 26);
		  	  return;
		    }
	    }
    
    int fd = open("bin/servertocliente", O_WRONLY);

    while(i < filtros[0]->numero_filtros){
        char comando[2000] = "";
        strcat(comando, "filter");
        strcat(comando, " ");
        strcat(comando, filtros[i]->filtro_name);
        strcat(comando, ":");
        strcat(comando, " ");
        char buffer[10];
        sprintf(buffer,"%d", filtros[i]->used);
        strcat(comando, buffer);
        strcat(comando, "/");
        sprintf(buffer,"%d", filtros[i]->max_used);
        strcat(comando, buffer);
        strcat(comando, " ");
        strcat(comando, "(running/max)");
        strcat(comando, "\n");

        if((write(fd, comando, strlen(comando)+1)) == -1){
		    write(1, "Could Not Write To The FIFO 'servertocliente'\n", 26);
	   	    return;
	    }

        i++;
    }
    int pi = getpid();
    char buffer[10];
    sprintf(buffer, "pid: %d \n", pi);
    write(fd, buffer, strlen(buffer));
    close(fd);
}

int main (int argc, char * argv[]){ 

    if(argc != 3){                              // caso a chamada de inicialização do servidor falhe
        write(1, "Argumetos Inválidos", 21);
        return 0;
    }
    //etc/aurrasd.config                        // estes são os ficheiros que o professor deu para testes
    //bin/aurrasd-filters
    char* config_filename = strdup(argv[1]);
    char* filters_folder = strdup(argv[2]);
    
    FILTROS *filtros = initFiltros(config_filename);  // isto tem que se alterar pois aqui ainda nao sabemos quantos filtros iremos ter
                                         // inicialização dos filtors

      // abertura do fifo de comunicação

    char* buffer = malloc(1024);
    int bytes_read = 0;
    
    while(1){ 
                                              // ciclo infinito em que o servidor vai estar constantemente a ler do fifo
    int fd = open("bin/clientetoserver", O_RDONLY);
    bytes_read = read(fd, buffer, 1024);

    if(bytes_read > 0){
        char** comandos = inputDivide(buffer); // duas possiveis chamadas ao servidor
                                               // a aplicação de filtros a uma musica
        if (strcmp(comandos[0], "transform") == 0) exec_transform(config_filename, filters_folder, comandos, filtros);

                                               // quando o cliente pede o status do servidor
      
        if (strcmp(comandos[0], "status") == 0) exec_status(filtros); // fazer este comando
        
        free(comandos);
       }
    }
    
    
    return 0;
}




