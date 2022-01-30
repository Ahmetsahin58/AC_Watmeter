
#include <LCD5110_Basic.h>     // ekran ibary
#include "ACS712.h"           //akım sensörü
#include <SoftwareSerial.h>  //bt libary
#include <Wire.h>  //Kütüphaneleri yükleyin
#include "RTClib.h" //Kütüphaneleri yükleyin

ACS712 sensor(ACS712_20A, A0);
RTC_DS3231 rtc;  //Rtc mödül 

LCD5110 myGLCD(8,9,10,12,11); //ekran bağlantıları

SoftwareSerial bt(7, 6);  //Bt mödül rx tex pinleri
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

unsigned long eskiZaman=0;   //timer değişkneleri
unsigned long yeniZaman;
unsigned long eskiZaman1=0;
unsigned long yeniZaman1;

#define b1 A3  //buton pinleri
#define b2 4
#define b3 5
#define b4 3
#define rl 13

const int Geri = 3;  //Menü buton atamaları
const int Ileri = 5;
const int Onay = 4;
const int Cik = A3;

int menu = 1; // Menü değişkneleri

bool Zm=false;  // Alarm zamanlama etkinlik değeri
long int dakika=2;   //Alarm Degeri


bool Bt=true ;  //Bluetooth aktiflik durmu
bool Role= 0;   //Role aktiflik durumu


float U = 210;  // Voltaj şebeke çarpanı
float I,P;      // Akım ve watt ondalık degerleri 

 
void setup()   // setup fonksiyonu
{

  digitalWrite(rl, LOW);  // giriş cıkış tanımları
  pinMode(Ileri, INPUT);
  pinMode(Geri, INPUT);
  pinMode(Onay, INPUT);
  pinMode(Cik, INPUT);
  pinMode(rl, OUTPUT);
  
  Serial.begin(9600);  //seri port başltama
  bt.begin(9600);      // bt seri haberleşme başlatma

  myGLCD.InitLCD();    //ekran başlatma
 sensor.calibrate();   //akım sensörü kalibre etme
  
 
 
 if (! rtc.begin()) {   //RTC başlatma başarılı değilse
      myGLCD.setFont(SmallFont);
      myGLCD.print("RTC Mod. Yok! ", LEFT, 0);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Veya hatali! ", LEFT, 16);
      
     while (1){   myGLCD.invert(true); //sonsuz döngüye girip ekranı tesrler.
     delay(1000);
     myGLCD.invert(false);
     delay(1000);}
  }

  if (rtc.lostPower()) { //RTc pili çıktı ise tekrar zamanı güncellesin.
    
    Serial.println("RTC güç kaybetti, zamanı ayarlayalım!"); // Pcden güncellerken seri porta yazdırsın .
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  //Saati Pcden Çekme;
  }



}





void loop()  //ana döngü 
{

MENU(); // Menu fonksiyonu 
bt1();  //bt haberleşme fonksiyonu
Rtc();  // zaman fonksiyonu
delay(10);

float I = sensor.getCurrentAC();  //sensörden akım oku
delay(10);
float P=I*U;  //güç girdisini yaklaşık elde et 

  yeniZaman = millis();  // başlangıçtan itibaren geçen süre
  if(yeniZaman-eskiZaman > 1000){ //ekrana yenileme hızı.
  if(I==0 || P<40.0){P=0;} // eger akım sıfır ise degreri sıfır yap
  myGLCD.clrScr();   //ekranı temizler
  myGLCD.setFont(SmallFont); //ekran fontu belirle
  myGLCD.printNumF(I, 1, LEFT, 8); //ekrana float bastır.
  myGLCD.setFont(MediumNumbers);
  myGLCD.printNumF(P,2,LEFT, 32);
  eskiZaman = yeniZaman;}
  myGLCD.setFont(SmallFont);
  myGLCD.print("Watt:", LEFT, 24);



if(!Role){  //röle konrtol et ve saçıp kapat
   digitalWrite(rl, LOW); // 13 pin röle aktif 
             
 
}
else{
    digitalWrite(rl, HIGH);  // 13. pin röle pasif
}

yeniZaman1 = millis();    //Alarm içi yazılımsal kesme yap ve alarm gelince döngüye gir.
if(yeniZaman1-eskiZaman1 > dakika*60000){
     myGLCD.clrScr();
  
     while (digitalRead(Cik) == 0 && Zm) {
   
       
      digitalWrite(rl, HIGH);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Alarm Devrede ", LEFT, 0);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Role Pasif ", LEFT, 16);
  
      myGLCD.invert(true); 
      delay(500);
      myGLCD.invert(false);
      delay(500);
      myGLCD.clrScr();
     

        }
     /* Eski zaman değeri yeni zaman değeri ile güncelleniyor */
     eskiZaman1 = yeniZaman1;
     Role=false;
     delay(50);  }

     
  }
  
















