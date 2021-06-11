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

char** start_filters(char** filters){
	int i = 0;
	while(i < 5){
		filters[i++] = NULL;
	}
	return filters;
}


char** takeFilters(int argc, char** argv){
	/* Estrutura Filtros
	   Array com espaço para 5 filtros.
	   filter[0] -> alto
	   filter[1] -> baixo
	   filter[2] -> eco
	   filter[3] -> rapido
	   filter[4] -> lento 
	*/
	int i = 4;
	char** filters = malloc(sizeof(char*) * 5);

	filters = start_filters(filters);
	
	while(i < argc){
		if (strcmp(argv[i], "alto") == 0) filters[0] = strdup("bin/aurrasd-filters/aurrasd-gain-double");
		else if (strcmp(argv[i], "baixo") == 0) filters[1] = strdup("bin/aurrasd-filters/aurrasd-gain-half");
		else if (strcmp(argv[i], "eco") == 0) filters[2] = strdup("bin/aurrasd-filters/aurrasd-echo");
		else if (strcmp(argv[i], "rapido") == 0) filters[3] = strdup("bin/aurrasd-filters/aurrasd-tempo-double");
		else if (strcmp(argv[i], "lento") == 0) filters[4] = strdup("bin/aurrasd-filters/aurrasd-tempo-half");
		i++;
	}
	return filters;
}



int main (int argc, char * argv[]){

    if(argc == 1){
	int p;	  
	    p = write(1, "./aurras status\n", 17);
	    if (p == -1) return 0;
	    p = write(1, "./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...\n", 78);
	    if (p == -1) return 0;
    }
    else if(strcmp("status", argv[1]) == 0) {
	    // codigo status servidor 
	 

    }
    else if(strcmp("transform", argv[1]) == 0 && argc > 4){
	    // codigo executar
	    if (mkfifo("bin/clientetoserver", 0777) == -1) {
		    if( errno != EEXIST){	    
		 	  write(1, "Could Not Creat The FIFO\n", 26);
		  	  return 0;
		    }
	    }
	   
	    char* comand = strdup(argv[1]);
	    char* finput = strdup(argv[2]);
	    char* foutput = strdup(argv[3]);
	    char ** filtros = takeFilters(argc, argv); 


	    char resposta[2000];

	    strcat(resposta, comand);
	    strcat(strcat(resposta, " "), finput);
	    strcat(strcat(resposta, " "),foutput);
	    int i = 0;
	    while(i < 5 ){
		    if(filtros[i] != NULL){		
		    strcat(strcat(resposta, " "), filtros[i]);		
		    }
		    i++;
	    }

	    
	    
	    int fd = open("bin/clientetoserver", O_WRONLY);
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