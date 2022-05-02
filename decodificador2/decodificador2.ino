#define EPS 15
#define AVG_LIM 50

//Frequency(Hz) of the input signal
const float FREQ = 440;

//States of Finite State Machine(FSM)
enum States{  
  FSM_INIT,
  FSM_AVG1,
  FSM_AVG2,
  FSM_PEAK_DETECT,
  FSM_AVG3,
  FSM_PEAK_DETECT2,
  FSM_HIGH_VAL,
  FSM_AVG4,
  FSM_DOWN_DETECT,
  FSM_AVG5,
  FSM_DOWN_DETECT2,
  FSM_MSG,
  FSM_AVG6,
  FSM_DOWN_DETECT3,
  FSM_AVG7,
  FSM_DOWN_DETECT4,
  FSM_END
};

//Stores the current state
States state;
uint16_t N;
uint8_t condition;

struct dataPP{
  int16_t left;
  int16_t right;
}typedef DataPP;

DataPP Default;
DataPP Avg;
DataPP AvgCurr,AvgOld;

//Calibration variables
DataPP highest;

//Information sent through audio
DataPP msg;

void PP(DataPP *sig, float freq){ 
  int16_t X_min = 4096;
  int16_t X_max = 0;
  int16_t Y_min = 4096;
  int16_t Y_max = 0;
  
  unsigned long T = 20*(1/freq);
  unsigned long t0 = (unsigned long)millis();

  while((unsigned long)(millis() - t0) <= T){
    int16_t leftIn = analogRead(A0);
    int16_t rightIn = analogRead(A2);

    X_min = leftIn < X_min ? leftIn : X_min;
    X_max = leftIn > X_max ? leftIn : X_max;
    Y_min = rightIn < Y_min ? rightIn : Y_min;
    Y_max = rightIn > Y_max ? rightIn : Y_max;    
  };
  
  sig->left = X_max - X_min;
  sig->right = Y_max - Y_min;
}
bool average(DataPP *sigIn,DataPP *store,DataPP *avg){  
  if(N <= AVG_LIM){
    avg->left += sigIn->left;
    avg->right += sigIn->right;
    N++;
    return 0;
  }else{
    store->left = avg->left/N;
    store->right = avg->right/N;
    N = 0;
    avg->left = 0;
    avg->right = 0;
    return 1;
  }      
}
void setup() {
  Serial.begin(500000);
  pinMode(A0,INPUT);
  pinMode(A2,INPUT);

  Default.left = Default.right = 0;
  PP(&Default,FREQ);
  
  highest.left = Default.left;  
  highest.right = Default.right;  
  //flag1 = flag2 = flag3 = flag4 = theEnd = 0;
  
  state = FSM_INIT;
  N = 0;
}
void loop() { 
  DataPP sigIn,sigOld;
  
  PP(&sigIn,FREQ);  
  /*
  bool rdy = average(&sigIn,&AvgCurr,&Avg);

  if(AvgCurr.left > 200 || AvgCurr.right > 200){
    AvgCurr.left = AvgOld.left;
    AvgCurr.right = AvgOld.right;
  }    
  */
  Serial.println("State = "+(String)state);
  switch(state){
    case FSM_INIT:                  
      state = FSM_AVG1;      
      break;
    case FSM_AVG1:
      state = average(&sigIn,&AvgCurr,&Avg) == 1 ? FSM_AVG2 : FSM_AVG1;
      if(state == FSM_AVG2){
        AvgOld.left = AvgCurr.left;
        AvgOld.right = AvgCurr.right;
      }
      break;
    case FSM_AVG2:        
      state = average(&sigIn,&AvgCurr,&Avg) == 1 ? FSM_PEAK_DETECT : FSM_AVG2;
      break;
    case FSM_PEAK_DETECT:
      state = AvgCurr.right > AvgOld.right+EPS && AvgCurr.left > AvgOld.left+EPS ? FSM_AVG3 : FSM_AVG2;
      Serial.println("Curr Right = "+(String)AvgCurr.right+", Curr Left = "+(String)AvgCurr.left);
      Serial.println("Old Right = "+(String)AvgOld.right+", Old Left = "+(String)AvgOld.left);
      if(state == FSM_AVG2){
        Default.left = AvgCurr.left;
        Default.right = AvgCurr.right;
      }
      break;
    case FSM_AVG3:
      state = average(&sigIn,&AvgCurr,&Avg) == 1 ? FSM_PEAK_DETECT2 : FSM_AVG3;
      if(N == 0){
        AvgOld.left = AvgCurr.left;
        AvgOld.right = AvgCurr.right;
        delay(10);
      }
      break;
    case FSM_PEAK_DETECT2:
      state =  AvgCurr.right < AvgOld.right+EPS && AvgCurr.right > AvgOld.right-EPS && AvgCurr.left < AvgOld.left+EPS && AvgCurr.left > AvgOld.left-EPS ? FSM_HIGH_VAL : FSM_AVG3;        
      break;
    case FSM_HIGH_VAL:
      highest.left = (AvgCurr.left+AvgOld.left)/2;
      highest.right = (AvgCurr.right+AvgOld.right)/2;        
      state = FSM_AVG4;
      AvgOld.right = AvgCurr.right;
      AvgOld.left = AvgCurr.left;
      break;
    case FSM_AVG4:
      state = average(&sigIn,&AvgCurr,&Avg) == 1 ? FSM_DOWN_DETECT : FSM_AVG4;  
      break;
    case FSM_DOWN_DETECT:
      Serial.println("Curr Right = "+(String)AvgCurr.right+", Curr Left = "+(String)AvgCurr.left);
      Serial.println("Old Right = "+(String)AvgOld.right+", Old Left = "+(String)AvgOld.left);
      state = AvgCurr.right < AvgOld.right+EPS && AvgCurr.right > AvgOld.right-EPS && AvgCurr.left < AvgOld.left+EPS && AvgCurr.left > AvgOld.left-EPS ? FSM_AVG4 : FSM_AVG5;                
      break;
    case FSM_AVG5:
      state = average(&sigIn,&AvgCurr,&Avg) == 1 ? FSM_DOWN_DETECT2 : FSM_AVG5;
      if(N == 0){
        AvgOld.left = AvgCurr.left;
        AvgOld.right = AvgCurr.right;
        delay(10);
      }
      break;
    case FSM_DOWN_DETECT2:
      state = AvgCurr.right < AvgOld.right+EPS && AvgCurr.right > AvgOld.right-EPS && AvgCurr.left < AvgOld.left+EPS && AvgCurr.left > AvgOld.left-EPS ? FSM_MSG : FSM_AVG5;
      break;
    case FSM_MSG:
      msg.left = (AvgCurr.left + AvgOld.left)/2;
      msg.right = (AvgCurr.right+AvgOld.right)/2;
      state = FSM_AVG6;
      AvgOld.left = AvgCurr.left;
      AvgOld.right = AvgCurr.right;
      break;
    case FSM_AVG6:
      state = average(&sigIn,&AvgCurr,&Avg) == 1 ? FSM_DOWN_DETECT3 : FSM_AVG6;           
      break;
    case FSM_DOWN_DETECT3:
      state = AvgCurr.right < AvgOld.right+EPS && AvgCurr.right > AvgOld.right-EPS && AvgCurr.left < AvgOld.left+EPS && AvgCurr.left > AvgOld.left-EPS ? FSM_AVG6 : FSM_AVG7;
      break;
    case FSM_AVG7:
      state = average(&sigIn,&AvgCurr,&Avg) == 1 ? FSM_DOWN_DETECT4 : FSM_AVG7;
      if(N == 0){
        AvgOld.left = AvgCurr.left;
        AvgOld.right = AvgCurr.right;
        delay(10);
      }
      break;
    case FSM_DOWN_DETECT4:
      state = AvgCurr.right < AvgOld.right+EPS && AvgCurr.right > AvgOld.right-EPS && AvgCurr.left < AvgOld.left+EPS && AvgCurr.left > AvgOld.left-EPS ? FSM_END : FSM_AVG7;
      break;
    case FSM_END:
      Serial.println("Default Right = "+(String)Default.right+", Default Left = "+(String)Default.left);
      Serial.println("Highest Right = "+(String)highest.right+", Highest Left = "+(String)highest.left);
      Serial.println("MSG Right = "+(String)msg.right+", MSG Left = "+(String)msg.left);
      highest.left = msg.left = AvgOld.left = 0;  
      highest.right = msg.right = AvgOld.right = 0;
      state = FSM_INIT;
      break;
  }  
}
