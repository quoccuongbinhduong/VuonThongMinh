#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <SerialCommand.h>  // Thêm vào sketch thư viện Serial Command
#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

byte degree[8] = {
  0B01110,
  0B01010,
  0B01110,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000
};


const byte RX = 3;          // Chân 3 được dùng làm chân RX
const byte TX = 2;          // Chân 2 được dùng làm chân TX
SoftwareSerial mySerial = SoftwareSerial(RX, TX); 
SerialCommand sCmd(mySerial); // Khai báo biến sử dụng thư viện Serial Command
#define QUAT 7
#define BOM 8
#define DHTPIN 5
#define RAINPIN 4
#define ASPIN 6
#define DEN 12
#define IN1 9
#define IN2 10
const int DHTTYPE = DHT11;
  
DHT dht(DHTPIN,DHTTYPE);
int NhietDo;int DoAm; int Mua; int AnhSang;
int DoAmDat[6];
int sNhietDo;int sDoAm; int sDoAmDat;
String MBT="OFF";
String MBT1="OFF";
String n="";
boolean Duytri;
String StrQuat;String StrBom;String StrMaiChe; String StrDen;
String StrQuat_W;String StrBom_W;String StrMaiChe_W; String StrDen_W;
void setup() {
  //Khởi tạo Serial ở baudrate 57600 để debug ở serial monitor
  dht.begin();
  Serial.begin(9600);
  //Khởi tạo Serial ở baudrate 57600 cho cổng Serial thứ hai, dùng cho việc kết nối với ESP8266
   mySerial.begin(57600);
  
  /*****************************************/
  pinMode(DHTPIN,INPUT);
  pinMode(RAINPIN,INPUT);
  pinMode(ASPIN,INPUT);
  pinMode(QUAT,OUTPUT);
  pinMode(BOM,OUTPUT);
  pinMode(DEN,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  /*****************************************/
  sCmd.addCommand("AUTO",DuyTri);
  sCmd.addCommand("SET",Setcaidat); 
  sCmd.addCommand("TAY",BATTHIETBI_HW);
  
  lcd.init();  
  lcd.backlight();
  lcd.print("Nhiet do: ");
  lcd.setCursor(0,1);
  lcd.print("Do am: ");
  lcd.createChar(1, degree);
 
}
void loop() {
 if(Serial.available()>0)
 {
  n=Serial.readString();
  }
 doccambien();
 CHEDO(n);
 Send();
 BATTHIETBI_AW();
 sCmd.readSerial();
}

void doccambien(){
  NhietDo=dht.readTemperature();
  DoAm=dht.readHumidity();
  DoAmDat[0]=map(analogRead(A0),0,1024,100,0);
  Mua=digitalRead(RAINPIN);
  AnhSang=digitalRead(ASPIN);

  lcd.setCursor(10,0);
  lcd.print(round(NhietDo));
  lcd.print(" ");
  lcd.write(1);
  lcd.print("C");
  lcd.setCursor(10,1);
  lcd.print(round(DoAm));
  lcd.print(" %");    
 
  }
/*****************GỬI GIÁ TRỊ CẢM BIẾN*******************/
void Send(){
  Serial.print(" T");Serial.print(NhietDo);
  Serial.print(" D");Serial.print(DoAm);
  Serial.print(" Z");Serial.print(DoAmDat[0]);
  Serial.print(" R");Serial.print(Mua);
  Serial.print(" S");Serial.print(AnhSang);
  Serial.print(StrQuat);
  Serial.print(StrBom);
  Serial.print(StrMaiChe);
  Serial.print(StrDen);
  /******************************************/
 StaticJsonBuffer<200> jsonBuffer2;
 JsonObject& root2 = jsonBuffer2.createObject();
 root2["NhietDo"] = NhietDo;
 root2["DoAm"] = DoAm;
 root2["DoAmDat1"] = DoAmDat[0];
 root2["Mua"] = Mua;
 root2["AnhSang"] = AnhSang; 
 //in ra cổng software serial để ESP8266 nhận
 mySerial.print("DOCCAMBIEN");   //gửi tên lệnh
 mySerial.print('\r');           // gửi \r
 root2.printTo(mySerial); //gửi chuỗi JSON
 mySerial.print('\r');           // gửi \r
 /***************************************/
  StaticJsonBuffer<200> jsonBuffer3;
  JsonObject& root3 = jsonBuffer3.createObject();
  root3["QUAT"] = StrQuat_W;
  root3["BOM"] = StrBom_W;
  root3["DEN"] = StrDen_W;
  root3["MAICHE"] = StrMaiChe_W;
  mySerial.print("THIETBI");   //gửi tên lệnh
  mySerial.print('\r');           // gửi \r
  root3.printTo(mySerial); //gửi chuỗi JSON
  mySerial.print('\r');           // gửi \r
  delay(1000);
  }
/************PHẦN WIFI***********************/
  //////////DOC CAM BIEN////////////
void DuyTri(){
  Duytri=true;
  }
void Setcaidat(){
  //Serial.println("GIA TRI SET:");
  n="";
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  //Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json
  sNhietDo = root["set"][0];//json -> tham số root --> phần tử thứ 0. Đừng lo lắng nếu bạn không có phần tử này, không có bị lỗi đâu!
  sDoAm = root["set"][1];//json -> tham số root --> phần tử thứ 0. Đừng lo lắng nếu bạn không có phần tử này, không có bị lỗi đâu!
  sDoAmDat = root["set"][2];//json -> tham số root --> phần tử thứ 0. Đừng lo lắng nếu bạn không có phần tử này, không có bị lỗi đâu!
  }
void BATQUAT_AW(){
      if(NhietDo<sNhietDo || DoAm<sDoAm){
        digitalWrite(QUAT,LOW);
        StrQuat_W=" ON";
        }
        else
        {
        digitalWrite(QUAT,HIGH);
        StrQuat_W=" OFF";
          }
}
void BATBOM_AW(){
      
      if(DoAmDat[0]>=sDoAmDat){
        digitalWrite(BOM,HIGH);
        StrBom_W=" OFF";
        }
        else
        {
          digitalWrite(BOM,LOW);
          StrBom_W=" ON";
          }

}
void BATMAICHE_AW(){
   if(Mua== 0 && MBT=="OFF"){
      digitalWrite(IN1,HIGH);
      digitalWrite(IN2,LOW);
      delay(5000);
      digitalWrite(IN1,LOW);
      digitalWrite(IN2,LOW);
      MBT="ON";
      StrMaiChe_W=" ON";
    }
    if(Mua== 1 && AnhSang== 0 && MBT=="ON"){ 
       digitalWrite(IN1,LOW);
       digitalWrite(IN2,HIGH);
       delay(5000);
       digitalWrite(IN1,LOW);
       digitalWrite(IN2,LOW);
       MBT="OFF";
       StrMaiChe_W=" OFF";
    }
}
void BATDEN_AW(){
      if(AnhSang==1){
            digitalWrite(DEN,LOW);
            StrDen_W=" ON";
        }
        else
        {
            digitalWrite(DEN,HIGH);
            StrDen_W=" OFF";
          }
}
///////////////////////////////////////////
void BATTHIETBI_AW(){
  if(Duytri==true){
  BATQUAT_AW();
  BATBOM_AW();
  BATMAICHE_AW();
  BATDEN_AW();
  }
  }
//////ON-OFF THIET BI DIEU KHIEN///////////////////
void BATTHIETBI_HW(){
  n="";
  Duytri=false;
  char *json = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
  //Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer; //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root = jsonBuffer.parseObject(json);//đặt một biến root mang kiểu json
  String BQUAT=root["QUAT"];
  String BBOM=root["BOM"];
  String BMAICHE=root["MAICHE"];
  String BDEN=root["DEN"];
  if(BQUAT=="0"){
        digitalWrite(QUAT,HIGH);
        StrQuat_W=" OFF";
          }
          if(BQUAT=="1"){
          digitalWrite(QUAT,LOW);
            StrQuat_W=" ON";
            }
  if(BBOM=="0"){
          digitalWrite(BOM,HIGH);
          StrBom_W=" OFF";
          }
          if(BBOM=="1"){
            digitalWrite(BOM,LOW);
            StrBom_W=" ON";
            }
  if(BDEN=="1"){
          digitalWrite(DEN,LOW);
          StrDen_W=" ON";
          }
          if(BDEN=="0"){
            digitalWrite(DEN,HIGH);
            StrDen_W=" OFF";
            }
  if(BMAICHE=="1" && MBT=="OFF"){
          MBT="ON";
          StrMaiChe_W=" ON";
          digitalWrite(IN2,LOW);
          digitalWrite(IN1,HIGH);
          delay(5000);
          digitalWrite(IN1,LOW);
          digitalWrite(IN2,LOW);
          }
          if(BMAICHE=="0" && MBT=="ON"){
            MBT="OFF";
            StrMaiChe_W=" OFF";
            digitalWrite(IN2,HIGH);
            digitalWrite(IN1,LOW);
            delay(5000);
            digitalWrite(IN1,LOW);
            digitalWrite(IN2,LOW);
            }
  }
/******************PHẦN VISUAL BASIC************/
void CHEDO(String StrInput){
      boolean Auto;
      String StrCmd;
      for(int c=0;c<StrInput.length();c++){
         StrCmd=StrInput.substring(c,c+1);
        if(StrCmd == "A" || StrCmd == "a"){
          Duytri=false;
          BATTHIETBI_AW();
          }
          if(StrCmd == "H" || StrCmd == "h"){
          Duytri=false;
          BATTHIETBI_HW();
            }
        }
}
