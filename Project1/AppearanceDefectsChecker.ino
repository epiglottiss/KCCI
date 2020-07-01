#include <LiquidCrystal_I2C.h> //LCD header file
#include <Wire.h> //LCD header file
#include <SPI.h>   //RFID header file
#include <MFRC522.h>  //RFID header file

#define RST_PIN   9     // reset핀 설정 for RFID
#define SS_PIN    10    // 데이터를 주고받는 역할의 핀( SS = Slave Selector ) for RFID

#define TOR 10000   //스텝모터 한바퀴 회전에 걸리는 시간
#define CPR 4      //스텝모터 한바퀴 회전에 검사할 횟수 360 / 90도 = 4회

MFRC522 mfrc(SS_PIN, RST_PIN);           // 이 코드에서 MFR522를 이용하기 위해 mfrc객체를 생성해 줍니다. for RFID

// 0x3F I2C 주소를 가지고 있는 16x2 LCD객체를 생성합니다.(I2C 주소는 LCD에 맞게 수정해야 합니다.)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // for LCD
//초음파 센서의 핀번호를 설정한다.
int trigPin = A0;                          //초음파모듈 전체 trig를 아날로그0번핀
int echoPin1 = A1;                          //초음파모듈1번 echo를 아날로그1번핀              
int echoPin2 = A2;                          //초음파모듈2번 echo를 아날로그2번핀               
int echoPin3 = A3;                          //초음파모듈1번 echo를 아날로그3번핀   

int step_IN1 = 2;                         //스텝모터 IN1을 7번핀에 연결합니다.
int step_IN2 = 3;                       //스텝모터 IN1을 11번핀에 연결합니다.
int step_IN3 = 4;                        //스텝모터 IN1을 12번핀에 연결합니다.
int step_IN4 = 5;                        //스텝모터 IN1을 13번핀에 연결합니다.
int stepCnt = 2048;               //2048스텝 = 360도
int stepCntPerCheck=stepCnt/CPR;  //1회 측정 간 스텝수
float stepDelay = (TOR/stepCnt);  

int LED_R = 6;                            // LED를 5번핀에 연결합니다.
int LED_B = 7;                            // LED를 7번핀에 연결합니다.
int buzzer = 8;                            // 부저를 8번핀에 연결합니다.

int numTones = 8;
int tones[] = {261, 294, 330, 349, 392, 440, 494, 523}; // 부저 음계 도~도

int Mode = 0; // RFID 용 
// 실행시 가장 먼저 호출되는 함수이며, 최초 1회만 실행됩니다.
// 변수를 선언하거나 초기화를 위한 코드를 포함합니다.

struct info {
  float sampleone;
  float sampletwo;
  float samplethree;
}; // 샘플 값을 저장하는 구조체 변수입니다.

void accessOK();
void accessDenied();
void updateShiftRegister();
void stepmotorWorking();

void setup() {
  // I2C LCD를 초기화 합니다..
  lcd.init();
  // I2C LCD의 백라이트를 켜줍니다.
  lcd.backlight();
  Serial.begin(9600);                     // 시리얼 통신, 속도는 9600
  SPI.begin();                             // SPI 초기화(SPI : 하나의 마스터와 다수의 SLAVE(종속적인 역활)간의 통신 방식)
  
  mfrc.PCD_Init();
  
  // trig를 출력모드로 설정, echo를 입력모드로 설정
  pinMode(trigPin, OUTPUT);                // A0핀을 출력으로 설정 trig
  pinMode(echoPin1, INPUT);                // A1핀을 입력으로 설정 echo1
  pinMode(echoPin2, INPUT);                // A2핀을 입력으로 설정 echo2
  pinMode(echoPin3, INPUT);                // A3핀을 입력으로 설정 echo3

  pinMode(step_IN1,OUTPUT);               //2
  pinMode(step_IN2,OUTPUT);               //3 
  pinMode(step_IN3,OUTPUT);               //4 
  pinMode(step_IN4,OUTPUT);               //5 
 
  pinMode(LED_R, OUTPUT);                 // 6번핀을 출력으로 설정 LED
  pinMode(LED_B, OUTPUT);                 // 7번핀을 출력으로 설정 LED
  pinMode(buzzer, OUTPUT);                 // 8번핀을 출력으로 설정 부저

 
}

