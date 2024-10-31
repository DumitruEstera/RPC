#ifndef UTILS_H
#define UTILS_H

#include "cJSON.h"  
#define MAX_ARGS 10


// Tipuri de date suportate
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR
} DataType;

// Structură pentru parametri
typedef struct {
    DataType type;
    union {
        int int_val;
        float float_val;
        char string_val[256];
    } value;
} Parameter;

// Structură pentru cerere RPC
typedef struct {
    char function_name[64];
    int arg_count;
    Parameter args[MAX_ARGS];
} RPCRequest;

// Structură pentru răspuns RPC
typedef struct {
    Parameter result;
    int success;
    char error_message[256];
} RPCResponse;


//Functii pentru serilizare/deserilizare
char* serialize_request_json(RPCRequest* req);
void deserialize_request_json(const char* json_str, RPCRequest* req);
char* serialize_response_json(RPCResponse* resp);
void deserialize_response_json(const char* json_str, RPCResponse* resp);

#endif /* UTILS_H */