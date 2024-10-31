#include "utils.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

// Convertește un Parameter în obiect JSON
cJSON* parameter_to_json(Parameter* param) {
    cJSON* json_param = cJSON_CreateObject();
    
    switch (param->type) {
        case TYPE_INT:
            cJSON_AddStringToObject(json_param, "type", "int");
            cJSON_AddNumberToObject(json_param, "value", param->value.int_val);
            break;
        case TYPE_FLOAT:
            cJSON_AddStringToObject(json_param, "type", "float");
            cJSON_AddNumberToObject(json_param, "value", param->value.float_val);
            break;
        case TYPE_CHAR:
            cJSON_AddStringToObject(json_param, "type", "string");
            cJSON_AddStringToObject(json_param, "value", param->value.string_val);
            break;
    }
    
    return json_param;
}

// Convertește un obiect JSON în Parameter
void json_to_parameter(cJSON* json_param, Parameter* param) {
    cJSON* type = cJSON_GetObjectItem(json_param, "type");
    cJSON* value = cJSON_GetObjectItem(json_param, "value");
    
    if (strcmp(type->valuestring, "int") == 0) {
        param->type = TYPE_INT;
        param->value.int_val = (int)value->valuedouble;
    }
    else if (strcmp(type->valuestring, "float") == 0) {
        param->type = TYPE_FLOAT;
        param->value.float_val = (float)value->valuedouble;
    }
    else if (strcmp(type->valuestring, "string") == 0) {
        param->type = TYPE_CHAR;
        strncpy(param->value.string_val, value->valuestring, 255);
        param->value.string_val[255] = '\0';
    }
}

// Serializează o cerere RPC în JSON
char* serialize_request_json(RPCRequest* req) {
    cJSON* root = cJSON_CreateObject();
    
    // Adaugă numele funcției
    cJSON_AddStringToObject(root, "function", req->function_name);
    
    // Adaugă argumentele
    cJSON* args = cJSON_CreateArray();
    for (int i = 0; i < req->arg_count; i++) {
        cJSON* param = parameter_to_json(&req->args[i]);
        cJSON_AddItemToArray(args, param);
    }
    cJSON_AddItemToObject(root, "args", args);
    
    // Convertește în string
    char* json_str = cJSON_Print(root);
    cJSON_Delete(root);
    
    return json_str;
}

// Deserializează o cerere RPC din JSON
void deserialize_request_json(const char* json_str, RPCRequest* req) {
    cJSON* root = cJSON_Parse(json_str);
    if (root == NULL) {
        printf("Eroare la parsarea JSON\n");
        return;
    }
    
    // Citește numele funcției
    cJSON* function = cJSON_GetObjectItem(root, "function");
    strncpy(req->function_name, function->valuestring, 63);
    req->function_name[63] = '\0';
    
    // Citește argumentele
    cJSON* args = cJSON_GetObjectItem(root, "args");
    req->arg_count = cJSON_GetArraySize(args);
    
    for (int i = 0; i < req->arg_count; i++) {
        cJSON* param = cJSON_GetArrayItem(args, i);
        json_to_parameter(param, &req->args[i]);
    }
    
    cJSON_Delete(root);
}

// Serializează un răspuns RPC în JSON
char* serialize_response_json(RPCResponse* resp) {
    cJSON* root = cJSON_CreateObject();
    
    // Adaugă statusul
    cJSON_AddBoolToObject(root, "success", resp->success);
    
    if (resp->success) {
        // Adaugă rezultatul
        cJSON* result = parameter_to_json(&resp->result);
        cJSON_AddItemToObject(root, "result", result);
    } else {
        // Adaugă mesajul de eroare
        cJSON_AddStringToObject(root, "error", resp->error_message);
    }
    
    // Convertește în string
    char* json_str = cJSON_Print(root);
    cJSON_Delete(root);
    
    return json_str;
}

// Deserializează un răspuns RPC din JSON
void deserialize_response_json(const char* json_str, RPCResponse* resp) {
    cJSON* root = cJSON_Parse(json_str);
    if (root == NULL) {
        printf("Eroare la parsarea JSON\n");
        return;
    }
    
    // Citește statusul
    cJSON* success = cJSON_GetObjectItem(root, "success");
    resp->success = success->valueint;
    
    if (resp->success) {
        // Citește rezultatul
        cJSON* result = cJSON_GetObjectItem(root, "result");
        json_to_parameter(result, &resp->result);
    } else {
        // Citește mesajul de eroare
        cJSON* error = cJSON_GetObjectItem(root, "error");
        strncpy(resp->error_message, error->valuestring, 255);
        resp->error_message[255] = '\0';
    }
    
    cJSON_Delete(root);
}