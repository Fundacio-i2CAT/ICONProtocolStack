
int setup_udp_socket(char *ipaddr, int port);

int send_udp(int udpsocket, char *msg, size_t msglen, char *ipaddr, int port);

int receive_udp(int udpsocket, char *buffer, size_t bufflen);

int close_udp_socket(int udpsocket);
