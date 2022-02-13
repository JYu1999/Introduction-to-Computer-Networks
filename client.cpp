//107030032_cli
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <string>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
using namespace std;

const int MAX_ARGS = 3;
const int PORT_ARG = 2;
const int IP_ARG = 1;
const int MAXPORT = 11899;
const int MINPORT = 11800;

struct roundResult
{
	int tooHigh;
	int tooLow;
	int equal;
};

roundResult toNet(roundResult toConvert);
roundResult notNet(roundResult toConv);

roundResult recResult(int sock);

long receiveLong(int sock);

void sendLong(long num, int sock);



int main(int argc, char *argv[])
{

	int status;
	bool won = false;
	string vicMess;
	long roundCount = 0;
	long numGuess;
	int numActual;
	bool goodInput;
	long tooHigh, tooLow, equal;

	if (argc != MAX_ARGS)
	{
		cerr << "Invalid number of arguments. Please input port # for first arg. Now exiting program.";
		exit(-1);
	}

	unsigned short portNum = (unsigned short)strtoul(argv[PORT_ARG], NULL, 0);
	if (portNum > MAXPORT || portNum < MINPORT)
	{
		cerr << "This port is not assigned to this program. Please try again with " << endl
			 << "numbers that are between 11800 & 11899. Now exiting. ";
		exit(-1);
	}

	unsigned long servIP;
	status = inet_pton(AF_INET, argv[IP_ARG], &servIP);
	if (status <= 0)
		exit(-1);
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = servIP;
	servAddr.sin_port = htons(portNum);

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{
		cerr << "Error with socket" << endl;
		exit(-1);
	}

	status = connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr));
	if (status < 0)
	{
		cerr << "Error with connect" << endl;
		exit(-1);
	}

	cerr << "Connected! " << endl
		 << endl;
	while (true)
	{
		cerr << "-------------------" << endl;
		cerr << "Press [s] to start the game" << endl;
		cerr << "Press [q] to quit" << endl;
		cerr << "-------------------" << endl;
		char signal;
		cin >> signal;
		if (signal == 'q')
		{
			sendLong(1, sock);
			break;
		}
		else
			sendLong(0, sock);
		roundCount = 0;
		won = false;

		cerr << " Welcome. This is a number guessing game. You have to guess the value of a number(between 0 and 999)" << endl;
		cerr << "Good luck!" << endl
			 << endl;

		do
		{
			cerr << "This is turn " << (roundCount + 1) << endl;

			cerr << "Guess a number"
				 << ": ";
			goodInput = false;

			while (!goodInput)
			{
				if (cin >> numGuess && !(numGuess > 999 || numGuess < 0))
					goodInput = true;
				else
				{
					cerr << "Invalid Input. Input must be between 0-999(only integers)." << endl;
					cerr << "Try again: ";
					cin.clear();
					cin.ignore();
				}
			}

			sendLong(numGuess, sock);

			roundResult tempRes;
			tempRes.equal = 0;
			tempRes = recResult(sock);

			if (tempRes.equal == 1)
			{
				won = true;
			}

			if (!won)
			{
				cerr << endl
					 << "lower than " << tempRes.tooHigh << endl;
				cerr << "higher than " << tempRes.tooLow << endl;
				cerr << endl;
			}

			roundCount++;
		} while (!won);

		roundCount = receiveLong(sock);

		cerr << "Congratulations! You have won " << "in " << roundCount << " turns!" << endl;
	}

	status = close(sock);
	if (status < 0)
	{
		cerr << "Error with close" << endl;
		exit(-1);
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

void sendLong(long num, int sock)
{
	long temp = htonl(num);
	int bytesSent = send(sock, (void *)&temp, sizeof(long), 0);
	if (bytesSent != sizeof(long))
		exit(-1);
}

roundResult recResult(int sock)
{
	roundResult tempRes;
	roundResult *rPointer = &tempRes;
	int bytesLeft = sizeof(tempRes);
	while (bytesLeft > 0)
	{
		int bytesRecv = recv(sock, (void *)rPointer, sizeof(tempRes), 0);
		if (bytesRecv <= 0)
		{
			cerr << "Error receiving results. Now exiting program.";
			cin.get();
			exit(-1);
		}
		bytesLeft = bytesLeft - bytesRecv;
	}
	tempRes = *rPointer;
	tempRes = notNet(tempRes);
	return tempRes;
}

long receiveLong(int sock)
{
	int bytesLeft = sizeof(long);
	long networkInt;
	char *bp = (char *)&networkInt;

	while (bytesLeft > 0)
	{
		int bytesRecv = recv(sock, (void *)bp, bytesLeft, 0);
		if (bytesRecv <= 0)
		{
			break;
		}
		else
		{
			bytesLeft = bytesLeft - bytesRecv;
			bp = bp +
				 bytesRecv;
		}
	}
	networkInt = ntohl(networkInt);
	return networkInt;
}


