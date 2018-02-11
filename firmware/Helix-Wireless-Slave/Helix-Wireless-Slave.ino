/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include "batteryService.h"
#include "blinkLED.h"
#include "config.h"
#include "keyScan.h"
#include "queues.h"
#include <bluefruit.h>

static BLEUart bleuart;
static BLEBas blebas;

static inline void blinkAdvLED() { blinkLED2(); }
static inline void turnOffAdvLED() { turnOffLED2(); }

void setup() {

    // シリアルをオンにすると消費電流が増えるのでデバッグ時以外はオフにする
    //Serial.begin(115200);

    Bluefruit.begin();
    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    Bluefruit.setTxPower(TX_POWER);
    Bluefruit.setName(DEVICE_NAME);
    Bluefruit.autoConnLed(false);
    Bluefruit.setConnectCallback(connect_callback);
    Bluefruit.setDisconnectCallback(disconnect_callback);
    Bluefruit.setConnIntervalMS(10, 20);

    // Configure and Start BLE Uart Service
    bleuart.begin();

    // Start BLE Battery Service
    blebas.begin();

    // Initialize Keyboard Resource
    initQueues();
    startBatteryService(blebas);
    // ループタスクのプライオリティを取得して他のタスクも同じプライオリティで作成する
    UBaseType_t priority = uxTaskPriorityGet(NULL);
    initLED(priority);
    startKeyScan(priority);

    // Set up and start advertising
    startAdv();
}

static void startAdv(void) {
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();

    // Include bleuart 128-bit uuid
    Bluefruit.Advertising.addService(bleuart);

    // Secondary Scan Response packet (optional)
    // Since there is no room for 'Name' in Advertising packet
    Bluefruit.ScanResponse.addName();

    /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   *
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html
   */
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(10);   // number of seconds in fast mode
    Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
    blinkAdvLED();
}

static void connect_callback(uint16_t conn_handle) {
    turnOffAdvLED();
}

static void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    blinkAdvLED();
}

void loop() {
    EventData data = {};
    xQueueReceive(eventQueue, &data, portMAX_DELAY);

    if (data.eventType == SCAN_KEY_EVENT) {
        // 配列にする、終端0を追加するため1バイト大きいサイズで領域確保
        uint size = data.ids.count() + 1;
        uint8_t buf[size];
        data.ids.toArray(buf);
        // 終端0を追加
        buf[size - 1] = 0;
        // 送る
        bleuart.write(buf, size);
    } else if (data.eventType == TIMER_EVENT) {
        data.timer->onTimer();
    }

    //dbgMemInfo();
}
