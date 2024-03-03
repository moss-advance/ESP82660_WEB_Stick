// 当牛做马的文件，完成动画和数据的结合
#ifndef LEDDRIVE_H
#define LEDDRIVE_H
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>
// 动画曲线
// AnimEaseFunction moveEase =
//      NeoEase::Linear;
//      NeoEase::QuadraticInOut;
//      NeoEase::CubicInOut;
        // NeoEase::QuarticInOut;
//      NeoEase::QuinticInOut;
//      NeoEase::SinusoidalInOut;
//      NeoEase::ExponentialInOut;
//      NeoEase::CircularInOut;
struct GlowStickStatus {
    // 枚举类型，用于表示不同的绘制状态
    enum Status {
        FILE_TRANSFER_IN_PROGRESS, // 文件传输中
        FILE_TRANSFER_FAILED,      // 文件传输失败
        FILE_PARSE_IN_PROGRESS,    // 文件解析中
        FILE_PARSE_FAILED,         // 文件解析失败
        DISPLAY_IN_PROGRESS,       // 展示中
        DISPLAY_COMPLETED,         // 展示完毕
        DISPLAY_FAILED             // 展示失败
    };

    Status currentStatus; // 当前状态
    String message;       // 可选的状态信息，用于说明失败原因或其他信息
};
// 全局的光辉棒状态
GlowStickStatus glowStickStatus;

const uint16_t PixelCount = 144; 
uint16_t animState=1;//保存draw当前行数
uint8_t Brightness = 120; //设置一个默认全局亮度
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount);

/**
 * @brief 
 * 这将初始化 NeoBitmapFile 以使用给定的文件。它将检查文件的内容中是否有有效的图像，并配置自身以从文件中读取数据。
    如果文件不是兼容的文件格式，它将返回 false。目前，它要求文件是 BMP、24 位或 32 位，没有压缩。
    传入的实例将由 NeoBitmapFile 对象管理，因此无需手动关闭它。file
 * 
 */
NeoBitmapFile<NeoGrbFeature, File> image;

NeoPixelAnimator animations(4);
// 转换 GlowStickStatus 结构体为 JSON 字符串
String toJson(GlowStickStatus& status) {
    JsonDocument doc;
    doc["currentStatus"] = status.currentStatus;
    doc["message"] = status.message;
    String jsonString;
    serializeJson(doc, jsonString);
    Serial.println(jsonString);
    return jsonString;
}
String getStickStatus(){
    return toJson(glowStickStatus);
}
void set_Brightness(String brightness){
    // 设置颜色
    uint8_t b = brightness.toInt();
    if(b>255 || b <0){
        Serial.println("brightness no range");
    }else{
        Brightness = b;
    } 
}
// 亮度加工厂。ColorBrignessFactory
RgbColor CBF(RgbColor color){
    // 将源color进行加权处理亮度
    RgbColor adjustedColor;
    adjustedColor.R = (color.R * Brightness) >> 8;
    adjustedColor.G = (color.G * Brightness) >> 8;
    adjustedColor.B = (color.B * Brightness) >> 8;
    return adjustedColor;
}
// 颜色转换工具
RgbColor hexToRgbColor(String hex) {
    if(hex.length() != 7){
        Serial.println("hex length err");
        return RgbColor(0, 0, 0);
    }
    if (hex.startsWith("#")) {
        hex = hex.substring(1);
    }

    // 将十六进制颜色代码转换为十进制数值
    long number = strtol(hex.c_str(), NULL, 16);

    // 提取红色、绿色和蓝色分量
    uint8_t r = (number >> 16) & 0xFF;
    uint8_t g = (number >> 8) & 0xFF;
    uint8_t b = number & 0xFF;

    // 使用 RGB 分量创建 RgbColor 对象并返回
    return CBF(RgbColor(r, g, b));
}

