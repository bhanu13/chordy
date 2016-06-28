#include "chord.h"

struct Peer {
	chord * c;
	Node * n;
	int peer_id;

	Peer(chord * chr, Node * N, int id)
	{
		c = chr;
		n = N;
		peer_id = id;
	}
};

string itos(int num)
{
    stringstream ss;
    ss << num;
    return ss.str();
};


chord::chord()
{
	//num_messages = 0;
	detect = 0;
	Node new_node;
	new_node.id = 0;
	new_node.alive = 1;

	for(int i = 0; i < M; i++)
	{
		new_node.FT[i] = 0;
	}

	for(int i = 0; i < pow(2,M); i++)
	{
		new_node.keys.push_back(i);
	}
	Peer * p = new Peer(this, NULL, new_node.id);
	pthread_t server_t;
	pthread_create(&server_t, NULL, create_server_callback, p);

	pthread_t failure_t;
	pthread_create(&failure_t, NULL, detect_failure_callback, this);	
	
	nodes.push_back(new_node);
	heart.push_back(1);
}

chord::~chord()
{
	return;
}

void * chord::detect_failure_callback(void * t)
{
	chord * C = ((chord * ) t);
	C->detect_failure();
	pthread_exit(NULL);
}

void chord::detect_failure()
{
	sleep(5);
	int id = 256;
	Peer * p = new Peer(this, NULL, id);
	pthread_t server_t;
	pthread_create(&server_t, NULL, create_server_callback, p);
	
	while(1)
	{
		if(detect == 1) 
		{
			for (size_t i = 0; i < nodes.size(); ++i)
			{
				heart[i] = 0;
				unicast_send(id, nodes[i].id, "S");
				usleep(1000);
			}
			// check if all heart beats received after 5 second
			sleep(5);
			
			for (size_t i = 0; i < heart.size(); ++i)
			{
				if(heart[i] == 0)
				{
					int dead_node = nodes[i].id;
					cout<<"Node "<<dead_node<<" is dead"<<endl;
					heart[i] = 1;
					heart.pop_back();
					cout<<"Telling everyone that "<<dead_node<<" died"<<endl;

					int	succ_id = nodes[find(dead_node)].FT[0];
					nodes.erase(std::find(nodes.begin(), nodes.end(), nodes[i]));
					request_updateFT(id);
					// Asking the successor to update the keys
					unicast_send(id, succ_id, "uk");
				}

			}
		}	

	}
}

void * chord::get_command(void * t)
{
	while(1)
	{
		cout<<"Enter a command: ";
		string cmd;
		getline(cin, cmd);

		int idx_join = cmd.find("join");
		int idx_find = cmd.find("find");
		int idx_show = cmd.find("show");
		int idx_show_all = cmd.find("all");
		int idx_crash = cmd.find("crash");
		int idx_close = cmd.find("exit");

		int idx_test = cmd.find("print");	// Prints all nodes Finger Table
		int idx_nodes = cmd.find("nodes");	// Prints all nodes
		int idx_pred = cmd.find("pred");	// Prints the predecessor
		int idx_rep = cmd.find("rep");		// A list of the replica keys of the predecessor
		int idx_detect = cmd.find("detect");

		if(idx_join >= 0)
		{
			int id_idx = cmd.find_first_of(NUMBER);
			char * id_ = &cmd[id_idx];
			int node_id = atoi(id_);
			cout<<"ID: "<<node_id<<endl;
			join(node_id);
		}
		else if(idx_find >= 0)
		{
			int node_idx = cmd.find_first_of(NUMBER);
			int space_id = (cmd.substr(node_idx)).find_first_of(" ");
			int key_idx = space_id + 1;
			int node_id = atoi(&(cmd[node_idx]));

			if(!nodeExists(node_id))
			{
				cout<<"No node for id: "<<node_id<<" found"<<endl;
				continue;
			}

			int key = atoi(&((cmd.substr(node_idx))[key_idx]));

			// cout<<"Finding from Node: "<<node_id<<", key: "<<key<<endl;
			find(node_id, key);
		}
		else if(idx_crash >= 0)
		{
			int node_idx = cmd.find_first_of(NUMBER);
			int node_id = atoi(&(cmd[node_idx]));
			// cout<<"ID: "<<node_id<<endl;

			crash(node_id);
		}
		else if(idx_close >= 0)
		{
			break;
		}
		else if(idx_show_all >= 0)
		{
			show_all();
		}
		else if(idx_show >= 0)
		{
			int node_idx = cmd.find_first_of(NUMBER);
			int node_id = atoi(&(cmd[node_idx]));
			// cout<<"ID: "<<node_id<<endl;
			show(node_id);
		}
		else if(idx_test >= 0)
		{
			for(size_t i = 0; i < nodes.size(); i++)
			{
				print_node(i);
			}
		}
		else if(idx_nodes >= 0)
		{
			// Print all nodes in the chord
			for(size_t i = 0; i < nodes.size(); i++)
			{
				cout<<nodes[i].id<<" ";
			}	
			cout<<endl;
		}
		else if(idx_pred >= 0)
		{
			int node_idx = cmd.find_first_of(NUMBER);
			int node_id = atoi(&(cmd[node_idx]));
			cout<<"Pred of "<<node_id<<" is "<<nodes[findpredidx(node_id)].id<<endl;
		}
		else if(idx_rep >= 0)
		{
			int node_idx = cmd.find_first_of(NUMBER);
			int node_id = atoi(&(cmd[node_idx]));
			// cout<<"ID: "<<node_id<<endl;
			showrep(node_id);
		}
		else if(idx_detect >= 0)
		{
			cout<<"Beginning Failure Detection"<<endl;
			detect = 1 - detect;
		}
		else
		{
			cout<<"Invalid Command"<<endl;
		}

	}
	return NULL;
}


