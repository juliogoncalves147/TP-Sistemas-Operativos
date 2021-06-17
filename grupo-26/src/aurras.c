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


int main (int argc, char * argv[]){

    if(argc == 1){ // comando de exemplificação de exemplos
	int p;	  
	    p = write(1, "./aurras status\n", 17);
	    if (p == -1) return 0;
	    p = write(1, "./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...\n", 78);
	    if (p == -1) return 0;
    }
    else if(strcmp("status", argv[1]) == 0 && argc == 2) { // comando em que pedidmos o status do servidor
	    // codigo status servidor 
	    if (mkfifo("bin/clientetoserver", 0777) == -1) { // criação do fifo de comunição
		    if(errno != EEXIST){	    
		 	  write(1, "Could Not Creat The FIFO\n", 26);
		  	  return 0;
		    }
	    } 

	    int fd = open("bin/clientetoserver", O_WRONLY); // escreve para o fifo
	    if((write(fd, argv[1], strlen(argv[1])+1)) == -1){
		    write(1, "Could Not Write To The FIFO 'clientetoserver'\n", 26);
	   	    return 0;
	    }
	    close(fd);
	    int bytes_read;
	    char buffer[1024];
	    fd = open("bin/servertocliente", O_RDONLY);

    	    while((bytes_read = read(fd, buffer, 1024)) > 0){
	    	  write(1, buffer, bytes_read);
	    }
	    close(fd);
    }
    else if(strcmp("transform", argv[1]) == 0 && argc > 4){ // quando é pedido ao servidor a aplicação de filtros
	    // codigo executar
	    if (mkfifo("bin/clientetoserver", 0777) == -1) { // criação do fifo de comunição
		    if( errno != EEXIST){	    
		 	  write(1, "Could Not Creat The FIFO\n", 26);
		  	  return 0;
		    }
	    }

	    char* comand = strdup(argv[1]); 
	    char* finput = strdup(argv[2]);
	    char* foutput = strdup(argv[3]); 


	    char resposta[2000];
					        	// vamos juntar todos os comandos numa string para escrever no fifo
	    strcat(resposta, comand);
	    strcat(strcat(resposta, " "), finput);
	    strcat(strcat(resposta, " "),foutput);
	    int i = 4;
	    while(i < argc ){
		    if(argv[i] != NULL){		
		    strcat(strcat(resposta, " "), argv[i]);		
		    }
		    i++;
	    }
	    
	    int fd = open("bin/clientetoserver", O_WRONLY); // escreve para o fifo
	    if (write(fd, resposta, strlen(resposta)+1) == -1){
		    write(1, "Could Not Write To The FIFO\n", 26);
	   	    return 0;
	    }
	    close(fd);
    }
    else {
	 int p = write(1, "Comando Inválido\n", 19);
	 if (p == -1) return 0;
    }
	

    return 0;
}