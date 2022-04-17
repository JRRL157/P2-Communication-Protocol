#define L 6
#define MIN 5
#define EPS 5

//These are RMS values
float lowL,low_midL,mid_highL,highL;
float lowR,low_midR,mid_highR,highR;

//Stores how many times a certain "key" was "pressed"
int teclado[10];

void avg(uint32_t *x, uint32_t *y){
  int len = pow(2,L);
  
  for(uint8_t i = 0;i<len;++i){
    *x += analogRead(A0);    
    *y += analogRead(A2);
  }

  *x = ((*x)>>L);
  *y = ((*y)>>L);
}
void rms(float *RmsL,float *RmsR,uint32_t *AvgL,uint32_t *AvgR){

  uint8_t len = pow(2,L);
  for(int i = 0;i<len;++i){
    int16_t x = (analogRead(A0) - *AvgL);
    *RmsL += x*x;
    int16_t y = (analogRead(A2) - *AvgR);
    *RmsR += y*y;
  }
  
  *RmsL = (sqrt((*RmsL)/len));
  *RmsR = (sqrt((*RmsR)/len));
}

void RMS(float* left,float* right){
  uint32_t AvgL = 0;
  uint32_t AvgR = 0;

  //Measure the Average value
  avg(&AvgL,&AvgR);

  //Serial.println("AVG esquerdo = "+(String)AvgL);
  //Serial.println("AVG direito = "+(String)AvgR);

  //Measure the RMS value

  float RmsL = 0;
  float RmsR = 0;

  rms(&RmsL,&RmsR,&AvgL,&AvgR);

  *left = RmsL;
  *right = RmsR;
}
int count_max(){
  int16_t maior = -1;
  int8_t index = -1;

  for(uint8_t i = 0;i<10;++i)
    if(teclado[i] > maior){
      maior = teclado[i];
      index = i;
    }

  return index;
}
void setup() {
  Serial.begin(115200);
  pinMode(A0,INPUT);
  pinMode(A2,INPUT);
  
  lowL = low_midL = mid_highL = highL = MIN;  
  lowR = low_midR = mid_highR = highR = MIN;
  memset(teclado, 0,sizeof(teclado));
}

void loop() { 
  //Doing the calibration for 0.1V, 0.3V, 0.6V and 0.9V,
  //not really volts, because the arduino will do a Analog-Digital conversion

  float RmsL,RmsR;

  RMS(&RmsL,&RmsR);

  
  Serial.println("Rms esquerdo = "+(String)RmsL);
  Serial.print(",");
  Serial.println("Rms direito = "+(String)RmsR);
  
  
  if(RmsL > lowL && lowL == MIN){ //Left
    lowL = RmsL;    
  }
  if(RmsR > lowR && lowR == MIN){ //Right
    lowR = RmsR;
  }

  if(RmsL > 2*lowL && RmsL < 3.2*lowL && low_midL == MIN && lowL != MIN){
    low_midL = RmsL;
  }
  if(RmsR > 2*lowR && RmsL < 3.2*lowR && low_midR == MIN && lowR != MIN){
    low_midR = RmsR;
  }

  if(RmsL > 4*lowL && RmsL < 6.3*lowL && mid_highL == MIN && low_midL != MIN){
    mid_highL = RmsL;
  }
  if(RmsR > 4*lowR && RmsR < 6.3*lowR && mid_highR == MIN && low_midR != MIN){
    mid_highR = RmsR;
  }

  if(RmsL > 7*lowL && RmsL < 10*lowL && highL == MIN && mid_highL != MIN){
    highL = RmsL;
  }
  if(RmsR > 7*lowR && RmsR < 10*lowR && highR == MIN && mid_highR != MIN){
    highR = RmsR;
  }
  
  //Reading the new values now that the calibration is already done!  
  if(highL != MIN && highR != MIN){    

    Serial.println("LowL = "+(String)lowL+", Low-MidL = "+(String)low_midL+",Mid-HighL = "+(String)mid_highL+",HighL = "+(String)highL);
    Serial.println("LowR = "+(String)lowR+", Low-MidR = "+(String)low_midR+",Mid-HighR = "+(String)mid_highR+",HighR = "+(String)highR);

    Serial.println("Rms esquerdo = "+(String)RmsL);
    Serial.print(",");
    Serial.println("Rms direito = "+(String)RmsR);
    
    if(RmsL >= lowL-(0.3*lowL) && RmsL <= lowL+(0.3*lowL) && RmsR >= lowR-(0.3*lowL) && RmsR <= lowR+(0.3*lowL)){
      //Serial.println("0");
      teclado[0]++;
    }else if(RmsL >= lowL-(0.3*lowL) && RmsL <= lowL+(0.3*lowL) && RmsR >= low_midR-(0.3*low_midR) && RmsR <= low_midR+(0.3*low_midR)){
      //Serial.println("1");
      teclado[1]++;
    }else if(RmsL >= low_midL-(0.3*low_midL) && RmsL <= low_midL+(0.3*low_midL) && RmsR >= lowR-(0.3*lowR) && RmsR <= lowR+(0.3*lowR)){
      //Serial.println("2");
      teclado[2]++;
    }else if(RmsL >= low_midL-(0.3*low_midL) && RmsL <= low_midL+(0.3*low_midL) && RmsR >= low_midR-(0.3*low_midR) && RmsR <= low_midR+(0.3*low_midR)){
      //Serial.println("3");
      teclado[3]++;
    }else if(RmsL >= lowL-(0.3*lowL) && RmsL <= lowL+(0.3*lowL) && RmsR >= mid_highR-(0.3*mid_highR) && RmsR <= mid_highR+(0.3*mid_highR)){
      //Serial.println("4");
      teclado[4]++;
    }else if(RmsL >= mid_highL-(0.3*mid_highL) && RmsL <= mid_highL+(0.3*mid_highL) && RmsR >= lowL-(0.3*lowR) && RmsR <= lowL+(0.3*lowR)){
      //Serial.println("5");
      teclado[5]++;
    }else if(RmsL >= mid_highL-(0.3*mid_highL) && RmsL <= mid_highL+(0.3*mid_highL) && RmsR >= mid_highR-(0.3*mid_highR) && RmsR <= mid_highR+(0.3*mid_highR)){
      //Serial.println("6");
      teclado[6]++;
    }else if(RmsL >= low_midL-(0.3*low_midL) && RmsL <= low_midL+(0.3*low_midL) && RmsR >= mid_highR-(0.3*mid_highR) && RmsR <= mid_highR+(0.3*mid_highR)){
      //Serial.println("7");
      teclado[7]++;
    }else if(RmsL >= mid_highL-(0.3*mid_highL) && RmsL <= mid_highL+(0.3*mid_highL) && RmsR >= low_midR-(0.3*low_midR) && RmsR <= low_midR+(0.3*low_midR)){
      //Serial.println("8");
      teclado[8]++;
    }else if(RmsL >= highL-(0.3*highL) && RmsL <= highL+(0.3*highL) && RmsR >= lowR-(0.3*lowR) && RmsR <= lowR+(0.3*lowR)){
      //Serial.println("9");
      teclado[9]++;
    }    

    for(int i = 0;i<10;++i)
      Serial.print((String)teclado[i] + ",");
    Serial.println("");
    
    //Going back to default values if the RmsL and RmsR are too low again (no input)
    if(RmsL < EPS && RmsR < EPS){
      Serial.println(count_max());
      lowL = low_midL = mid_highL = highL = MIN;  
      lowR = low_midR = mid_highR = highR = MIN;
      memset(teclado,0,sizeof(teclado));
    }
  }
  
}
