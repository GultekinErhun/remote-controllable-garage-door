#include <SoftwareSerial.h>
#include <EEPROM.h>
SoftwareSerial mySerial(8, 9);
#define led 11
#define M  10


char yoneticiler_no[M][14]={"+905511654615","+905558928638","+905511248713"};
char yetkililer_no[M][14];


char yonetici_ol[11]="yoneticiol";
char yetkili_ol[10]="yetkiliol";

int a;//mesaj geliyor mu kontrol etmeye yarıyor 
int b;//arama geliyor mu kontrol etmeye yarıyor 
int yonetici_sayisi;


String gelen_mesaj;
char num_temp[14];
String numtemp;
char num_yetki[14];
String numyetki;
String bosluk;
String mesaj_geliyor_mu; //Mesaj gelip gelmediğini sorgulamaya yarayan string
String arama_geliyor_mu; //Arama gelip gelmediğini sorgulamaya yarayan string

void ana_mesaj_fonksiyonu();
void ana_arama_foksiyonu();


int smsalgila();// hali hazırda sms var mı ona bakiyoruz
int aramaalgila();//hali hazırda arama var mı ona bakiyoruz
int  kimden(char gonderici[]);// mesajin/aramanın bir yetkili mi yöneticiden mi geldiğini kontrol ediyoruz 
void mesaj_oku();//komut icin mesajin icerigini ve gelen numaraya bakıyoruz 
void strtochar(String donusecek);// Stringleri char array yapmak icin

void komut_ne();//Tüm komutlari icinde bulunduran fonksiyon
void lambaac();
void lambakapa();
void yetkili_yazdir();
void ver(char yetki_ver_no[],char messagetemp[]);// Yeti vermek icin
void sms_yolla(String komut,String gonderilecek_numara);//sms ile bilgi sağlamak icin
void yonetici_yazdir();
int kac_yonetici();
void setup() {
  // Open serial communications and wait for port to open:
  
  Serial.begin(9600);
  pinMode(led,OUTPUT);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial begin ok");

  digitalWrite(led,LOW);   
  delay(1000);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  mySerial.println("AT");
  delay(100);
  bosluk= mySerial.readString();  // bufferi bosaltmak icin okuma yapıyoruz
  //Serial.println(bosluk);   
  mySerial.print("AT+CMGF=1\r");     // 1 SMS modu, 0 değil
  delay(100);
  bosluk= mySerial.readString();  
  //Serial.println(bosluk);     
}

//******************************************************************** 
void loop(){
  ana_mesaj_fonksiyonu();
  ana_arama_foksiyonu();
  delay(100);
  yetkili_yazdir();
  yonetici_yazdir();
}  
//********************************************************************
void ana_mesaj_fonksiyonu(){
  if (smsalgila()){
      Serial.println("Mesaj geldi");
      mesaj_oku();
      strtochar(numtemp);  //numtemp olan gecici string tipindekileri num_temp char arraye kaydediyoruz
      if(kac_yonetici()>0){
        if(kimden(num_temp)==1){   // eger yöneticiyse 1 esit deger dönüyor 
          komut_ne();
          }
      
      else if(kimden(num_temp)==2){ //eger yetkiliyse 2 degeri dönüyor
            if(1+gelen_mesaj.indexOf("lamba ac")){// indexOf bulamazsa -1 bulduğunda baslangıc indeksini dönderiyor
            lambaac();
              sms_yolla("Lamba acildi",numtemp);
            }
            else if(1+gelen_mesaj.indexOf("lamba kapa")){// indexOf bulamazsa -1 bulduğunda baslangıc indeksini dönderiyor
             lambakapa();
             sms_yolla("Lamba kapandi",numtemp);
            }
      }
      }
      
  }
      
      delay(100);
      
      
  }
  
