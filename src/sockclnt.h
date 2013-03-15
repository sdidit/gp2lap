#ifndef SOCKCLNT_H
#define SOCKCLNT_H

#define GP2LAP_ID 1000

void sockInit();
void sockExit();

// client will try and connect to the host 'server_name' on port 'port'.
// server name may be in xxx.xxx.xxx.xxx or in wobble.co.uk form
int connectToServer(char *server_name, unsigned short port);


// close sockets and tidy up
void disconnectFromServer(void);


// data to be sent is in a buffer pointed to by pData, the buffer contains size bytes
void sendDataPacket(void *pData, int size);


#endif	// SOCKCLNT_H
