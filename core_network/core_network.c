#include <time.h>
#include <netinet/ip.h>
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

void *cn_thread(void* param)
{
	thread_mask();
	char buff[IP_MAXPACKET];
	int sk = *(int*)param;
	struct pollfd my_poll = {sk, POLLIN, 0};
	// ACK until we press SIGINT
	while (!terminate)
	{
		// We check if a packet was received at the TUN device
		int ret = poll(&my_poll, 1, 50); // Wait some milisecs for inputs on TUN

		if (ret>0 && my_poll.revents == POLLIN) // a packet has been received at the TUN
		{	
			// Receiving
			int rebuts = receive_udp(sk, buff, IP_MAXPACKET);
			printf("MO: Received %d bytes, message: %s\n", rebuts, buff);
			//	TODO: Check if the received msg is actually from a Sensor Node of our net
			// Asnwering(acknowledging)
			time_t t = time(NULL);
			struct tm *t_bcn = localtime(&t);
			char ack[IP_MAXPACKET];
			strftime(ack, sizeof(ack), "%c", t_bcn);
			strcat(ack, " -> ACKNOWLEDGING ");
			strcat(ack, &(buff[rebuts-14]));
			//	TODO: Answer to the sendgin IP / remove harcoded address
			printf("MT: ACK sent, msg size: %d bytes\n", send_udp(sk, ack, strlen(ack), UN_IP, UN_PORT));
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

int main()
{
	// Obrir un socket UDP per rebre i enviar
	int sk = setup_udp_socket(CN_IP, CN_PORT);

	setup_routes(CN);

	// 	TODO: Crear un fitxer per loggejar el que ha arribat
	// Preparem el signal handler
	struct sigaction action;
	action.sa_handler = handler;
	sigaction(SIGINT, &action, NULL);

	
	// Començar a llegir del socket i respondre
	printf("Starting to read and answer in port %d \n", CN_PORT);
	printf("---> CN node active.\n-------------------Press CTRL+C to stop the node-------------------\n");

	// POSIX THREADS-----------------------------------------------
	pthread_t pthread_CN;
	if (pthread_create(&pthread_CN, NULL, cn_thread, (void*)&sk))
	{
		printf("ERROR creating the pthread\n");
		exit(-1);
	}
	// Ara esperem que acabin els threads i mirem si hi ha hagut errors
	void *ret;
	pthread_join(pthread_CN, &ret);
	if (ret)
	{
		printf("An error ocurred inside pthread_CN\n");
	}
	close_udp_socket(sk);
	printf("UDP socket closed.\n---> CN node was stopped successfully\n");
}
