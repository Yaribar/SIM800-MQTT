#include <Arduino.h>

#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 512
#define RXD2 16 //Hardware Serial
#define TXD2 17 //Hardware Serial
#define SAMPLING_PERIOD 100

#include <TinyGsmClient.h>
#include <PubSubClient.h>

//************************
//**  GPRS CREDENTIALS  **
//************************

const char apn[] = "internet.itelcel.com";
const char user[] = "webgprs";
const char pass[] = "webgprs2002";

//*************************
//** MQTT CONFIGURATION ***
//*************************

const String serial_number = "123456789";

const char *mqtt_server = "c-iot.ml"; //Domain name or IP
const uint16_t mqtt_port = 1883; //MQTT port TCP
const char *mqtt_user = ""; //MQTT user
const char *mqtt_pass = ""; //MQTT password


TinyGsm modem(Serial2);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

long lastMsg = 0; //Variable to save last message
char msg[25]; //Character array to receive message

//*************************
//**FUNCTION DECLARATION***
//*************************

void setupModem();
void reconnect();

//************************
//*******  SETUP  ********
//************************

void setup() {
    Serial.begin(115200); //initialize serial port
    Serial2.begin(115200, SERIAL_8N1, RXD2,TXD2); //Baud rate
    randomSeed(micros());
    setupModem(); 
    mqtt.setServer(mqtt_server, mqtt_port); // MQTT Broker setup
}

//*************************
//*******   LOOP   ********
//*************************

void loop() {
    if(!mqtt.connected()){//Check if the board is connected to the server
        reconnect();
    }

    mqtt.loop();
    
    long now = millis(); //this variable is useful to set a sample time
    if(now - lastMsg > SAMPLING_PERIOD){
        lastMsg = now;
        String msg_send = String(millis());
        msg_send.toCharArray(msg, 25);

        char topic[25];
        String topic_aux = serial_number + "/temp";
        topic_aux.toCharArray(topic,25);

        mqtt.publish(topic, msg);
    }
}

//************************
//***   SETUP MODEM   ****
//************************

void setupModem(){
    delay(10);
    Serial.println("Initializing modem...");
    modem.init();

    Serial.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
        Serial.println("fail");
        while (true);
    }
    Serial.println(" OK");

    Serial.print("Connecting to ");
    Serial.print(apn);
    if (!modem.gprsConnect(apn, user, pass)) {
        Serial.println(" fail");
        while (true);
    }
    Serial.println(" OK");
}

//***********************
//***   CONNECTION   ****
//***********************

void reconnect(){
    while(!mqtt.connected()){
        Serial.print("Trying connection MQTT...");

        String clientId = "esp32_";
        clientId += String(random(0xffff),HEX);

        if(mqtt.connect(clientId.c_str(),mqtt_user,mqtt_pass)){//connects to MQTT
            Serial.println("Connected!");

            char topic[25];
            String topic_aux = serial_number + "/temp";
            topic_aux.toCharArray(topic,25);
            mqtt.subscribe(topic);//subscribe to topic command

        }else{
            Serial.print("fail :( with error -> ");
            Serial.print(mqtt.state());
            Serial.println(" Try again in 5 seconds");

            delay(5000);
        }    
    }
}
