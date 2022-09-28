
#include "testbed_setup.h"

void setup_routes(enum node_type ntype)
{
    char commands[800];
    switch (ntype)
    {
    case UN:
        // Define a route to the CN through the SAT
        strcpy(commands, "sudo ip route add ");
        strcat(commands, CN_NET);
        strcat(commands, " via ");
        strcat(commands, SAT_IP);
        break;

    case SAT:
        // Set TUN device up
        strcpy(commands, "sudo ip addr add ");
        strcat(commands, SAT_TUN_IP);
        strcat(commands, SAT_TUN_MASK);
        strcat(commands, " dev tun0 \nsudo ip link set tun0 up \nsudo ip route add ");
        // Define a route to the CN through the TUN(i.e. through the user space where the BP node is)
        strcat(commands, CN_NET);
        strcat(commands, " via ");
        strcat(commands, SAT_TUN_IP);
        // Route for the bundles
        strcat(commands, "\nsudo ip route add ");
        strcat(commands, BA_IP);
        strcat(commands, " via ");
        strcat(commands, SAT_IP_DL);
        // Activate ip forwarding in the kernel (it is deactivated by default)
        strcat(commands,"\nsudo sysctl -w net.ipv4.ip_forward=1 > /dev/null\n");
        break;

    case BA:
        // Set TUN device up
        strcpy(commands, "sudo ip addr add ");
        strcat(commands, BA_TUN_IP);
        strcat(commands, BA_TUN_MASK);
        strcat(commands, " dev tun0 \nsudo ip link set tun0 up \nsudo ip route add ");
        // Define a route to the UN through the TUN(i.e. through the user space where the BP node is)
        strcat(commands, UN_NET);
        strcat(commands, " via ");
        strcat(commands, BA_TUN_IP);
        // Activate ip forwarding in the kernel (it is deactivated by default)
        strcat(commands,"\nsudo sysctl -w net.ipv4.ip_forward=1 > /dev/null\n");
        break;

    case CN:
        // Define a route to the UN through the BA
        strcpy(commands, "sudo ip route add ");
        strcat(commands, UN_NET);
        strcat(commands, " via ");
        strcat(commands, BA_IP);
        break;

    default:
        perror("ERROR: Node type not recognised");
        exit(-1);
    }
    // Run the resulting commands which will set up the necessary ip routes of the node
    system(commands);
    memset(commands, '\0', sizeof(commands));
}