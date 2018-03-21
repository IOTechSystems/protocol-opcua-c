/******************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "message_dispatcher.h"
#include "queue.h"
#include "edge_utils.h"
#include "edge_malloc.h"
#include "edge_logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TAG "message_handler"

#ifdef MULTI_THREAD
static pthread_t m_sendQ_Thread;
static pthread_t m_recvQ_Thread;
static pthread_mutex_t sendMutex;
static pthread_mutex_t recvMutex;

static bool b_sendQ_Thread_Running = false;
static bool b_recvQ_Thread_Running = false;

static Queue *recvQueue = NULL;
static Queue *sendQueue = NULL;
static const int queue_capacity = 1000;
#endif

static response_cb_t g_responseCallback = NULL;
static send_cb_t g_sendCallback = NULL;

static void handleMessage(EdgeMessage *data);

void delete_queue()
{
#ifdef MULTI_THREAD
    b_sendQ_Thread_Running = false;
    pthread_join(m_sendQ_Thread, NULL);

    b_recvQ_Thread_Running = false;
    pthread_join(m_recvQ_Thread, NULL);

    if (sendQueue != NULL)
    {
        pthread_mutex_lock(&sendMutex);
        while (!isEmpty(sendQueue))
        {
            EdgeMessage *data = dequeue(sendQueue);
            freeEdgeMessage(data);
        }
        pthread_mutex_unlock(&sendMutex);
        pthread_mutex_destroy(&sendMutex);

        EdgeFree(sendQueue->message);
        EdgeFree(sendQueue);
        sendQueue = NULL;
    }

    if (recvQueue != NULL)
    {
        pthread_mutex_lock(&recvMutex);
        while (!isEmpty(recvQueue))
        {
            EdgeMessage *data = dequeue(recvQueue);
            freeEdgeMessage(data);
        }
        pthread_mutex_unlock(&recvMutex);
        pthread_mutex_destroy(&recvMutex);

        EdgeFree(recvQueue->message);
        EdgeFree(recvQueue);
        recvQueue = NULL;
    }
#endif
}

#ifdef MULTI_THREAD
static void *sendQ_run(void *ptr)
{
    EdgeMessage *data;
    b_sendQ_Thread_Running = true;

    while (b_sendQ_Thread_Running)
    {
        if (!isEmpty(sendQueue))
        {
            pthread_mutex_lock(&sendMutex);
            data = dequeue(sendQueue); // retrieve the front element from queue
            pthread_mutex_unlock(&sendMutex);
            handleMessage(data); // process the queue message
            freeEdgeMessage(data);
        }
    }
    return NULL;
}
#endif

#ifdef MULTI_THREAD
static void *recvQ_run(void *ptr)
{
    EdgeMessage *data;
    b_recvQ_Thread_Running = true;

    while (b_recvQ_Thread_Running)
    {
        if (!isEmpty(recvQueue))
        {
            pthread_mutex_lock(&recvMutex);
            data = dequeue(recvQueue); // retrieve the front element from queue
            pthread_mutex_unlock(&recvMutex);
            handleMessage(data); // process the queue message
            //EdgeMessage *msg_to_delete = dequeue(recvQueue); // remove the element from queue
            freeEdgeMessage(data);
        }
    }
    return NULL;
}
#endif

bool add_to_sendQ(EdgeMessage *msg)
{
#ifdef MULTI_THREAD
    if (NULL == sendQueue)
    {
        sendQueue = createQueue(queue_capacity);
        pthread_create(&m_sendQ_Thread, NULL, &sendQ_run, NULL);
        pthread_mutex_init(&sendMutex, NULL);
    }
    pthread_mutex_lock(&sendMutex);
    bool ret = enqueue(sendQueue, msg);
    pthread_mutex_unlock(&sendMutex);
    return ret;
#else
    handleMessage(msg);
    return true;
#endif
}

bool add_to_recvQ(EdgeMessage *msg)
{
#ifdef MULTI_THREAD
    if (NULL == recvQueue)
    {
        recvQueue = createQueue(queue_capacity);
        pthread_create(&m_recvQ_Thread, NULL, &recvQ_run, NULL);
        pthread_mutex_init(&recvMutex, NULL);
    }
    pthread_mutex_lock(&recvMutex);
    bool ret = enqueue(recvQueue, msg);
    pthread_mutex_unlock(&recvMutex);
    return ret;
#else
    handleMessage(msg);
    return true;
#endif
}

static void handleMessage(EdgeMessage *data)
{
    if (SEND_REQUEST == data->type || SEND_REQUESTS == data->type)
    {
        g_sendCallback(data);
    }
    else if (GENERAL_RESPONSE == data->type || BROWSE_RESPONSE == data->type
             || REPORT == data->type || ERROR == data->type)
    {
        g_responseCallback(data);
    }
}

void registerMQCallback(response_cb_t resCallback, send_cb_t sendCallback)
{
    g_responseCallback = resCallback;
    g_sendCallback = sendCallback;
}