//********************************************************************
void  ana_arama_foksiyonu(){    
          if(aramaalgila()+1){                           //arama geliyorsa
              mySerial.println("ATH");
              delay(100);
              digitalWrite(led,HIGH);
              delay(1000);
              digitalWrite(led,LOW);
          }
}
//********************************************************************
 int smsalgila(){
  
  Serial.println ("SMS alim kontrolu basladi");
  mySerial.print("AT+CMGR=1\r");     // simdeki ilk mesaji okuma okunmassa mesaj içine girilmiyor
  delay(100);
  mesaj_geliyor_mu= mySerial.readString();
  a=mesaj_geliyor_mu.indexOf("+905"); //icinde '+905' bulamazsa -1 degeri dönderecek
  
  mySerial.print("AT+CMGD=1\r");//mesajlari siliyor
  delay(100);
  bosluk= mySerial.readString();  
  //Serial.println(bosluk);    
  mySerial.print("AT+CMGD=2\r");
  delay(100);   
  bosluk= mySerial.readString();  
  //Serial.println(bosluk);  
  
  return a+1;
  
}
//********************************************************************
int aramaalgila(){
          Serial.println("Arama alim kontrolu basladi");
          mySerial.print("AT+CLIP=1\r");
          delay(100);
          arama_geliyor_mu= mySerial.readString();  
          int b=arama_geliyor_mu.indexOf("+905");
          return b;
}

//********************************************************************
int kimden(char gonderici[]){
  Serial.println("3");
  int k=0;
  char listtemp[14];
  for(int i=1;i<1023;i++){
    
    if(EEPROM.read(1)==0){
              return 0;
            }
            
   else if( EEPROM.read(i)!=0){             // kıyaslama yaparken sadece dolu kısmı kıyaslasın diye
               if(!(strcmp(listtemp,gonderici))){      //numara aynıysa
                  return 1;
                }
              }
            }
  
  
  for(int i=0;i<M;i++) {
    if(!(strcmp(yetkililer_no[i],gonderici))){
        return 2;           
    }
  }
  Serial.println("tanınmayan numara");
  return 0; // hicbirinde ekli degilse onu yok sayiyor
}
//********************************************************************
void mesaj_oku(){
  Serial.println("4");
   gelen_mesaj = mesaj_geliyor_mu;         // mesaj geliyorsa gelen mesajin tamami gelen_mesaj'a kayt edildi
   //mesaj_geliyor_mu="";
   numtemp = gelen_mesaj.substring(gelen_mesaj.indexOf("+90"),gelen_mesaj.indexOf("+90")+13);   //gelen telefon noyu alıyor, +14 yapma nedenim
   Serial.print("gsm'den gelen mesaj: ");                                                       // 2. kısım son indeksi dahil etmeden yapıyor
   Serial.println(gelen_mesaj);  //   
}
//********************************************************************
void strtochar(String donusecek){
  Serial.println("5");
    donusecek.toCharArray(num_temp,14);  //Stringdekileri gecici char array'e kaydediyoruz
}
//********************************************************************
void sms_yolla(String komut,String gonderilecek_numara){
  Serial.println("6");
  mySerial.print("AT+CMGD=1\r"); // 1. mesaji sil
  delay(100);
  mySerial.print("AT+CMGF=1\r");   // sms moduna gec
  delay(1000);
  mySerial.print("AT+CMGS=\"");   // mesaj atma modu
  mySerial.print(gonderilecek_numara);        // mesaj atma modundan sonra girilen telefon no
  mySerial.print("\"\r");  
  delay(1000);
  mySerial.print(komut);
  delay(1000);
  mySerial.write(0x1A);     //mesajin bittigini belirten ctrl+z karakteri
  delay(1000);
  Serial.println("sms gonderildi");   
}
//********************************************************************
int veryonetim(char yonetim_ver_no[]){ // yeti verme fonksiyonumuz
    int k=0;
    for(int i=1;i<1023;i++){
      char listtemp[14];
      listtemp[k]=EEPROM.read(i);
      if(k==13){

            if(EEPROM.read(1)==0){
              for(int i=0,l=1;i<14;i++,l++){
                EEPROM.write(l,yonetim_ver_no[i]);  // eğer baslangıc bossa assagudaki fonksiyon islevsiz kalmasın diye
              }
              return 1;
            }
              
            if( EEPROM.read(i)!=0){             // kıyaslama yaparken sadece dolu kısmı kıyaslasın diye
              if(strcmp(listtemp,yonetim_ver_no)){      //onceden bu numara kayitli miydi bakmak icin 
                for(int i=1;i<1023;i++){               // nerde boş yer var ona bakıyoruz
                   if(EEPROM.read(i)==0){             
                      for(int l=0;l<14;l++){
                            EEPROM.write(i,yonetim_ver_no[l]);
                            i++;
                            return 1;
                      }
                   }
                }
              }
            }
            
          
               
      }
    }
      k+=1;
      if(k==14){
        k=0;
       }
      
}

 //********************************************************************