void Rtc(){  //RTC fonkisyonu
DateTime now = rtc.now();  // RTC NESNESİ OLUŞTUR.
 
 char buf1[] = "hh:mm:ss";   //BUFFER OLUŞTUR.
 char buf2[] = "MM-DD-YYYY"; //


myGLCD.setFont(SmallFont);
myGLCD.print(now.toString(buf2), RIGHT, 0); //buffera şimidiki zamanı yaz ve ekra bas 
myGLCD.setFont(SmallFont);
myGLCD.print(now.toString(buf1), RIGHT, 8);



  
}







void bt1 (){ //bluetooth fonksiyonu.

  if (bt.available() && Bt ) // Bluettoth hattı dolu ve menuden bt açık koşulu
  { 
    char dt= bt.read();      // bt veriyi oku 
    if(dt=='a') //a ise aç 
    {
      myGLCD.clrScr();
      Role=false;
      myGLCD.setFont(SmallFont);
      myGLCD.print("Role Aktif", LEFT, 16);
      delay(1000);
      
    }
      if(dt=='k') //k ise kapat
    {
      myGLCD.clrScr();
       Role=true;
       myGLCD.setFont(SmallFont);
       myGLCD.print("Role Pasif", LEFT, 16);
       delay(1000);
      
    }

       if(dt=='i')  // i ise info geç 
    {
      float I = sensor.getCurrentAC(); //akımı oku sensörden
        delay(10);
       float P=I*U;                   //Yaklaşık güç degeri hesabı
      
       String str = "";     // boş stirng dosyası oluşturuldu.
       str.concat(P);
       bt.print(str+" Watt"); // bt hattını güç verisini yazdır.



    }
      
     


     
  }
}



 
  

  
 



void MENU(){  // menu fonksiyonları 

  menu=constrain(menu,0,5);

 if(digitalRead(b1) ==1){
    }
    

 if(digitalRead(b2) ==1)
  { 
     while(digitalRead(Cik)==0){
     
        anaMenu();  }
        
        
      myGLCD.clrScr();
      delay (10);
  
    
       
  }
  if(digitalRead(b3) ==1){  
 }

   if(digitalRead(b4) ==1){
   }



  
}

void anaMenu() {
      menu=1;
      myGLCD.clrScr();
      myGLCD.setFont(SmallFont);
      myGLCD.print(">Alarm Durumu", LEFT, 0);
      myGLCD.print(" Dakika gir", LEFT, 8); 
      myGLCD.print(" Role Durumu", LEFT, 16);
      myGLCD.print(" BT Mod", LEFT, 24);
      delay(1000);
      
  while (digitalRead(b1) == 0) {
    
    while (digitalRead(Ileri)) {
      menu++;
      updateMenu();
      delay(500);

      
    }
    while (digitalRead(Geri)) {
      menu--;
      updateMenu();
     delay(500);
    }
    while (digitalRead(Onay)) {
      executeAction();
      updateMenu();
  


    }
  }

}

