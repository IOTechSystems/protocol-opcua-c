#include "write.h"
#include "common_client.h"

#include <stdio.h>

static void write(UA_Client *client, EdgeMessage *msg) {

  printf("[WRITE] Node to write :: %s\n", msg->request->nodeInfo->valueAlias);
  UA_Variant *myVariant = UA_Variant_new();

  int type = (int)msg->request->type - 1;
  if (type == UA_TYPES_STRING) {
    UA_String val = UA_STRING_ALLOC((char*) msg->request->value);
    UA_Variant_setScalarCopy(myVariant, &val, &UA_TYPES[type]);
  } else {
    UA_Variant_setScalarCopy(myVariant, msg->request->value, &UA_TYPES[type]);
  }
  
  UA_StatusCode retVal = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, msg->request->nodeInfo->valueAlias), myVariant);
  if  (retVal != UA_STATUSCODE_GOOD) {
    // send error callback;
    printf("Error in read node :: 0x%08x\n", retVal);
    return ;
  }

  printf("[WRITE] SUCCESS response received from server\n");
  EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
  if (response) {
    response->nodeInfo = msg->request->nodeInfo;
    response->requestId = msg->request->requestId;
    response->message = NULL;
//    response->value = UA_STATUSCODE_GOOD;

    EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = msg->endpointInfo;
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->responseLength = 1;
    resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
    resultMsg->responses[0] =response;

    onResponseMessage(resultMsg);

    free(response); response = NULL;
    free(resultMsg); resultMsg = NULL;

    UA_Variant_delete(myVariant);
  }
}

EdgeResult *executeWrite(UA_Client *client, EdgeMessage *msg) {
  EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
  if (!client) {
    printf("Client handle Invalid\n");
    result->code = STATUS_ERROR;
    return result;
  }
  write(client, msg);
  result->code = STATUS_OK;
  return result;
}