void chord::print_node(int idx)
{
	cout<<"At index: "<<idx<<", Node ID: "<<nodes[idx].id<<endl;

	for(int i = 0; i < M; i++)
	{
		cout<<"FT["<<i<<"] = "<<nodes[idx].FT[i]<<" ";
	}
	cout<<endl;
}

bool chord::nodeExists(int id)
{
	for(size_t i = 0; i<nodes.size(); i++)
	{
		if(nodes[i].id == id)
			return true;		
	}
	return false;
}

bool chord::isPred(int node, int dest_id)
{
	return (nodes[findpredidx(node)].id == dest_id);
}

int chord::findpredidx(int id)
{
	int pred_idx = nodes.size() - 1;
	for(size_t i = 0; i<nodes.size(); i++)
	{
		if(nodes[i].id < id)
			pred_idx = i;
		else
			break;
	}
	return pred_idx;
}

int chord::findsuccidx(int id)
{
	int succ_idx = 0;
	for(size_t i = 0; i<nodes.size(); i++)
	{
		if(nodes[i].id > id)
		{
			succ_idx = i;
			break;
		}
	}
	return succ_idx;
}

int chord::calc_ft_entry(int idx, int node_id)
{
	int peer = findfirstpeer((node_id + (int)pow(2, idx)) % (int)pow(2, M));
	return peer;
}

// Helper Function for filling the Finger Table
int chord::findfirstpeer(int num)
{
	for(size_t i = 0; i<nodes.size(); i++)
	{
		if(nodes[i].id >= num)
			return nodes[i].id;		
	}
	//cout<<"Error: Find First Peer"<<endl;
	return nodes[0].id;
}

void chord::setupFT(Node * n)
{
	int id = n->id;
	for(size_t i = 0; i<M; i++)
	{
		n->FT[i] = calc_ft_entry(i, id);
	}
	return;
}

void chord::addNode(Node n)
{
	nodes.push_back(n);
	heart.push_back(1);
	sort(nodes.begin(), nodes.end());
	return;
}

int chord::find(int node_id)
{
	for(size_t i = 0; i<nodes.size(); i++)
	{
		if(nodes[i].id == node_id)
			return i;
	}
	return -1;
}

void chord::join(int id)
{
	if(nodeExists(id))
	{
		cout<<"Node already exists."<<endl;
		return;
	}
	Node new_node;
	new_node.id = id;
	new_node.alive = 1;

	setupFT(&new_node);

	addNode(new_node);

	Peer * p = new Peer(this, NULL, id);

	pthread_t listen_t;
	pthread_create(&listen_t, NULL, create_server_callback, p);

	request_updateFT_join(id);
	updateKeys(id);
	request_updateKeys(id);


	return;
}

void chord::request_updateFT(int id)
{
	// Option 1:
	for(size_t i = 0;i<nodes.size(); i++)
	{
		if(nodes[i].id != id)
		{
			unicast_send(id, nodes[i].id, "u");
		}
	}
}

