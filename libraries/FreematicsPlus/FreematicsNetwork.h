/*************************************************************************
* Telematics Data Logger Class
* Distributed under BSD license
* Developed by Stanley Huang https://www.facebook.com/stanleyhuangyc
*************************************************************************/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "FreematicsBase.h"

#define EVENT_LOGIN 1
#define EVENT_LOGOUT 2
#define EVENT_SYNC 3
#define EVENT_RECONNECT 4
#define EVENT_COMMAND 5
#define EVENT_ACK 6

typedef struct {
  float lat;
  float lng;
  uint8_t year; /* year past 2000, e.g. 15 for 2015 */
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} NET_LOCATION;

class NullClient
{
public:
  virtual bool begin() { return true; }
  virtual void end() {}
  virtual bool open(const char* host, uint16_t port) = 0;
  virtual void close() {}
  virtual bool send(const char* data, unsigned int len) = 0;
  virtual char* receive(int* pbytes, unsigned int timeout) = 0;
  virtual bool getLocation(NET_LOCATION* loc) { return false; }
  virtual const char* deviceName() = 0;
};

class UDPClientWIFI : virtual NullClient
{
public:
    bool begin();
    void end();
    bool setup(const char* ssid, const char* password, unsigned int timeout = 15000);
    String getIP();
    int getSignal() { return 0; }
    bool open(const char* host, uint16_t port);
    void close();
    bool send(const char* data, unsigned int len);
    char* receive(int* pbytes = 0, unsigned int timeout = 5000);
    String queryIP(const char* host);
    const char* deviceName() { return "WIFI"; }
  private:
    void listAPs();
    char m_buffer[256] = {0};
    IPAddress udpIP;
    uint16_t udpPort;
    WiFiUDP udp;
};

class UDPClientSIM800 : virtual NullClient
{
public:
    bool begin(CFreematics* device);
    void end();
    bool setup(const char* apn, unsigned int timeout = 60000);
    String getIP();
    int getSignal();
    String getOperatorName();
    bool open(const char* host, uint16_t port);
    bool send(const char* data, unsigned int len);
    void close();
    char* receive(int* pbytes = 0, unsigned int timeout = 5000);
    bool getLocation(NET_LOCATION* loc);
    String queryIP(const char* host);
    const char* deviceName() { return "SIM800"; }
protected:
    bool sendCommand(const char* cmd, unsigned int timeout = 1000, const char* expected = "\r\nOK", bool terminated = false);
    char* checkIncoming(int* pbytes);
    char m_buffer[256] = {0};
    uint8_t m_stage = 0;
    CFreematics* m_device = 0;
};

class UDPClientSIM5360 : virtual NullClient
{
public:
    bool begin(CFreematics* device);
    void end();
    bool setup(const char* apn, bool only3G = false, bool roaming = false, unsigned int timeout = 60000);
    String getIP();
    int getSignal();
    String getOperatorName();
    bool open(const char* host, uint16_t port);
    void close();
    bool send(const char* data, unsigned int len);
    char* receive(int* pbytes = 0, unsigned int timeout = 5000);
    String queryIP(const char* host);
    virtual const char* deviceName() { return "SIM5360"; }
protected:
    // send command and check for expected response
    bool sendCommand(const char* cmd, unsigned int timeout = 1000, const char* expected = "\r\nOK\r\n", bool terminated = false);
    char* checkIncoming(int* pbytes);
    char m_buffer[256] = {0};
    char udpIP[16] = {0};
    uint16_t udpPort = 0;
    uint8_t m_stage = 0;
    CFreematics* m_device = 0;
};

class UDPClientSIM7600 : public UDPClientSIM5360
{
public:
    const char* deviceName() { return "SIM7600"; }
};
