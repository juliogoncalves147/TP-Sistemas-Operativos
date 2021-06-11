#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include<sys/wait.h>

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



    int fd = open("bin/clientetoserver", O_RDONLY);

    char buffer[1024];
    int bytes_read = 0;

    while((bytes_read = read(fd, buffer, 1024)) > 0){
       // write(1, buffer, bytes_read);
    }

    char** comandos = inputDivide(buffer);

    // teste para ver se recebe os comandos certos
    int i = 0;
    
    while(comandos[i] != NULL){
        printf("%s\n", comandos[i++]);
    }
    
    if (strcmp(comandos[0]))


    return 0;
}



