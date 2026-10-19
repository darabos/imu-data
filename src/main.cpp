#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include <M5Unified.h>
#include <cstring>

constexpr uint32_t DISPLAY_REFRESH_MS = 33;

bool displayBufferReady = false;
M5Canvas displayBuffer(&M5.Display);
uint32_t nextDisplayRefreshMs = 0;

void drawStatus()
{
    if (!displayBufferReady)
    {
        return;
    }
    displayBuffer.fillSprite(TFT_BLACK);
    // TODO: Plot recorded accelerometer data.
    displayBuffer.pushSprite(0, 0);
}

void setup()
{
    auto config = M5.config();
    config.output_power = false;
    config.internal_spk = false;
    config.external_rtc = false;
    M5.begin(config);
    M5.Display.setRotation(1);
    M5.Display.fillScreen(TFT_BLACK);
    displayBuffer.setColorDepth(16);
    displayBufferReady = displayBuffer.createSprite(M5.Display.width(), M5.Display.height()) != nullptr;
    drawStatus();
}

void loop()
{
    M5.update();
    if (M5.BtnB.wasClicked())
    {
        // TODO: Switch to next axis.
    }
    const uint32_t now = millis();
    const auto update = M5.Imu.update();
    if (update)
    {
        const auto &data = M5.Imu.getImuData();
        // TODO: Record data.accel.<selected axis>.
    }
    if (static_cast<int32_t>(now - nextDisplayRefreshMs) >= 0)
    {
        drawStatus();
        nextDisplayRefreshMs = now + DISPLAY_REFRESH_MS;
    }
    delay(5);
}
