#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FIREBASE_HOST "acessotags-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "npofv6AFlfyGRx4Qew1pPSwToHeoqQjjfOcH21lE"

const char* ssid = "iPhone Mauricio";
const char* password = "mauricio123";
 
#define SS_PIN 15
#define RST_PIN 0 
#define rPin 16 //D0
#define gPin 5 //D1
#define bPin 4  //D2
MFRC522 mfrc522(SS_PIN, RST_PIN);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
 
void setup() 
{
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Aproxime sua tag/cartão...");
  Serial.println();

  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }

  
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  timeClient.begin();
  timeClient.setTimeOffset(-10800);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

}
void loop() 
{

  //Firebase.setString("data",String(currentDate));

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  
  String content= "";
  String firebaseNome  = "";
  byte letter;
  String firebasePermissao = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  Serial.println();
  Serial.println("UID tag  : " + content);
  setColor(255, 255 , 255);
  content.trim();
  content.replace(" ", "_");
  
  Firebase.setString("ultimaConsulta",String(content));
  if(Firebase.getString("tags/" + content + "/acesso") != "cadastrado"){

      timeClient.update();
      unsigned long epochTime = timeClient.getEpochTime();
      struct tm *ptm = gmtime ((time_t *)&epochTime); 
      int monthDay = ptm->tm_mday;
      int currentMonth = ptm->tm_mon+1;
      int currentYear = ptm->tm_year+1900;
      String currentDate = String(monthDay) + "/" + String(currentMonth) + "/" + String(currentYear) + " " + timeClient.getHours() + ":" + timeClient.getMinutes();

    
      Firebase.setString("tags/" + content + "/" "acesso", "cadastrado");
      Firebase.setString("tags/" + content + "/" "dataCadastro", currentDate);
      Firebase.setString("tags/" + content + "/" "nome", "");
      Firebase.setString("tags/" + content + "/" "permissao", "");

    Serial.println("Mensagem : CADASTRO REALIZADO, AGUARDANDO PERMISSÃO..");
    setColor(0, 0, 255);
  }else{

    firebasePermissao = Firebase.getString("tags/" + content + "/permissao");
    firebaseNome = Firebase.getString("tags/" + content + "/nome");
    
    if(firebasePermissao == "permitido"){
        Serial.println("Mensagem : ENTRADA PERMITIDA");
        setColor(0, 255, 0);
          if(firebaseNome != ""){
            Serial.println("Bem-vindo(a) " + firebaseNome);
          }
    }
    
    if(firebasePermissao == ""){
        Serial.print("Mensagem : AGUARDANDO PERMISSÃO..");
        setColor(0, 0, 255);
        Serial.println();
    }
    
    if(firebasePermissao == "nao_permitido"){
        Serial.println();
        Serial.print("Mensagem : ENTRADA NÃO PERMITIDA");
        setColor(255, 0, 0);
        Serial.println();
    }
  }
 
  delay(3000);
  setColor(0,0,0);

} 


void setColor(int redValue, int greenValue, int blueValue){
  analogWrite(rPin, redValue);
  analogWrite(gPin, greenValue);
  analogWrite(bPin, blueValue);
}
