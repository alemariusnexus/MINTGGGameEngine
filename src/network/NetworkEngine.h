#pragma once

#include "../Globals.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include "esp_http_client.h"
#include "esp_wifi.h"
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#include <string>
#include <vector>


namespace MINTGGGameEngine
{


class NetworkEngine
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    friend void NetworkEngineESPEventHandler (
        void* arg, esp_event_base_t evtBase, int32_t evtID, void* evtData
        );
    friend esp_err_t NetworkEngineESPHTTPEventHandler(esp_http_client_event_t* evt);
#endif

private:
    struct WifiConfig
    {
        WifiConfig(const char* ssid, const char* password) : ssid(ssid), password(password) {}

        std::string ssid;
        std::string password;
    };

public:
    NetworkEngine();

    bool begin();

    void addWifiConfig(const char* ssid, const char* password);

    bool start();

    bool isConnected() const;

    uint8_t* sendHTTPGetRequestRaw(const std::string& url, size_t* outRespLen);
    std::string sendHTTPGetRequest(const std::string& url);

private:
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    void espEventHandler(esp_event_base_t evtBase, int32_t evtID, void* evtData);
    esp_err_t espHTTPEventHandler(esp_http_client_event_t *evt);
#endif

private:
    std::vector<WifiConfig> wifiConfigs;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    volatile bool connected;
    esp_http_client_handle_t httpClient;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
#endif
};


}