void loop() { 
   struct info OldDistance[4]; // 샘플 구조체 변수를 담는 배열입니다.
  struct info NewDistance[4]; // 비교하는 대상의 변수를 담는 배열입니다. 한 배열에 다 담을 수 있으나 변수명이 햇갈려서 두개로 나눴습니다.
  float duration1, duration2, duration3, distance1, distance2, distance3;//초음파센서를 위한 변수입니다.

  if ( ! mfrc.PICC_IsNewCardPresent() || ! mfrc.PICC_ReadCardSerial() ) {    //  태그 접촉이 되지 않았을때 또는 아이디가 읽혀지지 않았을때
    delay(500);        
    return;               
  }//카드비접촉
  
  if(mfrc.uid.uidByte[0] == 153 || mfrc.uid.uidByte[0] == 208) {  // 태그 ID가 맞을경우
      Mode = 1; 
      accessOK();
   }//올바른카드 접촉
   
   else {                                   // 다른 태그 ID일 경우
      Mode = 0;
      accessDenied();
   }//잘못된 카드 접촉
   
   if(Mode==1){
       switch(mfrc.uid.uidByte[0]){
           case 153:
           lcd.clear();                  // LCD 초기화.
          lcd.setCursor(0,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
         lcd.print("Milk");
              OldDistance[0].sampleone = 3.095f;
               OldDistance[0].sampletwo = 4.4825f;
               OldDistance[0].samplethree = 4.88f;
  
              OldDistance[1].sampleone = 2.82f;
              OldDistance[1].sampletwo = 4.0025f;
             OldDistance[1].samplethree = 4.01f;
      
              OldDistance[2].sampleone = 3.1775f;
              OldDistance[2].sampletwo = 3.6525f;
              OldDistance[2].samplethree = 3.99f;
  
             OldDistance[3].sampleone = 2.5475f;
             OldDistance[3].sampletwo = 3.9675f;
             OldDistance[3].samplethree = 4.185f;
              break;

            case 208:
             lcd.clear();                  // LCD 초기화.
          lcd.setCursor(0,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
         lcd.print("PET");
              OldDistance[0].sampleone = 3.66f;
               OldDistance[0].sampletwo = 4.7f;
               OldDistance[0].samplethree = 3.88f;
  
              OldDistance[1].sampleone = 3.85f;
              OldDistance[1].sampletwo = 4.7f;
             OldDistance[1].samplethree = 4.7f;
      
              OldDistance[2].sampleone = 3.765f;
              OldDistance[2].sampletwo = 4.8f;
              OldDistance[2].samplethree = 4.4f;
  
             OldDistance[3].sampleone = 3.7f;
             OldDistance[3].sampletwo = 5.06f;
             OldDistance[3].samplethree = 4.78f;
              break;            
              
          default : break;
       }
        for(int k=0;k<CPR;k++){                 //스텝모터 시작
          for(int j=0;j<stepCntPerCheck/4;j++){
             stepmotorWorking();
          }                  //스텝모터 지정 각도만큼 회전 완료
          delay(1000);        //스텝모터 이동 직후 초음파 측정 대기 0.5초
   
          digitalWrite(trigPin, HIGH);
          delay(10);
          digitalWrite(trigPin, LOW);
          duration1 = pulseIn(echoPin1, HIGH); 
          distance1 = ((float)(340 * duration1) / 10000) / 2;  
           
          digitalWrite(trigPin, HIGH);
          delay(10);
          digitalWrite(trigPin, LOW);
          duration2 = pulseIn(echoPin2, HIGH); 
          distance2 = ((float)(340 * duration2) / 10000) / 2;

          digitalWrite(trigPin, HIGH);
          delay(10);
          digitalWrite(trigPin, LOW);
          duration3 = pulseIn(echoPin3, HIGH); 
          distance3 = ((float)(340 * duration3) / 10000) / 2; 

          Serial.print(distance1);
          Serial.println("cm 하");
          Serial.print(distance2);          
          Serial.println("cm 중");
          Serial.print(distance3);          
          Serial.println("cm 상\n");
        
          //초음파로 확인한 거리 저장하기
            NewDistance[k].sampleone = distance1; // 배열안에 샘플 값 (비교하는 대상) 저장1
            NewDistance[k].sampletwo = distance2; // 배열안에 샘플 값 (비교하는 대상) 저장2
            NewDistance[k].samplethree = distance3; // 배열안에 샘플 값 (비교하는 대상) 저장3
    
      lcd.clear();                  // LCD 초기화.
      lcd.setCursor(0,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
      lcd.print(distance1);       // 문구를 출력합니다.
      lcd.setCursor(7,0);
      lcd.print(distance2);       // 문구를 출력합니다.
      lcd.setCursor(0,1);
      lcd.print(distance3);       // 문구를 출력합니다.

            
          delay(1000);        //초음파 측정 후 이동 대기 0.5초
        }//스텝모터 한바퀴 다 돌았습니다.
 
          //원본값과 비교
           char faultArr[12] = {'_','_','_','_','_','_','_','_','_','_','_','_' };
          int faultCnt=0;
            for (int i = 0; i<4;i++){
                if(NewDistance[i].sampleone < OldDistance[i].sampleone-1.5f || 
                    NewDistance[i].sampleone > OldDistance[i].sampleone +1.5f){
                     faultArr[3*i+0] = 'D';
                     faultCnt++;
                 }       
                if(NewDistance[i].sampletwo < OldDistance[i].sampletwo-1.5f || 
                   NewDistance[i].sampletwo > OldDistance[i].sampletwo +1.5f){ 
                    faultArr[3*i+1]='M'; 
                    faultCnt++;
                }
                if(NewDistance[i].samplethree < OldDistance[i].samplethree-1.5f || 
                    NewDistance[i].samplethree > OldDistance[i].samplethree +1.5f ){
                    faultArr[3*i+2]='U';
                     faultCnt++;
                }
            
          }//비교끝
 
      if(faultCnt == 0){
           lcd.clear();                  // LCD 초기화.
           lcd.setCursor(0,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
            lcd.print("same");       // 문구를 출력합니다.

            digitalWrite(LED_R, LOW);  //LED RED 끔
           digitalWrite(LED_B, HIGH);  //LED BLUE 끔
      }
      else{
              lcd.clear();                  // LCD 초기화.
              lcd.setCursor(0,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
              lcd.print("F:");       // 문구를 출력합니다.
              for(int j=0;j<3;j++){
                 lcd.setCursor(j+2,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
                 lcd.print(faultArr[j]);       // 문구를 출력합니다.      
              }
          
              lcd.setCursor(7,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
              lcd.print("L:");       // 문구를 출력합니다.
              for(int j=3;j<6;j++){
                 lcd.setCursor(j+7+2-3,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
                 lcd.print(faultArr[j]);       // 문구를 출력합니다.      
              }

              lcd.setCursor(0,1);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
              lcd.print("B:");       // 문구를 출력합니다.
              for(int j=6;j<9;j++){
                 lcd.setCursor(j+2-6,1);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
                 lcd.print(faultArr[j]);       // 문구를 출력합니다.      
              }     

              lcd.setCursor(7,1);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
              lcd.print("R:");       // 문구를 출력합니다.
              for(int j=9;j<12;j++){
                 lcd.setCursor(j+7+2-9,1);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
                 lcd.print(faultArr[j]);       // 문구를 출력합니다.      
              } 
    
              digitalWrite(LED_R, HIGH);  //LED RED 끔
              digitalWrite(LED_B, LOW);  //LED BLUE 끔

            } //불량위치 lcd 표시


     delay(500); 
      }//Mode==1일때
      

}//loop덮개

void accessOK(){
      lcd.clear();
      lcd.setCursor(0,0);           // 0번째 줄 0번째 셀부터 입력하게 합니다.
      lcd.print("GoodDayCommander");       // 문구를 출력합니다.
      lcd.setCursor(0,1);           // 1번째 줄 0번째 셀부터 입력하게 합니다.
      lcd.print(" Running Start! ");       // 문구를 출력합니다.

      digitalWrite(LED_R, LOW);  //LED RED 끔
      digitalWrite(LED_B, LOW);  //LED BLUE 끔
    
      tone(8,261,100);                  //음계 도
      delay(500);
      tone(8,330,100);                  //음계 미
      delay(500);
      tone(8,392,100);                  //음계 솔
      delay(500);
      tone(8,523,100);                  //음계 높은 도
      delay(500);
      //*************************value = X // X 에 정상품의 정보를 넣어야 합니다.  
}
void accessDenied(){
       lcd.clear();                          // LCD의 모든 내용을 삭제합니다.
      lcd.setCursor(0,0);                   // 0번째 줄 0번째 셀부터 입력하게 합니다.
      lcd.print("  Who Are You!?  ");       // 문구를 출력합니다.
      lcd.setCursor(0,1);                   // 1번째 줄 0번째 셀부터 입력하게 합니다.
      lcd.print("Get out of here!");       // 문구를 출력합니다.

      digitalWrite(LED_R, HIGH);  //LED RED 끔
      digitalWrite(LED_B, LOW);  //LED BLUE 끔
      
      tone(8,523,100);                  //음계 높은 도                    
      delay(300);
      tone(8,523,100);                  //음계 높은 도
      delay(500);
      tone(8,523,100);                  //음계 높은 도                    
      delay(300);
      tone(8,523,100);                  //음계 높은 도
      delay(500);
}

void stepmotorWorking(){
  digitalWrite(step_IN1,HIGH);
     digitalWrite(step_IN2,LOW);
     digitalWrite(step_IN3,LOW);
     digitalWrite(step_IN4,LOW);
     delay(stepDelay);

     digitalWrite(step_IN1,LOW);
     digitalWrite(step_IN2,HIGH);
     digitalWrite(step_IN3,LOW);
     digitalWrite(step_IN4,LOW);
     delay(stepDelay);

     digitalWrite(step_IN1,LOW);
     digitalWrite(step_IN2,LOW);
     digitalWrite(step_IN3,HIGH);
     digitalWrite(step_IN4,LOW);
     delay(stepDelay);

     digitalWrite(step_IN1,LOW);
     digitalWrite(step_IN2,LOW);
     digitalWrite(step_IN3,LOW);
     digitalWrite(step_IN4,HIGH);
     delay(stepDelay);
}
