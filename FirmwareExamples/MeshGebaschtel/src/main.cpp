#include <ESP8266WiFi.h>
#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>


#define NAME "h2obrain"

#define WIFI_SSID_PREFIX  "luxi_"
#define WIFI_SSID  WIFI_SSID_PREFIX NAME
#define WIFI_PSK   "luxi"
#define WIFI_MIN_RSSI -90  // -75

#define LED_PIN 4
#define LED_COUNT 214
// Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#define HORN 0
#define EYE 1

#define LED_MATRIX_WIDTH 19
#define LED_MATRIX_HEIGHT 11
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, 
  1, 2, // rows, cols - we use 2 cols to get enough LEDs for the scarf
  LED_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
    NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE + 
// progressive vs zigzag makes no difference for a 4 arrays next to one another
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800 );

void setScarfTop();
void setScarfBottom();
void setEye();
void setHorn();
void setMatrix();
void wifi_connect();
void wifi_create_ap();
String macToString(const unsigned char* mac);

void setup() {
  Serial.begin(115200);
  Serial.println("Spit Happens!");
  matrix->begin();
  matrix->setBrightness(15);
  matrix->setPixelColor(0, 255, 0, 0);
  matrix->setTextWrap(false);
  matrix->setTextSize(1);
  matrix->show();
  wifi_connect();
  wifi_create_ap();
}
int pos=0;
void loop() {
    pos++;

    matrix->clear();

    // set color for matrix first
    setMatrix();

    // set eye, horn, scarfs
    setEye();
    setHorn();
    setScarfTop();
    setScarfBottom();

    matrix->show();

    wifi_connect();

    delay(50);
}

void setMatrix()
{
    int fontSize = max(int(LED_MATRIX_WIDTH/8), 1);
    int animation1Frames = 78;
    int animation2Frames = 11*8*fontSize+1;
    int animation1Length = animation1Frames;
    int animation2Length = animation2Frames/2;
    int step = pos%(animation1Length + animation2Length);

    // horizontal text
    if (step<animation1Length) {
        int cursorPos = 7-step;
        matrix->setCursor(cursorPos,2);
        matrix->setTextWrap(false);
        matrix->setTextSize(1);
        matrix->setRotation(0);
        matrix->setTextColor(matrix->Color(0,0,255));
        matrix->print("  " NAME);
    }

    // vertical text
    else {
        step -= animation1Length;
        int cursorPos = 1*8*fontSize-step;
        matrix->setCursor(cursorPos*2,LED_MATRIX_WIDTH/2-fontSize*4+1);
        matrix->setTextWrap(false);
        matrix->setTextSize(fontSize);
        matrix->setRotation(3);
        if (step<(animation2Length/2-10)) {
            matrix->setTextColor(matrix->Color(0,255,255));
        }
        else {
            matrix->setTextColor(matrix->Color(255,255,0));
        }
        matrix->print("Spit Happens!");
    }
}

void setEye()
{
    matrix->setPixelColor(EYE, 0,255,30);
}

void setHorn()
{
    matrix->setPixelColor(HORN, 50,100,150);   
}

void setScarfBottom()
{
    int ledsScarfBottom[7] = {16,17,18,37,170,189,190};

    for(int i : ledsScarfBottom)
    {
        matrix->setPixelColor(i,255,0,255);
    }
}

void setScarfTop()
{
    int ledsScarfTop[7] = {207,208,209,210,211,212,213};

    for(int i : ledsScarfTop)
    {
        matrix->setPixelColor(i,255,0,255);
    }
}




// Replace these with your WiFi network settings

// WiFi.on* methods **must** only be called **after** entering setup().
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt);
void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt);

void wifiScanComplete(int state);

void wifi_create_ap() {
    // WiFi.softAPConfig
    boolean result = WiFi.softAP(WIFI_SSID, WIFI_PSK);
    if(result==true) {
        IPAddress myIP = WiFi.softAPIP();
    
        Serial.println("done!");
        Serial.println("");
        Serial.println("WiFi network name: " WIFI_SSID);
        Serial.println("WiFi network password: " WIFI_PSK);
        Serial.print("Host IP Address: ");
        Serial.println(myIP);
        Serial.println("");

        // Call "onStationConnected" each time a station connects
        stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);

        // Call "onStationDisconnected" each time a station disconnects
        stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);


    } else {
        Serial.println("error! Something went wrong...");
    }
}

