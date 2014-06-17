#include <signal.h>
#include "shurrik_client.h"
#include <ctype.h>

void ouch(int sig){
	printf("[notic] fifo file remove and shurrik_server stop, please 'ctrl+c' to shutdown \n");
	unlink(SERVER_FIFO_NAME);
	signal(SIGINT, SIG_DFL);
}

int main(){
	int server_fifo_fd, client_fifo_fd;
	ShurrikData my_data;
	int read_res;
	char client_fifo[256];
	char *tmp_char_ptr;
	char shurrik_tmp[64];

	signal(SIGINT, ouch);
	mkfifo(SERVER_FIFO_NAME, 0777);
	sprintf(shurrik_tmp,"chmod 0777 %s",SERVER_FIFO_NAME);
	system(shurrik_tmp);
	server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
	if (server_fifo_fd == -1){
		fprintf(stderr, "Server fifo failure\n");
		exit(EXIT_FAILURE);
	}
	

	for (;;){
		sleep(1);
		read_res = read(server_fifo_fd,&my_data,sizeof(my_data));
		if (read_res > 0){
			tmp_char_ptr = my_data.some_data;
			printf("%s\n",tmp_char_ptr);
			/*while (*tmp_char_ptr){
				*tmp_char_ptr = toupper(*tmp_char_ptr);
				tmp_char_ptr++;
			}
			sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
			
			client_fifo_fd = open(client_fifo, O_WRONLY);
			if (client_fifo_fd != -1){
				write(client_fifo_fd, &my_data, sizeof(my_data));
				close(client_fifo_fd);
			}*/
		}

	

	//close(server_fifo_fd);
		
	}
}
