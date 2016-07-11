// Program:	klient, IPK projekt 2
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

using namespace std;

#include "funkcie.h"


string umiestnenie_programu;

// Vypise error
void error(const char* msq)
{
	fprintf(stderr, "%s\n", msq);
	exit(EXIT_FAILURE);
}



//client    -­‐h    hostname    -­‐p    port    [-­‐d|u]    file_name
int main(int argc, char* argv[])
{
	// Hodnoty ktore pouzivame na ovladanie skriptu
	// Kopiruju argumeny
	bool download=false;
	bool upload=false;
	bool port_set=false;
	bool host_set=false;
	string file_name;
	string server_hostname;
	int port=-1;
	struct sockaddr_in server;
	struct hostent *host;

	// ARGUMENTY
	if(argc)
	{
		for(int i=1; i < argc; i++)
		{
			// Adresa servera
			if((string)argv[i]=="-h")
			{
				i++;
				if(i<argc)
				{
					server_hostname=(string)argv[i];
					host_set=true;
				}
				else
				{
					error("Zly argument -h");
				}
			}
			// Port
			else if ((string)argv[i]=="-p")
			{
				i++;
				if(i<argc)
				{
					port=atoi(argv[i]);
					port_set=true;
					if(port < 1 and port > 65535)
					{
						error("Zle cislo portu");				
					}
				}
				else
				{
					error("Zly argument -p");
				}				
			}
			// Download
			else if ((string)argv[i]=="-d")
			{
				i++;
				if(i<argc and not upload)
				{
					download=true;
					file_name=argv[i];
				}
				else
				{
					error("Zly argument -d");
				}
			}
			// Upload
			else if ((string)argv[i]=="-u")
			{
				i++;
				if(i<argc and not download)
				{
					upload=true;
					file_name=argv[i];
				}
				else
				{
					error("Zly argument -u");
				}
			}
			else
			{
				error("Neznamy argument");
			}

		}
	}
	else
	{
		error("Ziadne argumenty");
	}

	// Kontrola ci mame robit len jednu vec
	if(upload==download && download==true)
	{
		error("Naraz zapnuty prepinac -u a -d");
	}

	if(!port_set)
	{
		error("Nezadany port");
	}

	if(!host_set)
	{
		error("Nezadany host");
	}


	// Ulozime si umiestnenie programu
	smatch odpoved;
	string medzi=argv[0];
	regex_search(medzi,odpoved,regex("(.+/)[^/]+$"));
	umiestnenie_programu=odpoved[1];
	

	// ZISKANIE IP
	if ((host = gethostbyname(server_hostname.c_str())) == NULL) 
	{ 
		perror("CLIENT ERROR");
		return EXIT_FAILURE;
	}
	// Prenesieme data, aby sme mohli otvorit spojenie
	server.sin_family=AF_INET;
	server.sin_port=htons(port);
	memcpy(&(server.sin_addr), host->h_addr, host->h_length);

	// SOCKET
	int client_socket=socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket <= 0) 
    { 
		perror("CLIENT ERROR");
		return EXIT_FAILURE; 	
	}
	// CONNECT
	if (connect(client_socket,(struct sockaddr*)&server,sizeof(server)) < 0)
	{
		close(client_socket);
		perror("CLIENT ERROR");
		return EXIT_FAILURE;
	}


	// DOWNLOAD A UPLOAD
	if(upload)
	{
		cout << "CLIENT:UPLOAD" << endl;
		if(client_upload(client_socket,file_name))
		{
			// Nenastal crash, ale nejaky problem z protokolom
			if(errno!=0)
				perror("client_upload");	
		}
	}
	else if(download)
	{
		cout << "CLIENT:DOWNLOAD" << endl;
		if(client_download(client_socket,file_name))
		{
			// Nenastal crash, ale 402
			if(errno!=0)
				perror("client_download");	
		}
	}
	close(client_socket);
}