bool wifi_scanning = false;
void wifi_connect() {
    // finally connected
    static auto last_connection_status = WL_DISCONNECTED;
    auto connection_status = WiFi.status();
    if (last_connection_status != connection_status) {
        last_connection_status = connection_status;
        if (connection_status == WL_CONNECTED) {
            Serial.println("Connected, juhu!");
            Serial.println("success!");
            Serial.print("IP Address is: ");
            Serial.println(WiFi.localIP());
        }
    }
    if (!wifi_scanning) {
        bool find_aps = false;
        if (connection_status == WL_DISCONNECTED) {
            find_aps = true;
        } else
        if (WiFi.status() == WL_CONNECTED && WiFi.RSSI() < WIFI_MIN_RSSI) {
            find_aps= true;
        }
        if (find_aps) {
            // Start next scan
            wifi_scanning = true;
            // Connection is handled in the scan complete fuction
            WiFi.scanNetworksAsync(wifiScanComplete);
        }
    }
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
    Serial.print("Station connected: ");
    Serial.println(macToString(evt.mac));
    // Serial.println("success!");
    // Serial.print("IP Address is: ");
    // Serial.println(WiFi.localIP());
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
    Serial.print("Station disconnected: ");
    Serial.println(macToString(evt.mac));
}



void wifiScanComplete(int scanResult) {
    // TODO Best ap should maybe be the one furthest away, so the whole network is actually joined
    String ssid, best_ssid = "";
    int32_t rssi, best_rssi = WIFI_MIN_RSSI - 1;
    uint8_t encryptionType;
    uint8_t *bssid, best_bssid[6];
    int32_t channel;
    bool hidden;
    
    if (scanResult == 0) {
        Serial.println(F("No networks found"));
        return;
    } else if (scanResult <= 0) {
        Serial.printf(PSTR("WiFi scan error %d"), scanResult);
        return;
    }

    Serial.printf(PSTR("%d networks found:\n"), scanResult);

    // Print unsorted scan results
    for (int8_t i = 0; i < scanResult; i++) {
        WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);

        if (strncmp(ssid.c_str(), WIFI_SSID_PREFIX, sizeof(WIFI_SSID_PREFIX)) != 0) {
            continue;
        }

        if (best_rssi < rssi) {
            best_rssi = rssi;
            best_ssid = ssid;
            memcpy(best_bssid, bssid, sizeof(best_bssid));
        }

        // get extra info
        const bss_info *bssInfo = WiFi.getScanInfoByIndex(i);
        String phyMode;
        const char *wps = "";
        if (bssInfo) {
            phyMode.reserve(12);
            phyMode = F("802.11");
            String slash;
            if (bssInfo->phy_11b) {
                phyMode += 'b';
                slash = '/';
            }
            if (bssInfo->phy_11g) {
                phyMode += slash + 'g';
                slash = '/';
            }
            if (bssInfo->phy_11n) {
                phyMode += slash + 'n';
            }
            if (bssInfo->wps) {
                wps = PSTR("WPS");
            }
        }
        Serial.printf(PSTR("  %02d: [CH %02d] [%s] %ddBm %c %c %-11s %3S %s\n"), i, channel, macToString(bssid).c_str(), rssi, (encryptionType == ENC_TYPE_NONE) ? ' ' : '*', hidden ? 'H' : 'V', phyMode.c_str(), wps, ssid.c_str());
        // yield(); //
    }

    Serial.printf("Best ssid: %s [%s] %ddBm\n", best_ssid.c_str(), macToString(best_bssid).c_str(), rssi);
    wifi_scanning = false;

    if (best_rssi < WIFI_MIN_RSSI) {
        return;
    }


    if (WiFi.status() == WL_CONNECTED) {
        if (WiFi.RSSI() < WIFI_MIN_RSSI) {
            WiFi.disconnect();
        } else {
            return;
        }
    }

    // Connect 
    WiFi.begin(best_ssid, WIFI_PSK);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(best_ssid);
}

String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}