// Program:	server, IPK projekt 2
// Autor: 	Adam Ormandy (xorman00@stud.fit.vutbr.cz)
// Obsahuje funkcie pre lepsiu pracu z protokolom

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

// Lebo eva nepozna to_string
string my_to_string(int cislo)
{    
	ostringstream convert;
	convert << cislo;
	return convert.str();
}


// Funkcia na poslanie spravy
int send_message(string* message,int socket)
{
	if (send(socket, (*message).data(), (*message).size(), 0) <= 0)
	{
		perror("ERROR send_message");
		return 1;
	}
	return 0;
}


// Posklada spravu pre odpoved serveru na download poziadavku
string server_download_message(string nazov_suboru,string telo)
{
	string medzi="200\t"+nazov_suboru+'\t'+my_to_string(message_lenght(nazov_suboru,&telo))+'\t';
	medzi.append(telo.data(),telo.size());
	return medzi;
}



// Posklada spravu pre upload suboru
string upload_message(string nazov_suboru,string telo)
{
	string medzi="201\t"+nazov_suboru+'\t'+my_to_string(message_lenght(nazov_suboru,&telo))+'\t';
	medzi.append(telo.data(),telo.size());
	return medzi;
}


// Funkcia na parsovanie poziadavky
string download_message(string nazov_suboru)
{
	string medzi="202\t"+nazov_suboru+'\t';
	return medzi;
}


// Vypocita celkovu dlzku spravy pre upload
int message_lenght(string nazov_suboru,string *telo)
{
	int dlzka=strlen(KOD_UPLOAD_STRING)+strlen("\t\t\t")+nazov_suboru.size();
	dlzka+=(*telo).size();
	return dlzka+(my_to_string(dlzka)).size();
}


// Funckia pre clienta, upload
int client_upload(int socket,string nazov_suboru)
{
	// Nacitanie binarneho suboru
	string subor_data;
	// Nacitanie binarneho suboru
	if(binary_read(nazov_suboru,&subor_data)) 
	{
		perror("client_upload, SUBOR NEEXISTUJE ALEBO NEJDE NACITAT");
		return 2;
	}
	// Zostrojenie poziadavky
	string message=upload_message(nazov_suboru,subor_data);
	// Poslanie poziadavky
	if(send_message(&message,socket))
	{
		perror("client_upload");
		return 1;
	}
	return 0;
}


// Funckia pre clienta, download
int client_download(int socket,string nazov_suboru)
{
	// Zostrojenie poziadavky
	string message=download_message(nazov_suboru);
	// Poslanie poziadavky
	if(send_message(&message,socket))
	{
		perror("client_download, download message");
		return 1;
	}

	// Stiahnutie suboru
	char buffer[BUFFER_SIZE];
	bool nespracovana_poziadavka=true;
	int konec_hlavicky;
	string sprava;
	sprava.clear();


	while(nespracovana_poziadavka)
	{
		// Uz mame aspon hlavicku
		smatch odpoved;
		if(regex_search(sprava,odpoved,regex("200\t([^\t]+)\t([0-9]+)\t")))
		{
			konec_hlavicky=((string)odpoved[0]).size();
			nazov_suboru=odpoved[1];
			int velkost_poziadavky=atoi(((string)odpoved[2]).c_str());
			if(velkost_poziadavky==0)
			{
				perror("client_download, velkost suboru 0");
				errno = EXIT_FAILURE;
				return 1;			
			}

			// Nacitanie pokracovanie spravy
			while(sprava.size()<velkost_poziadavky)
			{
				int nacitane=0;
				bzero(buffer,BUFFER_SIZE);
				if ((nacitane=recv(socket, buffer, BUFFER_SIZE-1, 0)) < 0)
				{
					perror("client_download, linka");
					return 1;
				}
				sprava.append(buffer,nacitane);
			}

			//Nacitana cela sprava
			nespracovana_poziadavka=false;
		}
		// Server nerozumel poziadavke
		if(regex_search(sprava,regex("401")))
		{
			cout << "CLIENT: server nerozumel poziadavke" << endl;
			return 1;
		}
		// Ak server subor nenasiel
		if(regex_search(sprava,regex("402")))
		{
			cout << "CLIENT: server nenasiel subor "<< nazov_suboru << endl;
			return 1;
		}
		// Ak este nemame hlavicku, skusime donacitat
		else
		{
			int nacitane=0;
			bzero(buffer,BUFFER_SIZE);
			if ((nacitane=recv(socket, buffer, BUFFER_SIZE-1, 0)) < 0)
			{
				perror("client_download, linka");
				return 1;
			}
			sprava.append(buffer,nacitane);
		}
	}

	// Odstrihnutie hlavicky
	string subor_data = sprava.substr(konec_hlavicky);

	// Vypis suboru
	if( binary_write(nazov_suboru,subor_data) )
	{
		perror("client_download, output");
		return 2;
	}
}


