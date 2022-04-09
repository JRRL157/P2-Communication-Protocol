#define L 7

void avg(uint32_t *x, uint32_t *y){
  int len = pow(2,L);
  
  for(int i = 0;i<len;++i){
    *x += analogRead(A0);
    delayMicroseconds(100);
    *y += analogRead(A2);
    delayMicroseconds(100);
  }

  *x = ((*x)>>L);
  *y = ((*y)>>L);
}
void rms(float *RmsL,float *RmsR,uint32_t *AvgL,uint32_t *AvgR){

  int len = pow(2,L);
  for(int i = 0;i<len;++i){
    int16_t x = (analogRead(A0) - *AvgL);
    *RmsL += x*x;
    int16_t y = (analogRead(A2) - *AvgR);
    *RmsR += y*y;
  }
  
  *RmsL = (sqrt((*RmsL)/len));
  *RmsR = (sqrt((*RmsR)/len));
}

void setup() {
  Serial.begin(115200);
  pinMode(A0,INPUT);
  pinMode(A2,INPUT);  
}

void loop() { 
  
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
  
  Serial.println("Rms esquerdo = "+(String)RmsL);
  Serial.print(",");
  Serial.println("Rms direito = "+(String)RmsR);
}
