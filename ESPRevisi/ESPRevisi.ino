#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>

// define konektivitas
#define WIFI_SSID "barong"
#define WIFI_PASSWORD "bolaliar"

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset = 25200; // GMT+7 (Waktu Indonesia Barat)
const int   daylightOffset = 0; // Tanpa Daylight Saving Time

// Nama bulan
const char* namaBulan[12] = {
  "Januari", "Februari", "Maret", "April", "Mei", "Juni",
  "Juli", "Agustus", "September", "Oktober", "November", "Desember"
};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset, daylightOffset);

// define firbase account
#define FIREBASE_HOST "https://ta-firebase-alat-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "BH5YwL6RpP5nKYlvizIqlABbgkR0AdOlzNtOPTi7"
FirebaseData firebaseData;

// membuat variabel untuk software serial (Rx, Tx)
SoftwareSerial ESPSERIAL(13, 15);

// Inisialiasi variabel
int pengunjung, kondisi;
String kuotapengunjung, kontrol, admin, yearStr, monthString, dayString, hoursStr, minutesStr, secondsStr, barcode;

// penggunaan millis sebagai pengganti delay
unsigned long previousMillis = 0;
const long interval = 3000;

// variabel array untuk data parsing
String arrData[3];

void setup() {
  // Kode setup
  Serial.begin(9600);
  ESPSERIAL.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  // Menghubungkan ke Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("Connecting to WiFi...");
    digitalWrite(LED_BUILTIN, 1);
    delay(100);
    digitalWrite(LED_BUILTIN, 0);
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println(); 
  
  // Memulai waktu dari NTP server
  timeClient.begin();
  timeClient.update();
  
  Serial.println("Connected to WiFi");
  Serial.println("Current time:");
  Serial.println(timeClient.getFormattedTime());

  // SET UP Konektivitas Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  
  }

void getfirebase()
{
  if (Firebase.getString(firebaseData, "/Konfigurasi/Kuotapengunjung")){
      kuotapengunjung = firebaseData.stringData();
      //Serial.print("kuotapengunjung = ");
//      Serial.println(kuotapengunjung);
  }

  if (Firebase.getString(firebaseData, "/Konfigurasi/Admin")){
      admin = firebaseData.stringData();
      admin = admin.toInt();
//      Serial.print("admin = ");
//      Serial.println(admin);
  }
  if (Firebase.getString(firebaseData, "/Konfigurasi/Kontrol")){
      kontrol = firebaseData.stringData();
//      Serial.print("kontrol = ");
//      Serial.println(kontrol);
  }

}

void pushfirebase()
{
  if (Firebase.setFloat(firebaseData, "/Monitoring/Pengunjung", pengunjung)){
//      Serial.println("Jumlah pengunjung terkirim");
  } else{
//      Serial.println("Jumlah pengunjung tidak terkirim");
//      Serial.println("Karena: " + firebaseData.errorReason());
  } 
    
  if (Firebase.setFloat(firebaseData, "/Monitoring/Kondisi", kondisi)){
//      Serial.println("status terkirim");
//      Serial.println();
  } else{
//      Serial.println("status tidak terkirim");
//      Serial.println("Karena: " + firebaseData.errorReason());
  } 

    if (Firebase.setString(firebaseData,"/History/" + yearStr + "/" + monthString + "/" + dayString + "/" + hoursStr + ":" + minutesStr + ":" + secondsStr, barcode)){
//      Serial.println("History terkirim");
//      Serial.println();
  } else{
//      Serial.println("History tidak terkirim");
//      Serial.println("Karena: " + firebaseData.errorReason());
  } 
}

void getDateTime() {
  time_t now = timeClient.getEpochTime();
  struct tm* ptm = localtime(&now);

  int year = ptm->tm_year + 1900; // Tambahkan 1900 untuk mendapatkan tahun sebenarnya
  int month = ptm->tm_mon;        // Gunakan indeks bulan langsung
  int day = ptm->tm_mday;
  int hours = ptm->tm_hour;
  int minutes = ptm->tm_min;
  int seconds = ptm->tm_sec;

  yearStr = String(year);
  monthString = String(namaBulan[month]);
  dayString = String(day);
  hoursStr = String(hours);
  minutesStr = String(minutes);
  secondsStr = String(seconds);

  // Lainnya: Anda dapat menggunakan variabel-variabel ini sesuai kebutuhan Anda
}

void loop() {
  // Kode loop
  timeClient.update(); // Update waktu dari server NTP

  getDateTime();
  // Kode loop

  // Request data dari Firebase
  getfirebase(); 
  
  // Request data dari arduino Mega 2560  
  unsigned long currentMillis = millis(); // baca waktu millis saat ini
  if (currentMillis - previousMillis >= interval) {
    // update previousMillis    
    previousMillis = currentMillis;
    String data = "";
    while (Serial.available() > 0) {
      data += char(Serial.read());
      //Serial.println("Ada");      
    }
    // Menghapus spasi pada data yang diterima
    data.trim();
    //uji data
    if (data != "") {
      //format data "10#100#78"
      //parsing data (pecah data) = array
      int index = 0;
      for (int i = 0; i < data.length(); i++) {
        char delimiter = '#';
        if (data[i] != delimiter) {
          arrData[index] += data[i];
        } else {
          index++;
        }
      }
      
      // pastikan bahwa data yang dikirmkan lengkap
      if (index >= 0) {
        // tampilkan nilai sensor ke serial monitor
//        Serial.println(arrData[0]);
//        Serial.println(arrData[1]);
//        Serial.println(arrData[2]);
        // kirim data ke Firebase
        pengunjung = arrData[0].toInt();
        kondisi = arrData[1].toInt();
        barcode = arrData[2];
        //barcode = "88888";
        pushfirebase();
      }
      arrData[0] = "";
      arrData[1] = "";
      arrData[2] = "";
      
    }
    
    // Request data ke arduino Mega 2560
    String request = String(kuotapengunjung)+"#"+String(admin)+"#"+String(kontrol)+"#"+"ya";
    Serial.println(request);
    ESPSERIAL.println(request);
//    barcode = "88888";
//    pushfirebase();
  }
}
