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
#include "keymap.h"
#include "queues.h"
#include <bluefruit.h>

static BLEDis bledis;
static BLEBas blebas;
static BLEHidAdafruit blehid;
static BLEClientUart clientUart;

static inline void blinkScanLED() { blinkLED1(); }
static inline void blinkAdvLED() { blinkLED2(); }
static inline void turnOffScanLED() { turnOffLED1(); }
static inline void turnOffAdvLED() { turnOffLED2(); }

void setup() {

    // シリアルをオンにすると消費電流が増えるのでデバッグ時以外はオフにする
    //Serial.begin(115200);

    Bluefruit.begin(1, 1);
    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    Bluefruit.setTxPower(TX_POWER);
    Bluefruit.setName(DEVICE_NAME);
    Bluefruit.autoConnLed(false);
    Bluefruit.setConnectCallback(prph_connect_callback);
    Bluefruit.setDisconnectCallback(prph_disconnect_callback);

    // Configure and Start Device Information Service
    bledis.setManufacturer(MANUFACTURER_NAME);
    bledis.setModel(MODEL_NUMBER);
    bledis.begin();

    // Start BLE Battery Service
    blebas.begin();

    /* Start BLE HID
   * Note: Apple requires BLE device must have min connection interval >= 20m
   * ( The smaller the connection interval the faster we could send data).
   * However for HID and MIDI device, Apple could accept min connection interval
   * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min
   * and max
   * connection interval to 11.25  ms and 15 ms respectively for best
   * performance.
   */
    blehid.begin();

    clientUart.begin();
    clientUart.setRxCallback(bleuart_rx_callback);

    // Initialize Keyboard Resource
    initQueues();
    startBatteryService(blebas);
    // ループタスクのプライオリティを取得して他のタスクも同じプライオリティで作成する
    UBaseType_t priority = uxTaskPriorityGet(NULL);
    initLED(priority);
    initKeymap(blehid);
    startKeyScan(priority);

    // Callbacks for Central
    Bluefruit.Central.setConnectCallback(cent_connect_callback);
    Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);
    Bluefruit.Central.setConnIntervalMS(10, 20);

    /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Filter only accept bleuart service
   * - Don't use active scan
   * - Start(timeout) with timeout = 0 will scan forever (until connected)
   */
    Bluefruit.Scanner.setRxCallback(scan_callback);
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
    Bluefruit.Scanner.filterUuid(clientUart.uuid);
    Bluefruit.Scanner.useActiveScan(false);
    Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds
    blinkScanLED();             //scan status led*/

    /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
   */
    /* Bluefruit.setConnInterval(9, 12); */

    // Set up and start advertising
    startAdv();
}

static void startAdv(void) {
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

    // Include BLE HID service
    Bluefruit.Advertising.addService(blehid);

    // There is enough room for the dev name in the advertising packet
    Bluefruit.Advertising.addName();

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
    Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
    blinkAdvLED();                              // advertising status led
}

void loop() {
    static UInt8Set scanIDs, bleIDs;

    EventData data = {};
    xQueueReceive(eventQueue, &data, portMAX_DELAY);

    if (data.eventType == SCAN_KEY_EVENT || data.eventType == BLE_KEY_EVENT) {
        if (data.eventType == SCAN_KEY_EVENT) {
            scanIDs = data.ids;
        } else if (data.eventType == BLE_KEY_EVENT) {
            bleIDs = data.ids;
        }
        applyToKeymap(scanIDs | bleIDs);

    } else if (data.eventType == TIMER_EVENT) {
        data.timer->onTimer();
    }

    //dbgMemInfo();
}

/*------------------------------------------------------------------*/
/* Peripheral
 *------------------------------------------------------------------*/

static void prph_connect_callback(uint16_t conn_handle) {
    turnOffAdvLED();
}

static void prph_disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    blinkAdvLED();
}

/*------------------------------------------------------------------*/
/* Central
 *------------------------------------------------------------------*/

static void scan_callback(ble_gap_evt_adv_report_t *report) {
    // Check if advertising contain BleUart service
    if (Bluefruit.Scanner.checkReportForService(report, clientUart)) {
        // Connect to device with bleuart service in advertising
        Bluefruit.Central.connect(report);
    }
}

static void cent_connect_callback(uint16_t conn_handle) {
    if (clientUart.discover(conn_handle)) {
        // Enable TXD's notify
        clientUart.enableTXD();
        turnOffScanLED();
    } else {
        // disconect since we couldn't find bleuart service
        Bluefruit.Central.disconnect(conn_handle);
    }
}

static void cent_disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    blinkScanLED();
    // 切断されたらキーが押しっぱなしにならないように空のデータを送る
    EventData data = {
        .eventType = BLE_KEY_EVENT,
    };
    xQueueSend(eventQueue, &data, portMAX_DELAY);
}

static void bleuart_rx_callback(BLEClientUart &uart_svc) {
    EventData data = {
        .eventType = BLE_KEY_EVENT,
    };
    while (uart_svc.available()) {
        data.ids.add(uart_svc.read());
    }
    // 終端0を取る
    data.ids.remove(0);
    // loopに送る
    xQueueSend(eventQueue, &data, portMAX_DELAY);
}
