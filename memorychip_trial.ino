#include <SPI.h>
#include <ArduinoJson.h>
#include <WiFi.h>
  
// Pin Definitions
#define CS_PIN 5  // Chip Select (CS) pin connected to the memory chip

// Command Definitions for Winbond W25QXX
#define CMD_READ_ID 0x90
#define CMD_READ_DATA 0x03
#define CMD_WRITE_ENABLE 0x06
#define CMD_PAGE_PROGRAM 0x02
#define CMD_SECTOR_ERASE 0x20
#define CMD_READ_STATUS 0x05
#define read_unique_id 0x4B
#define manufacturing_id 0x92
// SPI Settings
SPISettings spiSettings(1000000, MSBFIRST, SPI_MODE0);
int temperature, humidity;
void setup() {
  Serial.begin(115200);

  // Initialize SPI
  SPI.begin();
  pinMode(CS_PIN, OUTPUT);

  digitalWrite(CS_PIN, HIGH);  // Set CS high initially

  // Verify chip ID
  uint16_t chipID = readChipID();
  Serial.print("Winbond Chip ID: 0x");
  Serial.println(chipID, HEX);

  uint16_t chipID1 = readuniqid();
  Serial.print("Winbond Unique ID: 0x");
  Serial.println(chipID1, HEX);

  uint16_t chipID2 = manu_id();
  Serial.print("Winbond manu_f ID: 0x");
  Serial.println(chipID2, HEX);

  if (chipID == 0) {
    Serial.println("Failed to detect the memory chip!");
    while (1)
      ;
  }
  humidity =10;
  temperature=10;
}

void loop() {
  String macAddress = WiFi.macAddress();
  StaticJsonDocument<200> doc;
  temperature = temperature+1;
  humidity = humidity+1;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["mac_id"] = macAddress.c_str();
 // doc["timestamp"] = millis();  // Use system time or add RTC if available

  // Serialize JSON to a string
  char jsonBuffer[100];
  serializeJson(doc, jsonBuffer);
  Serial.println(jsonBuffer);
   
  //memory chip
  uint32_t address = 0x00000;
  //char dataToWrite[] = "Aartikanwar Solanki";
  char buffer[100];

  //Serial.println("Erasing sector...");
  //eraseSector(address);

  Serial.println("Writing data...");
  writeData(address, jsonBuffer, strlen(jsonBuffer));

  Serial.println("Reading data...");
  readData(address, buffer, strlen(jsonBuffer));  //pasing empty buffer

  Serial.print("Data written: ");
  Serial.println(jsonBuffer);

  Serial.print("Data read: ");
  Serial.println(buffer);  //printing similar buffer

  delay(5000);
}

// Function to Read Chip ID
uint16_t readChipID() {
  uint16_t chipID = 0;

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_ID);
  SPI.transfer(0x00);  // Dummy bytes
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  chipID = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  return chipID;
}

// Function to Read Status Register
byte readStatus() {
  byte status;

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_STATUS);
  status = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  return status;
}

// Function to Enable Write
void enableWrite() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_WRITE_ENABLE);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

// Function to Erase a Sector
void eraseSector(uint32_t address) {
  enableWrite();  // Enable write operation

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_SECTOR_ERASE);
  SPI.transfer((address >> 16) & 0xFF);  // Address MSB
  SPI.transfer((address >> 8) & 0xFF);   // Address Middle Byte
  SPI.transfer(address & 0xFF);          // Address LSB
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  // Wait for erase operation to complete
  while (readStatus() & 0x01) {
    delay(1);
  }
}

// Function to Write Data
void writeData(uint32_t address, const char *data, int length) {
  enableWrite();

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_PAGE_PROGRAM);
  SPI.transfer((address >> 16) & 0xFF);  // Address MSB
  SPI.transfer((address >> 8) & 0xFF);   // Address Middle Byte
  SPI.transfer(address & 0xFF);          // Address LSB
  for (int i = 0; i < length; i++) {
    SPI.transfer(data[i]);  // Write data
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  // Wait for write operation to complete
  while (readStatus() & 0x01) {
    delay(1);
  }
}


// Function to Read Data
void readData(uint32_t address, char *result, int length) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_DATA);
  SPI.transfer((address >> 16) & 0xFF);  // Address MSB
  SPI.transfer((address >> 8) & 0xFF);   // Address Middle Byte
  SPI.transfer(address & 0xFF);          // Address LSB
  for (int i = 0; i < length; i++) {
    result[i] = SPI.transfer(0x00);  // Read data
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

uint16_t readuniqid() {
  uint16_t chipID = 0;
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(read_unique_id);
  SPI.transfer(0x00);  // Dummy bytes
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  chipID = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  return chipID;
}

uint16_t manu_id() {
  uint16_t chipID = 0;
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(manufacturing_id);
  SPI.transfer(0x00);  // Dummy bytes
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  chipID = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  return chipID;
}

void json_print() {
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  //doc["mac_id"] = macAddress.c_str();
  doc["timestamp"] = millis();  // Use system time or add RTC if available

  // Serialize JSON to a string
  char jsonBuffer[100];
  serializeJson(doc, jsonBuffer);
  Serial.println(jsonBuffer);
}