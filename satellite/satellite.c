#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include "bp_node.h"
#include "testbed_setup.h"

#define N_PKT_WAIT 15

// Funció que s'encarrega d'inicialitzar el TUN
static int tun_alloc(int flags)
{
    struct ifreq ifr;
    int fd, err;
    char *clonedev = "/dev/net/tun";

    if ((fd = open(clonedev, O_RDWR)) < 0) {
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = flags;


    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        close(fd);
        return err;
    }

    printf("Opening tun/tap device: %s for reading\n", ifr.ifr_name);

    return fd;
}

// Variables globals per a semaphors i SIGINT
static int terminate = 0;
sem_t ion_mutex;
static void handler(int sig)
{
	terminate = 1;
	printf("\nTerminating node, please wait...\n");
}
// Avoids children pthreads from getting interrupted by SIGINT
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

void *mo_thread(void *param)
{
	// Ignore SIGINT
	thread_mask();
	// Variables handling TUN and polling
	int tun_fd = *(int*)param;
	struct pollfd my_poll = {tun_fd, POLLIN, 0};
	// BP ipn addresses
	char *txeid = malloc(sizeof(SAT_MO_EID));
	char *rxeid = malloc(sizeof(BA_MO_EID));
	strcpy(txeid, SAT_MO_EID);
	strcpy(rxeid, BA_MO_EID);

	// Variables handling storage for calling BP at once with multiple IP packets
	// (BP calls are more efficient this way)
	int n_packets = 0;
	size_t packets_size[5];
	size_t buff_pointer = 0;
	char my_packets[IP_MAXPACKET*5];
	int loops = 0;

	// Loop fins que premin una tecla
	while(!terminate)
	{
		// Call send_bundle if at least N_PKT_WAIT packets are ready
		// or every 20 loops if packets are available
		if (n_packets == N_PKT_WAIT || (n_packets > 0 && loops >= 30)){
			sem_wait(&ion_mutex);
			if (send_bundle(txeid, rxeid, my_packets, packets_size, n_packets, (int)7200))
			{
				printf("Could not send bundle\n");
				return((void*)-1);
			}
			sem_post(&ion_mutex);
			buff_pointer = 0;
			n_packets = 0;
			loops = 0;
		}
		else{
			loops++;
		}
		// We check if a packet was received at the TUN device
		int ret = poll(&my_poll, 1, 1); // Wait some milisecs for inputs on TUN

		if (ret>0 && my_poll.revents == POLLIN) // a packet has been received at the TUN
		{
			packets_size[n_packets] = read(tun_fd, my_packets+buff_pointer, IP_MAXPACKET);
			if (packets_size[n_packets] < 0) {
				perror("Error reading from TUN interface\n");
				close(tun_fd);
				exit(1);
			}
			// Check if its contains a UDP datagram
			if (((struct iphdr*)(my_packets+buff_pointer))->protocol == 17){
			// Add here extra checks (e.g: check if sender is in our database)
				buff_pointer += packets_size[n_packets];
				n_packets++;
			}
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

void *mt_thread(void *param)
{
	// Set mask to ignore SIGINT
	thread_mask();
	int tun_fd = *(int*)param;
	int bytes;
	// Rebem bundles i reenviem el contingut amb el TUN
	char *buffer = malloc(IP_MAXPACKET);
	char *rxeid = malloc(sizeof(SAT_MT_EID));
	strcpy(rxeid, SAT_MT_EID);
	// Loop fins que clickin ^C
	while(!terminate){
		// Fem poll a veure si hi ha algun bundle
		sem_wait(&ion_mutex);
		bytes = receive_bundle(rxeid, buffer, IP_MAXPACKET);
		sem_post(&ion_mutex);
		if (bytes == -1) // ERROR while polling for bundles
		{
			perror("Some ERROR ocurred while polling for bundles");
			return ((void*)-1);
		}
		else if (bytes > 0) // A Bundle has been received
		{	
			// printf("MT: Bundle received with size %d , content(no headers): %s \n", bytes, (char*)buffer+HEADER_SIZE);
			// printf("MT: Sent %d bytes as an IP+UDP packet through the TUN device\n", (int)write(tun_fd, buffer, bytes));
			write(tun_fd, buffer, bytes);
			memset(buffer, 0, IP_MAXPACKET);
		}
		else if (bytes == 0) // Bundle polling timeout: wait and poll again
		{
			continue;
		}
		else{
			printf("ERROR while polling\n");
			return ((void*)-1);
		}
	}
	return(NULL);
}

int main()
{
	printf("Creating virtual network device(TUN)...\n");
	int tun_fd;
	/* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *        IFF_NO_PI - Do not provide packet information
     */
    tun_fd = tun_alloc(IFF_TUN | IFF_NO_PI);
    if (tun_fd < 0) {
        perror("Allocating interface\n");
        exit(1);
    }
	// IP routes handling
    setup_routes(SAT);

	// Encenem el node ION
	char ionstart[150];
	strcpy(ionstart, "ionstart -I ");
	strcat(ionstart, SAT_ION_CONFIG_FILE);
	strcat(ionstart, " > /dev/null");
	printf("Starting ION node 170...\n");	
	system(ionstart); 

	// Preparem el signal handler
	struct sigaction action;
	action.sa_handler = handler;
	sigaction(SIGINT, &action, NULL);

	// Preparem el semàfor per que ION no peti
	sem_init(&ion_mutex, 0, 1);
	printf("---> Satellite node ready.\n-------------------Press CTRL+C to stop the node-------------------\n");
	
	// POSIX THREADS init
	pthread_t pthread_MO, pthread_MT;
	if (pthread_create(&pthread_MO, NULL, mo_thread, (void*)&tun_fd))
	{
		printf("ERROR creating the pthread\n");
		exit(-1);
	}
	if (pthread_create(&pthread_MT, NULL, mt_thread, (void*)&tun_fd))
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
		printf("An error ocurred inside pthread_MT\n");
	}

	// Tanquem el TUN
	printf("Closing down TUN...\n");
	close(tun_fd);
	printf("TUN device closed.\n");
	// Borrem el semàfor i tanquem ION-DTN
	printf("Stopping BP node...\n");
	sem_destroy(&ion_mutex);
	system("ionstop > /dev/null");
	printf("Bundle Node terminated.\n---> Satellite node was stopped successfully\n");
}
