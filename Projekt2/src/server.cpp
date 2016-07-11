// Program:	server, IPK projekt 2
// Autor: 	Adam Ormandy (xorman00@stud.fit.vutbr.cz)

#include <iostream>
#include <stdio.h>
#include <regex> 
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fstream>
#include <signal.h>
#include <stdexcept>

using namespace std;

#include "funkcie.h"

string umiestnenie_programu;

// Vypise error
void error(const char* msq)
{
	perror(msq); 
	exit(EXIT_FAILURE);
}


// Spracovanie poziadavky
int poziadavka(int server_socket)
{
	// Zakladne struktury
	int nacitane=0;
	string answer;
	char buffer[BUFFER_SIZE];
	bzero(buffer,BUFFER_SIZE);
	bool nedokoncena_faza=true;


	// Nacitame poziadavku az pokym sa neda rozhodnut ci je to download alebo upload
	while(nedokoncena_faza)
	{

		if ((nacitane=recv(server_socket, buffer, BUFFER_SIZE-1, 0)) < 0)
		{
			return errno;
		}
		answer.append(buffer,nacitane);
		bzero(buffer,BUFFER_SIZE);

		// Ci je sa mozne uz rozhodnut
		// Lebo kod poziadavky je 3-miestny
		if(answer.size()>=3)
		{
			nedokoncena_faza=false;
			// Rozhodovanie
			if(regex_search(answer,regex("201\t")))
			{
				cout << "UPLOAD" << endl;
				server_upload(server_socket,answer);
			}
			else if(regex_search(answer,regex("202\t")))
			{
				cout << "DOWNLOAD" << endl;
				server_download(server_socket,answer);
			}
			else
			{
				perror("server_client, neznama poziadavka z clienta");
			}
		}
	}
	return errno;
}








int main(int argc, char* argv[])
{
	// Zakladne struktury
	int port; 
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	// ARGUMENTY
	if(argc==3)
	{
		if ((string)argv[1]=="-p")
		{
			port=atoi(argv[2]);
			if(port < 0 and port > 65535)
			{
				perror("Zle cislo portu");
				return 1;			
			}
		}
		else
		{
			perror("Neznamy argument");
			return 1;
		}
	}
	else
	{
		error("Ziadne argumenty");
		return 1;
	}

	// Ulozime si umiestnenie programu
	smatch odpoved;
	string medzi=argv[0];
	regex_search(medzi,odpoved,regex("(.+/)[^/]+$"));
	umiestnenie_programu=odpoved[1];



	// SOCKET
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) 
	{ 
		error("ERROR socket"); 
	}


	// BINDING
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	// Terajsia adresa servera
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	// samotny BIND   
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR binding");
	}

	// LISTEN
	// cakanie na spojenia
	listen(sockfd,5);

	// ACCEPT a spracovanie poziadavky
	// Aby sa ine ako hlavne skoncili
	bool si_hlavny_server=true;
	try
	{
		while(si_hlavny_server)
		{
			newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0)
			{
				error("ERROR accept");		
			}

			int pid = fork();
			if (pid == 0) 
			{
				// Nie je hlavne server
				si_hlavny_server=false;
				// Spracovanie poziadavky
				if(poziadavka(newsockfd))
				{
					close(newsockfd);
					close(sockfd);
					perror("server, poziadavka");
					return 1;
				}
				close(newsockfd);
				close(sockfd);
				return 0;
			}
			else 
			{
				if (pid == -1) 
				{
					perror("fork");
					close(newsockfd);
					close(sockfd);
					return 1;
				}
				close(newsockfd);
			}
		}		
	}
	catch(...)
	{
		close(sockfd);
		cout << "SERVER, UKONCUJEM" << endl;
	}
	return 0;
}