bool guangjian(String hexColor){
    animations.StopAll();
    strip.ClearTo(0);
    AnimEaseFunction moveEase = NeoEase::CubicInOut;
    animations.StartAnimation(0,1500,[=](const AnimationParam& param){
        // 设置曲线
       float progress = moveEase(param.progress);
         // 计算当前灯珠位置
        uint16_t currentPixel = (uint16_t)((float)progress * PixelCount);
        // 计算当前颜色
        RgbColor currentColor = RgbColor::LinearBlend(RgbColor(0, 0, 0), hexToRgbColor(hexColor),progress);
        // 点亮灯珠
        for (uint16_t i = 0; i <= currentPixel; i++) {
            strip.SetPixelColor(i, currentColor);
        }
        strip.Show();
    });
    return true;
}
bool donggan(String hexColor1,String hexColor2){
    animations.StopAll();
   animations.StartAnimation(1, 50, [=](const AnimationParam& param) {
        // 如果动画已经完成，则重启动画
        if (param.state == AnimationState_Completed) {
            int r = random(PixelCount);
            Serial.println(r);
            for (uint16_t i = 0; i < PixelCount; i++) {
                if (i < r) {
                    strip.SetPixelColor(i, RgbColor::LinearBlend(hexToRgbColor(hexColor1), hexToRgbColor(hexColor2),(float)i/PixelCount));
                } else {
                    strip.SetPixelColor(i, RgbColor(0,0,0));
                }
            }
            // 刷新显示
            strip.Show();
            animations.ChangeAnimationDuration(param.index,random(80,150));
            animations.RestartAnimation(param.index);
        }
    });
    return true;
}

bool fashe(String hexColor){
    const u8 pixel_num = 7;
    animations.StopAll();
    strip.ClearTo(0);
    for(uint8_t temp = 0;temp<pixel_num;temp++){
        strip.SetPixelColor(pixel_num-temp,RgbColor::LinearBlend(RgbColor(55, 55, 55), hexToRgbColor(hexColor),(float)(pixel_num-temp*0.2)/pixel_num));
    }
    animations.StartAnimation(2,3000,[=](const AnimationParam& param){
        strip.ShiftRight(1);
        strip.Show();
    });
    Serial.println("发射成功");

    return true;
}

bool draw() {
    animations.StopAll();
    File bmp = LittleFS.open("verisign.bmp","r");
    Serial.println("Image loading...");
    glowStickStatus.currentStatus = glowStickStatus.FILE_PARSE_IN_PROGRESS;
   if(!image.Begin(bmp)){
        glowStickStatus.currentStatus = glowStickStatus.FILE_PARSE_FAILED;
        Serial.println("Image load 失败");
        return false;
   }
    glowStickStatus.currentStatus = glowStickStatus.DISPLAY_IN_PROGRESS;
    Serial.println("Image loaded successfully");
    uint16_t interval = 30;
    animations.StartAnimation(3,interval,[&](const AnimationParam& param){
        if (param.state == AnimationState_Completed){
            strip.ClearTo(RgbColor(0,0,0));
            Serial.printf("animState=%d\n",animState);
            image.Blt(strip,0,0,animState,image.Width());
            if (animState <= image.Height())
            {
                // draw the complete row at animState to the complete strip
                image.Blt(strip, 0, 0, animState, image.Width());
                animState++; // increment and wrap
                strip.Show();
                animations.RestartAnimation(param.index);
            }else{
                glowStickStatus.currentStatus = glowStickStatus.DISPLAY_COMPLETED;
                animState=1;
                fashe("0xff0000");
            }
        }        
    });
    return true;
}
bool saveConfig(String JsonString) {
    File config = LittleFS.open("/index.json", "w");
    if (!config) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    config.print(JsonString);
    config.close();
    Serial.println("Configuration saved successfully");
    return true;
}

bool saveImg(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    // 只在 index 为 0 时创建文件
    File img;
    if (!index) {
         img = LittleFS.open("/verisign.bmp", "w");
        glowStickStatus.currentStatus = glowStickStatus.FILE_TRANSFER_IN_PROGRESS;
        if (!img) {
            Serial.println("Failed to open file for writing");
            request->send(500, "application/json", "{\"message\":\"Failed to open file for writing\"}");
            glowStickStatus.currentStatus = glowStickStatus.FILE_TRANSFER_FAILED;
            return false;
        }
        img.write(data, len); // 将第一部分数据写入文件
        img.close();
    } else {
         img = LittleFS.open("/verisign.bmp", "a"); // 以追加模式打开文件
        if (!img) {
            Serial.println("Failed to open file for appending");
            request->send(500, "application/json", "{\"message\":\"Failed to open file for appending\"}");
            glowStickStatus.currentStatus = glowStickStatus.FILE_TRANSFER_FAILED;
            return false;
        }
        img.write(data, len); // 将后续部分数据追加写入文件
    }

    // 如果是最后一个部分，发送成功响应
    if (final) {
         Serial.printf("数据大小为%d",img.size());
        img.close();
        Serial.println("File write complete");
        request->send(200, "application/json", "{\"message\":\"Image upload successful\"}");
    }

    return true;
}

#endif