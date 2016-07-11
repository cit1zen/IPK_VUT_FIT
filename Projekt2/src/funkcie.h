// Program:	server, IPK projekt 2
// Autor: 	Adam Ormandy (xorman00@stud.fit.vutbr.cz)
// Obsahuje funkcie pre lepsiu pracu z protokolom

#ifndef FUNKCIE_H
#define FUNKCIE_H


// Velkost bufferov
#define BUFFER_SIZE 1024


// OK
#define OK 200
#define OK_STRING "200"


// UPLOAD
#define KOD_UPLOAD 201
#define KOD_UPLOAD_STRING "201"


// DOWNLOAD
#define KOD_DOWNLOAD 202
#define KOD_DOWNLOAD_STRING "202"


// CHYBY
#define ZLA_POZIADAVKA 401
#define SERVER_DOWN_NEMA_SUBOR 402



// Funkcia na poslanie spravy
int send_message(string* message,int socket);

// Posklada spravu pre upload suboru
string upload_message(string nazov_suboru,string* telo);

// Funkcia na parsovanie poziadavky
string download_message(string nazov_suboru,string* telo);

// Nacitanie binarneho suboru
int binary_read(string nazov_suboru,string* obsah_suboru);

// Ulozenie binarneho suboru
int binary_write(string nazov_suboru,string obsah_suboru);

// Vypocita celkovu dlzku spravy
int message_lenght(string nazov_suboru,string* telo);

// Funckia pre clienta, upload
int client_upload(int socket,string nazov_suboru);

// Funckia pre clienta, download
int client_download(int socket,string nazov_suboru);

// Funckia pre server, upload
int server_upload(int socket,string doteraz_nacitana_sprava);

// Funckia pre server, download
int server_download(int socket,string doteraz_nacitana_sprava);

#endif