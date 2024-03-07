// 定义自己的着色器
#pragma once
template<typename T_COLOR_OBJECT> 
class MyNeoShaderNop {
public:
   MyNeoShaderNop() {
    initializeBrightnessTable();
}

    bool IsDirty() const {
        return true;
    }

    void Dirty() {}
    void ResetDirty() {}

    uint8_t get_Brightness(){
        return Brightness;
    }
    void set_Brightness(uint8_t brightness){
        brightness /= scale;
        if(Brightness!=brightness){
            Brightness = brightness;
        }
    }
    T_COLOR_OBJECT Apply(uint16_t, T_COLOR_OBJECT color) {
        for (uint32_t i = 0; i < sizeof(T_COLOR_OBJECT); ++i) {
            color[i] = brightnessTable[Brightness][color[i]];
        }
        return color;
    }

private:
    uint8_t Brightness; // 亮度调节系数(0-17)对应
    // 亮度查找表
    uint8_t brightnessTable[18][256];
    uint8_t scale = 15; //刻度
    // 初始化亮度查找表
    void initializeBrightnessTable() {
        for (uint16_t i = 0; i < 18; i++) {
            for (uint16_t j = 0; j < 256; ++j) {
                brightnessTable[i][j] = (j * i * scale) >> 8;
            }
        }
    }
};


