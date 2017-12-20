

/*
 * Server.c
 *
 *  Created on: May 12, 2017
 *      Author: root
 */

#include "Server.h"

list * mylist = NULL;

void createServerSocket(int * server_sock){
	int optVal = 1;

	if((*server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
	    perror("[!]Server socket creation failed.\n");
	    exit(1);
	}

	//setting the socket option to allow forcible binding to a port that is already in use.
	if(setsockopt(*server_sock, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof optVal) == -1){
		perror("[!]Setting socket option failed.\n");
		exit(1);
	}

	printf("[*]Successfully created server socket.\n");

}


void populateSockAddrIn(struct sockaddr_in 	* serv_addr ){
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_port = htons(MYPORT);
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	// zero the rest of the struct
	memset(&(serv_addr->sin_zero), '\0', 8);

}


void bindSocket(int server_sock, struct sockaddr_in * serv_addr){

	 if(bind(server_sock, (struct sockaddr *)serv_addr,
			 sizeof(struct sockaddr)) == -1){
	    perror("[!]Call to bind failed.\n");
	    exit(1);
	 }else
		 printf("[*]Call to bind succeeded.\n");

}


void startListening(int server_sock){

	 if(listen(server_sock, MAXCLIENTQEUE ) == -1){
	    perror("[!]Call to listen failed.\n");
	    exit(1);
	 }else
		 printf("[*]Call to listen succeeded.\n");

}




void acceptClients(int server_sock, int * totalNumClients){

	pthread_t acceptThread_id = 0;
	char serverInput = 'c';
	acceptClientStruct accCLSTR = {server_sock, totalNumClients};

	if(pthread_create( &acceptThread_id, NULL,  acceptClientsFunctionality, (void*) &accCLSTR) < 0) {
			printf("\t[-]Could not create thread for accepting..\n");
			exit(1);
	}


	while(serverInput != SERVERQUITCHAR){
		serverInput = getchar();
	}

	printf("Thank you for using %s! Good Bye.\n", SERVERNAME);

	//TODO:Tear down client linked list
	destroyList(&mylist, closeSockets);

	close(server_sock);


	return;
}

void * acceptClientsFunctionality(void * accCLSTR_IN){

	acceptClientStruct * accCLSTR = (acceptClientStruct *) accCLSTR_IN;
	int server_sock = accCLSTR->server_sock;
	int * totalNumClients = accCLSTR->totalNumClients;


	pthread_t thread_id 			= 0;
	int client_sock 				= 0;
	struct sockaddr_in 	client_addr = {0};
	char * clientIPAddr = NULL;
	clientStruct * clientStructPTR = NULL;


	socklen_t structLen = sizeof(struct sockaddr_in);

	printf("[*]Waiting for incoming connections...\n");

	while((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &structLen))) {
		clientIPAddr = inet_ntoa(client_addr.sin_addr);
		printf("\t[-]Connection accepted! Client at %s client socket: %d...\n", clientIPAddr, client_sock);

		clientStructPTR = createClientStruct(client_sock, clientIPAddr);

		if(pthread_create( &thread_id, NULL,  connectionHandler, (void*) clientStructPTR) < 0) {
			printf("\t[-]Could not create thread for client at %s...\n", clientIPAddr);
		}else{
			(*totalNumClients) +=1;

			//TODO: ADD Client  to linked list.
			if(*totalNumClients == 1){

				mylist = initializeList((void *) clientStructPTR);

			}else{

				addMember(&mylist, (void *) clientStructPTR);
			}

		}

	}

	return NULL;
}