void chord::request_updateFT_join(int id)
{

	//Option 2:
	//vector<int> v;
	for(int i = 0; i<M; i++)
	{
		// Not Correct Calculation.
		int num = (int)(id - pow(2, i));
		if(num < 0)
		{
			num = num + 256;
		}
		int pred_id = nodes[findpredidx_join(num) ].id;
		if(pred_id == id/* && (std::find(v.begin(), v.end(), pred_id) == v.end())*/)
		{
			pred_id = nodes[findpredidx(pred_id)].id;
		}
		string msg = itos(i) + " j";
		//cout<<msg<<endl;
		//num_messages++;
		unicast_send(id, pred_id, msg);
		//v.push_back(pred_id);
	}	
}

int chord::findpredidx_join(int id)
{
	int pred_idx = nodes.size() - 1;
	for(size_t i = 0; i<nodes.size(); i++)
	{
		if(nodes[i].id <= id)
			pred_idx = i;
		else
			break;
	}
	return pred_idx;
}



void chord::request_updateKeys(int id)
{
	int	succ_id = nodes[find(id)].FT[0];
	//num_messages++;
	unicast_send(id, succ_id, "uk");
	// Send Update Keys message for the replica keys also

	return;	
}

void chord::updateFT(int new_node_id, int curr_node_id, int idx)
{
	//cout<<"new_node_id: "<<new_node_id<<", curr_node_id: "<<curr_node_id<<", idx: "<<idx<<endl;
	if((nodes[find(curr_node_id)].FT[idx] != new_node_id) && /*(new_node_id == calc_ft_entry(idx, curr_node_id))*/((new_node_id >= curr_node_id && new_node_id <= calc_ft_entry(idx, curr_node_id)) || (new_node_id <= curr_node_id && (((curr_node_id + (int)pow(2, idx)) % 256) <= new_node_id))))
	{
		//setupFT(&(nodes[find(curr_node_id)]));
		nodes[find(curr_node_id)].FT[idx] = calc_ft_entry(idx, curr_node_id);
		int pred_id = nodes[findpredidx(curr_node_id)].id;
		if(pred_id == new_node_id)
		{
			pred_id = nodes[findpredidx(pred_id)].id;
		}
		unicast_send(new_node_id, pred_id, itos(idx) + " j");
		//num_messages++;
	}
	/*else if(new_node_id >= curr_node_id || new_node_id < nodes[find(curr_node_id)].FT[idx])
	{
		int pred_id = nodes[findpredidx(curr_node_id)].id;
		unicast_send(new_node_id, pred_id, itos(idx) + " j");
		num_messages++;
	}*/
}

void chord::updateKeys(int id)
{
	int	pred_id = nodes[findpredidx(id)].id;
	vector<int> v;
	for(int i = pred_id + 1; i<= id; i++)
		v.push_back(i);

	if(find(id) == 0)
	{
		v.clear();
		for(int i = 0; i<= id; i++)
			v.push_back(i);

		for(int i = pred_id + 1; i<= 255; i++)
			v.push_back(i);
	}


	nodes[find(id)].rep_keys = nodes[findpredidx(id)].keys;

	nodes[find(id)].keys = v;
	updateRepKeys(id);
	return;
}

void chord::updateRepKeys(int id)
{
	int succ_id = nodes[find(id)].FT[0];
	nodes[find(succ_id)].rep_keys = nodes[find(id)].keys;
	return;
}


void chord::find(int id, int key)
{
	int node_idx = find(id);
	for (size_t i = 0; i < (nodes[node_idx]).keys.size(); ++i)
	{
		if(key == (nodes[node_idx]).keys[i])
		{
			cout<<"Found key: "<<key<<" at Node "<<id<<endl;
			return;
		}
	}

	int largest_succ = -1;
  	vector<int> v(nodes[node_idx].FT, nodes[node_idx].FT + M);
  	sort(v.begin(), v.end());

	for (int i = 0; i < M; i++)
	{
		if(v[i] <= key)
			largest_succ = i;
		else
			break;
	}

	cout<<"Searching for "<<key<<" on node "<<id<<endl;

	if(largest_succ == -1 || ( (id < key) && (v[largest_succ] < id) ) )
	{
		unicast_send(id, nodes[node_idx].FT[0], "find @" + itos(key));
		//num_messages++;
	}
	else
	{
		unicast_send(id, v[largest_succ], "find @" + itos(key));
		//num_messages++;
	}
	
	return;
}


