#include <broker/mosquitto.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* defines */
#define BROKER		"localhost"
#define PORT		1883
#define KEEPALIVE	500

/* prototypes */
void message_callback(struct mosquitto * mosq, void * obj , const struct mosquitto_message * message);
void publish_callback(struct mosquitto * mosq, void * pbj, int mid);
void subscribe_callback(struct mosquitto * mosq, void * obj, int mid, int qos, const int * granted_qos);
void cleanUp (struct mosquitto * pMosq);
void printError (int ec);

/* @breif Main application entry point
 * @return On sucess 0, otherwsie 1.
 */
int mmain (void)
{
	struct mosquitto * m_pMqtt;
	int rc;
	
	/* initialise the library */
    mosquitto_lib_init();
    
    /* Create a new handle to a mosquitto object */
    m_pMqtt = mosquitto_new(NULL, true, NULL);
    if (m_pMqtt == NULL)
    {
    	fprintf(stderr, "Failed to allocate mosquitto instance\r\n");
        mosquitto_lib_cleanup();
    	exit(1);
    } 

	/* setup any callback that we want to use */
    mosquitto_publish_callback_set(m_pMqtt, publish_callback);
    mosquitto_subscribe_callback_set(m_pMqtt, subscribe_callback);
    mosquitto_message_callback_set(m_pMqtt, message_callback);

    /* connect to the broker */
    rc = mosquitto_connect(m_pMqtt, BROKER, PORT, KEEPALIVE); 
    if (rc != MOSQ_ERR_SUCCESS)
    {
		printError(rc);
    	cleanUp(m_pMqtt);
    	exit(1);
    }
    
    /* ask the library to start a thread for us to service the connection */
    rc = mosquitto_loop_start(m_pMqtt);
    if (rc != MOSQ_ERR_SUCCESS)
    {
		printError(rc);
    	cleanUp(m_pMqtt);
    	exit(1);
    }
 
 	/* At this point we are connected to the broker and the network
 	 * thread is running so we can subscribe and publish as we please.
 	 * Any additional processing can be done using the callbacks we stup earlier
 	 */
 
    sleep(1);
    
    /* subscribe to a topic */
    rc = mosquitto_subscribe(m_pMqtt, NULL, "testtopic", 0);
    if (rc != MOSQ_ERR_SUCCESS)
    {
		printError(rc);
    	cleanUp(m_pMqtt);
    	exit(1);
    }
    
    sleep(1);

	/* publish a message */    
    rc = mosquitto_publish(m_pMqtt, NULL, "testtopic", 6, "hello", 0, false);
    if (rc != MOSQ_ERR_SUCCESS)
    {
		printError(rc);
    	cleanUp(m_pMqtt);
    	exit(1);
    }

    sleep(1);
    
    /* publish another message */
    rc = mosquitto_publish(m_pMqtt, NULL, "testtopic", 6, "12345", 0, false);
    if (rc != MOSQ_ERR_SUCCESS)
    {
		printError(rc);
    	cleanUp(m_pMqtt);
    	exit(1);
    }
    
    sleep(1);
    
	/* terminate the thread */
    rc = mosquitto_loop_stop(m_pMqtt, true);
    if (rc != MOSQ_ERR_SUCCESS)
    {
		printError(rc);
    	cleanUp(m_pMqtt);
    	exit(1);
    }
     
    /* clean up resources before exiting */
    cleanUp(m_pMqtt);

	return 0;
}

/* @breif Received message callback 
 * @param mosq Pointer to mosquitto object
 * @param obj Pointer to object passed into mosquitto_new (if any)
 * @param message The received message
 * @return None
 */
void message_callback(struct mosquitto * mosq, void * obj , const struct mosquitto_message * message)
{
	printf("\r\nMessage Callback\r\n");	
	printf("\tID: %d\r\n", message->mid);
	printf("\tTopic: %s\r\n", message->topic);
	/* Assume the payload is a string in this example */
	printf("\tPayload: %s\r\n", (char *)message->payload);
}

/* @breif Publish callback 
 * @param mosq Pointer to mosquitto object
 * @param obj Pointer to object passed into mosquitto_new (if any)
 * @param mid The id assigned to the message
 * @return None
 */
void publish_callback(struct mosquitto * mosq, void * pbj, int mid)
{
	printf("\r\nPublish Callback\r\n");	
	printf("\tID: %d\r\n", mid);
}

/* @breif Publish callback 
 * @param mosq Pointer to mosquitto object
 * @param obj Pointer to object passed into mosquitto_new (if any)
 * @param mid The id assigned to the message
 * @param qos_count 
 * @param granted_qos
 * @return None
 */
void subscribe_callback(struct mosquitto * mosq, void * obj, int mid, int qos, const int * granted_qos)
{
	printf("\r\nSubscribe Callback\r\n");	
	printf("\tID: %d\r\n", mid);
}

/* @brief Helper function to clean up resources 
 * @param pMosq Pointer to create mosquitto object
 * @return None
 */
void cleanUp (struct mosquitto * pMosq)
{
	/* destroy allocated object */
    mosquitto_destroy(pMosq);
    
    /* cleanup library */
    mosquitto_lib_cleanup();	
}

/* @brief Helper function to print error messages 
 * @param ec Error code returne from a mosquitto lib function 
 * @return None
 */
void printError (int ec)
{
   	fprintf(stderr, "ERROR: %s\r\n", mosquitto_strerror(ec));    
}