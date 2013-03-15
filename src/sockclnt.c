
#include "sockclnt.h"
#include "podcar.h"
#include <tcp.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#undef byte
#include "basiclog.h"

#define MAXTRIES 5
 
static int conn_socket;
extern int h_errno;

void sockInit()
{
	sock_init();
}

void sockExit()
{
	sock_exit();
}

int connectToServer(char *server_name, unsigned short port)
{
	// client will try and connect to the host 'server_name' on port 'port'.
	// server name may be in xxx.xxx.xxx.xxx or in wobble.co.uk form
	unsigned int addr;
	struct sockaddr_in server;
	struct hostent *hp;
	int my_id = GP2LAP_ID;

	//
	// Attempt to detect if we should call gethostbyname() or
	// gethostbyaddr()

	if ((server_name[0]>='a' && server_name[0]<='z') || (server_name[0]>='A' && server_name[0]<='Z')) {
		/* server address is a name */
		hp = gethostbyname(server_name);
	} else {
		/* Convert nnn.nnn address to a usable one */
		addr = inet_addr(server_name);
		if ((signed int)addr == -1)	return 0;
		hp = gethostbyaddr((char*)&addr, 4, AF_INET);
	}
	if (hp == NULL) {
		sprintf(strbuf,"Client: Cannot resolve address [%s]: Error %d\n", server_name, h_errno);
		LogLine(strbuf);
		return 0;
	}

	//
	// Copy the resolved information into the sockaddr_in structure
	//
	memset(&server, 0, sizeof(server));
	memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(port);

	// Open a socket
	conn_socket = socket(AF_INET,SOCK_STREAM,0); 
	if (conn_socket < 0)
	{
		sprintf(strbuf,"Client: Error Opening socket: Error %d\n", errno);
		LogLine(strbuf);
		return 0;
	}

	sprintf(strbuf, "Client connecting to: %s\n",hp->h_name);
	LogLine(strbuf);
	if (connect(conn_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		sprintf(strbuf,"connect() failed: %d\n",errno);
		LogLine(strbuf);
		return 0;
	}

	// create non-blocking socket.
//	if(fcntl(conn_socket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
//	{
//		//LogLine("non blocking failed\n");
//		return 0;
//	}

	// send my ID to server
	sendDataPacket(&my_id, sizeof(my_id));

	return 1;
}

void disconnectFromServer(void)
{
	// close sockets and tidy up
	shutdown(conn_socket, 2);
}

void sendDataPacket(void *pData, int size)
{
	// data to be sent is in a buffer pointed to by pData, the buffer contains size bytes

	static int retval, tries, bytes_sent;

	// try and send the data ...
	// we might need several calls to send(..) to dispatch the whole data buffer.
	// we'll allow up to MAXTRIES.
	bytes_sent = 0;
	tries = 0;
	while ((bytes_sent < size) && (tries < MAXTRIES)) {
		retval = send(conn_socket, (char*)pData + bytes_sent, size - bytes_sent, 0);
		if (retval == SOCKET_ERROR) {
			break; // if there's an error, finish without fuss
		}
		bytes_sent += retval;
		tries ++;
	}
}

/*
// comment this out or delete it when you've understood how to use the functions
int main(int argc, char **argv) {

	PodCarStruct data;

	int i; // used in delay loop

	// initialise sockets
	sockInit();

	if(!connectToServer("puck", 5001))
	{
		fprintf(stderr, "Unable to connect to server\n");
		return 0;
	}

	data.time = 0.0;
	while (1)
	{
		// put some dummy data in CarStruct
		data.time++;

		// send data packet
		sendDataPacket(&data, sizeof(PodCarStruct));

		// now wait for a bit so we don't send billions of packets over network,
		// obviously we don't need to do this when integrated into gp2lap!
		for(i=0; i<11250000; i++);
	}

	disconnectFromServer();

	sockExit();

	return 1;
}
*/
