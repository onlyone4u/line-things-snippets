#include <BLEPeripheral.h>

// Device Name: Maximum 20 bytes
#define DEVICE_NAME "LINE Things nRF51"

// User service UUID: Change this to your generated service UUID
#define USER_SERVICE_UUID "91E4E176-D0B9-464D-9FE4-52EE3E9F1552"
// User service characteristics
#define WRITE_CHARACTERISTIC_UUID "E9062E71-9E62-4BC6-B0D3-35CDCD9B027B"
#define NOTIFY_CHARACTERISTIC_UUID "62FBD229-6EDD-4D1A-B554-5C4E1BB29169"

// PSDI Service UUID: Fixed value for Developer Trial
#define PSDI_SERVICE_UUID "E625601E-9E55-4597-A598-76018A0D293D"
#define PSDI_CHARACTERISTIC_UUID "26E2B12B-85F0-4F3F-9FDD-91D114270E6E"

#define BUTTON 6
#define LED1 4

BLEPeripheral blePeripheral;
BLEBondStore bleBondStore;

// Setup User Service
BLEService userService(USER_SERVICE_UUID);
BLEUnsignedCharCharacteristic writeCharacteristic(WRITE_CHARACTERISTIC_UUID, BLEWrite);
BLEUnsignedCharCharacteristic notifyCharacteristic(NOTIFY_CHARACTERISTIC_UUID, BLENotify);
// Setup PSDI Service
BLEService psdiService(PSDI_SERVICE_UUID);
BLECharacteristic psdiCharacteristic(PSDI_CHARACTERISTIC_UUID, BLERead, sizeof(uint32_t) * 2);

volatile int btnAction = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  pinMode(LED1, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  // Clear bond store if push button for 3 secs on start up
  // You can bond only one central device on each peripheral device by library restriction
  if (!digitalRead(BUTTON)) {
    delay(3000);
    if (!digitalRead(BUTTON)) {
      bleBondStore.clearData();
      Serial.println("Cleared bond store");
    }
  }

  attachInterrupt(BUTTON, buttonAction, CHANGE);

  blePeripheral.setDeviceName(DEVICE_NAME);
  blePeripheral.setLocalName(DEVICE_NAME);
  blePeripheral.setBondStore(bleBondStore);
  blePeripheral.setAdvertisedServiceUuid(userService.uuid());

  blePeripheral.addAttribute(userService);
  blePeripheral.addAttribute(writeCharacteristic);
  blePeripheral.addAttribute(notifyCharacteristic);
  blePeripheral.addAttribute(psdiService);
  blePeripheral.addAttribute(psdiCharacteristic);

  // Set callback
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  writeCharacteristic.setEventHandler(BLEWritten, writeLEDCallback);

  // Set PSDI (Product Specific Device ID) value
  uint32_t deviceAddr[] = { NRF_FICR->DEVICEADDR[0], NRF_FICR->DEVICEADDR[1] };
  psdiCharacteristic.setValue((unsigned char *)deviceAddr, sizeof(deviceAddr));

  Serial.println("Starting peripheral");
  blePeripheral.begin();
  Serial.println("Ready to Connect");
}

void loop() {
  BLECentral central = blePeripheral.central();

  if (central && central.connected() && btnAction > 0) {
    uint8_t btnRead = !digitalRead(BUTTON);
    btnAction = 0;
    notifyCharacteristic.setValue(btnRead);
    delay(20);
  }

  blePeripheral.poll();
}

void buttonAction() {
  btnAction++;
}

void writeLEDCallback(BLECentral& central, BLECharacteristic& characteristic) {
  if (writeCharacteristic.value()) {
    Serial.println("ON");
    digitalWrite(LED1, HIGH);
  } else {
    Serial.println("OFF");
    digitalWrite(LED1, LOW);
  }
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}
