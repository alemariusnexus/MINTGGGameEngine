#include "NetworkEngine.h"

#include "../util/Log.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include "esp_system.h"
#include "esp_wifi.h"
#endif

#include <algorithm>


LOG_USE_TAG("NetworkEngine")


namespace MINTGGGameEngine
{

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
struct HTTPRequestContext
{
    NetworkEngine* engine;

    uint8_t* respBuf;
    size_t respBufSize;
    size_t respNumWritten;
    size_t respLenTotal;
};
#endif



#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

void NetworkEngineESPEventHandler (
    void* arg, esp_event_base_t evtBase, int32_t evtID, void* evtData
) {
    static_cast<NetworkEngine*>(arg)->espEventHandler(evtBase, evtID, evtData);
}

esp_err_t NetworkEngineESPHTTPEventHandler(esp_http_client_event_t* evt)
{
    return static_cast<HTTPRequestContext*>(evt->user_data)->engine->espHTTPEventHandler(evt);
}

#endif





NetworkEngine::NetworkEngine()
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    : connected(false)
#endif
{
}

bool NetworkEngine::begin()
{
    return true;
}

void NetworkEngine::addWifiConfig(const char* ssid, const char* password)
{
    wifiConfigs.emplace_back(ssid, password);
}

bool NetworkEngine::start()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (wifiConfigs.empty()) {
        return false;
    }

    esp_err_t res;

    res = esp_netif_init();
    if (res != ESP_OK) {
        LogError("Error initializing ESP Netif: %s", esp_err_to_name(res));
        return false;
    }

    res = esp_event_loop_create_default();
    if (res != ESP_OK) {
        LogError("Error creating event loop: %s", esp_err_to_name(res));
        return false;
    }
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    res = esp_wifi_init(&cfg);
    if (res != ESP_OK) {
        LogError("Error initializing WiFi: %s", esp_err_to_name(res));
        return false;
    }

    esp_event_handler_instance_t instAnyID;
    esp_event_handler_instance_t instGotIP;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &NetworkEngineESPEventHandler,
                                                        this,
                                                        &instAnyID));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &NetworkEngineESPEventHandler,
                                                        this,
                                                        &instGotIP));

    wifi_config_t wifiCfg = {
        .sta = {
            .threshold = {
                .authmode = WIFI_AUTH_WPA_PSK
            },
            .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK
        }
    };
    if (wifiConfigs[0].ssid.length() >= sizeof(wifiCfg.sta.ssid)) {
        LogError("WiFi SSID too long.");
        return false;
    }
    if (wifiConfigs[0].password.length() >= sizeof(wifiCfg.sta.password)) {
        LogError("WiFi password too long.");
        return false;
    }
    strcpy(reinterpret_cast<char*>(wifiCfg.sta.ssid), wifiConfigs[0].ssid.c_str());
    strcpy(reinterpret_cast<char*>(wifiCfg.sta.password), wifiConfigs[0].password.c_str());

    res = esp_wifi_set_mode(WIFI_MODE_STA);
    if (res != ESP_OK) {
        LogError("Error setting WiFi mode: %s", esp_err_to_name(res));
        return false;
    }

    res = esp_wifi_set_config(WIFI_IF_STA, &wifiCfg);
    if (res != ESP_OK) {
        LogError("Error setting WiFi config: %s", esp_err_to_name(res));
        return false;
    }

    res = esp_wifi_start();
    if (res != ESP_OK) {
        LogError("Error starting WiFi: %s", esp_err_to_name(res));
        return false;
    }

    return true;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    if (wifiConfigs.empty()) {
        return false;
    }
    WiFi.begin(wifiConfigs[0].ssid.c_str(), wifiConfigs[0].password.c_str());
    return true;
#else
    LogWarning(TAG, "NetworkEngine is not supported on this platform!");
    return false;
#endif
}

bool NetworkEngine::isConnected() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return connected;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    return WiFi.status() == WL_CONNECTED;
#endif
}

