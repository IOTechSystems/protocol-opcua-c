#include "subscription.h"
#include "edge_utils.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct subscriptionInfo {
  EdgeMessage* msg;
  int subId;
  int monId;
} subscriptionInfo;

static int subscriptionCount = 0;
static bool subscription_thread_running = false;
static pthread_t subscription_thread;

static edgeMap *subscriptionList;

static keyValue getSubscriptionId(char *valueAlias) {
  edgeMapNode* temp = subscriptionList->head;
  while (temp != NULL) {
    if(!strcmp(temp->key, valueAlias)) {
      return temp->value;
    }
    temp = temp->next;
  }
  return NULL;
}

static void removeSubscriptionFromMap(char *valueAlias) {
  edgeMapNode *temp = subscriptionList->head;
  edgeMapNode *prev = NULL;
  while(temp != NULL) {
    if(!strcmp(temp->key, valueAlias)) {
      if (prev == NULL) {
        subscriptionList->head = temp->next;
      } else {
        prev->next = temp->next;
      }
      free (temp); temp = NULL;
      return ;
    }
    prev = temp;
    temp = temp->next;
  }
}

#ifdef TEST_SUBSCRIPTION_LIST
static void printMap() {
  edgeMapNode* temp = subscriptionList->head;
  while (temp != NULL) {
    temp = temp->next;
  }
}
#endif


void sendPublishRequest(UA_Client *client) {
  UA_Client_Subscriptions_manuallySendPublishRequest(client);
}

static void
monitoredItemHandler(UA_UInt32 monId, UA_DataValue *value, void *context) {
  if( value->hasValue) {
    printf("value is present, monId :: %d\n", monId);

    char *valueAlias = (char*) context;
    subscriptionInfo* subInfo =  (subscriptionInfo*) getSubscriptionId(valueAlias);
    //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

    EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
    if (response) {
      response->nodeInfo = subInfo->msg->request->nodeInfo;
      response->requestId = subInfo->msg->request->requestId;

      EdgeVersatility *versatility = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
      versatility->arrayLength = 0;
      versatility->isArray = false;
      versatility->value = value->value.data;

      if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN])
        response->type = Boolean;
      else if(value->value.type == &UA_TYPES[UA_TYPES_INT16]) {
        response->type = Int16;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_UINT16]) {
        response->type = UInt16;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_INT32]) {
        response->type = Int32;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_UINT32]) {
        response->type = UInt32;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_INT64]) {
        response->type = Int64;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_UINT64]) {
        response->type = UInt64;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_FLOAT]) {
        response->type = Float;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
        response->type = Double;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_STRING]) {
        UA_String str = *((UA_String*) value->value.data);
        versatility->value = (void*) str.data;
        response->type = String;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_BYTE]) {
        response->type = Byte;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_DATETIME]) {
        response->type = DateTime;
      }
      response->message = versatility;


      EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
      resultMsg->endpointInfo = subInfo->msg->endpointInfo;
      resultMsg->type = REPORT;
      resultMsg->responseLength = 1;
      resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
      resultMsg->responses[0] =response;

      onResponseMessage(resultMsg);

      free(response); response = NULL;
      free(resultMsg); resultMsg = NULL;
    }
  }
}

static void* subscription_thread_handler(void* ptr) {
  printf(">>>>>>>>>>>>>>>>>> subscription thread created <<<<<<<<<<<<<<<<<<<< \n");
  UA_Client *client = (UA_Client*) ptr;
  subscription_thread_running = true;
  while (subscription_thread_running) {
    sendPublishRequest(client);
    sleep(1);
  }
  printf(">>>>>>>>>>>>>>>>>> subscription thread destroyed <<<<<<<<<<<<<<<<<<<< \n");
  return NULL;
}

