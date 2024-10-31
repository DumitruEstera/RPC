#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"


#pragma region ClientSockets

#define PORT 8080
#define BUFFER_SIZE 1024

// Structura pentru conexiune
typedef struct client{
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
}client;

void init(struct client *c){
    c->sock = 0;
    for(int i=0; i<BUFFER_SIZE; i++){
        c->buffer[i] = 0;
    }

   // Crearea socket-ului
    if ((c->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }
    
    // Configurarea adresei serverului
    c->serv_addr.sin_family = AF_INET;
    c->serv_addr.sin_port = htons(PORT);
    
    // Convertirea adresei IP din text în format binar
    if (inet_pton(AF_INET, "127.0.0.1", &(c->serv_addr).sin_addr) <= 0) {
        perror("Adresă invalidă");
        exit(EXIT_FAILURE);
    }
    
    // Conectarea la server
    if (connect(c->sock, (struct sockaddr *)&(c->serv_addr), sizeof(c->serv_addr)) < 0) {
        perror("Conexiunea a eșuat");
        exit(EXIT_FAILURE);
    }
    
    printf("Conectat la server\n");

}

void sendMessageToServer(struct client *c, const char* message){
     // Trimiterea unui mesaj către server
    send(c->sock, message, strlen(message), 0);
    printf("Mesaj trimis către server\n");
    
   // Primirea răspunsului de la server
    memset(c->buffer, 0, BUFFER_SIZE);  // Curățăm buffer-ul
    int valread = read(c->sock, c->buffer, BUFFER_SIZE);
    
    if (valread > 0) {
        printf("Răspuns primit de la server: %s\n", c->buffer);
        
        // Deserializăm răspunsul
        RPCResponse response;
        deserialize_response_json(c->buffer, &response);
        
        // Procesăm răspunsul
        if (response.success) {
            printf("Rezultatul operației: ");
            switch(response.result.type) {
                case TYPE_INT:
                    printf("%d\n", response.result.value.int_val);
                    break;
                case TYPE_FLOAT:
                    printf("%f\n", response.result.value.float_val);
                    break;
                case TYPE_CHAR:
                    printf("%s\n", response.result.value.string_val);
                    break;
            }
        } else {
            printf("Eroare: %s\n", response.error_message);
        }
    }
    
    // Închiderea conexiunii
    close(c->sock);
}

#pragma endregion



int main() {

    struct client *myClient = (struct client*)malloc(sizeof(struct client));
    init(myClient);

    RPCRequest* rpcRequest = (RPCRequest*)malloc(sizeof(RPCRequest));
    if(rpcRequest == NULL){
        exit(-1);
    }

    strcpy(rpcRequest->function_name, "suma");
    rpcRequest->arg_count = 2;
    rpcRequest->args[0].type = TYPE_INT;
    rpcRequest->args[0].value.int_val = 3;
    rpcRequest->args[1].type = TYPE_INT;
    rpcRequest->args[1].value.int_val = 4;
    char* jsonReq = serialize_request_json(rpcRequest);

    sendMessageToServer(myClient, jsonReq);

    return 0;
}