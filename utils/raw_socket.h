
int setup_raw_socket(char *ipaddr, int port);

int send_raw(int rawsocket, char *msg, size_t msglen, char *ipaddr, int port);

int receive_raw(int rawsocket, char *buffer, size_t bufflen);

int close_raw_socket(int rawsocket);