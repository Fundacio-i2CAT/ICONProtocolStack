![i2cat](https://ametic.es/sites/default/files//i2cat_w.png)
# ICON Testbed v1.0.1

Related Jira project: ICAT-3133

SPACE-COMMS, 2022

CODEOWNERS: oriol.fuste@i2cat.net, joan.ruizdeazua@i2cat.net

MANTAINERS: oriol.fuste@i2cat.net

## Short description
C code that implements the ICON project's protocol stack, adapted to the needs of the available hardware testbed.

In the context of Catalonia's New Space Strategy, the Space Communications department is developing and validating a new protocol stack capable of combining IoT standards with Delay Tolerant Network protocols for space communications. The project aims at providing a solution for massive Machine Type Communications(mMTC) over non-Terrestrial Networks(NTN), with the focus on fulfilling the store&forward and reliability needs of these networks.

## Status
The code currently supports the stack basic implementation as an isolated C thread integrated with Linux IP sockets as lower layer protocols.
Even though integration with other protocols is posible, APIs are not currently available. 
Upgrades will follow i2CAT's SpaceComms team needs and its projects.

## Pre-requisites
- Linux(>=5.15.0) based OS
- BPv7 installation of ION-DTN: https://sourceforge.net/projects/ion-dtn/
- Build tools: GNU make
- GNU C Library


## How to install, build & run
After cloning the repository to local: 
The folders `/user_node`, `/satellite`, `/bp_agent` and `/core_network` contain the main C code of their respective node type and the Makefile that will compile and generate the file to be executed. The `/utils` folder contains shared code that is used by all nodes. 

To correctly setup the code, the user must review and update the `/utils/testbed_setup.h` file with the addresses that match the networks and interfaces of the user's setup. It is important that all the nodes/platforms involved are using the same address values in `/utils/testbed_setup.h` to ensure correct working.

The default ION configuration files (found inside the `satellite` and `bp_agent` folders) should also be modified to match the addresses of the interfaces used. The default contact plan in these files is for 24h long visibility, it should be adapted to the desired scenario. ION also accepts contact plan updates during execution, for advanced usage check ION's documentation.

To compile the code of a node into an executable, after updating `testbed_setup.h` and the ION files, the user should run `make` inside the folder of the desired node type.
After successful compilation, the user can run the executable (e.g. `sudo ./user_node.out`) and the initilization of the node will start. To stop the node, once it is running properly, press `CNTRL+C` and the node shutdown process will start.


Notes:
*	All executable(.out) files should be executed with sudo privileges.

## Copyright
This code has been developed by Fundació Privada Internet i Innovació Digital a Catalunya (i2CAT).
i2CAT is a *non-profit research and innovation centre* that  promotes mission-driven knowledge to solve business challenges, co-create solutions with a transformative impact, empower citizens through open and participative digital social innovation with territorial capillarity, and promote pioneering and strategic initiatives.
i2CAT *aims to transfer* research project results to private companies in order to create social and economic impact via the out-licensing of intellectual property and the creation of spin-offs.
Find more information of i2CAT projects and IP rights at https://i2cat.net/tech-transfer/


## Licence
This code is licensed under the terms AGPLv3. Information about the license can be located at [link](https://www.gnu.org/licenses/agpl-3.0.html).
If you find that this license doesn't fit with your requirements regarding the use, distribution or redistribution of our code for your specific work, please, don’t hesitate to contact the intellectual property managers in i2CAT at the following address: techtransfer@i2cat.net

