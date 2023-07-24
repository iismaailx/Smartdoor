// inisialisasi library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// define GPIO untuk Ultrasonic
#define echoPin 22
#define trigPin 24
// #define echoPin2 10
// #define trigPin2 11
// define GPIO untuk Motor Driver BTS7960
#define R_IS 2
#define L_IS 3
#define R_EN 4
#define L_EN 5
#define RPWM 6
#define LPWM 7
// set alamat LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// define indikator
#define alarm 51
#define LED 53
// inisialisasi variabel
long duration;
int distance = 10,  counter;
int pengunjung;
String barcode = "";
int kuotapengunjung;
// pembuatan custom karakter lcd
byte gate[] = {
  B00100,
  B01110,
  B11111,
  B01010,
  B01010,
  B01010,
  B11011,
  B00000
};
// pembuatan array
String arrReceive[4];
static int result = 0;
int total;
byte repair = 0;
byte jobdone = 0;
byte lastkontrol = 0;
byte admin, kondisi, kontrol;
int temp_pengunjung = 0; // Variabel sementara untuk menyimpan nilai pengunjung
bool full = false;
void setup() {
// Mengaktifkan komunikasi serial  
  Serial.begin(9600);
  Serial3.begin(9600);
// komunikasi serial GM Scanner
  Serial1.begin(9600);
  Serial3.setTimeout(100);
// Mengaktifkan LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
// Mengaktifkan Sensor Ultrasonic
  pinMode(trigPin, OUTPUT);  
  //pinMode(trigPin2, OUTPUT);
  pinMode(echoPin, INPUT);
  //pinMode(echoPin2, INPUT);
  pinMode(alarm, OUTPUT);
// Mengaktifkan Indicator
  pinMode(LED, OUTPUT);
// Mengaktifkan Motor Servo
  pinMode(R_IS, OUTPUT);
  pinMode(L_IS, OUTPUT);  
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  digitalWrite(R_IS, LOW);
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_IS, LOW);
  digitalWrite(L_EN, HIGH);  
}
void getdor() // Fungsi Untuk Membuka dan Menutup Pintu
{
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 40);
  delay(500);
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
  delay(2000);
  analogWrite(RPWM, 40);
  analogWrite(LPWM, 0);
  delay(500);
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
  delay(2000);
}


void loop(){
  int sts;
  ////////////////////PEMBACAAN GM SCANNER 66/////////////////////////
  while (Serial3.available() > 0) {
    barcode = Serial3.readString();
    if (barcode != "" && repair == 0 && full == false) {
      getdor();
      temp_pengunjung = temp_pengunjung + 1; // Menambah nilai pengunjung pada variabel sementara
      
    }
  }

  ///////////////////////PEMBACAAN ULTASONIC/////////////////////////
  // 1. Pembacaan ultasonic point pertama
  digitalWrite(trigPin, LOW); // Set trigger signal low for 2us
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034) / 2.0; // Konversi pulsa ke jarak
  delay(300);
  if (distance != 0 && distance <= 15 && temp_pengunjung > 0 && repair == 0) {
    Serial.print("aktif");
    temp_pengunjung = temp_pengunjung - 1; // Mengurangi nilai pengunjung pada variabel sementara
    getdor();
  }

  if (temp_pengunjung < 0) {
    temp_pengunjung = 0;
  }

  // Menyalin nilai pengunjung dari variabel sementara ke variabel utama
  pengunjung = temp_pengunjung;

  Serial.print("ini barcode : ");
  Serial.println(barcode);
  Serial.print("");
  Serial.print("ini jarak : ");
  Serial.println(distance);
  Serial.print("ini pengunjung : ");
  Serial.println(pengunjung);
  Serial.println("");
  

  //////////////////////// MENGIRIM DATA KE ARDUINO MEGA ////////////////////
  String minta = "";
  while (Serial1.available() > 0) {
    minta += char(Serial1.read());
    //Serial.println("ada");
  }
  minta.trim();
  if (minta != "") {
    int index = 0;
    for (int i = 0; i < minta.length(); i++) {
      char delimiter = '#';
      if (minta[i] != delimiter) {
        arrReceive[index] += minta[i];
      } else {
        index++;
      }
    }

    if (index >= 0) 
    {
      // Serial.print(arrReceive[0]);
      // Serial.print(arrReceive[1]);
      // Serial.print(arrReceive[2]);
      // Serial.print(arrReceive[3]);
      Serial.println("");
      kuotapengunjung = arrReceive[0].toInt();
      if(kuotapengunjung == 0){
        kuotapengunjung = 5;
      }
      Serial.print("kuota Pengunjung : ");
      Serial.println(kuotapengunjung);
      admin = byte(arrReceive[1].toInt());
      kontrol = byte(arrReceive[2].toInt());
      Serial.print("ini admin : ");
      Serial.println(admin);
      Serial.print("ini kontrol : ");
      Serial.println(kontrol);
      
      if (arrReceive[3] == "ya") 
      {
        sendata();
      }
      
    }
    // menghapus pengunjung data
    arrReceive[0] = "";
    arrReceive[1] = "";
    arrReceive[2] = "";
    arrReceive[3] = "";
  }
  
  if(admin == 0 && kontrol == 0 ){
    repair = 0;
  }
  
  if(kontrol != lastkontrol){
    jobdone = 0;

  }

  if (admin == 1 && kontrol == 1 && jobdone == 0) {
    repair = 1;
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 45);
    delay(300);
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
    delay(1000);
    jobdone = 1;
    lastkontrol = 1;
    Serial.println("Buka");
  } 

  if (admin == 1 && kontrol == 2 && jobdone == 0) {
    repair = 1;
    analogWrite(RPWM, 45);
    analogWrite(LPWM, 0);
    delay(300);
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
    delay(1000);
    jobdone = 1;
    lastkontrol = 2;
    Serial.println("Tutup");
  }

  lcd.createChar(0, gate);
  lcd.setCursor(1, 0);
  lcd.write((byte)0);
  lcd.createChar(0, gate);
  lcd.setCursor(14, 0);
  lcd.write((byte)0);
  lcd.setCursor(3, 0);
  lcd.print("SOUTH GATE");
  lcd.setCursor(12, 0);
  if (pengunjung >= kuotapengunjung) {
    lcd.setCursor(2, 1);
    lcd.print("Full Capacity");
    lcd.setCursor(13, 1);
    lcd.print(""); // Membersihkan karakter pengunjung sebelumnya
    full = true;
    kondisi = 1;
  } else {
    lcd.setCursor(2, 1);
    lcd.print("Visitors : ");
    lcd.setCursor(12, 1);
    lcd.print("    "); // Membersihkan karakter pengunjung sebelumnya
    lcd.setCursor(12, 1);
    lcd.print(pengunjung);
    full = false;
  }
  minta = "";
  delay(1000);
  }
  
void sendata(){ 
  String datakirim = String(pengunjung) + "#" + String(kondisi)+"#"+barcode;
  Serial1.println(datakirim);
  Serial.print("Data terkirim! : ");
  Serial.println(datakirim);
  digitalWrite(LED, HIGH);
  delay(20);
  digitalWrite(LED, LOW);
  delay(20);
  barcode = "";
}