void * connectionHandler(void * clientStructPTRIN) {

	char welcomeMessage[100] 	= {0};
	clientStruct * clientStructPTR = (clientStruct *)clientStructPTRIN;
	int clientsock = clientStructPTR->sockFD;

	char userName[USERNAMELEN] = {0};

	sprintf(welcomeMessage,
				"[*]Welcome to SuperServer Chat...\n"
				"\tPlease provide me with your user name (at most %d characters):",
				USERNAMELEN-3);

	clientStructPTR->userName = userName;


	//Initial message to the client
	send(clientsock, welcomeMessage, strlen(welcomeMessage), 0);
	recv(clientsock, userName, USERNAMELEN-3, 0);
	flushTCPBuff(clientsock);

	removeNewLines(userName, USERNAMELEN);

	//Username acknowledgment to the client
	sprintf(welcomeMessage,
			"[*]Happy to have you %s.\n"
			"\tWhat would you like to tell the group:\n >>", userName);

	send(clientsock, welcomeMessage, strlen(welcomeMessage), 0);

	//Recieve from the client
	recieveFromClient(clientsock, userName);

	//The client has disconnected
	printf("\n[*] Client with socket %d and IP Address %s has disconnected!\n",
			clientStructPTR->sockFD,clientStructPTR->ipAddr );
	close(clientsock);
	//TODO remove client from list and free(clientStructPTR);.
	removeMember(&mylist, (void * )&clientsock, cmpFunc);
	free(clientStructPTR);
	clientStructPTR = NULL;

	return NULL;
}


void recieveFromClient(int clientSock, char * userName){
	int amountRecieved 	= 0,
		totalRecieved 	= 0,
		currentBuffSize = INITIALCLIENTBUFFERLEN;

	 char messageToAll[INITIALCLIENTBUFFERLEN+USERNAMELEN+1];

	//char * tmpBuff = NULL;
	char * clientBuffer = calloc(INITIALCLIENTBUFFERLEN, 1);

	/* I am keeping a log of everything the clients send.
	 * Hence why I must realloc. I update the current writing
	 * position by adding total received to the buffer pointer.
	 */
	while ((amountRecieved = recv(clientSock, clientBuffer + totalRecieved, INITIALCLIENTBUFFERLEN, 0)) > 0) {

		//printf("%s:%s\n", userName, clientBuffer + totalRecieved);
		sprintf(messageToAll, "\n%s: %s\n", userName, clientBuffer + totalRecieved);
		sendToAll (clientSock, messageToAll);

		totalRecieved += amountRecieved;
	    currentBuffSize += INITIALCLIENTBUFFERLEN;
	    if((clientBuffer = realloc(clientBuffer, currentBuffSize)) == NULL){
	    	printf("[!]Realloc of client buffer failed for...\n"
	    			"\tUser name: %s\n"
	    			"\tSocket file descriptor: %d\n",
					userName, clientSock);
	    	return;
	    }
	}

	free(clientBuffer);
	clientBuffer = NULL;
	return;
}

clientStruct * createClientStruct(int client_sock, char * clientIPAddr){

	clientStruct * clientStructPTR = calloc(sizeof(clientStruct), 1);
	clientStructPTR->sockFD = client_sock;
	memcpy(clientStructPTR->ipAddr, clientIPAddr, min(strlen(clientIPAddr), IPADDRLEN));

	return clientStructPTR;

}

void closeSockets(void * clientStructIN){

	clientStruct * clientStruct_local =  (clientStruct *) clientStructIN;
	close(clientStruct_local->sockFD);

}

void flushTCPBuff(int clientsock){

	char temp = '0';
	int recRet = 1;

	while(recRet > 0)
		recRet = recv(clientsock, &temp, 1, MSG_DONTWAIT);

	return;

}



void removeNewLines(char * buffer, int len){

	int var;

	for (var = 0; var < len; ++var)
		if(*(buffer+var) == '\n') *(buffer + var) = '\0';

	return;

}

int min(int a, int b){

	if(a<b) return a;

	return b;
}


int cmpFunc(void * client_socket, void * client_Structure){

	int * client_sock = (int *) client_socket;
	clientStruct * client_Struct = (clientStruct *) client_Structure;

	if(* client_sock == client_Struct->sockFD){
		return 1;
	}

	return 0;


}

void sendToAll (int sock, char * message){

	list * curr = getHead(&mylist);
	clientStruct * currClient = NULL;

	do{
		currClient = (clientStruct *) curr->data;
		if(currClient->sockFD != sock && currClient->userName[0] != 0x00){
			send(currClient->sockFD, message, strlen(message), 0);
		}

	}while((curr = curr->next) != NULL);

	return;

}

