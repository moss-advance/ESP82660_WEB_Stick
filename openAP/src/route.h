#ifndef ROUTE_H
#define ROUTE_H
#include <Arduino.h>
#include <LittleFS.h>
// #include <DNSServer.h>
#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LEDdrive.h"
AsyncWebServer server(80);
// DNSServer dnsServer;

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}
void routeInit()
{
    // dnsServer.start(53, "*", WiFi.softAPIP());
    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });

    // 静态资源调制
    server.serveStatic("/", LittleFS, "/");
    //自定义交互功能
    server.on("/colorchange", HTTP_POST, [](AsyncWebServerRequest *request)
              {
                    // 读取对应颜色表单进行处理
                    String jsonStr = request->getParam(0)->value();
                        // Allocate JsonBuffer
                    JsonDocument doc;

                    // Deserialize the JSON document
                    DeserializationError error = deserializeJson(doc, jsonStr);
                        if (error) {
                            Serial.print(F("deserializeJson() failed: "));
                            Serial.println(error.c_str());
                            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
                        }
                            serializeJson(doc, Serial);//输出json到串口
                            Serial.print(doc["t"].as<String>());
                            Serial.println("表单颜色发生了变化");
                            
                            String mode = doc["v"].as<String>();

                            if(mode=="f1"){
                                if(guangjian(doc["c"].as<String>())){
                                    Serial.println("成功");
                                    request->send(200, "application/json", "{\"message\":\"颜色调制成功\"}");
                                }else{
                                    request->send(200, "application/json", "{\"message\":\"颜色调制失败，请重试\"}");
                                }
                            }else if(mode=="f2"){
                                Serial.println("成功");
                                if(donggan(doc["c1"].as<String>(),doc["c2"].as<String>())){
                                    request->send(200, "application/json", "{\"message\":\"颜色调制成功\"}");
                                }else{
                                    request->send(200, "application/json", "{\"message\":\"颜色调制失败，请重试\"}");
                                }
                            }else if(mode == "f3"){
                                if(fashe(doc["c"].as<String>())){
                                    Serial.println("成功");
                                    request->send(200, "application/json", "{\"message\":\"颜色调制成功\"}");
                                }else{
                                    request->send(200, "application/json", "{\"message\":\"颜色调制失败，请重试\"}");
                                }
                            }
         });
    server.on("/draw", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                draw();
            });
    server.on("/StickStatus",HTTP_GET,[](AsyncWebServerRequest *request)
            {
                 request->send(200, "application/json",getStickStatus());
            });
    server.on("/set_Brightness", HTTP_POST, [](AsyncWebServerRequest *request)
              {
                        set_Brightness(request->getParam(0)->value());
                        request->send(200, "application/json", "{\"message\":\"调节完毕\"}"); 
             });
    server.on("/knock", HTTP_POST, [](AsyncWebServerRequest *request)
              { Serial.println("knock");
                if(fashe(request->getParam(0)->value())){
                    request->send(200, "application/json", "{\"message\":\"发送成功\"}"); 
                }
                request->send(500, "application/json", "{\"message\":\"发送失败\"}"); 
            });
    server.on("/saveConfig", HTTP_POST,[](AsyncWebServerRequest *request)
            { 
                if(saveConfig(request->getParam(0)->value())){
                    request->send(200, "application/json", "{\"message\":\"保存配置成功\"}"); 
                }else{
                    request->send(200, "application/json", "{\"message\":\"保存失败\"}"); 
                }
            });
    server.on("/img", HTTP_POST, [](AsyncWebServerRequest *request)
              {},
                [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                  saveImg(request,filename,index,data,len,final);
            }
            );

    server.onNotFound(notFound);
    server.begin();
}

#endif