#include <SPI.h>
#include <Ethernet.h>

#define POWER_SWITCH 13
#define RESET_SWITCH 12

#define SHORTING 2
#define SHORT_INPUT 3

const bool debug = false;

const byte mac[] = { 0xA2, 0x6B, 0xA3, 0x93, 0xEE, 0x07 };
const char server[] = "192.168.100.3";
const char method[] = "/ui/sessions/signin";

const long req_time = 60000;
const long boot_time = 50000;

EthernetClient client;

bool init_ethernet() {
  Serial.println("[Ethernet] Initialize Ethernet with DHCP:");

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("[Ethernet] Ethernet cable is not connected.");
    return false;
  }  

  if (Ethernet.begin(mac) == 0) {
    Serial.println("[Ethernet] Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("[Ethernet] Ethernet shield was not found. Sorry, can't run without hardware. :(");
      return false;
    }
    
    return false;
  }

  Serial.print("[Ethernet] DHCP assigned IP: ");
  Serial.println(Ethernet.localIP());
  return true;
}

bool get_request() {
  if (!client.connect(server, 80)) {
    Serial.println("[Request] Connection failed");
    return false;
  }

  client.println("GET " + String(method) + " HTTP/1.1");
  client.println("Host: " + String(server));
  client.println("Connection: close");
  client.println();

  Serial.println("[Request] Connected!");

  return true;
}

void shorting() {
  Serial.println("[SHORTED] Now device is management mode. If you wanna disable it, Please unplugged short pin.");
  while (is_shorted(SHORT_INPUT)) {
    delay(1);    
  }
}

bool is_shorted(uint8_t input) {
  return digitalRead(input);
}

void push_button(uint8_t btn) {
  digitalWrite(btn, 1);
  delay(50);
  digitalWrite(btn, 0);
}

void setup() {
  Serial.begin(9600);
  pinMode(POWER_SWITCH, OUTPUT);
  pinMode(SHORTING, OUTPUT);
  pinMode(SHORT_INPUT, INPUT);

  digitalWrite(SHORTING, 1);
  if (is_shorted(SHORT_INPUT)) {
    shorting();
  }

  if (!init_ethernet()) {
    Serial.println("[Ethernet] Connection failed");
    Serial.println("[FATA] If you wanna using management, please connect ethernet cable and reboot power management device.");
    while (true) {
      delay(1);
    }
  }
}

void loop() {
  if (is_shorted(SHORT_INPUT)) {
    shorting();
  }

  if (get_request()) {
    delay(req_time);
    return;
  }

  Serial.println("[Core] Server not connected, pushing power button and booting...");
  push_button(POWER_SWITCH);
  delay(boot_time);
}
