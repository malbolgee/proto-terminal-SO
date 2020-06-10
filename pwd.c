#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv){
	char buffer[PATH_MAX];
	char error[50];
	
	if(argc == 1 || !strcmp(argv[1], "-L")){
		printf("%s\n", getenv("PWD") );
		
	}
	else if(!strcmp(argv[1], "-P")){
		printf("%s\n", getcwd(buffer, PATH_MAX) );
	}
	else{

		errno = EINVAL;
		snprintf(error, sizeof error, "pwd: %s", argv[1]);
		perror(error);
		
	}

	return 0;

}
