// 自定义数据处理，将hex数据解析
#ifndef RGBCOLORPARSER_H
#define RGBCOLORPARSER_H
#include <LittleFS.h>
#include <NeoPixelBusLg.h>
#define ARRAY_SIZE 144*128// 存储RGB颜色的字符
class RgbColorParser
{
public:
    RgbColorParser(const char *filename) : filename(filename), numColors(0) {}
    // 初始化函数，读取文件并解析RGB数据
    bool begin()
    {
        File file = LittleFS.open(filename, "r");
        if (!file)
        {
            Serial.println("Failed to open file");
            return false;
        }
        Serial.println("开始读");
        size_t count = 0;
        while (file.available() && count < ARRAY_SIZE) {
            long v = file.parseInt();
            colors[count] = (uint8_t)v;
            count++;
            if(count%576 == 0){
                ESP.wdtFeed();
            }
        }
        numColors = count;
        // 关闭文件
        file.close();
        return true;
    }

    // 获取指定索引处的RGB颜色
    RgbColor getColorAtIndex(int index)
    {
        size_t s = index * 3;
        if ((s + 2) < numColors)
        {
            return RgbColor(colors[s], colors[s + 1], colors[s + 2]);
        }
        else
        {
            // 如果索引无效，则返回默认颜色
            return RgbColor();
        }
    }
    // 给宽度计算高度
    size_t computedHeig(size_t width)
    {
        if (width == 0)
        {
            return 0;
        }
        return static_cast<int>(floor(numColors / width));
    }

private:
    const char *filename;   // 文件名 地址
    uint8_t colors[ARRAY_SIZE]; 
    size_t numColors;       // RGB颜色的总数
};
#endif