/*
 * WiFi Tech-support:
 * "Have you tried turning it off and on again?"
 * 
 * This is a small program for an ESP32 which will periodically check if there is internet connectivity.
 * If there is not then it will shortly trigger a relay which is connected to the power supply of the modem, therefore rebooting it.
 * 
 * The program runs a state machine with 3 states; test, pass, and fail.
 * In test state it will ping cloudfare's 1.1.1.1 servers.
 * If this is succesfull it will go to the pass state, and otherwise will go to the fail state.
 * The pass state will wait for few minutes, and will then go back to the test state.
 * The fail state will trigger the relay for a short time, wait until it is connected to the WiFi, 
 * and will then wait for another few minutes before returning to the test state.
 * 
 * This program uses the build-in WiFi library, and the ESP32Ping library (https://github.com/marian-craciunescu/ESP32Ping)
 * 
 * (modem crashes frequently now I'm mostly working from home, and I got tired of rebooting it myself everyday)
 */

#include <WiFi.h>
#include <ESP32Ping.h>

#define MINUTES (60L * 1000L)
#define SECONDS  1000L


const int relay_pin = 13;

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const IPAddress remote_ip(1, 1, 1, 1); //cloudfare

const unsigned long  PASS_DELAY = 1 * MINUTES;  
const unsigned long  FAIL_DELAY = 10 * MINUTES;  
const unsigned long  RESET_PULSE_DELAY = 5 * SECONDS; 

enum {  
    TEST_STATE=0, PASS_STATE=1, FAIL_STATE=2
};
int state = TEST_STATE;


void reboot_modem() {
  Serial.println("Rebooting modem");
  digitalWrite(relay_pin, 1);
  delay(RESET_PULSE_DELAY);
  digitalWrite(relay_pin, 0);

  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected with ip ");  
  Serial.println(WiFi.localIP());
}


void setup() {
  Serial.begin(115200);
  delay(10);
  
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected with ip ");  
  Serial.println(WiFi.localIP());
}


void loop() {
  switch (state) {
    case TEST_STATE:
      if (Ping.ping(remote_ip)) {
        Serial.println("pass");
        state = PASS_STATE;
      } else {
        Serial.println("fail");
        state = FAIL_STATE;
      }
      break;

    case PASS_STATE:
      delay(PASS_DELAY);
      state = TEST_STATE;
      break;

    case FAIL_STATE:
      reboot_modem();
      delay(FAIL_DELAY);
      state = TEST_STATE;
      break;
  }
}
