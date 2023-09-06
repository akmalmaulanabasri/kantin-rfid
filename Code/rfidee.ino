#include <LiquidCrystal_I2C.h>

#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Informasi jaringan WiFi
const char* ssid = "XYZ";
const char* password = "12345678";

// Informasi Firebase
#define FIREBASE_HOST "https://rfidkantin-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyDBBB875QO88NHqWHomJC6MHFm7l4bi_ak"

FirebaseData firebaseData;
  
//constexpr uint8_t RST_PIN = 3;
//constexpr uint8_t SS_PIN = 4;
#define SS_PIN D8
#define RST_PIN D0

MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

// Inisialisasi memulai mengirim NUID
byte nuidPICC[4];

LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
 
  // Pemanggilan pertama memerlukan parameter jumlah kolom dan baris
  // Ini harus sama dengan yang dimasukan pada konstruktor.
  lcd.begin(16,2);
  lcd.init();
 
  // Nyalakan backlight
  lcd.backlight();
 
  // Pindahkan kursor ke kolom 0 dan baris 0
  // (baris 1)
  lcd.setCursor(0, 0);
 
  // Cetak hellow ke layar
  lcd.print("E-CANTEEN");
 
  // Pindahkan kursor ke baris berikutnya dan cetak lagi
  lcd.setCursor(0, 1);      
  lcd.print("PPLGWIKRAMAA");
  Serial.begin(115200); // Memulai serial 
// Menghubungkan ke jaringan WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  Serial.println("Menghubungkan ke jaringan WiFi..");
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi terhubung");
  Serial.println("Alamat IP: ");
  Serial.println(WiFi.localIP());

  // Menghubungkan ke Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Firebase terhubung");
  SPI.begin(); // Memulai komunikasi SPI
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop() {

  // Cek untuk kartu baru
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verifikasi apakah NUID sudah dibaca
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
    rfid.uid.uidByte[1] != nuidPICC[1] ||
    rfid.uid.uidByte[2] != nuidPICC[2] ||
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
 
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    Serial.println();
    String content= "";
    byte letter;
    for (byte i = 0; i < rfid.uid.size; i++) 
    {
       Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
       Serial.print(rfid.uid.uidByte[i], HEX);
       content.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
       content.concat(String(rfid.uid.uidByte[i], HEX));
    }
      lcd.setCursor(0, 1);
      lcd.clear();
      lcd.print(content);
      Firebase.setInt(firebaseData, "/card/", content);
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();



}


/**
 * Sub routine mengirim nilai HEX.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Sub routine mengirim nilai DEX.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
