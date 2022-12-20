#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include "udp_socket.h"
#include "testbed_setup.h"

// Packet size in Bytes
#define INDIA_PKT_SIZE 46
// Delay in miliseconds
#define INDIA_PKT_PERIODICITY_MILIS 32
// Contact duration in seconds
#define INDIA_CONTACT_DURATION 240

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

	while (!terminate)
	{
		// We check if a packet was received at the socket
		int ret = poll(&my_poll, 1, 1); // Wait 1 milisecs for inputs on the socket

		if (ret>0 && my_poll.revents == POLLIN) // a packet has been received at the socket
		{	
			// Receive and print
			int rebuts = receive_udp(sk, buff, IP_MAXPACKET);
			if(rebuts < 0){
				printf("ERROR while receiving UDP packet\n");
				return((void*)-1);
			}
			printf("MT: Received %d bytes, message: %s\n", rebuts, buff);
			//	TODO: Check if the msg is actually from the CN
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
	long int count = 0;
	char msg[INDIA_PKT_SIZE];
	memset(msg, 1, INDIA_PKT_SIZE);
	
	int milisecs;
	int b_sent = 0;
	struct timeval tv;
	time_t time_now;
	struct tm *tmnow;
	// Send until we press SIGINT or INDIA_CONTACT_DURATION has passed
	while(!terminate)
	{
		gettimeofday(&tv, NULL);
		time_now = tv.tv_sec;
		tmnow = localtime(&time_now);
		milisecs = tv.tv_usec / 1000;
		int written_b = strftime(msg, sizeof(msg), "%Y-%m-%d %H:%M:%S", tmnow);
		sprintf(msg + written_b, "%03d%ld", milisecs, count);

		// Call function to send through socket
		b_sent += send_udp(sk, msg, INDIA_PKT_SIZE, CN_IP, CN_PORT);
		// Delete content, just in case
		memset(msg, 1, INDIA_PKT_SIZE);
		count++;
		struct timespec sleep_time = {
			.tv_sec = INDIA_PKT_PERIODICITY_MILIS / 1000,
			.tv_nsec = (INDIA_PKT_PERIODICITY_MILIS % 1000) * 1000000};
		nanosleep(&sleep_time, &sleep_time);// wait before sending the next packet
	}
	printf("A total of %ld packets and %d were sent.\n", count, b_sent);
	return(NULL);
}

void *timer_thread(void* param)
{
	thread_mask();
	sleep(INDIA_CONTACT_DURATION);
	terminate = 1;
	return(NULL);
}

int main()
{
	setup_routes(UN);

	// Route created, setup socket
	int sk = setup_udp_socket(UN_IP, UN_PORT);

	// Prepare signal handler
	struct sigaction action;
	action.sa_handler = handler;
	sigaction(SIGINT, &action, NULL);

	printf("---> User node active.\n-------------------Press CTRL+C to stop the node-------------------\n");

	// POSIX THREADS-----------------------------------------------
	// pthread_t pthread_MO, pthread_MT;
	pthread_t pthread_MO, pthread_timer;
	if (pthread_create(&pthread_MO, NULL, mo_thread, (void*)&sk))
	{
		printf("ERROR creating the pthread\n");
		exit(-1);
	}
	// if (pthread_create(&pthread_MT, NULL, mt_thread, (void*)&sk))
	// {
	// 	printf("ERROR creating the pthread\n");
	// 	exit(-1);
	// }
	if (pthread_create(&pthread_timer, NULL, timer_thread, (void*)&sk))
	{
		printf("ERROR creating the pthread\n");
		exit(-1);
	}
	// Wait for threads to stop and check for errors
	void *ret;
	pthread_join(pthread_MO, &ret);
	if (ret)
	{
		printf("An error ocurred inside pthread_MO\n");
	}
	// pthread_join(pthread_MT, &ret);
	// if (ret)
	// {
	// 	printf("An error ocurred inside pthread_MT\n");
	// }

	// Close socket
	close_udp_socket(sk);
	printf("UDP socket closed.\n---> User node was stopped successfully\n");
}
