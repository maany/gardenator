#define RELAY_LIGHT_PIN 5
const int LDR_PIN = A0;
const int LDR_CUTOFF = 200;
void setup(){
    Serial.begin(9600);
    pinMode(RELAY_LIGHT_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);
}
void loop(){
    int ldr_value = analogRead(LDR_PIN);
    
    // Print LDR Value
    Serial.print("LDR Value: ");
    Serial.println(ldr_value);
    
    // Turn Light on if light is too little
    // TODO add constaint on total time light has been present i.e light:dark ratio
    if(ldr_value <= LDR_CUTOFF){
        digitalWrite(RELAY_LIGHT_PIN, HIGH);
    } else {
        digitalWrite(RELAY_LIGHT_PIN, LOW);
    }
}