#include "aws_iot_config.h"

aws_iot_mqtt_client myClient; // init iot_mqtt_client
char msg[32]; // read-write buffer
int cnt = 0; // loop counts
int rc = -100; // return value placeholder
bool success_connect = false; // whether it is connected
// Basic callback function that prints out the message
void msg_callback(char* src, unsigned int len, Message_status_t flag) {
 if(flag == STATUS_NORMAL) {
 Serial.println("CALLBACK:");
 int i;
 for(i = 0; i < (int)(len); i++) {
 Serial.print(src[i]);
 }
 Serial.println("");
 }
}
...
void loop() {
 if(success_connect) {
 // Generate a new message in each loop and publish to "topic1"
 sprintf(msg, "new message %d", cnt);
 if((rc = myClient.publish("topic1", msg, strlen(msg), 1, false)) != 0) {
 Serial.println(F("Publish failed!"));
 Serial.println(rc);
 }
 
...
 
 delay(1000);