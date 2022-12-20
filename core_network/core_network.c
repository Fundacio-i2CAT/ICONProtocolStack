#include <time.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include "udp_socket.h"
#include "testbed_setup.h"

#define ACK_SIZE 50

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
	long int packet_counter = 0, total_bytes = 0, n_bytes;
	char buff[IP_MAXPACKET];
	int sk = *(int*)param;
	struct pollfd my_poll = {sk, POLLIN, 0};

	// Receive messages until we press SIGINT
	while (!terminate)
	{
		// We check if a packet was received at the TUN device
		int ret = poll(&my_poll, 1, 1); // Wait 1 milisec for inputs on TUN
		if (ret>0 && my_poll.revents == POLLIN) // a packet has been received at the TUN
		{	
			n_bytes = receive_udp(sk, buff, IP_MAXPACKET);
			if (n_bytes  > 0)
			{
				// TODO: Check if it is our packet sent by the user terminal
				total_bytes += n_bytes;
				packet_counter++;
			}
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
	printf("\n\nWe received a total of %ld packets and %ld bytes.\n", packet_counter, total_bytes);
	return(NULL);
}

void *mt_thread(void *param)
{
	thread_mask();
	int sk = *(int*)param;
	long int count = 0;
	char msg[ACK_SIZE];
	// memset(msg, 1, ACK_SIZE);
	
	int milisecs;
	struct timeval tv;
	time_t time_now;
	struct tm *tmnow;
	// Send until we press SIGINT
	while(!terminate)
	{
		gettimeofday(&tv, NULL);
		time_now = tv.tv_sec;
		tmnow = localtime(&time_now);
		milisecs = tv.tv_usec / 1000;
		int written_b = strftime(msg, sizeof(msg), "MESSAGE FROM CORE NETWORK. Sent at: %Y-%m-%d %H:%M:%S", tmnow);
		sprintf(msg + written_b, "%03d%ld", milisecs, count);

		// Call socket send function
		send_udp(sk, msg, ACK_SIZE, UN_IP, UN_PORT);
		// Delete content just in case
		memset(msg, 1, ACK_SIZE);
		count++;
		sleep(5);
	}
	printf("A total of %ld MT packets were sent.\n", count);
	return(NULL);
}

int main()
{
	// Open socket UDP for sending and receiving
	int sk = setup_udp_socket(CN_IP, CN_PORT);

	setup_routes(CN);

	// 	TODO: Create a log file to store metrics
	// Prepare signal handler
	struct sigaction action;
	action.sa_handler = handler;
	sigaction(SIGINT, &action, NULL);

	
	printf("Starting to read and answer in port %d \n", CN_PORT);
	printf("---> CN node active.\n-------------------Press CTRL+C to stop the node-------------------\n");

	// POSIX THREADS-----------------------------------------------
	// pthread_t pthread_CN, pthread_MT;
	pthread_t pthread_CN;
	if (pthread_create(&pthread_CN, NULL, cn_thread, (void*)&sk))
	{
		printf("ERROR creating the pthread\n");
		exit(-1);
	}
	// if (pthread_create(&pthread_MT, NULL, mt_thread, (void*)&sk))
	// {
	// 	printf("ERROR creating the pthread\n");
	// 	exit(-1);
	// }
	// Wait for threads to stop and check for errors
	void *ret;
	pthread_join(pthread_CN, &ret);
	if (ret)
	{
		printf("An error ocurred inside pthread_CN\n");
	}
	// pthread_join(pthread_MT, &ret);
	// if (ret)
	// {
	// 	printf("An error ocurred inside pthread_MT\n");
	// }

	close_udp_socket(sk);
	printf("UDP socket closed.\n---> CN node was stopped successfully\n");
}
