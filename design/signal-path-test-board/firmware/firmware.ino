#define ATTEN 23
#define S0 22
#define S1 21
#define S2 20
#define S3 19

elapsedMicros sinceStart;

void setup() {
  // put your setup code here, to run once:
  pinMode(ATTEN, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  analogWriteResolution(12);
  analogReference(INTERNAL);

  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);

  digitalWrite(ATTEN, HIGH);
  

  
}

void loop() {
  double rel =  sinceStart / 1e6;
  analogWrite(A12, (int)(1551.0 * (sin(628.318 * rel)+ 1.0)));

}
