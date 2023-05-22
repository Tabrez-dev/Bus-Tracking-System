#include <Adafruit_Sensor.h>
#include "DHT.h"
#define DHTPIN 13   // Any pin 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#include "ThingSpeak.h"
#include <WiFiClient.h>
#include "twilio.hpp"

const int LEDPIN = 22; 
const int PushButton=15;


#include <TinyGPS.h>
TinyGPS gps;
#include <SoftwareSerial.h>
SoftwareSerial ss(16,17);//arduino rx,tx
#include <WiFi.h>
const char* ssid     = "Tab";
const char* password = "123456789";
WiFiServer server(80);
String yazi;

//for thngspeak interface
WiFiClient  client;
unsigned long Channel_ID = 1873256;     //thingspeak 
const char * API_Key = "2CZJFL968HS5IXR1";
unsigned long last_time = 0;
unsigned long Delay = 5000;

//TWILIO

static const char *account_sid = "ACe64315c421fb501ca4b3536277251ca6";
static const char *auth_token = "7328d9d51cb4674eb8385bf0339b721f";
// Phone number should start with "+<countrycode>"
static const char *from_number = "+15736523616";
static const char *message="406- BASAVANAHALLI NEXT STOP NELMANGALA. Live location: 192.168.254.41";
// You choose!
// Phone number should start with "+<countrycode>"
static const char *to_number = "+919591777586";
  String response;
//  Kempegowda Bus Station, Majestic, Anand Rao Circle, Race Course Road, Sivananda Stores, Bangalore Golf Course, Windsor Manor, Palace Guttahalli, Cauvery Theater,
Twilio *twilio;



void setup()
{
   pinMode(LEDPIN, OUTPUT);
    pinMode(PushButton, INPUT);
    Serial.begin(9600);
    ss.begin(9600);
    Serial.print("Connecting to ");
    Serial.println(ssid);

if(WiFi.status() != WL_CONNECTED){
      Serial.print("Connecting...");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

 
      Serial.println(F("DHTxx test!"));
     dht.begin();
     delay(2000);
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");    // this is the address to use for viewing the map
    Serial.println(WiFi.localIP());
    server.begin();
    twilio = new Twilio(account_sid, auth_token);
}

void loop(){
  Serial.println("IP address: ");    // this is the address to use for viewing the map
    Serial.println(WiFi.localIP());
     int Push_button_state = digitalRead(PushButton);
      if ( Push_button_state == HIGH )
    {
      digitalWrite(LEDPIN, HIGH);
      delay(500);
       digitalWrite(LEDPIN, LOW);
          bool success = twilio->send_message(to_number, from_number, message, response);
         if (success) {
                Serial.println("Sent message successfully!");
              }else {
                  Serial.println(response);
                    }
    }
    else 
    {
    digitalWrite(LEDPIN, LOW); 
    }
  smartdelay(1000);
  

     if ((millis() - last_time) > Delay) {
      float flat, flon;
  unsigned long age;
   gps.f_get_position(&flat, &flon, &age);
     float h = dht.readHumidity();
     float t = dht.readTemperature();
     float lati=flat;
     float lan=flon;
    
     ThingSpeak.setField(1,t);
     ThingSpeak.setField(2,h);
     ThingSpeak.setField(3,lati);
     ThingSpeak.setField(4,lan);

      int Data = ThingSpeak.writeFields(Channel_ID, API_Key);
     
     if(Data == 200){
      Serial.println("Channel updated successfully!");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(Data));
    }
    last_time = millis();
  }

    
 WiFiClient client = server.available();    // listen for incoming clients
  float flat, flon;
  unsigned long age;
   gps.f_get_position(&flat, &flon, &age);
  if (client) {                             
    Serial.println("new client");          
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            
      if (client.available()) {             // if there's client data
        char c = client.read();          // read a byte
          if (c == '\n') {                      // check for newline character,
          if (currentLine.length() == 0) {  // if line is blank it means its the end of the client HTTP request
      
yazi="<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta http-equiv='X-UA-Compatible' content='ie=edge'><title>My Google Map</title><style>#map{height:400px;width:100%;}</style></head> <body><h1>My Google Map</h1><div id='map'></div><script>function initMap(){var options = {zoom:8,center:{lat:";
    yazi+=flat;
    yazi+=",lng:";
    yazi+=flon;
    yazi+="}};var map = new google.maps.Map(document.getElementById('map'), options);google.maps.event.addListener(map, 'click', function(event){addMarker({coords:event.latLng});});var markers = [{coords:{lat:";
yazi+=flat;
yazi+=",lng:";
yazi+=flon;
yazi+="}}];for(var i = 0;i < markers.length;i++){addMarker(markers[i]);}function addMarker(props){var marker = new google.maps.Marker({position:props.coords,map:map,});if(props.iconImage){marker.setIcon(props.iconImage);}if(props.content){var infoWindow = new google.maps.InfoWindow({content:props.content});marker.addListener('click', function(){infoWindow.open(map, marker);});}}}</script><script async defer src='https://maps.googleapis.com/maps/api/js?key=AIzaSyDHNUG9E870MPZ38LzijxoPyPgtiUFYjTM&callback=initMap'></script></body></html>";
 
 
 
 client.print(yazi);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {   currentLine = ""; }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
         // here you can check for any keypresses if your web server page has any
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    }
}
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);

  float flat, flon;
unsigned long fix_age; // returns +- latitude/longitude in degrees
gps.f_get_position(&flat, &flon, &fix_age);
if (fix_age == TinyGPS::GPS_INVALID_AGE)
  Serial.println("No fix detected");
else if (fix_age > 5000)
  Serial.println("Warning: possible stale data!");
else
  Serial.println("Data is current.");
}