#define EPS 2

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
DataPP lowest;
DataPP low;
DataPP high;
DataPP highest;
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
void process(uint8_t NextState,uint8_t CurrentState,DataPP *sigIn,DataPP *store,uint8_t condition){
  if(condition){    
    state = NextState;
    store->left = store->left / N;
    store->right = store->right / N;
    N = 0;
  }else{
    if(N <= 50){      
      store->left += sigIn->left;
      store->right += sigIn->right;
      N++;
    }else{
      Serial.println(store->left/N);
      Serial.print(",");
      Serial.println(store->right/N);
      N = 0;
      store->left = 0;
      store->right = 0;
    }
    state = CurrentState;
  }  
}
void setup() {
  Serial.begin(500000);
  pinMode(A0,INPUT);
  pinMode(A2,INPUT);

  Default.left = Default.right = 0;  
  PP(&Default,FREQ);
  
  lowest.left = low.left = high.left = highest.left = Default.left;  
  lowest.right = low.right = high.right = highest.right = Default.right;  
  //flag1 = flag2 = flag3 = flag4 = theEnd = 0;
  
  state = FSM_INIT;
  N = 0;
}
void loop() { 
  DataPP sigIn,sigOld;    

  PP(&sigIn,FREQ);
  //delayMicroseconds(1000);

  /*
  Serial.println("(SigLeft) = "+(String)sigIn.left);  
  Serial.print(",");
  Serial.println("(SigRight) = "+(String)sigIn.right);
  */
    
  /*
  Serial.println("Current state: "+(String)state);
  Serial.println("Current N: "+(String)N);
  
  /*
  Serial.println("(DefaultLeft) = "+(String)Default.left);
  Serial.print(",");
  Serial.println("(DefaultRight) = "+(String)Default.right);
  */
  
  switch(state){
    case FSM_INIT:      
      //condition = sigIn.left > 1.3*sigOld.left && sigIn.right > 1.3*sigOld.right;
      
      process(FSM_LOWEST,FSM_INIT,&sigIn,&Default,condition);
      
      if(condition){
        Serial.println("(DefaultLeft) = "+(String)Default.left);
        Serial.print(",");
        Serial.println("(DefaultRight) = "+(String)Default.right);
      }       
      
      break;
    case FSM_LOWEST:
      condition = sigIn.left > 1.12*sigOld.left && sigIn.right > 1.12*sigOld.right;
      
      process(FSM_LOW,FSM_LOWEST,&sigIn,&lowest,condition);

      
      if(condition){
        Serial.println("(LowestLeft) = "+(String)lowest.left);
        Serial.print(",");
        Serial.println("(LowestRight) = "+(String)lowest.right);
      }
        
      break;
    case FSM_LOW:      
      condition = sigIn.left > 1.12*sigOld.left && sigIn.right > 1.12*sigOld.right;
  
      process(FSM_HIGH,FSM_LOW,&sigIn,&low,condition);
      
      if(condition){
        Serial.println("(LowLeft) = "+(String)low.left);
        Serial.print(",");
        Serial.println("(LowRight) = "+(String)low.right);
      }
      
      break;
    case FSM_HIGH:
      condition = sigIn.left > 1.12*sigOld.left && sigIn.right > 1.12*sigOld.right;
      
      process(FSM_HIGHEST,FSM_HIGH,&sigIn,&high,condition);

      
      if(condition){
        Serial.println("(HighLeft) = "+(String)high.left);
        Serial.print(",");
        Serial.println("(HighRight) = "+(String)high.right);
      } 
        
      break;
    case FSM_HIGHEST:
      condition = (sigIn.left < 0.95*sigOld.left || sigIn.right < 0.95*sigOld.right) || 
                  (sigIn.left > 0.9*sigOld.left && sigIn.left < 1.1*sigOld.left && sigIn.right < 0.6*sigOld.right); //Botão 9

      process(FSM_MSG,FSM_HIGHEST,&sigIn,&highest,condition);

      
      if(condition){
        Serial.println("(HighestLeft) = "+(String)highest.left);
        Serial.print(",");
        Serial.println("(HighestRight) = "+(String)highest.right);
      }
       
      break;
    case FSM_MSG:
      condition = sigIn.left < sigOld.left && sigIn.right < sigOld.right;

      process(FSM_OUTPUT,FSM_MSG,&sigIn,&msg,condition);

      
      if(condition){
        Serial.println("(MsgLeft) = "+(String)msg.left);
        Serial.print(",");
        Serial.println("(MsgRight) = "+(String)msg.right);
      }
       
      break;
    case FSM_OUTPUT:
      
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
      break;
  }
  
  sigOld.left = sigIn.left;
  sigOld.right = sigIn.right;  
  
  //Serial.println("(LOldPP) = "+(String)sigOld.left+"\t(ROldPP) = "+(String)sigOld.right);
  
  //Serial.println("LowestMinL = "+(String)lowestMinL+",LowestMaxL = "+(String)lowestMaxL+",LowestMinR = "+(String)lowestMinR+",LowestMaxR = "+(String)lowestMaxR);
  //Serial.println("LowMinL = "+(String)lowMinL+",LowMaxL = "+(String)lowMaxL+",LowMinR = "+(String)lowMinR+",LowMaxR = "+(String)lowMaxR);
  //Serial.println("HighMinL = "+(String)highMinL+",HighMaxL = "+(String)highMaxL+",HighMinR = "+(String)highMinR+",HighMaxR = "+(String)highMaxR);
  //Serial.println("HighestMinL = "+(String)highestMinL+",HighestMaxL = "+(String)highestMaxL+",HighestMinR = "+(String)highestMinR+",HighestMaxR = "+(String)highestMaxR);

}