static void createSub(UA_Client *client, EdgeMessage *msg) {
  EdgeSubRequest *subReq = msg->request->subMsg;

  UA_UInt32 subId = 0;
  UA_SubscriptionSettings settings = {
    subReq->publishingInterval, /* .requestedPublishingInterval */
    subReq->lifetimeCount, /* .requestedLifetimeCount */
    subReq->maxKeepAliveCount, /* .requestedMaxKeepAliveCount */
    subReq->maxNotificationsPerPublish, /* .maxNotificationsPerPublish */
    subReq->publishingEnabled, /* .publishingEnabled */
    subReq->priority /* .priority */
  };

  /* Create a subscription */
  UA_Client_Subscriptions_new(client, settings, &subId);
  if(subId) {
    printf("Create subscription succeeded, id %u\n", subId);
  } else {
    // TODO: Handle Error
    printf("Error in creating subscription\n");
    return ;
  }

  EdgeMessage *msgCopy = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  memcpy(msgCopy, msg, sizeof *msg);

  /* Add a MonitoredItem */
  UA_NodeId monitorThis = UA_NODEID_STRING(1, msg->request->nodeInfo->valueAlias);
  UA_UInt32 monId = 0;
  UA_Client_Subscriptions_addMonitoredItem(client, subId, monitorThis, UA_ATTRIBUTEID_VALUE,
                                           &monitoredItemHandler, msgCopy->request->nodeInfo->valueAlias, &monId);
  if (monId) {
    printf("Monitoring 'the.answer', id %u\n", subId);
  } else {
    // TODO: Handle Error
    printf("Error in adding monitored item to subscription\n");
    return ;
  }

  if (0 == subscriptionCount) {
    /* initiate thread for manually sending publish request. */
    pthread_create(&subscription_thread, NULL, &subscription_thread_handler, (void*) client);    

    subscriptionInfo *subInfo = (subscriptionInfo*) malloc(sizeof(subscriptionInfo));
    subInfo->msg = msgCopy;
    subInfo->subId = subId;
    subInfo->monId = monId;

    /* Create subscription list */
    subscriptionList = createMap();
    if (subscriptionList)
      insertMapElement(subscriptionList, (keyValue) msg->request->nodeInfo->valueAlias, (keyValue) subInfo);

  }
  subscriptionCount++;

  /* The first publish request should return the initial value of the variable */
  UA_Client_Subscriptions_manuallySendPublishRequest(client);
}

static void deleteSub(UA_Client *client, EdgeMessage *msg) {

  subscriptionInfo* subInfo = (subscriptionInfo*) getSubscriptionId(msg->request->nodeInfo->valueAlias);
  //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

  UA_StatusCode retVal = UA_Client_Subscriptions_remove(client, subInfo->subId);

  if (UA_STATUSCODE_GOOD == retVal) {
    printf("subscription deleted successfully\n\n");
    removeSubscriptionFromMap(msg->request->nodeInfo->valueAlias);
  } else {
    printf("subscription delete error : 0x%08x\n\n", retVal);
  }

  subscriptionCount--;

  if (0 == subscriptionCount) {
    /* destroy the subscription thread */
    /* delete the subscription thread as there are no existing subscriptions request */
    subscription_thread_running = false;
    pthread_cancel(subscription_thread);
  }
}

static void modifySub(UA_Client *client, EdgeMessage *msg) {
  subscriptionInfo* subInfo =  (subscriptionInfo*) getSubscriptionId(msg->request->nodeInfo->valueAlias);
  //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

  EdgeSubRequest *subReq = msg->request->subMsg;

  UA_ModifySubscriptionRequest modifySubscriptionRequest;
  UA_ModifySubscriptionRequest_init(&modifySubscriptionRequest);
  modifySubscriptionRequest.subscriptionId = subInfo->subId;
  modifySubscriptionRequest.maxNotificationsPerPublish = subReq->maxNotificationsPerPublish;
  modifySubscriptionRequest.priority = subReq->priority;
  modifySubscriptionRequest.requestedLifetimeCount = subReq->lifetimeCount;
  modifySubscriptionRequest.requestedMaxKeepAliveCount = subReq->maxKeepAliveCount;
  modifySubscriptionRequest.requestedPublishingInterval = subReq->publishingInterval;

  UA_ModifySubscriptionResponse response = UA_Client_Service_modifySubscription(client, modifySubscriptionRequest);
  if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
    printf ("Error in modify subscription :: 0x%08x\n\n", response.responseHeader.serviceResult);
    return ;
  } else {
    printf("modify suhscription success\n\n");
  }
  UA_ModifySubscriptionRequest_deleteMembers(&modifySubscriptionRequest);

  // modifyMonitoredItems
  UA_ModifyMonitoredItemsRequest modifyMonitoredItemsRequest;
  UA_ModifyMonitoredItemsRequest_init(&modifyMonitoredItemsRequest);
  modifyMonitoredItemsRequest.subscriptionId = subInfo->subId;
  modifyMonitoredItemsRequest.itemsToModifySize = 1;
  modifyMonitoredItemsRequest.itemsToModify = UA_malloc(sizeof(UA_MonitoredItemModifyRequest));
  modifyMonitoredItemsRequest.itemsToModify[0].monitoredItemId = 1;   //monId;
