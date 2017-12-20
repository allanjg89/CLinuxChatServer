
/*
 * Server.h
 *
 *  Created on: May 12, 2017
 *      Author: root
 */

#ifndef SERVER_H_
#define SERVER_H_

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <errno.h>
	#include <string.h>
	#include <sys/types.h>
	#include <time.h>
	#include <pthread.h>
	#include <signal.h>
	#include "list.h"

	#define MYPORT 4950
	#define MAXBUFLEN 500
	#define MAXCLIENTQEUE 5
	#define USERNAMELEN 12
	#define INITIALCLIENTBUFFERLEN 200
	#define IPADDRLEN 20
	#define SERVERQUITCHAR 'q'
	#define SERVERNAME "SUPER SERVER"

	typedef struct _clientStruct{

		char ipAddr[IPADDRLEN];
		char * userName;
		int sockFD;

	}clientStruct;

	typedef struct _acceptClientStruct{
		int server_sock;
		int * totalNumClients;
		//list * mylist;

	} acceptClientStruct;



	void createServerSocket(int * server_sock);
	void populateSockAddrIn(struct sockaddr_in 	* serv_addr );
	void bindSocket(int server_sock, struct sockaddr_in * serv_addr);
	void startListening(int server_sock);
	void acceptClients(int server_sock, int * totalNumClients);
	void * acceptClientsFunctionality(void * accCLSTR_IN);
	void * connectionHandler(void *socket_desc);
	void recieveFromClient(int clientSock, char * userName);
	void removeNewLines(char * buffer, int len);
	void setCTRL_CHandling();
	void ctrl_cHandler(int signal);
	void flushTCPBuff(int clientsock);
	void closeSockets(void * clientStruct);
	void sendToAll (int sock, char * message);
	int min(int a, int b);
	int cmpFunc(void * value, void * clientStruct);
	int cleanUpThreads();
	clientStruct * createClientStruct(int client_sock, char * clientIPAddr);

#endif /* SERVER_H_ */