// Funckia pre server, upload
// uploadovanie suboru na server
int server_upload(int socket,string doteraz_nacitana_sprava)
{
	char buffer[BUFFER_SIZE];
	bool nespracovana_poziadavka=true;
	string nazov_suboru;
	int konec_hlavicky;

	while(nespracovana_poziadavka)
	{
		// Uz mame aspon hlavicku
		smatch odpoved;
		if(regex_search(doteraz_nacitana_sprava,odpoved,regex("[0-9]{3}\t([^\t]+)\t([0-9]+)\t")))
		{
			konec_hlavicky=((string)odpoved[0]).size();
			nazov_suboru=odpoved[1];
			int velkost_poziadavky=atoi(((string)odpoved[2]).c_str());
			if(velkost_poziadavky==0)
			{
				perror("server_upload, zla velkost suboru");
				return 1;			
			}

			// Nacitanie pokracovanie spravy
			while(doteraz_nacitana_sprava.size()<velkost_poziadavky)
			{
				int nacitane=0;
				bzero(buffer,BUFFER_SIZE);
				if ((nacitane=recv(socket, buffer, BUFFER_SIZE-1, 0)) < 0)
				{
					perror("server_upload, linka");
					return 1;
				}
				doteraz_nacitana_sprava.append(buffer,nacitane);
			}

			//Nacitana cela sprava
			nespracovana_poziadavka=false;
		}
		// Ak este nemame hlavicku, skusime donacitat
		else
		{
			int nacitane=0;
			bzero(buffer,BUFFER_SIZE);
			if (nacitane=recv(socket, buffer, BUFFER_SIZE-1, 0) < 0)
			{
				perror("server_upload, linka");
				return 1;
			}
			doteraz_nacitana_sprava.append(buffer,nacitane);
		}
	}

	// Odstrihnutie hlavicky
	string subor_data = doteraz_nacitana_sprava.substr(konec_hlavicky);
	if( binary_write(nazov_suboru,subor_data) )
	{
		perror("server_upload, output");
		return 2;
	}

	return 0;
}


// Funckia pre server, download
// downloadovanie suboru zo serveru
int server_download(int socket,string doteraz_nacitana_sprava)
{
	char buffer[BUFFER_SIZE];
	bool nespracovana_poziadavka=true;
	string nazov_suboru;

	while(nespracovana_poziadavka)
	{
		// Uz mame aspon hlavicku
		smatch odpoved;
		if(regex_search(doteraz_nacitana_sprava,odpoved,regex("202\t([^\t]+)\t")))
		{
			// Vytiahneme informacie z hlavicky
			nazov_suboru=odpoved[1];

			//Nacitana cela sprava
			nespracovana_poziadavka=false;
		}
		// Ak este nemame hlavicku, skusime donacitat
		else
		{
			int nacitane=0;
			bzero(buffer,BUFFER_SIZE);
			if (nacitane=recv(socket, buffer, BUFFER_SIZE-1, 0) < 0)
			{
				perror("server_download, linka");
				return 1;
			}
			doteraz_nacitana_sprava.append(buffer,nacitane);
		}
	}

	// Nacitanie binarneho suboru
	string sprava;
	if(binary_read(nazov_suboru,&sprava)) 
	{
		perror("server_download, nejde nacitat");
		// Posleme chybovu spravu clientovy, aby necakal
		string sprava="402   ";
		if(send_message(&sprava,socket))
		{
			perror("server_download, nejde poslat 402");
		}
		return 2;
	}

	// Poslanie suboru klientovy
	string message=server_download_message(nazov_suboru,sprava);
	if(send_message(&message,socket))
	{
		return 1;
	}

	return 0;
}



// Nacitanie binarneho suboru
int binary_read(string nazov_suboru,string* obsah_suboru)
{
	extern string umiestnenie_programu;
	(*obsah_suboru).clear();
	ifstream subor ((umiestnenie_programu+nazov_suboru).c_str(), ios::binary);
	if(subor.is_open())
	{
		// Velkost nacitanej casti
		subor.seekg (0, ios::end);
		int length = subor.tellg();
		subor.seekg (0, ios::beg);
		char read_buffer[length];
		// Samotne nacitanie
		subor.read(read_buffer, length);
		(*obsah_suboru).append(read_buffer,length);
		subor.close();
	}
	else
	{
		if(errno==0)
			return 0;
		else
			return 2;
	}
	return 0;
}


// Ulozenie binarneho suboru
int binary_write(string nazov_suboru,string obsah_suboru)
{
	extern string umiestnenie_programu;
	// Vypis suboru
	ofstream subor ((umiestnenie_programu+nazov_suboru).c_str(), ios::out | ios::binary);
	if(subor.is_open())
	{
		subor.write (obsah_suboru.data(), obsah_suboru.size());
		subor.close();		
	}
	else
	{
		if(errno==0)
			return 0;
		else
			return 2;
	}
	return 0;
}

