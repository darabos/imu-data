#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include <M5Unified.h>
#include <cstring>

constexpr uint32_t DISPLAY_REFRESH_MS = 33;
constexpr size_t SENSOR_HISTORY_SIZE = 240;
constexpr float ACCEL_PLOT_RANGE_G = 2.0f;
constexpr float GYRO_PLOT_RANGE_DPS = 500.0f;
constexpr const char *AXIS_LABELS[] = {"X", "Y", "Z"};
constexpr uint16_t AXIS_COLORS[] = {TFT_RED, TFT_GREEN, TFT_BLUE};

bool displayBufferReady = false;
M5Canvas displayBuffer(&M5.Display);
uint32_t nextDisplayRefreshMs = 0;
size_t selectedAxis = 0;
bool showGyroscope = false;
float sensorHistory[SENSOR_HISTORY_SIZE] = {};
size_t nextSensorSample = 0;
size_t sensorSampleCount = 0;

void drawStatus()
{
    if (!displayBufferReady)
    {
        return;
    }
    displayBuffer.fillSprite(TFT_BLACK);
    char label[32];
    const float latestSample = sensorSampleCount == 0
                                   ? 0.0f
                                   : sensorHistory[(nextSensorSample + SENSOR_HISTORY_SIZE - 1) % SENSOR_HISTORY_SIZE];
    const float plotRange = showGyroscope ? GYRO_PLOT_RANGE_DPS : ACCEL_PLOT_RANGE_G;
    std::snprintf(label, sizeof(label), "%s %s  %.2f %s", showGyroscope ? "GYRO" : "ACCEL",
                  AXIS_LABELS[selectedAxis], latestSample, showGyroscope ? "dps" : "g");
    displayBuffer.setTextColor(TFT_WHITE, TFT_BLACK);
    displayBuffer.drawString(label, 4, 4);

    const int graphLeft = 4;
    const int graphRight = displayBuffer.width() - 5;
    const int graphTop = 25;
    const int graphBottom = displayBuffer.height() - 9;
    const int graphWidth = graphRight - graphLeft;
    const int graphHeight = graphBottom - graphTop;
    for (int gridLine = 0; gridLine <= 4; ++gridLine)
    {
        const int y = graphTop + gridLine * graphHeight / 4;
        displayBuffer.drawFastHLine(graphLeft, y, graphWidth, gridLine == 2 ? TFT_DARKGREY : 0x3186);
    }

    if (sensorSampleCount > 0)
    {
        const size_t oldestSample = (nextSensorSample + SENSOR_HISTORY_SIZE - sensorSampleCount) % SENSOR_HISTORY_SIZE;
        int previousX = graphLeft;
        int previousY = graphBottom;
        for (size_t index = 0; index < sensorSampleCount; ++index)
        {
            const int x = graphLeft + static_cast<int>(index * graphWidth / std::max<size_t>(sensorSampleCount - 1, 1));
            const float sample = std::max(-plotRange,
                                           std::min(sensorHistory[(oldestSample + index) % SENSOR_HISTORY_SIZE],
                                                    plotRange));
            const int y = graphTop + static_cast<int>((plotRange - sample) * graphHeight / (2.0f * plotRange));
            if (index > 0)
            {
                displayBuffer.drawLine(previousX, previousY, x, y, AXIS_COLORS[selectedAxis]);
            }
            previousX = x;
            previousY = y;
        }
    }
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
        selectedAxis = (selectedAxis + 1) % 3;
    }
    if (M5.BtnA.wasClicked())
    {
        showGyroscope = !showGyroscope;
        nextSensorSample = 0;
        sensorSampleCount = 0;
    }
    const uint32_t now = millis();
    const auto update = M5.Imu.update();
    if (update)
    {
        const auto &data = M5.Imu.getImuData();
        const auto &sensor = showGyroscope ? data.gyro : data.accel;
        const float sample = selectedAxis == 0 ? sensor.x : selectedAxis == 1 ? sensor.y : sensor.z;
        sensorHistory[nextSensorSample] = sample;
        nextSensorSample = (nextSensorSample + 1) % SENSOR_HISTORY_SIZE;
        sensorSampleCount = std::min(sensorSampleCount + 1, SENSOR_HISTORY_SIZE);
    }
    if (static_cast<int32_t>(now - nextDisplayRefreshMs) >= 0)
    {
        drawStatus();
        nextDisplayRefreshMs = now + DISPLAY_REFRESH_MS;
    }
    delay(5);
}
