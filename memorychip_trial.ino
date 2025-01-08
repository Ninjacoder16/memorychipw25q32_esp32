#include <SPI.h>

// Pin Definitions
#define CS_PIN 5 // Chip Select (CS) pin connected to the memory chip

// Command Definitions for Winbond W25QXX
#define CMD_READ_ID        0x90
#define CMD_READ_DATA      0x03
#define CMD_WRITE_ENABLE   0x06
#define CMD_PAGE_PROGRAM   0x02
#define CMD_SECTOR_ERASE   0x20
#define CMD_READ_STATUS    0x05

// SPI Settings
SPISettings spiSettings(1000000, MSBFIRST, SPI_MODE0);

void setup() {
  Serial.begin(115200);

  // Initialize SPI
  SPI.begin();
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH); // Set CS high initially

  // Verify chip ID
  uint16_t chipID = readChipID();
  Serial.print("Winbond Chip ID: 0x");
  Serial.println(chipID, HEX);

  if (chipID == 0) {
    Serial.println("Failed to detect the memory chip!");
    while (1);
  }
}

void loop() {
  uint32_t address = 0x200000; // Address to write and read
  byte dataToWrite = 0x61;    // Data to write
  byte dataRead;

  // Erase sector
  Serial.println("Erasing sector...");
  eraseSector(address);

  // Write data
  Serial.println("Writing data...");
  writeData(address, dataToWrite);

  // Read data
  Serial.println("Reading data...");
  dataRead = readData(address);

  // Display the result
  Serial.print("Data written: 0x");
  Serial.println(dataToWrite, HEX);
  Serial.print("Data read: 0x");
  Serial.println(dataRead, HEX);

  delay(5000); // Repeat every 5 seconds
}

// Function to Read Chip ID
uint16_t readChipID() {
  uint16_t chipID = 0;

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_ID);
  SPI.transfer(0x00); // Dummy bytes
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
  enableWrite(); // Enable write operation

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_SECTOR_ERASE);
  SPI.transfer((address >> 16) & 0xFF); // Address MSB
  SPI.transfer((address >> 8) & 0xFF);  // Address Middle Byte
  SPI.transfer(address & 0xFF);         // Address LSB
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  // Wait for erase operation to complete
  while (readStatus() & 0x01) {
    delay(1);
  }
}

// Function to Write Data
void writeData(uint32_t address, byte data) {
  enableWrite(); // Enable write operation

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_PAGE_PROGRAM);
  SPI.transfer((address >> 16) & 0xFF); // Address MSB
  SPI.transfer((address >> 8) & 0xFF);  // Address Middle Byte
  SPI.transfer(address & 0xFF);         // Address LSB
  SPI.transfer(data);                   // Data byte
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  // Wait for write operation to complete
  while (readStatus() & 0x01) {
    delay(1);
  }
}

// Function to Read Data
byte readData(uint32_t address) {
  byte result;

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_DATA);
  SPI.transfer((address >> 16) & 0xFF); // Address MSB
  SPI.transfer((address >> 8) & 0xFF);  // Address Middle Byte
  SPI.transfer(address & 0xFF);         // Address LSB
  result = SPI.transfer(0x00);          // Dummy byte to read data
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  return result;
}
