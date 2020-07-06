#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <curl/curl.h>
#include<json-c/json.h>
#include "MQTTAsync.h"
#define ADDRESS     "ssl://airtrik.com:8883" 
#define CLIENTID    "kshitij"
#define PAYLOAD     "Successfully delivered string using C program"
#define QOS         2
#define TIMEOUT     10000L

MQTTAsync client;
MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;

MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
MQTTAsync_token token;

volatile MQTTAsync_token deliveredtoken;
int finished = 0;


struct json_object *parsed_json;
struct json_object *username; 
struct json_object *password;
struct json_object *error;
struct json_object *name;

char *Global_key;

int onReceive(char* msg, char *deviceId);

void onDisconnect(void* context, MQTTAsync_successData* response)
{
        finished = 1;
}

void onSend(void* context, MQTTAsync_successData* response)
{
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    int rc;
    opts.onSuccess = onDisconnect;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int i,j=0;
    char* payloadptr;
    int flag=0;
   
     for(i=0; i<topicLen; i++)
    {
        if(flag == 1)
        {
            j++;  
                        
        }
        else if(topicName[i]=='/'){
            flag=1;
        }
    }
    
    char *topic = (char*)malloc(sizeof(char*)*j);
    j=0;flag=0;
    for(i=0; i<topicLen; i++)
    {
                if(flag == 1)
        {
           
            topic[j] = topicName[i];    
            j++;    
        }
        else if(topicName[i]=='/'){
            flag=1;
        }
    }
    topic[j]='\0';
    onReceive(message->payload,topic);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void onSuccessfulSubscribe()
{
   
}

void onSubscribeFailure(){
    printf("Subscribing Failed\n");
}


void subscribe_topic(char *device){
    char *topic = (char *) malloc(2 + strlen(Global_key)+ strlen(device));
    strcpy(topic, Global_key);
    strcat(topic,"/");
    strcat(topic, device);
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    opts.onSuccess = onSuccessfulSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;
    deliveredtoken = 0;
    rc = MQTTAsync_subscribe(client, topic, QOS, &opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        exit(EXIT_FAILURE);
    } 
}

void connlost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


int send_message(char* device,char* message)
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    opts.onSuccess = onSend;
    opts.context = client;
    pubmsg.payload = message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;   
    char *topic = (char *) malloc(2 + strlen(Global_key)+ strlen(device));
    strcpy(topic, Global_key);
    strcat(topic,"/");
    strcat(topic, device);
    if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
            printf("Failed to start sendMessage, return code %d\n", rc);
            exit(EXIT_FAILURE);
    }    
    return rc;
}
int mqtt_init(char* user,char* pass,void (*onConnectionEstablished)(),int (*onMessageArrived)())
{

    int rc;
    int x=MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    int xx = MQTTAsync_setCallbacks(client, NULL, connlost, msgarrvd, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnectionEstablished;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.ssl = &ssl_opts; 
    conn_opts.username = user;
    conn_opts.password = pass;
    conn_opts.context = client;
    rc = MQTTAsync_connect(client, &conn_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
            printf("Failed to start connect, return code %d\n", rc);
            exit(EXIT_FAILURE);
    }
    printf("press q and press enter to exit\n");
    int ch;
    do { ch =getchar();}while(ch!='q');
}
struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size*nmemb;
}
 

int init(char *key,void (*onConnectionEstablished)(),int (*onMessageArrived)())
{
    CURL *curl;
    CURLcode res;
    Global_key = key;
    struct string s;
    init_string(&s);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://airtrik.com/iot/");
        char *prepend = "key=";
        char * finalKey = (char *) malloc(1 + strlen(key)+ strlen(prepend) );
        strcpy(finalKey, prepend);
        strcat(finalKey, key);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalKey);
        #ifdef SKIP_PEER_VERIFICATION
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        #endif
     
        #ifdef SKIP_HOSTNAME_VERIFICATION
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        #endif
        curl_easy_setopt (curl, CURLOPT_VERBOSE, 0L); //0 disable messages/verbose
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        }
        parsed_json = json_tokener_parse(s.ptr);        
        json_object_object_get_ex(parsed_json,"username",&username);
        json_object_object_get_ex(parsed_json,"password",&password);
        json_object_object_get_ex(parsed_json,"error",&error);
        /*Still left JSON device and name parser */
        if(json_object_get_string(error)){
            if(strcmp(json_object_get_string(error),"Error Wrong Credentials"))
            {
                printf("Please enter Correct key \n");
                return 0;
            }   
        }
        mqtt_init((char*)json_object_get_string(username),(char*)json_object_get_string(password),onConnectionEstablished,onMessageArrived);
        free(s.ptr);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 1;
}


