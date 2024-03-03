#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "route.h"
#include "ESPAsyncWebServer.h"

void setup()
{
	// your other setup stuff...
	WiFi.softAP("一个优秀的wifi");
	server.begin();
	Serial.begin(115200);
	Serial.flush();
	// Initialize LittleFS
	if(!LittleFS.begin()){
		Serial.println("An Error has occurred while mounting LittleFS");
		return;
	}
	WiFi.begin();
	Serial.println("wifi加载成功");
	strip.Begin();
	Serial.println("NeoPixelBus加载成功");
	routeInit();//路径问题处理
}

void loop()
{
	animations.UpdateAnimations();
}