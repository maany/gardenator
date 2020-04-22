#define RELAY_LIGHT_PIN 5

void setup(){
    Serial.begin(9600);
    pinMode(RELAY_LIGHT_PIN, OUTPUT);
}
void loop(){
    Serial.println("Hello World");
    delay(1000);
    digitalWrite(RELAY_LIGHT_PIN, HIGH);
    delay(4000);
    digitalWrite(RELAY_LIGHT_PIN, LOW);
}