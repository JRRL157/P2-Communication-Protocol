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
  float left;
  float right;
}typedef DataPP;

DataPP DefaultAVG,DefaultRMS;
DataPP lowestAVG,lowestRMS;
DataPP lowAVG,lowRMS;
DataPP highAVG,highRMS;
DataPP highestAVG,highestRMS;
DataPP msgAVG,msgRMS;
//uint8_t flag1,flag2,flag3,flag4,theEnd;

void AVG(DataPP *sig, float freq){ 
  uint32_t N = 0;
  
  float T = 1000*(1/freq);
  unsigned long t0 = (unsigned long)millis();

  while((unsigned long)(millis() - t0) <= T){
    uint32_t leftIn = analogRead(A0);
    uint32_t rightIn = analogRead(A2);

    sig->left += leftIn;
    sig->right += rightIn;
    N++;
  };
  sig->left /= N;
  sig->right /= N;
}
void RMS(DataPP *Xrms,DataPP *avg,float freq){
  float T = 1000*(1/freq);
  float l;
  float r;
  uint32_t N = 0;
  
  unsigned long t0 = (unsigned long)millis();

  while((unsigned long)(millis() - t0) <= T){
    l = analogRead(A0) - avg->left;
    r = analogRead(A2) - avg->right;
    Xrms->left += l*l;
    Xrms->right += r*r;
    N++;
  };
  Xrms->left = sqrt(Xrms->left/N);
  Xrms->right = sqrt(Xrms->right/N);
}

void process(uint8_t NextState,uint8_t CurrentState,DataPP *sigIn,DataPP *store,uint8_t condition){
  if(condition){    
    state = NextState;
    store->left = store->left / N;
    store->right = store->right / N;
    N = 0;
  }else{
    if(N <= 100){      
      store->left += sigIn->left;
      store->right += sigIn->right;
      N++;
    }else{
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

  DefaultAVG.left = DefaultAVG.right = 0;
  DefaultRMS.left = DefaultRMS.right = 0;
  AVG(&DefaultAVG,FREQ);
  RMS(&DefaultRMS,&DefaultAVG,FREQ);
  
  lowestAVG.left = lowAVG.left = highAVG.left = highestAVG.left = DefaultAVG.left;
  lowestRMS.left = lowRMS.left = highRMS.left = highestRMS.left = DefaultRMS.left;
  lowestAVG.right = lowAVG.right = highAVG.right = highestAVG.right = DefaultAVG.right;
  lowestRMS.right = lowRMS.right = highRMS.right = highestRMS.right = DefaultRMS.right;
  //flag1 = flag2 = flag3 = flag4 = theEnd = 0;
  
  state = 20;
  N = 0;
}
void loop() { 
  DataPP avgIn;
  DataPP avgOld;
  DataPP rmsIn;
  DataPP rmsOld;

  avgIn.left = avgIn.right = 0;  
  rmsIn.left = rmsIn.right = 0;
  AVG(&avgIn,FREQ);
  RMS(&rmsIn,&avgIn,FREQ);
  //delayMicroseconds(1000);
  
  Serial.println("(SigLeft) = "+(String)rmsIn.left);  
  Serial.print(",");
  Serial.println("(SigRight) = "+(String)rmsIn.right);
    
  /*
  Serial.println("Current state: "+(String)state);
  Serial.println("Current N: "+(String)N);
  
  /*
  Serial.println("(DefaultLeft) = "+(String)Default.left);
  Serial.print(",");
  Serial.println("(DefaultRight) = "+(String)Default.right);
  */
  /*
  switch(state){
    case FSM_INIT:      
      condition = sigIn.left > 1.3*sigOld.left && sigIn.right > 1.3*sigOld.right;
      
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
      AVG(&Default,FREQ);
      lowest.left = low.left = high.left = highest.left = Default.left;
      lowest.right = low.right = high.right = highest.right = Default.right;
      break;
  }
  */
  avgOld.left = avgIn.left;
  avgOld.right = avgIn.right;
  rmsOld.left = rmsIn.left;
  rmsOld.right = rmsIn.right;
  
  //Serial.println("(LOldPP) = "+(String)sigOld.left+"\t(ROldPP) = "+(String)sigOld.right);
  
  //Serial.println("LowestMinL = "+(String)lowestMinL+",LowestMaxL = "+(String)lowestMaxL+",LowestMinR = "+(String)lowestMinR+",LowestMaxR = "+(String)lowestMaxR);
  //Serial.println("LowMinL = "+(String)lowMinL+",LowMaxL = "+(String)lowMaxL+",LowMinR = "+(String)lowMinR+",LowMaxR = "+(String)lowMaxR);
  //Serial.println("HighMinL = "+(String)highMinL+",HighMaxL = "+(String)highMaxL+",HighMinR = "+(String)highMinR+",HighMaxR = "+(String)highMaxR);
  //Serial.println("HighestMinL = "+(String)highestMinL+",HighestMaxL = "+(String)highestMaxL+",HighestMinR = "+(String)highestMinR+",HighestMaxR = "+(String)highestMaxR);

}