void updateMenu() {
  switch (menu) {
    case 0:
      menu = 5;

      break;
    case 1:
      myGLCD.clrScr();
      myGLCD.setFont(SmallFont);
      myGLCD.print(">Alarm Durumu", LEFT, 0);
      myGLCD.print(" Dakika gir", LEFT, 8); 
      myGLCD.print(" Role Durumu", LEFT, 16);
      myGLCD.print(" BT Mod", LEFT, 24);  
      break;
    case 2:
      myGLCD.clrScr();
      myGLCD.setFont(SmallFont);
      myGLCD.print(" Alarm Durumu", LEFT, 0);
      myGLCD.print(">Dakika gir", LEFT, 8); 
      myGLCD.print(" Role Durumu", LEFT, 16);
      myGLCD.print(" BT Mod", LEFT, 24);

      break;
    case 3:
       myGLCD.clrScr();
      myGLCD.setFont(SmallFont);
      myGLCD.print(" Alarm Durumu", LEFT, 0);
      myGLCD.print(" Dakika gir", LEFT, 8); 
      myGLCD.print(">Role Durumu", LEFT, 16);
      myGLCD.print(" BT Mod", LEFT, 24);

      break;
    case 4:
      myGLCD.clrScr();
      myGLCD.setFont(SmallFont);
      myGLCD.print(" Alarm Durumu", LEFT, 0);
      myGLCD.print(" Dakika gir", LEFT, 8); 
      myGLCD.print(" Role Durumu", LEFT, 16);
      myGLCD.print(">BT Mod", LEFT, 24);

      break;
    case 5:

      menu = 0;
      break;
  }
}

void executeAction() {
  switch (menu) {
    case 1:
      action1();
      break;
    case 2:
      action2();
      break;
    case 3:
      action3();
      break;
    case 4:
      action4();
      menu = 1;
      break;

  }
}






















void action1() {   // alarm aktifligi asiyonu


 myGLCD.clrScr();
      
  while (digitalRead(Cik) == 0) {
    
     myGLCD.setFont(SmallFont);
     myGLCD.print("Alarm Durumu:", LEFT, 0);
 

    while (digitalRead(Ileri)) {
     Zm=true;
     myGLCD.clrScr();
     delay(300);
     
    }
    while (digitalRead(Geri)) {
     Zm=false;
     myGLCD.clrScr();
     delay(300);


    }

      if(Zm){
       myGLCD.setFont(SmallFont);
      myGLCD.print("Alarm Aktif", LEFT, 24);
   
    }
        if(!Zm){
           
      myGLCD.setFont(SmallFont);
      myGLCD.print("Alarm Pasif", LEFT, 24);
    }
    



}

}







void action2() {  // alarm dakişkası ayarlama 

 myGLCD.clrScr();
      
  while (digitalRead(Cik) == 0) {
    
      dakika = constrain(dakika, 1,1500); //bir günlük zaman ayarı.
  
      myGLCD.setFont(SmallFont);
      myGLCD.print("Alarm:(DK)", LEFT, 0);
   


    while (digitalRead(Ileri)) {
     dakika= constrain(dakika, 0,60);  dakika--;
     myGLCD.clrScr();
     delay(300);
     
    }
    while (digitalRead(Geri)) {
      myGLCD.clrScr();
      dakika++;
     delay(300);


    }
    
    myGLCD.setFont(SmallFont);
    myGLCD.printNumI(dakika, LEFT, 16);

  }
}



void  action3() {   //role durmu ayarlama
      myGLCD.clrScr();
    
  while (digitalRead(Cik) == 0) {

         
     myGLCD.setFont(SmallFont);
     myGLCD.print("Role Durumu:", LEFT, 0);
     
    while (digitalRead(Ileri)) {
      Role=true;
     
    }
    while (digitalRead(Geri)) {

      Role=false;
    
     
    }
    if(!Role){
       myGLCD.setFont(SmallFont);
      myGLCD.print("Role Aktif", LEFT, 24);
   
    }
    if(Role){
           
      myGLCD.setFont(SmallFont);
      myGLCD.print("Role Pasif", LEFT, 24);
    }
    



      
    

  }

}


void action4() {       // Bt aktif veya pasif aksiyonu 
  myGLCD.clrScr();
  while (digitalRead(Cik) == 0) {
       myGLCD.setFont(SmallFont);
     myGLCD.print("BT Durumu:", LEFT, 0);
     
    while (digitalRead(Ileri)) {
      Bt=true;
     
    }
    while (digitalRead(Geri)) {

      Bt=false;
    
     
    }
    if(!Bt){
      myGLCD.setFont(SmallFont);
      myGLCD.print("BT Pasif", LEFT, 24);
   
    }
    if(Bt){
           
      myGLCD.setFont(SmallFont);
      myGLCD.print("BT Aktif", LEFT, 24);
    }
    
  }

}
