#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "udp_socket.h"

int setup_udp_socket(char *ipaddr, int port)
{
    //Inicialitzar UDP socket
	int udpsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(udpsocket < 0)
		{
			perror("socket() error)");
			exit(-1);
		}
	printf("Socket initialized \n");

	//crear struct per guardar la info de la adreça IP
	struct sockaddr_in so_addr;
	so_addr.sin_family = AF_INET;
	so_addr.sin_addr.s_addr = inet_addr(ipaddr);
	so_addr.sin_port = htons(port);

	//enllaçar el raw socket a la adreça
	if (bind(udpsocket, (struct sockaddr*)&so_addr, sizeof(so_addr)) < -1)
		{
			perror("socket() error)");
			exit(-1);
		}
	else
		{
			printf("Bind succesfull \n");
		}
    return udpsocket;
}

int send_udp(int udpsocket, char *msg, size_t msglen, char *ipaddr, int port)
{
    // adreça destí
	struct sockaddr_in destiny_addr;
	destiny_addr.sin_family = AF_INET;
	destiny_addr.sin_addr.s_addr = inet_addr(ipaddr);
	destiny_addr.sin_port = htons(port);


	// Enviem packet UDP+IP
	int ret = sendto(udpsocket, msg, msglen, 0, (struct sockaddr*)&destiny_addr, sizeof(destiny_addr));
	if(ret < 0)
	{
		perror("socket() error)");
		exit(-1);
	}
	else
	{
		return ret;
	}
}

int receive_udp(int udpsocket, char *buffer, size_t bufflen)
{
        //recv() amb flags=0, omplirà el buffer amb el primer que li arribi de qualsevol adreça origen
		int rebuts = recv(udpsocket, buffer, bufflen, 0);
		if(rebuts < 0) 
		{
			perror("socket() error)");
			exit(-1);
		}
		else 
		{
			return rebuts;
		}
}

int close_udp_socket(int udpsocket)
{
    return close(udpsocket);
}
