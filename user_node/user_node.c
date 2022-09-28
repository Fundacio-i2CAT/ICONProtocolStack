#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>

#include "udp_socket.h"
#include "testbed_setup.h"


// Variables globals per a fer SIGINT
static int terminate = 0;

static void handler(int sig)
{
	terminate = 1;
	printf("\nTerminating node, please wait...\n");
}

static void thread_mask()
{
	sigset_t sa_mask;
    sigemptyset(&sa_mask);
    sigaddset(&sa_mask, SIGINT);
    if(pthread_sigmask(SIG_BLOCK, &sa_mask, NULL))
	{
		printf("ERROR seeting the thread sigmask");
	}
}

void *mt_thread(void* param)
{
	thread_mask();
	char buff[IP_MAXPACKET];
	int sk = *(int*)param;
	struct pollfd my_poll = {sk, POLLIN, 0};
	// ACK until we press SIGINT
	while (!terminate)
	{
		// We check if a packet was received at the socket
		int ret = poll(&my_poll, 1, 50); // Wait some milisecs for inputs on the socket

		if (ret>0 && my_poll.revents == POLLIN) // a packet has been received at the socket
		{	
			// Receiving
			int rebuts = receive_udp(sk, buff, IP_MAXPACKET);
			if(rebuts < 0){
				printf("ERROR while receiving UDP packet\n");
				return((void*)-1);
			}
			printf("MT: Received %d bytes, message: %s\n", rebuts, buff);
			//	TODO: Check if the ack msg is actually from the CN
			memset(buff, 0, IP_MAXPACKET);
		}
		else if (ret == 0) // Poll timeout
		{
			continue;
		}
		else{
			printf("ERROR while polling\n");
			return((void*)-1);
		}
	}
	return(NULL);
}

void *mo_thread(void* param)
{
	thread_mask();
	int sk = *(int*)param;
	int count = 0;
	char msg[IP_MAXPACKET];
	char msg_num[sizeof(int)];
	// Send until we press SIGINT
	while(!terminate)
	{
		time_t t = time(NULL);
		struct tm *t_bcn = localtime(&t);
		char s[64];
		strftime(s, sizeof(s), "%c", t_bcn);
		strcpy(msg, s);
		strcat(msg, " -> I'M MESSAGE Nº ");
		sprintf(msg_num, "%d", count);
		strcat(msg, msg_num);
		// Cridem la funció per enviar a travès del socket
		int ret = send_udp(sk, msg, strlen(msg), CN_IP, CN_PORT);
		printf("MO: Message sent, IP packet size: %d bytes\n", ret);
		printf("MO: Content: %s\n", msg);
		// Borrem el contingut per si de cas
		memset(msg, 0, IP_MAXPACKET);
		count++;
		sleep(3);
	}
	return(NULL);
}

int main()
{
	setup_routes(UN);

	// Ruta creada, preparem el socket
	int sk = setup_udp_socket(UN_IP, UN_PORT);

	// Preparem el signal handler
	struct sigaction action;
	action.sa_handler = handler;
	sigaction(SIGINT, &action, NULL);

	printf("---> User node active.\n-------------------Press CTRL+C to stop the node-------------------\n");

	// POSIX THREADS-----------------------------------------------
	pthread_t pthread_MO, pthread_MT;
	if (pthread_create(&pthread_MO, NULL, mo_thread, (void*)&sk))
	{
		printf("ERROR creating the pthread\n");
		exit(-1);
	}
	if (pthread_create(&pthread_MT, NULL, mt_thread, (void*)&sk))
	{
		printf("ERROR creating the pthread\n");
		exit(-1);
	}
	// Ara esperem que acabin els threads i mirem si hi ha hagut errors
	void *ret;
	pthread_join(pthread_MO, &ret);
	if (ret)
	{
		printf("An error ocurred inside pthread_MO\n");
	}
	pthread_join(pthread_MT, &ret);
	if (ret)
	{
		printf("An error ocurred inside pthread_M<t>\n");
	}

	// Tanquem socket
	close_udp_socket(sk);
	printf("UDP socket closed.\n---> User node was stopped successfully\n");
}
