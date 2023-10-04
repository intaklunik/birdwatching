#include "network.h"
#include <WiFi.h>
#include <arpa/inet.h>

#define SSID "***"
#define PASSWORD "***"

#define IPADDRESS "192.168.0.110"
#define PORT 12345

#define MAX_TRY 10

int network_init()
{
  WiFi.begin(SSID, PASSWORD);

  for (int i = 0; i < MAX_TRY && WiFi.status() != WL_CONNECTED; ++i) {
    delay(500);
    Serial.println("...");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi didn't connected");
    return -1;
  }

  Serial.print("WiFi connected with IP ");
  Serial.println(WiFi.localIP());

  return 0;
}

void connect_to_server(uint8_t *buffer, uint32_t size)
{
  WiFiClient client;
  while(!client.connect(IPADDRESS, PORT))
    ;
  Serial.println("Client connected");

  size = htonl(size);
  
  client.write((char *)&size, sizeof(size));
  Serial.println("Sent a size of the buffer to the host");
  
  client.write(buffer, size);
  Serial.println("Sent data to the host");

  while (client.connected()) {
   Serial.println("Still connected...");
   delay(2000); 
  }

  client.stop();
  Serial.println("Stop communication");
}
