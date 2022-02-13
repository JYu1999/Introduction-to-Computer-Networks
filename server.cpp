#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sstream>
#include <time.h>
using namespace std;

const int MAX_ARGS = 2;
const int PORT_ARG = 1;
const int MAX_PENDING = 5;
const int MAXPORT = 11899;
const int MINPORT = 11800;

struct arg_t
{
	int sock;
	int roundCount;
};

struct roundResult
{
	int tooHigh;
	int tooLow;
	int equal;
};

long receiveLong(arg_t connInfo, bool &abort);
void sendLong(long num, arg_t connInfo);

void sendResult(roundResult result, arg_t connInfo);


roundResult notNet(roundResult toConv);
roundResult toNet(roundResult toConvert);

void *func(void *args_pa)
{

	arg_t *args_p;
	args_p = (arg_t *)args_pa;

	srand(time(NULL));
	args_p->roundCount = 0;
	long roundCount = 0;
	long actualNums;
	long numsGuess;
	long numHigh, numOn, numLow;
	bool won = false;
	roundResult result;
	roundResult *rPointer;
	bool exit = false;
	long wantToPlay;

	pthread_detach(pthread_self());

	while (true)
	{

		wantToPlay = receiveLong(*args_p, exit);
		if (wantToPlay == 1)
		{
			exit = true;
			break;
		}
		else
		{
			exit = false;
			won = false;
			roundCount = 0;

			actualNums = (rand() % 1000);
			cerr << "Num = " << actualNums << endl;

			while (!exit && !won)
			{
				cerr << endl
					 << endl;
				numHigh = 999;
				numOn = 0;
				numLow = 0;
				do
				{
					numsGuess = receiveLong(*args_p, exit);
					if (!exit)
					{
						cerr << "Received Guess: " << numsGuess << endl;
						if (numsGuess < actualNums)
							numLow = numsGuess;
						else if (numsGuess > actualNums)
							numHigh = numsGuess;
						else
							numOn = 1;
					}

					result.tooHigh = numHigh;
					result.tooLow = numLow;
					result.equal = numOn;

					sendResult(result, *args_p);

					roundCount++;

					if (numOn == 1)
					{
						won = true;
						cerr << "Correct Guess" << endl<<endl;
					}

				} while (!won);

				if (!exit)
				{
					sendLong(roundCount, *args_p);
				}
			}
		}
	}

	if (exit)
	{
		cerr << endl
			 << "User has left prematurely! " << endl;
		cerr << endl
			 << "Now awaiting a new client!" << endl;
	}
	cerr << "Close the sock" << endl;
	close(args_p->sock);

	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

	if (argc != MAX_ARGS)
	{
		cerr << "Invalid number of arguments. Please input IP address for first arg, then port # for "
			 << " second one. Now exiting program.";
		exit(-1);
	}

	unsigned short portNum = (unsigned short)strtoul(argv[PORT_ARG], NULL, 0);
	if (portNum > MAXPORT || portNum < MINPORT)
	{
		cerr << "This port is not assigned to this program. Please try again with " << endl
			 << "numbers that are between 11800 & 11899. Now exiting. ";

		exit(-1);
	}
	int status;
	int clientSock;

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{
		cerr << "Error with socket. Now exiting program. " << endl;
		close(sock);
		exit(-1);
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(portNum);

	status = bind(sock, (struct sockaddr *)&servAddr,
				  sizeof(servAddr));
	if (status < 0)
	{
		cerr << "Error with bind. Now exiting program. " << endl;
		close(sock);
		exit(-1);
	}

	status = listen(sock, MAX_PENDING);
	cerr << "Now listening for a new client to connect to server!" << endl;
	if (status < 0)
	{
		cerr << "Error with listen. Now exiting program. " << endl;
		close(sock);
		exit(-1);
	}
	
	while (true)
	{
		
		pthread_t tid;

		struct sockaddr_in clientAddr;
		socklen_t addrLen = sizeof(clientAddr);
		clientSock = accept(sock, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSock < 0)
		{
			cerr << "Error with accept. Now exiting program. " << endl;
			close(clientSock);
			exit(-1);
		}

		arg_t *args_p = new arg_t;
		args_p->sock = clientSock;

		status = pthread_create(&tid, NULL, func, (void *)args_p);
		if (status)
		{
			cerr << "Error creating threads, return code is " << status << ". Now exiting " << endl;
			close(clientSock);
			exit(-1);
		}
		cerr << "Client thread started." << endl;
	}
}

roundResult toNet(roundResult toConvert)
{
	toConvert.tooHigh = htonl(toConvert.tooHigh);
	toConvert.tooLow = htonl(toConvert.tooLow);
	toConvert.equal = htonl(toConvert.equal);

	return toConvert;
}

roundResult notNet(roundResult toConv)
{
	toConv.tooHigh = ntohl(toConv.tooHigh);
	toConv.tooLow = ntohl(toConv.tooLow);
	toConv.equal = ntohl(toConv.equal);

	return toConv;
}

void sendLong(long num, arg_t connInfo)
{
	long temp = htonl(num);
	int bytesSent = send(connInfo.sock, (void *)&temp, sizeof(long), 0);
	if (bytesSent != sizeof(long))
	{
		cerr << "Error sending long! Now exiting. ";
		close(connInfo.sock);
		exit(-1);
	}
}

long receiveLong(arg_t connInfo, bool &abort)
{
	int bytesLeft = sizeof(long);
	long networkInt;
	char *bp = (char *)&networkInt;

	while (bytesLeft > 0)
	{
		int bytesRecv = recv(connInfo.sock, (void *)bp, bytesLeft, 0);
		if (bytesRecv <= 0)
		{
			abort = true;
			break;
		}
		else
		{
			bytesLeft = bytesLeft - bytesRecv;
			bp = bp + bytesRecv;
		}
	}
	if (!abort)
	{
		networkInt = ntohl(networkInt);
		return networkInt;
	}
	else
		return 0;
}

void sendResult(roundResult result, arg_t connInfo)
{
	result = toNet(result);
	roundResult *rPointer;
	rPointer = &result;
	int bytesSent = send(connInfo.sock, (void *)rPointer, sizeof(result), 0);
	if (bytesSent != sizeof(result))
	{
		cerr << "Error sending results! Now exiting program.";
		close(connInfo.sock);
		exit(-1);
	}
}

