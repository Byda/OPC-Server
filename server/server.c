/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

#include <signal.h>
#include <stdio.h>
#include "../open62541-lib/open62541.h"
#include "../model/model.h"

UA_Boolean running = true;
float temperature = 20.0;
bool stateLED1;
bool stateButton1;


// Khai bao ham callback read cho Temperature Variable
static void
beforeReadTemperature(UA_Server *server,
		const UA_NodeId *sessionId, void *sessionContext,
		const UA_NodeId *nodeid, void *nodeContext,
		const UA_NumericRange *range, const UA_DataValue *data) {

	float tmp = 1.0 * (rand()%100)/100 - 0.5;
	temperature += tmp;
	
	UA_Variant value;
	//Copy the Temperature to a variant 'value'
	UA_Variant_setScalar(&value, &temperature, &UA_TYPES[UA_TYPES_FLOAT]);
	//Copy the 'value' to Value of Node
	UA_Server_writeValue(server, UA_NODEID_STRING(2, "TS1_Temperature"), value);
}


// Khai bao ham callback write cho Run cua LED1
static void
afterWriteLED1(UA_Server *server,
		const UA_NodeId *sessionId, void *sessionContext,
		const UA_NodeId *nodeid, void *nodeContext,
		const UA_NumericRange *range, const UA_DataValue *data) {
	UA_Variant value;
	UA_Server_readValue(server,  UA_NODEID_NUMERIC(2, 2007), &value);
	stateLED1  = * (UA_Boolean*) value.data;

	if(stateLED1 == true)
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
			"LED 1 is now ON");
	else
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
			"LED 1 is now OFF");
	}


//Khai bao ham callback read cho State cua Button1
static void
beforeReadButton1(UA_Server *server,
		const UA_NodeId *sessionId, void *sessionContext,
		const UA_NodeId *nodeid, void *nodeContext,
		const UA_NumericRange *range, const UA_DataValue *data) {
	
	UA_Variant value;
	//Copy the State of Button1 to a variant 'value'
	UA_Variant_setScalar(&value, &stateButton1, &UA_TYPES[UA_TYPES_BOOLEAN]);
	UA_Server_writeValue(server, UA_NODEID_NUMERIC(2, 2005), value);
}




static void stopHandler(int sign)
{
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}

int main(int argc, char * argv[])
{
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

// Khoi chay mot Server Application trong
    UA_Server *server = UA_Server_new();


// Cai dat de co the nhap IP address va portnumber
    if(argc > 2){
    		/*hostname or ip address and a port number argument  are available*/
	UA_Int16 port_number = atoi(argv[2]);
	UA_ServerConfig_setMinimal(UA_Server_getConfig(server), port_number, 0);
    }
    else
	UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    if(argc > 1) {
		/*hostname or ip address argument are available*/
	/*copy the hostname from char * to an open62541 variable*/ 
	UA_String hostname;
	UA_String_init(&hostname);
	hostname.length = strlen(argv[1]);
	hostname.data = (UA_Byte *) argv[1];

	/*change the configuration*/
	UA_ServerConfig_setCustomHostname(UA_Server_getConfig(server), hostname);
    }


// Tien hanh add Information Model vao Server
    UA_StatusCode retval = PLC1_Server(server);
    /* create nodes from nodeset */
    if (retval != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Could not add the example nodeset. ");
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    }    

  
//Get Namespace Index
    size_t ns0;
    UA_UInt16 isFound0 = UA_Server_getNamespaceByName(server, UA_STRING("http://opcfoundation.org/UA/"), &ns0);
    if (isFound0 == UA_STATUSCODE_GOOD) UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Namespace0 have index:  %ld", ns0);
    else UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Not found Namespace0 in Information Model!!!");

    size_t ns1;
    UA_UInt16 isFound1 = UA_Server_getNamespaceByName(server, UA_STRING("PLC1"), &ns1);
     if (isFound1 == UA_STATUSCODE_GOOD) UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Namespace1 have index:  %ld", ns1);
    else UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Not found Namespace1 in Information Model!!!");   


// Add Callback Method Read to Button node 
    UA_ValueCallback callback;
    callback.onRead = beforeReadButton1;
    callback.onWrite = NULL;
    UA_Server_setVariableNode_valueCallback(server, UA_NODEID_NUMERIC(2, 2005), callback);
   


// Add Callback Method Write to LED1 node
    UA_ValueCallback callback1;
    callback1.onRead = NULL;
    callback1.onWrite = afterWriteLED1;
    UA_Server_setVariableNode_valueCallback(server, UA_NODEID_NUMERIC(2, 2007), callback1);


    
// Them Object Temperature Sensor vao address space

    //Add a new object called TemperatureSensor
    UA_NodeId ns1_ts_Id; 
    UA_ObjectAttributes otsAttr = UA_ObjectAttributes_default;
    UA_Server_addObjectNode(server, UA_NODEID_STRING(ns1, "NS1_TempSens"),
		    UA_NODEID_NUMERIC(ns0, UA_NS0ID_OBJECTSFOLDER),
		    UA_NODEID_NUMERIC(ns0, UA_NS0ID_ORGANIZES),
		    UA_QUALIFIEDNAME(ns1, "TempSensor"),
		    UA_NODEID_NUMERIC(ns0, UA_NS0ID_BASEOBJECTTYPE),
		    otsAttr, NULL, &ns1_ts_Id);

    //Add a new object called TemperatureSensor1
    UA_NodeId ts_ts1_Id;
    UA_ObjectAttributes ots1Attr = UA_ObjectAttributes_default;
    UA_Server_addObjectNode(server, UA_NODEID_STRING(ns1, "TS1_TempSens"),
		    ns1_ts_Id,
		    UA_NODEID_NUMERIC(ns0, UA_NS0ID_HASCOMPONENT),
		    UA_QUALIFIEDNAME(ns1, "TempSensor1"),
		    UA_NODEID_NUMERIC(ns0, UA_NS0ID_BASEOBJECTTYPE),
		    ots1Attr, NULL, &ts_ts1_Id);

    //Add the variable Temperature to Object
    UA_VariableAttributes tvAttr = UA_VariableAttributes_default;
    		// Bien temperature da duoc dinh nghia o line10 //
    UA_Variant_setScalar(&tvAttr.value, &temperature, &UA_TYPES[UA_TYPES_FLOAT]);
    UA_Server_addVariableNode(server, UA_NODEID_STRING(ns1, "TS1_Temperature"),
                    ts_ts1_Id,
                    UA_NODEID_NUMERIC(ns0, UA_NS0ID_HASCOMPONENT),
                    UA_QUALIFIEDNAME(ns1, "Temperature"),
                    UA_NODEID_NUMERIC(ns0, UA_NS0ID_BASEDATAVARIABLETYPE),
                   tvAttr, NULL, NULL); 

    //Add Callback Method to Temperature Node
    UA_ValueCallback callback2;
    callback2.onRead = beforeReadTemperature;
    callback2.onWrite = NULL;
    UA_Server_setVariableNode_valueCallback(server, UA_NODEID_STRING(ns1, "TS1_Temperature"), callback2);


// Run Server    
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server is starting....");
    retval = UA_Server_run(server, &running);
    /* Den day, CT vao vong lap vo han cho den khi user nhan Ctrl+C */
    /* Truoc do, CT chay tuan tu tu tren xuong duoi */
  
// Delete Server

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server is shutting down....");
    UA_Server_delete(server);

    return (int) retval;
}

