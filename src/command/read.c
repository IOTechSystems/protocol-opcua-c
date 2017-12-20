#include "read.h"
#include "common_client.h"

#include <stdio.h>

static void read(UA_Client *client, EdgeMessage *msg) {

  printf("[READ] Node to read :: %s\n", msg->request->nodeInfo->valueAlias);
  UA_Variant *val = UA_Variant_new();
  UA_StatusCode retVal = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, msg->request->nodeInfo->valueAlias), val);
  if  (retVal != UA_STATUSCODE_GOOD) {
    // send error callback;
    printf("Error in read node :: 0x%08x\n", retVal);
    return ;
  }

  printf("[READ] SUCCESS response received from server\n");
  EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
  if (response) {
    response->nodeInfo = msg->request->nodeInfo;
    response->requestId = msg->request->requestId;

    EdgeVersatility *versatility = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
    versatility->arrayLength = 0;
    versatility->isArray = false;
    versatility->value = val->data;

    if (val->type == &UA_TYPES[UA_TYPES_BOOLEAN])
      response->type = Boolean;
    else if(val->type == &UA_TYPES[UA_TYPES_INT16]) {
      response->type = Int16;
    } else if(val->type == &UA_TYPES[UA_TYPES_UINT16]) {
      response->type = UInt16;
    } else if(val->type == &UA_TYPES[UA_TYPES_INT32]) {
      response->type = Int32;
    } else if(val->type == &UA_TYPES[UA_TYPES_UINT32]) {
      response->type = UInt32;
    } else if(val->type == &UA_TYPES[UA_TYPES_INT64]) {
      response->type = Int64;
    } else if(val->type == &UA_TYPES[UA_TYPES_UINT64]) {
      response->type = UInt64;
    } else if(val->type == &UA_TYPES[UA_TYPES_FLOAT]) {
      response->type = Float;
    } else if(val->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
      response->type = Double;
    } else if(val->type == &UA_TYPES[UA_TYPES_STRING]) {
      UA_String str = *((UA_String*) val->data);
      versatility->value = (void*) str.data;
      response->type = String;
    } else if(val->type == &UA_TYPES[UA_TYPES_BYTE]) {
      response->type = Byte;
    } else if(val->type == &UA_TYPES[UA_TYPES_DATETIME]) {
      response->type = DateTime;
    }
    response->message = versatility;


    EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = msg->endpointInfo;
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->responseLength = 1;
    resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
    resultMsg->responses[0] =response;

    onResponseMessage(resultMsg);

    free(response); response = NULL;
    free(resultMsg); resultMsg = NULL;

    UA_Variant_delete(val);
  }
}


EdgeResult* executeRead(UA_Client *client, EdgeMessage *msg) {
  EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
  if (!client) {
    printf("Client handle Invalid\n");
    result->code = STATUS_ERROR;
    return result;
  }
  read(client, msg);
  result->code = STATUS_OK;
  return result;
}