#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "raw_socket.h"



int setup_raw_socket(char *ipaddr, int port)
{
	printf("Creating raw socket...\n");
	// creant raw socket (IP+UDP headers ja estaven inclosos al bundle rebut)
	int rawsocket = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if(rawsocket < 0)
	{
		perror("socket() error)");
		exit(-1);
	}
	printf("Socket initialized \n");

	// adreça des de on s'envia
	struct sockaddr_in so_addr;
	so_addr.sin_family = AF_INET;
	so_addr.sin_addr.s_addr = inet_addr(ipaddr);
	so_addr.sin_port = htons(port);

	// enllaçem socket a la adreça
	if (bind(rawsocket, (struct sockaddr*)&so_addr, sizeof(so_addr)) < -1)
	{
		perror("socket() error)");
		exit(-1);
	}
	else
	{
		printf("Bind succesfull \n");
		return rawsocket;
	}
}

int send_raw(int rawsocket, char *msg, size_t msglen, char *ipaddr, int port)
{

	// adreça destí (crec que no importa en realitat ja que no afegirem cap header, tot i això hauria de ser externa per a que no el routegi cap al loopback)
	struct sockaddr_in destiny_addr;
	destiny_addr.sin_family = AF_INET;
	destiny_addr.sin_addr.s_addr = inet_addr(ipaddr);
	destiny_addr.sin_port = htons(port);


	// Enviem packet raw, sense afegir headers(ja els porta inclosos)
	int ret = sendto(rawsocket, msg, msglen, 0, (struct sockaddr*)&destiny_addr, sizeof(destiny_addr));
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

int receive_raw(int rawsocket, char *buffer, size_t bufflen)
{
        //recv() amb flags=0, omplirà el buffer amb el primer que li arribi de qualsevol adreça origen
		int rebuts = recv(rawsocket, buffer, bufflen, 0);
		if(rebuts < 0) {
			perror("socket() error)");
			exit(-1);
		}
		else {
			return rebuts;
		}
}

int close_raw_socket(int rawsocket)
{
	return close(rawsocket);
}