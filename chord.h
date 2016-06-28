#ifndef _CLIENT_H
#define _CLIENT_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <cstdlib>
#include "math.h"
#include <sstream>
#include <vector>
#include <algorithm>

#define M 8
#define IP "localhost"
#define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define NUMBER "0123456789"
#define SERVER_PORT 7000
#define MAX_MSG_L 100
#define FD_ID 256

using namespace std;

struct Node {
	int id;
	int FT[M];				// Finger Table
	int socketfd;
	int alive;

	vector<int> keys;
	vector<int> rep_keys;

	bool operator < (const Node & n) const
	{
		return this->id < n.id;
	}

	bool operator == (const Node & n) const
	{
		return (this->id == n.id);
	}
};

class chord
{
public:
	chord();
	~chord();

	// Function Prototypes for all commands

	// Thread that takes in commands
	void * get_command(void * t);
	int calc_ft_entry(int idx, int node_id);

	void join(int id);
	void find(int node, int key);
	void crash(int node);
	void show_all();
	void show(int node);


	//--------Helpers--------------------

	bool nodeExists(int id);
	bool isPred(int node, int dest_id);
	int findpredidx(int id);
	int findsuccidx(int id);
	
	// Finger Table Functions
	void setupFT(Node * n);
	void updateFT(int new_node_id, int curr_node_id, int idx);


	void request_updateFT(int id);
	void request_updateFT_join(int id);
	int findpredidx_join(int id);
	void request_updateKeys(int id);
	void updateKeys(int id);
	int findfirstpeer(int num);

	// Join Helpers
	void addNode(Node n);
	
	// Find a node with the given id
	int find(int node_id);
	// Debugging Functions
	void print_node(int idx);

	void updateKeys();
	void updateRepKeys(int id);

	void showrep(int id);

	//-----------------------------------

	void* get_in_addr(struct sockaddr* sa);
	void sigchld_handler(int s);
	static int server_response(chord * C, int id, int socket);
	static void * create_server_callback(void * t);
	void create_server(int id);
	int create_listen_server(const char* port_to_bind, int id, int(*handleConnection)(chord *, int, int));
	void unicast_send(int src_id, int dest_id, string msg);


	static void * get_data_callback(void * t);
	void get_data(Node * n, int peer_id);
	void connect_to_peer(int node_idx, int dest_id);

	int connect_to_server_(const char* domain_or_ip, const char* port_string);

	// Failure Detector Functions
	static void * detect_failure_callback(void * t);
	void detect_failure();


	/* data */
	vector<Node> nodes;
	vector<int> heart;
	int detect;
	//int num_messages;
};


#endif