uint8_t* NetworkEngine::sendHTTPGetRequestRaw(const std::string& url, size_t* outRespLen)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    HTTPRequestContext reqCtx = {
        .engine = this,
        .respBuf = nullptr,
        .respBufSize = 0,
        .respNumWritten = 0,
        .respLenTotal = 0
    };

    esp_http_client_config_t config = {
        .url = "https://dlerch.de/esptest.php",
        .disable_auto_redirect = false,
        .event_handler = &NetworkEngineESPHTTPEventHandler,
        .user_data = &reqCtx
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        if (reqCtx.respBuf) {
            reqCtx.respBuf[reqCtx.respNumWritten] = '\0';
        }
        if (outRespLen) {
            *outRespLen = reqCtx.respNumWritten;
        }
    } else {
        LogError("HTTP GET request failed: %s", esp_err_to_name(err));
        free(reqCtx.respBuf);
        reqCtx.respBuf = nullptr;
        if (outRespLen) {
            *outRespLen = 0;
        }
    }

    esp_http_client_cleanup(client);

    return reqCtx.respBuf;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    HTTPClient http;
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.begin(url.c_str());
    int httpCode = http.GET();

    if (httpCode < 0) {
        LogError("Error sending HTTP request: %s", http.errorToString(httpCode));
        if (outRespLen) *outRespLen = 0;
        return nullptr;
    }

    String respArdStr = http.getString();
    uint8_t* respStr = static_cast<uint8_t*>(malloc(respArdStr.length()+1));
    if (!respStr) {
        if (outRespLen) *outRespLen = 0;
        return nullptr;
    }

    memcpy(respStr, respArdStr.c_str(), respArdStr.length());
    respStr[respArdStr.length()] = '\0';
    if (outRespLen) *outRespLen = respArdStr.length();
    return respStr;
#else
    if (outRespLen) *outRespLen = 0;
    return nullptr;
#endif
}

std::string NetworkEngine::sendHTTPGetRequest(const std::string& url)
{
    size_t dataLen;
    uint8_t* data = sendHTTPGetRequestRaw(url, &dataLen);
    if (!data) {
        return {};
    }
    // TODO: Use custom allocator to be able to catch out of memory errors
    std::string respStr(reinterpret_cast<char*>(data), dataLen);
    free(data);
    return respStr;
}




#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

void NetworkEngine::espEventHandler (
    esp_event_base_t evtBase, int32_t evtID, void* evtData
) {
    esp_err_t res;

    if (evtBase == WIFI_EVENT  &&  evtID == WIFI_EVENT_STA_START) {
        res = esp_wifi_connect();
        if (res != ESP_OK) {
            LogError("Error connecting WiFi: %s", esp_err_to_name(res));
        }
    } else if (evtBase == WIFI_EVENT  &&  evtID == WIFI_EVENT_STA_DISCONNECTED) {
        connected = false;
        res = esp_wifi_connect();
        if (res != ESP_OK) {
            LogError("Error connecting WiFi: %s", esp_err_to_name(res));
        }
    } else if (evtBase == IP_EVENT  &&  evtID == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* evt = (ip_event_got_ip_t*) evtData;
        connected = true;

        LogInfo("Got IP: " IPSTR, IP2STR(&evt->ip_info.ip));
    }
}

esp_err_t NetworkEngine::espHTTPEventHandler(esp_http_client_event_t* evt)
{
    HTTPRequestContext* reqCtx = static_cast<HTTPRequestContext*>(evt->user_data);

    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        if (!esp_http_client_is_chunked_response(evt->client)) {
            if (!reqCtx->respBuf) {
                int64_t contentLen = esp_http_client_get_content_length(evt->client);
                if (contentLen >= 0) {
                    reqCtx->respBuf = static_cast<uint8_t*>(malloc(contentLen+1));
                    reqCtx->respBufSize = contentLen;
                }
            }

            if (reqCtx->respBuf) {
                const size_t copySize = std::min(static_cast<size_t>(evt->data_len),
                        reqCtx->respBufSize-reqCtx->respNumWritten);
                memcpy(reqCtx->respBuf + reqCtx->respNumWritten, evt->data, copySize);
                reqCtx->respNumWritten += copySize;
                reqCtx->respLenTotal += static_cast<size_t>(evt->data_len);
            }
        }
    }

    return ESP_OK;
}

#endif


}
