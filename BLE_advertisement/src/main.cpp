#include <Arduino.h>
#include <SoftwareSerial.h>
#include <bluefruit.h>
#include <FreeRTOS.h>

/* ================================================================================================================= */
/* Static declarations */
/* ================================================================================================================= */

#define CONF_EDDYSTONE
// #define CONF_IBEACON

#define BLE_DEFAULT_DEVICE_NAME     "ItsaBeacon"
#define BLE_DEFAULT_DEVICE_NAME_LEN 31u

#define BLE_REG_TO_MS(x) ((x * 625u) / 1000u)
#define BLE_MS_TO_REG(x) ((x * 1000u) / 625u)

#define RSSI_AT_1M_NRF52840_ADAFRUIT_ITSY_BITSY (-54)
#define RSSI_AT_0M_NRF52840_ADAFRUIT_ITSY_BITSY (-40)

#define EDDYSTONE_URL "https://github.com/milli9d"

typedef struct _sys_info {
    const char name[BLE_DEFAULT_DEVICE_NAME_LEN];
    ble_gap_addr_t mac;
} sys_info;

/* ================================================================================================================= */
/* Static defintions */
/* ================================================================================================================= */

static sys_info sys = { 0u };

/* ================================================================================================================= */
/* Private API */
/* ================================================================================================================= */

/**
 * @brief Pretty print a ble_gap_addr_t
 * @param mac
 */
static void inline print_ble_gap_addr_t(ble_gap_addr_t& mac)
{
    printf("BLE MAC Address:"
           "%02X:%02X:%02X:%02X:%02X:%02X\n",
           mac.addr[0u], mac.addr[1u], mac.addr[2u], mac.addr[3u], mac.addr[4u], mac.addr[5u]);
}

/**
 * @brief Begin advertisement
 * @param
 */
static void advertisement_begin(void)
{
    /* set beacon packet */
#if defined(CONF_IBEACON)
    ble_uuid128_t uuid = { 0xba, 0x1a, 0x30, 0x5b, 0xb5, 0x1b, 0x4d, 0x10,
                           0x90, 0x9d, 0xda, 0xe4, 0x69, 0x8a, 0xdc, 0x64 };
    BLEBeacon beacon((const uint8_t*) &uuid, 1, 2, RSSI_AT_1M_NRF52840_ADAFRUIT_ITSY_BITSY);
    Bluefruit.Advertising.setBeacon(beacon);
#elif defined(CONF_EDDYSTONE)
    EddyStoneUrl url(RSSI_AT_0M_NRF52840_ADAFRUIT_ITSY_BITSY, EDDYSTONE_URL);
    Bluefruit.Advertising.setBeacon(url);
#endif

    /**
     * Scan response packet is sent when a device scans for BLE devices; if in DIRECTED mode, only sent when scan comes
     * from authorized device
     */
    uint8_t data[] = "00M9D";
    Bluefruit.ScanResponse.addName();
    Bluefruit.ScanResponse.addManufacturerData(&data[0u], sizeof(data));

    /* Start Advertising
     * - Enable auto advertising if disconnected
     * - Timeout for fast mode is 30 seconds
     * - Start(timeout) with timeout = 0 will advertise forever (until
     * connected)
     *
     * Apple Beacon specs
     * - Type: Non-connectable, scannable, undirected
     * - Fixed interval: 100 ms -> fast = slow = 100 ms
     */
    Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED);
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(BLE_MS_TO_REG(1280u), BLE_MS_TO_REG(1280u));
    Bluefruit.Advertising.setFastTimeout(30u);
    Bluefruit.Advertising.start(0u);
}

/**
 * @brief Setup BLE
 * @param
 */
static void setup_BLE(void)
{
    /* start BLE */
    while (!Bluefruit.begin()) {
        printf("BLE init error, retrying!\n");
        delay(1000u);
    }
    printf("BLE inited successfully!");

    /* configure BLE led */
    Bluefruit.autoConnLed(true);

    /* get MAC address of BLE device */
    sys.mac = Bluefruit.getAddr();
    print_ble_gap_addr_t(sys.mac);

    /* set BLE device config */
    strncpy((char*) sys.name, BLE_DEFAULT_DEVICE_NAME, BLE_DEFAULT_DEVICE_NAME_LEN);
    Bluefruit.setName(sys.name);
    Bluefruit.setAppearance(BLE_APPEARANCE_GENERIC_TAG);
    Bluefruit.setTxPower(RADIO_TXPOWER_TXPOWER_0dBm);

    /* start advertisement */
    advertisement_begin();
}

/* ================================================================================================================= */
/* PUBLIC */
/* ================================================================================================================= */

void setup()
{
    Serial.begin(115200);
    setup_BLE();
}

int32_t count = 0u;

void loop()
{
    /* suspend after 10 seconds */
    if (millis() >= 10u * 1000u) {
        printf("Going AWOL!\n");
        suspendLoop();
    }

    /* print status */
    printf("Hello itsyBitsy nrf52840 Beacon!\n");
    print_ble_gap_addr_t(sys.mac);
    delay(1000u);
}