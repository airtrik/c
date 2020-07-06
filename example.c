#include"airtrik.h"
void onconnect(){
    printf("Connection Established\n");
    subscribe_topic("1");
    send_message("1","hello from client");
    int ch;
}    



int onReceive(char* msg, char *deviceId){
    printf("Message Received \n");
    printf("deviceId : %s\nmsg : %s \n",deviceId,msg);
    return 1;
}



int main(int argc, char const *argv[])
{
    init("__APP_KEY__",onconnect,onReceive);

	return 0;
}