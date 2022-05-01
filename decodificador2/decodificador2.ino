#define EPS 10
#define AVG_LIM 50

//Frequency(Hz) of the input signal
const float FREQ = 200;

//States of Finite State Machine(FSM)
enum States{
  FSM_INIT,
  FSM_LOWEST,
  FSM_LOW,
  FSM_HIGH,
  FSM_HIGHEST,
  FSM_MSG,
  FSM_OUTPUT
};

//Stores the current state
uint8_t state;
uint16_t N;
uint8_t condition;

struct dataPP{
  uint16_t left;
  uint16_t right;
}typedef DataPP;

DataPP Default;
DataPP Avg;
DataPP AvgIn,AvgOld;
DataPP old;

//Calibration variables
DataPP lowest;
DataPP low;
DataPP high;
DataPP highest;

//Information sent through audio
DataPP msg;
//uint8_t flag1,flag2,flag3,flag4,theEnd;

void PP(DataPP *sig, float freq){ 
  uint16_t X_min = 4096;
  uint16_t X_max = 0;
  uint16_t Y_min = 4096;
  uint16_t Y_max = 0;
  
  unsigned long T = 20*(1/freq);
  unsigned long t0 = (unsigned long)millis();

  while((unsigned long)(millis() - t0) <= T){
    uint16_t leftIn = analogRead(A0);
    uint16_t rightIn = analogRead(A2);

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
  Serial.begin(115200);
  pinMode(A0,INPUT);
  pinMode(A2,INPUT);

  Default.left = Default.right = 0;  
  PP(&Default,FREQ);
  
  lowest.left = low.left = high.left = highest.left = Default.left;  
  lowest.right = low.right = high.right = highest.right = Default.right;
  old.left = old.right = 4096;
  //flag1 = flag2 = flag3 = flag4 = theEnd = 0;
  
  state = FSM_INIT;
  N = 0;
}
void loop() { 
  DataPP sigIn,sigOld;
  
  PP(&sigIn,FREQ);
  //delayMicroseconds(1000);
  bool rdy = average(&sigIn,&AvgIn,&Avg);

  if(AvgIn.left > 200 || AvgIn.right > 200){
    AvgIn.left = AvgOld.left;
    AvgIn.right = AvgOld.right;
  }    

  /*
  Serial.println("(SigLeft) = "+(String)sigIn.left);  
  Serial.print(",");
  Serial.println("(SigRight) = "+(String)sigIn.right);
  */
    
  /*
  Serial.println("Current state: "+(String)state);
  Serial.println("Current N: "+(String)N);
  */
  
  /*
  Serial.println("(DefaultLeft) = "+(String)Default.left);
  Serial.print(",");
  Serial.println("(DefaultRight) = "+(String)Default.right);
  */
  
  if(rdy){
    switch(state){
      case FSM_INIT:                  
        condition = AvgIn.left-AvgOld.left > EPS || AvgIn.right-AvgOld.right > EPS;
        /*
        Serial.println(Default.left);
        Serial.print(",");
        Serial.println(Default.right);
        */
        
        if(condition){
          Serial.println("DefaultL:"+(String)AvgOld.left);
          Serial.print(",");
          Serial.println("DefaultR:"+(String)AvgOld.right);
          Default.left = AvgOld.left;
          Default.right = AvgOld.right;
          state = FSM_LOWEST;
          AvgOld.left = AvgIn.left;
          AvgOld.right = AvgIn.right;
        }       
        break;
      case FSM_LOWEST:      
        
        condition = AvgIn.left-AvgOld.left > EPS || AvgIn.right-AvgOld.right > EPS;
        
        if(condition){
          Serial.println("LowestL:"+(String)AvgOld.left);
          Serial.print(",");
          Serial.println("LowestR:"+(String)AvgOld.right);
          lowest.left = AvgOld.left;
          lowest.right = AvgOld.right;
          state = FSM_LOWEST;
          AvgOld.left = AvgIn.left;
          AvgOld.right = AvgIn.right;   
        }      
        break;
      case FSM_LOW:              
        condition = AvgIn.left-AvgOld.left > EPS || AvgIn.right-AvgOld.right > EPS;
        
        if(condition){
          Serial.println("LowL:"+(String)AvgOld.left);
          Serial.print(",");
          Serial.println("LowR:"+(String)AvgOld.right);
          low.left = AvgOld.left;
          low.right = AvgOld.right;
          state = FSM_LOWEST;
          AvgOld.left = AvgIn.left;
          AvgOld.right = AvgIn.right;
        }      
        break;
      case FSM_HIGH:
        condition = AvgIn.left-AvgOld.left > EPS || AvgIn.right-AvgOld.right > EPS;
        
        if(condition){
          Serial.println("HighL:"+(String)AvgOld.left);
          Serial.print(",");
          Serial.println("HighR:"+(String)AvgOld.right);
          high.left = AvgOld.left;
          high.right = AvgOld.right;
          state = FSM_LOWEST;
          AvgOld.left = AvgIn.left;
          AvgOld.right = AvgIn.right;
        }       
        break;
      case FSM_HIGHEST:
        condition = AvgOld.right > AvgIn.right+2*EPS;
        
        if(condition){
          Serial.println("HighestL:"+(String)AvgOld.left);
          Serial.print(",");
          Serial.println("HighestR:"+(String)AvgOld.right);
          highest.left = AvgOld.left;
          highest.right = AvgOld.right;
          state = FSM_LOWEST;
          AvgOld.left = AvgIn.left;
          AvgOld.right = AvgIn.right;
        }      
        break;
      case FSM_MSG:
        average(&sigIn,&msg,&Avg);
        
        condition = msg.left < EPS && msg.right < EPS;
        
        if(condition){
          Serial.println(AvgOld.left);
          Serial.print(",");
          Serial.println(AvgOld.right);
          msg.left = AvgOld.left;
          msg.right = AvgOld.right;
          state = FSM_LOWEST;
          AvgOld.left = AvgIn.left;
          AvgOld.right = AvgIn.right;
        }
        break;
      case FSM_OUTPUT:
        Serial.println("DefaultLeft = "+(String)Default.left+";DefaultRight = "+(String)Default.right);
        Serial.println("LowestLeft = "+(String)lowest.left+";LowestRight = "+(String)lowest.right);
        Serial.println("LowLeft = "+(String)low.left+";LowRight = "+(String)low.right);
        Serial.println("HighLeft = "+(String)high.left+";HighRight = "+(String)high.right);
        Serial.println("HighestLeft = "+(String)highest.left+";HighestRight = "+(String)highest.right);
        
        if( msg.left < lowest.left+EPS && msg.left > lowest.left-EPS && msg.right < lowest.right+EPS && msg.right > lowest.right-EPS)
          Serial.println("BOTAO 0");          
        else if( msg.left < lowest.left+EPS && msg.left > lowest.left-EPS && msg.right < low.right+EPS && msg.right > low.right-EPS)
          Serial.println("BOTAO 1");
        else if( msg.left < low.left+EPS && msg.left > low.left-EPS && msg.right < lowest.right+EPS && msg.right > lowest.right-EPS)
          Serial.println("BOTAO 2");
        else if( msg.left < low.left+EPS && msg.left > low.left-EPS && msg.right < low.right+EPS && msg.right > low.right-EPS)
          Serial.println("BOTAO 3");
        else if( msg.left < high.left+EPS && msg.left > high.left-EPS && msg.right < lowest.right+EPS && msg.right > lowest.right-EPS)
          Serial.println("BOTAO 4");
        else if( msg.left < lowest.left+EPS && msg.left > lowest.left-EPS && msg.right < high.right+EPS && msg.right > high.right-EPS)
          Serial.println("BOTAO 5");
        else if( msg.left < high.left+EPS && msg.left > high.left-EPS && msg.right < low.right+EPS && msg.right > low.right-EPS)
          Serial.println("BOTAO 6");
        else if( msg.left < low.left+EPS && msg.left > low.left-EPS && msg.right < high.right+EPS && msg.right > high.right-EPS)
          Serial.println("BOTAO 7");
        else if( msg.left < high.left+EPS && msg.left > high.left-EPS && msg.right < high.right+EPS && msg.right > high.right-EPS)
          Serial.println("BOTAO 8");
        else if( msg.left < highest.left+EPS && msg.left > highest.left-EPS && msg.right < lowest.right+EPS && msg.right > lowest.right-EPS)
          Serial.println("BOTAO 9");
        
        state = FSM_INIT;
        PP(&Default,FREQ);
        lowest.left = low.left = high.left = highest.left = Default.left;
        lowest.right = low.right = high.right = highest.right = Default.right;
        old.left = old.right = 4096;
        break;
    }
  }
  
  delay(10);
  AvgOld.left = AvgIn.left;
  AvgOld.right = AvgIn.right;
}
