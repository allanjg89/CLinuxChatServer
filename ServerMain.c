
/*
 * ServerMain.c
 *
 *  Created on: May 12, 2017
 *      Author: root
 */

#include "Server.h"



void testList();

int main(int argc, char * argv[]){

	int server_sock 		= 0,
		totalNumClients 	= 0;

	struct sockaddr_in 	serv_addr 	= {0};


	createServerSocket(&server_sock);
	populateSockAddrIn(&serv_addr);
	bindSocket(server_sock, &serv_addr);
	startListening(server_sock);

	printf("\n\n[*]You have initialized %s!\n"
				"\tMy IP Address is: %s\n"
				"\tMy port is: %d\n"
				"\tMy Socket File Descriptor is: %d\n"
				"\tEnter the character \'%c\' at any time to exit....n\n\n",
				SERVERNAME, inet_ntoa(serv_addr.sin_addr),MYPORT, server_sock, SERVERQUITCHAR);


	acceptClients(server_sock,  &totalNumClients);

	return 0;
}



void testList(){

	clientStruct * client1 = createClientStruct(1, "111111111");
	clientStruct * client2 = createClientStruct(2, "222222222");
	clientStruct * client3 = createClientStruct(3, "333333333");
	clientStruct * temp = NULL;
	list * myList = NULL;
	int findVal1 = 2;
	int findVal2 = 3;
	int findVal3 = 1;

	myList = initializeList((void *) client1);
	addMember(&myList, (void *) client2);
	addMember(&myList, (void *) client3);

	temp = (clientStruct *) find_member(&myList, &(findVal1), cmpFunc);
	printf("%p\t%p\n", temp, myList);
	removeMember(&myList, &(findVal1), cmpFunc);//remove middle
	removeMember(&myList, &(findVal2), cmpFunc);//remove tail
	removeMember(&myList, &(findVal3), cmpFunc);//remove head

}



