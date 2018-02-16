#include <gtest/gtest.h>
#include <iostream>
#include <math.h>

extern "C"
{
#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_utils.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "open62541.h"
}

#define TAG "writeTest"

extern char node_arr[13][30];

void testWrite_P1(char *endpointUri)
{
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    char *value = (char*) malloc(sizeof(char) * 10);
    strcpy(value, "test_str");
    insertWriteAccessNode(&msg, node_arr[0], value, 1);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);

    msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    double dVal = 22.32;
    insertWriteAccessNode(&msg, node_arr[3], (void *) &dVal, 1);
    result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

void testWrite_P2(char *endpointUri)
{
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    char **values = (char**) malloc(sizeof(char*) * 5);
    for (int i = 0; i < 5; i++)
    {
        values[i] = (char*) malloc(sizeof(char) * 10);
        snprintf(values[i], 10, "test%d", i+1);
    }
    insertWriteAccessNode(&msg, node_arr[11], values, 5);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
    for (int i = 0; i < 5; i++)
    {
        free(values[i]);
        values[i] = NULL;
    }
    free(values);

    msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    values = (char**) malloc(sizeof(char*) * 5);
    for (int i = 0; i < 5; i++)
    {
        values[i] = (char*) malloc(sizeof(char) * 15);
        snprintf(values[i], 15, "ByteString%d", i+1);
    }
    insertWriteAccessNode(&msg, node_arr[12], values, 5);
    result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);

    msg = createEdgeAttributeMessage(endpointUri, 1, CMD_READ);
    insertReadAccessNode(&msg, node_arr[12]);
    result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);

    msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    double *d_values = (double *) EdgeMalloc(sizeof(double) * 5);
    for (int i = 1; i <= 5; i++)
    {
        d_values[i] = i + 100.23;
    }
    insertWriteAccessNode(&msg, node_arr[10], d_values, 5);
    result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);

    msg = createEdgeAttributeMessage(endpointUri, 1, CMD_READ);
    insertReadAccessNode(&msg, node_arr[10]);
    result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

void testWrite_P3(char *endpointUri)
{
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    char *value = (char*) malloc(sizeof(char) * 10);
    strcpy(value, "test_str");
    insertWriteAccessNode(&msg, node_arr[8], value, 1);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

void testWrite_P4(char *endpointUri)
{
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    double dVal = 22.22;
    insertWriteAccessNode(&msg, "{2;S;v=11}InvalidNode", &dVal, 1);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

void testWriteWithoutEndpoint()
{
    EdgeMessage *msg = createEdgeAttributeMessage(NULL, 1, CMD_WRITE);
    ASSERT_EQ(NULL!=msg, false);
    char *value = (char*) malloc(sizeof(char) * 10);
    strcpy(value, "test_str");
    insertWriteAccessNode(&msg, node_arr[0], value, 1);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testWriteWithoutValueAlias(char *endpointUri)
{
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, 1, CMD_WRITE);
    EXPECT_EQ(NULL!=msg, true);
    char *value = (char*) malloc(sizeof(char) * 10);
    strcpy(value, "test_str");
    insertWriteAccessNode(&msg, NULL, value, 1);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testWriteWithoutMessage()
{
    EdgeResult result = sendRequest(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

