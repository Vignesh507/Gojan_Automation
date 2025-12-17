int relay[]={18,19,22,23,21,4,15};
void setup() {
 for(int i=0;i<7;i++){
   pinMode(relay[i],OUTPUT);
  }
}

void loop() {
  for(int i=0;i<7;i++) {
    digitalWrite(relay[i],1);
    delay(1000);
  }
  for(int i=7;i>=0;i--){
     digitalWrite(relay[i],0);
     delay(1000);
  }


}
