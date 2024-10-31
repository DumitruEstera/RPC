#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include "utils.h"

#pragma region Sockets

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct servere
{
    int server_fd;
    int new_socket;
    struct sockaddr_in address;
    int opt;
    int addrlen;
    char buffer[BUFFER_SIZE];
}servere;


int suma(int a, int b){
    return a+b;
}

void init(struct servere *s){
    s->opt = 1;
    s->addrlen = sizeof(s->address);
    for(int i=0; i<BUFFER_SIZE; i++){
        s->buffer[i] = 0;
    }
    // Crearea socket-ului
    if ((s->server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    // Setarea opțiunilor socket-ului
    if (setsockopt(s->server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(s->opt), sizeof(s->opt))) {
        perror("Eroare la setarea opțiunilor socket");
        exit(EXIT_FAILURE);
    }

    // Configurarea adresei
    s->address.sin_family = AF_INET;
    s->address.sin_addr.s_addr = INADDR_ANY;
    s->address.sin_port = htons(PORT);

    // Legarea socket-ului la port
    if (bind(s->server_fd, (struct sockaddr *)&(s->address), sizeof(s->address)) < 0) {
        perror("Eroare la bind");
        exit(EXIT_FAILURE);
    }
    
    // Ascultarea pentru conexiuni
    if (listen(s->server_fd, 3) < 0) {
        perror("Eroare la listen");
        exit(EXIT_FAILURE);
    }
}

void receiveMessage(struct servere *s){
    printf("Așteptare conexiuni noi...\n");
        
    // Acceptarea unei conexiuni noi
    if ((s->new_socket = accept(s->server_fd, (struct sockaddr *)&(s->address), (socklen_t*)&(s->addrlen))) < 0) {
        perror("Eroare la accept");
        exit(EXIT_FAILURE);
    }
    
    // Obținerea adresei IP a clientului
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((s->address).sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Conexiune nouă de la %s:%d\n", client_ip, ntohs(s->address.sin_port));
    
    // Citirea datelor de la client
    int valread = read(s->new_socket, s->buffer, BUFFER_SIZE);
    printf("Client: %s\n", s->buffer);

    // Procesăm cererea
    RPCRequest *clientRequest = (RPCRequest*)malloc(sizeof(RPCRequest));
    RPCResponse *response = (RPCResponse*)malloc(sizeof(RPCResponse));
    deserialize_request_json(s->buffer, clientRequest);
    
    if(strcmp(clientRequest->function_name, "suma") == 0) {
        int rezultat = suma(clientRequest->args[0].value.int_val, 
                            clientRequest->args[1].value.int_val);
        
        response->success = 1;
        response->result.type = TYPE_INT;
        response->result.value.int_val = rezultat;
    } else {
        response->success = 0;
        strcpy(response->error_message, "Funcție necunoscută");
    }
    // Serializăm și trimitem răspunsul
    char* json_response = serialize_response_json(response);
    send(s->new_socket, json_response, strlen(json_response), 0);
    printf("Răspuns trimis către client: %s\n", json_response);
    
    // Eliberăm memoria
    free(json_response);
    free(clientRequest);
    free(response);
    // Închiderea conexiunii cu acest client
    close(s->new_socket);
}


#pragma endregion


int main() {
    struct servere *myServer = (struct servere*)malloc(sizeof(struct servere));

    init(myServer);
    
    printf("Serverul ascultă pe portul %d...\n", PORT);
    
    while(1) {
        receiveMessage(myServer);
    }
    
    return 0;
}