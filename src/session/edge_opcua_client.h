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

#ifndef EDGE_OPCUA_CLIENT_H
#define EDGE_OPCUA_CLIENT_H

#include <stdbool.h>

#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool connect_client(char *endpoint);
    void disconnect_client(EdgeEndPointInfo *epInfo);
    EdgeResult getClientEndpoints(char *endpointUri);
    EdgeResult readNodesFromServer(EdgeMessage *msg);
    EdgeResult writeNodesInServer(EdgeMessage *msg);
    EdgeResult browseNodesInServer(EdgeMessage *msg);
    EdgeResult browseViewsInServer(EdgeMessage *msg);
    EdgeResult browseNextInServer(EdgeMessage *msg);
    EdgeResult callMethodInServer(EdgeMessage *msg);
    EdgeResult executeSubscriptionInServer(EdgeMessage *msg);

#ifdef __cplusplus
}
#endif

#endif