void veryetki(char yetki_ver_no[]){
 
    for(int i=0;i<M;i++){
      if(strcmp(yetkililer_no[i],yetki_ver_no) ){  // eger verilecek  no onceden ekliyse diye       
         for(int i=0;i<M;i++){
            if(!(yetkililer_no[i][0])){  //                (yetkililer_no[i]) liste boşsa degeri 0 dönderiyor
            strcpy(yetkililer_no[i], yetki_ver_no);    
            Serial.println("Yetki verildi"); 
            }
            break;    
        }
      }
    }
  
   
}
//********************************************************************
int kac_yonetici(){
  yonetici_sayisi=EEPROM.read(0);// eeprom 0. indekste kaç tane yönetici oldugu bilgisi sakli
  return yonetici_sayisi;
}

void yonetici_sifirla(){
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i,0);
    Serial.println(EEPROM.read(i));
  }
  digitalWrite(led,HIGH);
  delay(300);
  digitalWrite(led,LOW);
  delay(300);
  digitalWrite(led,HIGH);
  delay(300);
  digitalWrite(led,LOW);
   delay(300);
  digitalWrite(led,HIGH);
  delay(300);
  digitalWrite(led,LOW);
  delay(300);
}
//********************************************************************
void lambaac(){
  Serial.println("Led acik");
  digitalWrite(led,HIGH);
  }



  
void lambakapa(){
    Serial.println("Led kapali");
    digitalWrite(led,LOW);
  }
  

//********************************************************************
void komut_ne(){
  Serial.println("9");
  if(1+gelen_mesaj.indexOf("lamba ac")){// indexOf bulamazsa -1 bulduğunda baslangıc indeksini dönderiyor
    lambaac();
    //sms_yolla("Lamba acildi",numtemp);
  }
  else if(1+gelen_mesaj.indexOf("lamba kapa")){// indexOf bulamazsa -1 bulduğunda baslangıc indeksini dönderiyor
    lambakapa();
    //sms_yolla("Lamba kapandi",numtemp);
  }
  else if(1+gelen_mesaj.indexOf("yetkiver")){
    numyetki=gelen_mesaj.substring(gelen_mesaj.indexOf("#"),gelen_mesaj.indexOf("#")+14);// yetki verilecek numara
    numyetki=numtemp.substring(numtemp.indexOf("+"),numtemp.indexOf("+")+13); // # çıkarma işlemi yapıyoruz
    Serial.print("Yetki verilen numara ");
    Serial.print(numyetki);
    strtochar(numyetki);
    veryetki(num_yetki);
    //sms_yolla("Yetki verildi",numtemp);
    //sms_yolla("Size yetki verildi",numyetki)
  }

  else if(1+gelen_mesaj.indexOf("yonetimver")){
    numyetki=gelen_mesaj.substring(gelen_mesaj.indexOf("#"),gelen_mesaj.indexOf("#")+14);// yetki verilecek numara
    numyetki=numtemp.substring(numtemp.indexOf("+"),numtemp.indexOf("+")+13); // # çıkarma işlemi yapıyoruz
    Serial.print("Yetki verilen numara ");
    Serial.print(numyetki);
    strtochar(numyetki);
    veryonetim(num_yetki);
    //sms_yolla("Yonetim verildi",numtemp);
    //sms_yolla("Size yonetim verildi",numyetki);
  }
  
}
//********************************************************************
void yetkili_yazdir(){
  Serial.println("Yetkililer:");
  for(int i=0;i<M;i++){
         if(yetkililer_no[i][0]){  //                (yetkililer_no[i]) liste boşsa degeri 0 dönderiyor
            Serial.println(yetkililer_no[i]);   
        }       
  }
  Serial.println("Yoneticiler:");
  for(int i=0;i<M;i++){
         if(yoneticiler_no[i][0]){  //                (yetkililer_no[i]) liste boşsa degeri 0 dönderiyor
            Serial.println(yoneticiler_no[i]);   
        }
  }
}  

void yonetici_yazdir(){
  for(int i=1;i<1023;i++){
    if( EEPROM.read(i)) break;
    Serial.print(EEPROM.read(i));
    if(i%14==1){
      Serial.println("");
    }
   
    
}
}