//  UA_MonitoringParameters parameters = modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters;
  UA_MonitoringParameters_init(&modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters);
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).clientHandle = (UA_UInt32) 1;
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).discardOldest = true;
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).samplingInterval = subReq->samplingInterval;
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).queueSize = subReq->queueSize;
  UA_ModifyMonitoredItemsResponse modifyMonitoredItemsResponse;
  __UA_Client_Service(client, &modifyMonitoredItemsRequest, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST],
                &modifyMonitoredItemsResponse, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
  if (UA_STATUSCODE_GOOD == modifyMonitoredItemsResponse.responseHeader.serviceResult) {
    printf("modify monitored item success\n\n");
  } else {
    printf("modify monitored item failed\n\n");
    return ;
  }
  UA_ModifyMonitoredItemsRequest_deleteMembers(&modifyMonitoredItemsRequest);
  UA_ModifyMonitoredItemsResponse_deleteMembers(&modifyMonitoredItemsResponse);

  // setMonitoringMode
  UA_SetMonitoringModeRequest setMonitoringModeRequest;
  UA_SetMonitoringModeRequest_init(&setMonitoringModeRequest);
  setMonitoringModeRequest.subscriptionId = subInfo->subId;
  setMonitoringModeRequest.monitoredItemIdsSize = 1;
  setMonitoringModeRequest.monitoredItemIds = UA_malloc(sizeof(UA_UInt32));
  setMonitoringModeRequest.monitoredItemIds[0] = 1; //monId;
  setMonitoringModeRequest.monitoringMode = UA_MONITORINGMODE_REPORTING;
  UA_SetMonitoringModeResponse setMonitoringModeResponse;
  __UA_Client_Service(client, &setMonitoringModeRequest, &UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST],
                      &setMonitoringModeResponse, &UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
  if (UA_STATUSCODE_GOOD == setMonitoringModeResponse.responseHeader.serviceResult) {
    printf("set monitor mode success\n\n");
  } else {
    printf("set monitor mode failed\n\n");
    return ;
  }
  UA_SetMonitoringModeRequest_deleteMembers(&setMonitoringModeRequest);
  UA_SetMonitoringModeResponse_deleteMembers(&setMonitoringModeResponse);

  // setPublishingMode
  UA_SetPublishingModeRequest setPublishingModeRequest;
  UA_SetPublishingModeRequest_init(&setPublishingModeRequest);
  setPublishingModeRequest.subscriptionIdsSize = 1;
  setPublishingModeRequest.subscriptionIds = UA_malloc(sizeof(UA_UInt32));
  setPublishingModeRequest.subscriptionIds[0] = subInfo->subId;
  setPublishingModeRequest.publishingEnabled = subReq->publishingEnabled;     //UA_TRUE;
  UA_SetPublishingModeResponse setPublishingModeResponse;
  __UA_Client_Service(client, &setPublishingModeRequest, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST],
                      &setPublishingModeResponse, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
  if (UA_STATUSCODE_GOOD ==  setPublishingModeResponse.responseHeader.serviceResult) {
    printf("set publish mode success\n\n");
  } else {
    printf("set publish mode failed\n\n");
    return ;
  }
  UA_SetPublishingModeRequest_deleteMembers(&setPublishingModeRequest);
  UA_SetPublishingModeResponse_deleteMembers(&setPublishingModeResponse);


  UA_Client_Subscriptions_manuallySendPublishRequest(client);
}

EdgeResult* executeSub(UA_Client *client, EdgeMessage *msg) {
  printf("execute subscription\n");
  EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
  if (!client) {
    printf("client handle invalid!\n");
    result->code = STATUS_ERROR;
    return result;
  }

  EdgeRequest *req = msg->request;
  EdgeSubRequest *subReq = req->subMsg;
  if (subReq->subType == Edge_Create_Sub) {
    printf("create subscription\n");
    createSub(client, msg);
  } else if (subReq->subType == Edge_Modify_Sub) {
    printf("modify subscription\n");
    modifySub(client, msg);
  } else if (subReq->subType == Edge_Delete_Sub) {
    printf("delete subscription\n");
    deleteSub(client, msg);
  }

  result->code = STATUS_OK;
  return result;
}
