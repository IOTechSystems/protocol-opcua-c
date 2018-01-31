/* ****************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 = the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "edge_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "edge_utils"

edgeMap *createMap()
{
    edgeMap *map = (edgeMap *) malloc(sizeof(edgeMap));
    map->head = NULL;
    return map;
}

void insertMapElement(edgeMap *map, keyValue key, keyValue value)
{
    edgeMapNode *node = (edgeMapNode *) malloc(sizeof(edgeMapNode));
    VERIFY_NON_NULL_NR(node);
    node->key = key;
    node->value = value;
    node->next = NULL;

    edgeMapNode *temp = map->head;

    if (temp == NULL)
    {
        map->head = node;
    }
    else
    {
        while (temp->next != NULL)
            temp = temp->next;

        temp->next = node;
    }
}

keyValue getMapElement(edgeMap *map, keyValue key)
{
    edgeMapNode *temp = map->head;

    while (temp != NULL)
    {
        if (temp->key == key)
        {
            return temp->value;
        }
        temp = temp->next;
    }

    return NULL;
}

void deleteMap(edgeMap *map)
{
    edgeMapNode *temp = map->head;
    edgeMapNode *xtemp;

    while (temp != NULL)
    {
        xtemp = temp->next;
        free(temp);
        temp = xtemp;
    }

    map->head = NULL;
}

static List *createListNode(void *data)
{
    List *node = (List *)calloc(1, sizeof(List));
    if (!node)
    {
        return NULL;
    }

    node->data = data;
    return node;
}

bool addListNode(List **head, void *data)
{
    if (IS_NULL(head) || IS_NULL(data))
    {
        return false;
    }

    List *newnode = createListNode(data);
    if (IS_NULL(newnode))
    {
        return false;
    }

    newnode->link = *head;
    *head = newnode;
    return true;
}

unsigned int getListSize(List *ptr)
{
    if(IS_NULL(ptr))
    {
        return 0;
    }

    int size = 0;
    while(ptr)
    {
        size++;
        ptr=ptr->link;
    }
    return size;
}

void deleteListNode(List **head, void *data)
{
    if (!head || !data)
    {
        return;
    }

    List *ptr = *head, *prev = NULL;
    while (ptr && ptr->data != data)
    {
        prev = ptr;
        ptr = ptr->link;
    }

    if (!ptr)
    {
        return;
    }

    if (prev)
    {
        prev->link = ptr->link;
    }
    else
    {
        *head = ptr->link;
    }

    ptr->link = NULL;
    free(ptr);
}

void deleteList(List **head)
{
    if (!head)
    {
        return;
    }

    List *next = NULL;
    List *ptr = *head;
    while (ptr)
    {
        next = ptr->link;
        free(ptr);
        ptr = next;
    }
    *head = NULL;
}

char *cloneString(const char *str)
{
    if (!str)
    {
        return NULL;
    }
    int len = strlen(str);
    char *clone = (char *)malloc(len + 1);
    if (!clone)
    {
        return NULL;
    }
    memcpy(clone, str, len + 1);
    return clone;
}

void *clone(const void *src, int lenInbytes)
{
    if(!src || lenInbytes < 1)
    {
        return NULL;
    }

    void *cloned = malloc(lenInbytes);
    if(!cloned)
    {
        return NULL;
    }

    memcpy(cloned, src, lenInbytes);
    return cloned;
}

char *convertUAStringToString(UA_String *uaStr)
{
    if (!uaStr || uaStr->length <= 0)
    {
        return NULL;
    }

    char *str = (char *)malloc(uaStr->length + 1);
    if (!str)
    {
        return NULL;
    }

    memcpy(str, uaStr->data, uaStr->length);
    str[uaStr->length] = '\0';
    return str;
}

void freeEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    if (!config)
    {
        return;
    }

    free(config->serverName);
    free(config->bindAddress);
    free(config);
}

void freeEdgeApplicationConfig(EdgeApplicationConfig *config)
{
    if(!config)
    {
        return;
    }

    free(config->applicationUri);
    free(config->productUri);
    free(config->applicationName);
    free(config->gatewayServerUri);
    free(config->discoveryProfileUri);
    for(int i = 0; i < config->discoveryUrlsSize; ++i)
    {
        free(config->discoveryUrls[i]);
    }
    free(config->discoveryUrls);
    free(config);
}

void freeEdgeContinuationPoint(EdgeContinuationPoint *cp)
{
    if (!cp)
    {
        return;
    }

    free(cp->continuationPoint);
    free(cp);
}

void freeEdgeContinuationPointList(EdgeContinuationPointList *cpList)
{
    if (!cpList)
    {
        return;
    }

    for (int i = 0; i < cpList->count; ++i)
    {
        freeEdgeContinuationPoint(cpList->cp[i]);
    }

    free(cpList->cp);
    free(cpList);
}

void freeEdgeDevice(EdgeDevice *dev)
{
    if(!dev)
    {
        return;
    }

    if (dev->endpointsInfo)
    {
        for (int i = 0; i < dev->num_endpoints; ++i)
        {
            freeEdgeEndpointInfo(dev->endpointsInfo[i]);
        }
    }
    free(dev->endpointsInfo);
    free(dev->address);
    free(dev->serverName);
    free(dev);
}

EdgeEndpointConfig *cloneEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    if (!config)
    {
        return NULL;
    }

    EdgeEndpointConfig *clone = (EdgeEndpointConfig *)calloc(1, sizeof(EdgeEndpointConfig));
    if (!clone)
    {
        return NULL;
    }

    clone->requestTimeout = config->requestTimeout;
    clone->bindPort = config->bindPort;
    if (config->serverName)
    {
        clone->serverName = cloneString(config->serverName);
        if (!clone->serverName)
        {
            goto ERROR;
        }
    }

    if (config->bindAddress)
    {
        clone->bindAddress = cloneString(config->bindAddress);
        if (!clone->bindAddress)
        {
            goto ERROR;
        }
    }

    return clone;

ERROR:
    freeEdgeEndpointConfig(clone);
    return NULL;
}

EdgeApplicationConfig *cloneEdgeApplicationConfig(EdgeApplicationConfig *config)
{
    if (!config)
    {
        return NULL;
    }

    EdgeApplicationConfig *clone = (EdgeApplicationConfig *)calloc(1, sizeof(EdgeApplicationConfig));
    if (!clone)
    {
        return NULL;
    }

    clone->applicationType = config->applicationType;

    if (config->applicationUri)
    {
        clone->applicationUri = cloneString(config->applicationUri);
        if (!clone->applicationUri)
        {
            goto ERROR;
        }
    }

    if (config->productUri)
    {
        clone->productUri = cloneString(config->productUri);
        if (!clone->productUri)
        {
            goto ERROR;
        }
    }

    if (config->applicationName)
    {
        clone->applicationName = cloneString(config->applicationName);
        if (!clone->applicationName)
        {
            goto ERROR;
        }
    }

    if (config->gatewayServerUri)
    {
        clone->gatewayServerUri = cloneString(config->gatewayServerUri);
        if (!clone->gatewayServerUri)
        {
            goto ERROR;
        }
    }

    if (config->discoveryProfileUri)
    {
        clone->discoveryProfileUri = cloneString(config->discoveryProfileUri);
        if (!clone->discoveryProfileUri)
        {
            goto ERROR;
        }
    }

    clone->discoveryUrlsSize = config->discoveryUrlsSize;
    clone->discoveryUrls = (char **)calloc(config->discoveryUrlsSize, sizeof(char *));
    if(!clone->discoveryUrls)
    {
        goto ERROR;
    }

    for(int i = 0; i < clone->discoveryUrlsSize; ++i)
    {
        if (config->discoveryUrls[i])
        {
            clone->discoveryUrls[i] = cloneString(config->discoveryUrls[i]);
            if (!clone->discoveryUrls[i])
            {
                goto ERROR;
            }
        }
    }

    return clone;

ERROR:
    freeEdgeApplicationConfig(clone);
    return NULL;
}

void freeEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    if (!endpointInfo)
    {
        return;
    }
    free(endpointInfo->endpointUri);
    freeEdgeEndpointConfig(endpointInfo->endpointConfig);
    freeEdgeApplicationConfig(endpointInfo->appConfig);
    free(endpointInfo->securityPolicyUri);
    free(endpointInfo->transportProfileUri);
    free(endpointInfo);
}

EdgeEndPointInfo *cloneEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    if (!endpointInfo)
    {
        return NULL;
    }

    EdgeEndPointInfo *clone = (EdgeEndPointInfo *)calloc(1, sizeof(EdgeEndPointInfo));
    if (!clone)
    {
        return NULL;
    }

    clone->securityMode = endpointInfo->securityMode;
    clone->securityLevel = endpointInfo->securityLevel;

    if (endpointInfo->endpointUri)
    {
        clone->endpointUri = cloneString(endpointInfo->endpointUri);
        if (!clone->endpointUri)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->securityPolicyUri)
    {
        clone->securityPolicyUri = cloneString(endpointInfo->securityPolicyUri);
        if (!clone->securityPolicyUri)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->transportProfileUri)
    {
        clone->transportProfileUri = cloneString(endpointInfo->transportProfileUri);
        if (!clone->transportProfileUri)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->endpointConfig)
    {
        clone->endpointConfig = cloneEdgeEndpointConfig(endpointInfo->endpointConfig);
        if (!clone->endpointConfig)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->appConfig)
    {
        clone->appConfig = cloneEdgeApplicationConfig(endpointInfo->appConfig);
        if (!clone->appConfig)
        {
            goto ERROR;
        }
    }

    return clone;

ERROR:
    freeEdgeEndpointInfo(clone);
    return NULL;
}

void freeEdgeBrowseResult(EdgeBrowseResult *browseResult, int browseResultLength)
{
    if (!browseResult)
    {
        return;
    }
    for (int i = 0; i < browseResultLength; ++i)
    {
        free(browseResult[i].browseName);
    }
    free(browseResult);
}

void freeEdgeNodeId(EdgeNodeId *nodeId)
{
    if (!nodeId)
    {
        return;
    }
    free(nodeId->nodeUri);
    free(nodeId->nodeId);
    free(nodeId);
}

void freeEdgeNodeInfo(EdgeNodeInfo *nodeInfo)
{
    if (!nodeInfo)
    {
        return;
    }

    free(nodeInfo->methodName);
    freeEdgeNodeId(nodeInfo->nodeId);
    free(nodeInfo->valueAlias);
    free(nodeInfo);
}

void freeEdgeArgument(EdgeArgument *arg)
{
    if (!arg)
    {
        return;
    }

    free(arg->scalarValue);
    free(arg->arrayData);
    free(arg);
}

void freeEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams)
{
    if (!methodParams)
    {
        return;
    }

    for (int i = 0; i < methodParams->num_inpArgs; ++i)
    {
        freeEdgeArgument(methodParams->inpArg[i]);
    }
    free(methodParams->inpArg);

    for (int i = 0; i < methodParams->num_outArgs; ++i)
    {
        freeEdgeArgument(methodParams->outArg[i]);
    }
    free(methodParams->outArg);

    free(methodParams);
}

void freeEdgeRequest(EdgeRequest *req)
{
    if (!req)
    {
        return;
    }

    free(req->value);
    free(req->subMsg);
    freeEdgeMethodRequestParams(req->methodParams);
    freeEdgeNodeInfo(req->nodeInfo);
    free(req);
}

void freeEdgeRequests(EdgeRequest **requests, int requestLength)
{
    if (!requests)
    {
        return;
    }

    for (int i = 0; i < requestLength; ++i)
    {
        freeEdgeRequest(requests[i]);
    }
    free(requests);
}

void freeEdgeVersatility(EdgeVersatility *versatileValue)
{
    if (!versatileValue)
    {
        return;
    }

    free(versatileValue->value);
    free(versatileValue);
}

void freeEdgeDiagnosticInfo(EdgeDiagnosticInfo *info)
{
    if (!info)
    {
        return;
    }

    free(info->additionalInfo);
    free(info->innerDiagnosticInfo);
    free(info->msg);
    free(info);
}

void freeEdgeResponse(EdgeResponse *response)
{
    if (!response)
    {
        return;
    }

    freeEdgeVersatility(response->message);
    free(response->value);
    freeEdgeNodeInfo(response->nodeInfo);
    free(response->result);
    freeEdgeDiagnosticInfo(response->m_diagnosticInfo);
    free(response);
}

void freeEdgeResponses(EdgeResponse **responses, int responseLength)
{
    if (!responses)
    {
        return;
    }

    for (int i = 0; i < responseLength; ++i)
    {
        freeEdgeResponse(responses[i]);
    }
    free(responses);
}

void freeEdgeMessage(EdgeMessage *msg)
{
    if (!msg)
    {
        return;
    }

    freeEdgeEndpointInfo(msg->endpointInfo);
    freeEdgeRequest(msg->request);
    freeEdgeRequests(msg->requests, msg->requestLength);
    freeEdgeResponses(msg->responses, msg->responseLength);
    free(msg->result);
    free(msg->browseParam);
    freeEdgeBrowseResult(msg->browseResult, msg->browseResultLength);
    freeEdgeContinuationPointList(msg->cpList);
    free(msg);
}

EdgeResult *createEdgeResult(EdgeStatusCode code)
{
    EdgeResult *result = (EdgeResult *)calloc(1, sizeof(EdgeResult));
    if (!result)
    {
        return NULL;
    }
    result->code = code;
    return result;
}

bool isNodeClassValid(UA_NodeClass nodeClass)
{
    bool valid = true;
    switch (nodeClass)
    {
        case UA_NODECLASS_UNSPECIFIED:
        case UA_NODECLASS_OBJECT:
        case UA_NODECLASS_VARIABLE:
        case UA_NODECLASS_METHOD:
        case UA_NODECLASS_OBJECTTYPE:
        case UA_NODECLASS_VARIABLETYPE:
        case UA_NODECLASS_REFERENCETYPE:
        case UA_NODECLASS_DATATYPE:
        case UA_NODECLASS_VIEW:
            valid = true;
            break;
        default:
            valid = false;
            break;
    }
    return valid;
}

EdgeNodeId *cloneEdgeNodeId(EdgeNodeId *nodeId)
{
    if (!nodeId)
    {
        return NULL;
    }

    EdgeNodeId *clone = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if (!clone)
    {
        return NULL;
    }

    clone->nameSpace = nodeId->nameSpace;
    if (nodeId->nodeUri)
    {
        clone->nodeUri = cloneString(nodeId->nodeUri);
        if (!clone->nodeUri)
        {
            free(clone);
            return NULL;
        }
    }
    clone->nodeIdentifier = nodeId->nodeIdentifier;
    clone->type = nodeId->type;
    if (nodeId->nodeId)
    {
        clone->nodeId = cloneString(nodeId->nodeId);
        if (!clone->nodeId)
        {
            free(clone->nodeUri);
            free(clone);
            return NULL;
        }
    }
    clone->integerNodeId = nodeId->integerNodeId;

    return clone;
}

EdgeNodeInfo *cloneEdgeNodeInfo(EdgeNodeInfo *nodeInfo)
{
    if (!nodeInfo)
    {
        return NULL;
    }

    EdgeNodeInfo *clone = (EdgeNodeInfo *)calloc(1, sizeof(EdgeNodeInfo));
    if (!clone)
    {
        return NULL;
    }

    if (nodeInfo->methodName)
    {
        clone->methodName = cloneString(nodeInfo->methodName);
        if (!clone->methodName)
        {
            free(clone);
            return NULL;
        }
    }

    if (nodeInfo->nodeId)
    {
        clone->nodeId = cloneEdgeNodeId(nodeInfo->nodeId);
        if (!clone->nodeId)
        {
            free(clone->methodName);
            free(clone);
            return NULL;
        }
    }

    if (nodeInfo->valueAlias)
    {
        clone->valueAlias = cloneString(nodeInfo->valueAlias);
        if (!clone->valueAlias)
        {
            freeEdgeNodeId(clone->nodeId);
            free(clone->methodName);
            free(clone);
            return NULL;
        }
    }

    return clone;
}

// USAGE

/*
 edgeMap* X = createMap();

 insert(X, 10, "arya");
 insert(X, 20, "mango");
 insert(X, 25, "apple");

 char* ret = (char *)get(X, 25);

 deleteMap(X);
 */