void chord::crash(int id)
{
	cout<<"Node "<<id<<" crashed"<<endl;

	Node new_node;
	new_node.id = id;
	nodes[find(id)].alive = 0;
	close(nodes[find(id)].socketfd);

	// nodes.erase(std::find(nodes.begin(), nodes.end(), new_node));
	return;
}

void chord::show_all()
{
	for(size_t i = 0; i<nodes.size(); i++)
	{
		cout<<"For Node: "<<nodes[i].id<<endl;
		print_node(i);
		show(nodes[i].id);
	}
	return;
}

void chord::show(int id)
{
	int node_idx = find(id);
	if(node_idx == -1) 
	{
		cout<<"No Such Node Exists with ID "<<id<<endl;
		return;
	}
	for(size_t i = 0; i<nodes[node_idx].keys.size(); i++)
	{
		cout<<nodes[node_idx].keys[i]<<", ";
	}
	cout<<endl;
	return;
}

void chord::showrep(int id)
{
	int node_idx = find(id);
	if(node_idx == -1) 
	{
		cout<<"No Such Node Exists with ID "<<id<<endl;
		return;
	}
	for(size_t i = 0; i<nodes[node_idx].rep_keys.size(); i++)
	{
		cout<<nodes[node_idx].rep_keys[i]<<", ";
	}
	cout<<endl;
	return;
}


//---------------------------------------------------------------------------------
// get sockaddr, IPv4 or IPv6:
void* chord::get_in_addr(struct sockaddr* sa)
{
	if(sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	else
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void chord::sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


int chord::server_response(chord * C, int id, int socket)
{
	char buffer[MAX_MSG_L];

	memset(buffer, 0, MAX_MSG_L);
	if(recv(socket, buffer, MAX_MSG_L, 0) == -1)
		return -1;

	string response = string(buffer);
	int id_idx = response.find_first_of(NUMBER);
	int space_idx = response.find_first_of(' ');
	int idx_update = response.find_first_of("u");
	int idx_updateK = response.find_first_of("k");
	int idx_find = response.find_first_of("find");
	int idx_join = response.find_first_of("j");

	int src_id = atoi(&(response[id_idx]));

	// For Failure Detector
	int idx_checkfailure = response.find_first_of("S");
	int idx_failure = response.find_first_of("R");	

	// Add Commands sent here...
	if(idx_updateK >= 0)
	{
		//cout<<"Updating keys on node "<<id<<endl;
		C->updateKeys(id);
	}
	else if(idx_update >= 0)
	{
		//cout<<"Updating FT on node "<<id<<endl;
		C->setupFT(&(C->nodes[C->find(id)]));
		//C->unicast_send(id, nodes[findpredidx(id)].id, "u");
	}
	else if(idx_find >= 0)
	{
		//cout<<"HERE!!"<<endl;
		int idx_atr = response.find_first_of('@');
		int key = atoi(&(response[idx_atr + 1]));
		C->find(id, key);
	}
	else if(idx_checkfailure >= 0)
	{
		//cout<<response<<endl;
		C->setupFT(&(C->nodes[C->find(id)]));
		C->unicast_send(id, FD_ID, "R");
	}
	else if(idx_failure >= 0)
	{
		C->heart[C->find(src_id)] = 1;
	}
	else if(idx_join >= 0)
	{
		int FT_idx = atoi(&(response[space_idx+1]));
		//cout<<"FT IDX: "<<FT_idx<<endl;
		C->updateFT(src_id, id, FT_idx);
	}
	else
	{
		cout<<response<<endl;
	}

	return 0;
}


void * chord::create_server_callback(void * t)
{
	Peer * p = (Peer *)t;
	chord * c = p->c;
	c->create_server(p->peer_id);
	delete p;
	p = NULL;
	pthread_exit(NULL);
}

void chord::create_server(int id)
{
	string port = itos(SERVER_PORT + id);
	char * port_ = &port[0u];

	create_listen_server((char *)port_, id, server_response);
	pthread_exit(NULL);
}

//NOTE: only root can bind ports < 1024.
int chord::create_listen_server(const char* port_to_bind, int id, int(*handleConnection)(chord *, int, int))
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	// struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if((rv = getaddrinfo(NULL, port_to_bind, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			perror("setsockopt");
			return -1;
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		return -1;
	}
	freeaddrinfo(servinfo); // all done with this structure

	if(listen(sockfd, 10) == -1) //queue up to 10 un-accept()ed connect()s
	{
		perror("listen");
		return -1;
	}

	// sa.sa_handler = sigchld_handler; // reap all dead processes
	// sigemptyset(&sa.sa_mask);
	// sa.sa_flags = SA_RESTART;
	// if(sigaction(SIGCHLD, &sa, NULL) == -1)
	// {
	// 	perror("sigaction");
	// 	return -1;
	// }

	printf("server: bound to port %s and waiting for connections...\n", port_to_bind);
	if(id != 256)
		nodes[find(id)].socketfd = sockfd;

	while(id == 256 || nodes[find(id)].alive == 1) // main accept() loop
	{
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
		if(new_fd == -1)
		{
			perror("accept");
			continue;
		}
		
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
		// printf("server: got connection from %s\n", s);

		// Might have to create a new thread to support receiving of multiple messages at the same time
		handleConnection(this, id, new_fd);

		close(new_fd);
	}
	close(sockfd);
	cout<<"Closing server "<<id<<endl;
	pthread_exit(NULL);
}

