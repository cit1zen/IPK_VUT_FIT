// Program:	Webklient, IPK projekt 1
// Autor: 	Adam Ormandy

#include <iostream>
#include <stdio.h>
#include <regex> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fstream>

#define BUFFER_SIZE 1024
#define HTTP_PORT 80

using namespace std;

// Vypise error
void error(const char* msq)
{
	perror(msq); 
	exit(EXIT_FAILURE);
}


// Posle message a nacita odpoved
int request_answer(string message,string* answer,sockaddr_in server)
{
	// Vytvorime socket
	int client_socket=socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket <= 0) 
    { 
		error("ERROR socket"); 	
	}
	// CONNECT
	if (connect(client_socket,(struct sockaddr*)&server,sizeof(server)) < 0)
	{
  		error("ERROR connect");
	}
	// Posleme poziadavok
	
	if (send(client_socket, message.c_str(), strlen(message.c_str()), 0) == -1)
	{
		// CLOSE
		close( client_socket );
		return errno;
	}
	// Nacitame odpoved
	// + gadzi oprava aby to aj eva rozchodila
	(*answer).clear();
	int nacitane=0;
	char buffer[BUFFER_SIZE];
	bzero(buffer,BUFFER_SIZE);
    while ((nacitane=recv(client_socket, buffer, BUFFER_SIZE-1, 0)) > 0) 
    {      
    	//buffer[nacitane]='\0';
        (*answer).append(buffer,nacitane);
        bzero(buffer,BUFFER_SIZE);
    }
    // CLOSE
    close( client_socket );
    return errno;
}



// Odstatenie chunkovosti prijatych dat
// Pomohol stackoverflow
string chunk_removal(string chunk_ridden) {
	
	string size;
	string chunkless;
	chunkless.clear();
	size_t chunk_size = 1;

	while (chunk_size > 0) {
		size = chunk_ridden.substr(0,chunk_ridden.find("\r\n"));
		chunk_size = strtol(size.c_str(), NULL, 16);
		chunkless.append(chunk_ridden.substr(chunk_ridden.find("\r\n") + 2, chunk_size ));
		int lel=chunk_ridden.find("\r\n") + 4 + chunk_size;
		chunk_ridden = chunk_ridden.substr(chunk_ridden.find("\r\n") + 4 + chunk_size);
	}
	return chunkless;
}



