/*  This header file contains configuration values that
 *  should be adapted to the hosting environment.
 *  Each macro's description can be found in 
 *  comment on top of it.
*/

// Some std common dependencies
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// Enum used to handle routing setup. Values: UN, SAT, BA or CN.
enum node_type{
    UN,     // User Node
    SAT,    // Satellite
    BA,     // Bundle Agent
    CN      // Core Network
};

// Sets up the IP routes of the machine according to the type of node and the addresses 
// specified in testbed_setup.h
void setup_routes(enum node_type ntype);
//-------------------------------------------------------------------------------------------------

// Total size of the combined UDP + IP headers
#define HEADER_SIZE (sizeof(struct iphdr) + sizeof(struct udphdr))

//----------------------------------------------------------------------------------------User Node

// IPv4 address to be used by the User Node
#define UN_IP "100.100.100.160"
// TCP/UDP port to be used by the User Node
#define UN_PORT 20582
// IPv4 address of the network where the User node is located
#define UN_NET "100.100.100.0/24"

//----------------------------------------------------------------------------------------Satellite

// IPv4 addres to be used by the satellite in the uplink network
#define SAT_IP "100.100.100.170"
// IPv4 addres to be used by the satellite in the downlink
// This address must be the one used by the BP node
#define SAT_IP_BP "200.200.200.170"
// Satellite's TUN IPv4 network address
#define SAT_TUN_IP "70.70.70.1"
// Satellite's TUN IPv4 network address mask in '/X' format
#define SAT_TUN_MASK "/24"
// File containing the configuration script for the ION Bundle Node of the Satellite.
// It's contents must comply with the addresses used in testbed_setup.h
#define SAT_ION_CONFIG_FILE "node170.ionconfig"
// BP endpoint ID of the satellite bundle node who handles the MO flow
#define SAT_MO_EID "ipn:170.1"
// BP endpoint ID of the satellite bundle node who handles the MT flow
#define SAT_MT_EID "ipn:170.2"

//-------------------------------------------------------------------------------------Bundle Agent

// IPv4 address to be used by the Bundle Agent
#define BA_IP "200.200.200.180"
// Bunde Agent's TUN IPv4 network address
#define BA_TUN_IP "80.80.80.1"
// Bunde Agent's TUN IPv4 network address mask in '/X' format
#define BA_TUN_MASK "/24"
// File containing the configuration script for the ION Bundle Node of the Satellite.
// It's contents must comply with the addresses used in testbed_setup.h
#define BA_ION_CONFIG_FILE "node180.ionconfig"
// BP endpoint ID of the BA bundle node who handles the MO flow
#define BA_MO_EID "ipn:180.1"
// BP endpoint ID of the BA bundle node who handles the MT flow
#define BA_MT_EID "ipn:180.2"

//-------------------------------------------------------------------------------------Core Network

// IPv4 address to be used by the Core Network
#define CN_IP "200.200.200.190"
// TCP/UDP port to be used by the Core Node
#define CN_PORT 20583
// IPv4 address of the network where the Core Network is located
#define CN_NET "200.200.200.0/24"