int delay_channel()
{
	return ((rand()%10) + 1);
}

void chord::unicast_send(int src_id, int dest_id, string msg)
{
	string port = itos(SERVER_PORT + dest_id);
	char *port_ = &port[0u];
	int socket = connect_to_server_(IP, port_);

	msg = itos(src_id) + " " + msg;

	if(socket != -1)
	{
		char *s_msg = &msg[0u];
		//cout<<"Unicasting "<<msg<<" to "<<dest_id<<endl;
		
		// Comment when testing
		if(src_id != 256 || dest_id != 256)
			usleep(delay_channel()*120000);				// SEND DELAYED
		
		if(send(socket, s_msg, msg.length(), 0) == -1)
		{
			perror("send");
		}
	}
	else
		cout<<"Unable to connect P"<<src_id<<" to P"<<dest_id<<endl;
	close(socket);
	return;
}


void * chord::get_data_callback(void * t)
{
	Peer * p = (Peer *)t;
	chord * c = p->c;
	c->get_data(p->n, p->peer_id);
	delete p;
	p = NULL;
	pthread_exit(NULL);
}

void chord::get_data(Node * n, int peer_id)
{
	// Differentiate between client and server
	int socket = -1;
	
	int bytes_received;
	char recv_msg[30];
	memset(recv_msg, 0, 30);

	while(socket != -1)
	{
		string cmd;
		if((bytes_received = recv(socket, recv_msg, 29, 0)) == -1)
		{
			perror("recv");
			pthread_exit(NULL);
		}
		if(recv_msg[0] != '\0')
		{
			cout<<"P"<<n->id<<": "<<recv_msg<<" from "<<peer_id<<endl;
			cmd = recv_msg;
			memset(recv_msg, 0, 30);
		}

		// CODE FOR DIFFERENT COMMANDS
		// if(cmd[0] == 'd')
		// {
		// 	for(int i = 0; i<NUM_ALPHA; i++)
		// 	{
		// 		char alphabet = (char)('a' + i);
		// 		cout<<string(&alphabet)<<" "<<vars[id][i]<<endl;
		// 	}
		// }
	}
	cout<<"Exiting the receiving thread at P"<<n->id<<endl;
	pthread_exit(NULL);
}

// Index of Source and ID for destination node
void chord::connect_to_peer(int node_idx, int dest_id)
{
	string port = itos(SERVER_PORT + dest_id);
	char *port_ = &port[0u];
	int server_socket = connect_to_server_(IP, port_);

	int src_id = nodes[node_idx].id;
	if(server_socket < 0)
	{
		cout<<"Unable to connect "<<src_id<<" to peer "<<dest_id<<endl;
		return;
	}

	cout<<"Connected P"<<src_id<< " to P"<<dest_id<<endl;
	
	Peer * p = new Peer(this, &nodes[node_idx], dest_id);

	pthread_t t_recv;
	pthread_create(&t_recv, NULL, get_data_callback, p);
	return;
}

// Open a new connection to a listening server.
int chord::connect_to_server_(const char* domain_or_ip, const char* port_string)
{
	struct addrinfo hints, *servinfo, *p;
	int rv, sockfd;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(domain_or_ip, port_string, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			// perror("client: socket");
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			// perror("client: connect");
			continue;
		}
		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return -1;
	}
	freeaddrinfo(servinfo); // all done with this structure
	
	return sockfd;
}
//--------------------------------------------------------------------------------------------------------------------------------