// MAIN
int main(int argc, char* argv[])
{
	// Zakladne struktury
	int port = HTTP_PORT; 
	struct sockaddr_in server;
	struct hostent *host;
	string server_hostname="www.fit.vutbr.cz";
	string server_filepath="";
	string nazov_vystupneho_suboru="index.html";
	string verzia_http="1.1";
	// String pre spravy ktore posielame
	string request;


	// Z argumentu ziskam co chceme stiahnut
	string argument;
	if(argc>1)
	{
		argument=argv[1];
	}
	for(int i=2; i < argc; i++)
	{
		argument+=" ";
    	argument+=argv[i];
	}

	// Parse host+port
	smatch najdene;
	regex_search (argument,najdene,regex("//([^/:]*)(:([0-9]+))?"));
	if(najdene.size()<2)
	{
		error("ARGUMENTY");
	}
	server_hostname=najdene[1];
	if(najdene.size()==4)
	{
		string dummy=najdene[3];
		int medzi=atoi(dummy.c_str());
		if(medzi!=0)
			port=medzi;	
	}
	
	// Parse cesta k suboru + meno suboru
	regex_search (argument,najdene,regex("//[^/]*/(.*)"));
	if(najdene.size()==2)
	{
		server_filepath=najdene[1];
		regex_search (server_filepath,najdene,regex("([^/]+)$"));
		if(najdene.size()==2)
			nazov_vystupneho_suboru=najdene[1];
	}


	// Nahradenie znakov
	// ~
	while(server_filepath.find('~',0)!=-1)
	{
		server_filepath = regex_replace (server_filepath,regex("~"),"%7E");
	}
	// space
	while(server_filepath.find(' ',0)!=-1)
	{
		server_filepath = regex_replace (server_filepath,regex("\\ "),"%20");
	}
	while(server_hostname.find('~',0)!=-1)
	{
		server_hostname = regex_replace (server_hostname,regex("~"),"%7E");
	}
	// space
	while(server_hostname.find(' ',0)!=-1)
	{
		server_hostname = regex_replace (server_hostname,regex("\\ "),"%20");
	}



	// Ziskame IP adresu
	if ((host = gethostbyname(server_hostname.c_str())) == NULL) 
	{ 
		error("ERROR DNS"); 
	}
	// Prenesieme data, aby sme mohli otvorit spojenie
	server.sin_family=AF_INET;
	server.sin_port=htons(port);
	memcpy(&(server.sin_addr), host->h_addr, host->h_length);

	#ifdef DEBUG
		fprintf(stderr,"DNS\n");
	#endif	





	/*
	While ktory sa ukonci ak nastane nejaka z podmienok:
	- uspecne nacitame co mame nacitat
	- prekrocime pocet presmerovani, >5
	- chyba z rodiny 4xx,5xx
	*/
	int presmerovania=0;
	int navratovy_kod;
	while(true)
	{
		string location;
		string head=" ";
		// HEAD poziadavka
		request="HEAD /" + server_filepath + " HTTP/"+verzia_http+"\r\nHost: " + server_hostname + "\r\nConnection: close \r\n\r\n";
		if (request_answer(request,&head,server))
		{
			error("ERROR HEAD");
		}	
		// Vyparsovanie udajov
		smatch odpoved;
		regex_search (head,odpoved,regex("([0-9.]+) ([0-9]+)"));
		// Verzia http
		verzia_http=odpoved[1];
		// Kod ktori nam http dalo
		string dummy=odpoved[2];
		navratovy_kod=atoi(dummy.c_str());


		// Nasli sme to co sme hladali
		if(navratovy_kod==200)
		{
			// Content-location
			regex_search (head,odpoved,regex("Content-Location: (.+)"));
			if(odpoved.size()==2)
				server_filepath=odpoved[1];
			break;
		}
		// Presmerovanie
		else if(navratovy_kod==301 || navratovy_kod==302)
		{
			// Vycerpali sme pocet presmerovani
			if(presmerovania>5)
			{
				break;
			}
			// Location
			regex_search (head,odpoved,regex("Location: (.+)"));

			argument=odpoved[1];
			regex_search (argument,najdene,regex("//([^/:]*)(:([0-9]+))?"));
			if(najdene.size()<2)
			{
				error("ARGUMENTY");
			}
			server_hostname=najdene[1];
			if(najdene.size()==4)
			{
				string dummy=najdene[3];
				int medzi=atoi(dummy.c_str());
			if(medzi!=0)
				port=medzi;	
			}
			// Parse cesta k suboru + meno suboru
			regex_search (argument,najdene,regex("www.[^/]*/(.*)"));
			if(najdene.size()==2)
			{
				server_filepath=najdene[1];
			}

			// Najdeme novu adresu serveru
			if ((host = gethostbyname(server_hostname.c_str())) == NULL) 
			{ 
				error("ERROR DNS"); 
			}
			memcpy(&(server.sin_addr), host->h_addr, host->h_length);
			presmerovania++;
		}
		// Nejaka chyba
		else
		{
			fprintf(stderr, "%d\n", navratovy_kod);
			break;
		}
	}




	// Po najdeny suboru ho nacitame
	if(navratovy_kod==200)
	{
		string get;
		request="GET /" + server_filepath + " HTTP/"+verzia_http+"\r\nHost: " + server_hostname + "\r\nConnection: close \r\n\r\n";
		if (request_answer(request,&get,server))
		{
			error("ERROR SUBOR");
		}


		// Odstranenie HEAD + zistenie ci sme chunked
		bool chunked=true;
		int pozicia_body=get.find("\r\n\r\n");
		{
			string head=get.substr (0,pozicia_body);
			if(get.find("chunked")==-1)
				chunked=false;
			get.erase(0,pozicia_body+4);
		}
		// Odstranenie chunkov
		string chunkless;
		if(chunked)
			chunkless=chunk_removal(get);
		else
			chunkless=get;



		// Ulozenie suboru
    	ofstream subor (nazov_vystupneho_suboru.c_str(), ios::out | ios::binary);
    	if(!subor)
    		error("ERROR OUTPUT");
    	subor.write (chunkless.c_str(), chunkless.size());
    	subor.close();
	}
	return EXIT_SUCCESS;
}









