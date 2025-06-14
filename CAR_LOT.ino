#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <Servo.h> // For controlling the servo motor
//#include <Wire.h>
//#include <LiquidCrystal_I2C.h> // For controlling the LCD display

// Wi-Fi credentials
const char* ssid = "AndroidAP_3092";
const char* password = "ayaan2016";

// ThingSpeak channel details
unsigned long channelID = 2704658;
const char* apiKey = "32MSZR5SR9CQH0WA";

WiFiClient client;

Servo barrierServo;  // Servo motor for the entry/exit barrier
//LiquidCrystal_I2C lcd(0x27, 16, 2); // 20x4 LCD display

// Pin Definitions
#define ir_car1 D1  // Parking slot 1 sensor
#define ir_car2 D2  // Parking slot 2 sensor
#define ir_car3 D3  // Parking slot 3 sensor
#define ir_car4 D4  // Parking slot 4 sensor
#define ir_car5 D5  // Parking slot 5 sensor
#define ir_car6 D6  // Parking slot 6 sensor
#define ir_entry D7 // IR sensor at the entry
#define ir_exit D8  // IR sensor at the exit
#define servoPin D0 //servo motor at entry /exit

int S1 = 0, S2 = 0, S3 = 0, S4 = 0, S5 = 0, S6 = 0;
int slot = 6;   // Total parking slots available
int flagEntry = 0, flagExit = 0;
unsigned long previousMillis = 0;
bool showFirstThreeSlots = true;

void setup() {
    Serial.begin(115200);
    //lcd.begin(16, 2);
    //lcd.backlight();

    // Initialize IR sensor pins as input
    pinMode(ir_car1, INPUT);
    pinMode(ir_car2, INPUT);
    pinMode(ir_car3, INPUT);
    pinMode(ir_car4, INPUT);
    pinMode(ir_car5, INPUT);
    pinMode(ir_car6, INPUT);
    pinMode(ir_entry, INPUT);
    pinMode(ir_exit, INPUT);

    // Servo Setup
    barrierServo.attach(servoPin);  // Single servo motor for both entry and exit barrier
    barrierServo.write(90);   // Initially, barrier is closed (90 degrees)
    //Wire.begin(D2,D1);
    
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi");

    // Initialize ThingSpeak
    ThingSpeak.begin(client);
}

void Read_Sensor() {
    // Read the status of each parking slot sensor
    S1 = digitalRead(ir_car1) == LOW ? 1 : 0;
    S2 = digitalRead(ir_car2) == LOW ? 1 : 0;
    S3 = digitalRead(ir_car3) == LOW ? 1 : 0;
    S4 = digitalRead(ir_car4) == LOW ? 1 : 0;
    S5 = digitalRead(ir_car5) == LOW ? 1 : 0;
    S6 = digitalRead(ir_car6) == LOW ? 1 : 0;

    int total = S1 + S2 + S3 + S4 + S5 + S6;
    slot = 6 - total; // Calculate available slots
}

void ServoControl() {
    // Entry gate control
    if (digitalRead(ir_entry) == LOW && flagEntry == 0) {
        if (slot > 0) { // Only open if parking is available
            flagEntry = 1;
            barrierServo.write(0);  // Open the barrier
            delay(3000);            // Wait for the car to pass through
            barrierServo.write(90); // Close the barrier
            slot--;                 // Decrease available slots
        } //else {
           // lcd.setCursor(0, 0);
           // lcd.print("Parking Full");
           // delay(2000);
           // lcd.clear();
       // }
    }

    // Exit gate control
    if (digitalRead(ir_exit) == LOW && flagExit == 0) {
        flagExit = 1;
        barrierServo.write(0);  // Open the barrier
        delay(3000);            // Wait for the car to pass through
        barrierServo.write(90); // Close the barrier
        slot++;                 // Increase available slots
    }

    // Reset the flags
    if (flagEntry == 1) flagEntry = 0;
    if (flagExit == 1) flagExit = 0;
}

void SendDataToThingSpeak() {
    // Set the field data in ThingSpeak
    ThingSpeak.setField(1, S1);
    ThingSpeak.setField(2, S2);
    ThingSpeak.setField(3, S3);
    ThingSpeak.setField(4, S4);
    ThingSpeak.setField(5, S5);
    ThingSpeak.setField(6, S6);
    ThingSpeak.setField(7, slot); // Available slots

    // Write the data to ThingSpeak
    int responseCode = ThingSpeak.writeFields(channelID, apiKey);
    if (responseCode == 200) {
        Serial.println("Data sent to ThingSpeak successfully!");
    } else {
        Serial.println("Failed to send data to ThingSpeak, response code: " + String(responseCode));
    }
}

//void DisplaySlots() {
    //unsigned long currentMillis = millis();
   // if (currentMillis - previousMillis >= 5000) {  // Switch every 5 seconds
       // previousMillis = currentMillis;
        //showFirstThreeSlots = !showFirstThreeSlots; // Toggle display
    //}

   // lcd.clear();
   // lcd.setCursor(0, 0);
   // lcd.print("Slots Avail: ");
   // lcd.print(slot);

    //if (showFirstThreeSlots) {
        // Display Slots 1, 2, and 3
        //lcd.setCursor(0, 1);
        //lcd.print("S1:");
        //lcd.print(S1 ? "F " : "E ");
        //lcd.print("S2:");
        //lcd.print(S2 ? "F " : "E ");
        //lcd.setCursor(8, 1);
        //lcd.print("S3:");
       // lcd.print(S3 ? "F " : "E ");
   // } else {
        // Display Slots 4, 5, and 6
        //lcd.setCursor(0, 1);
        //lcd.print("S4:");
        //lcd.print(S4 ? "F " : "E ");
        //lcd.print("S5:");
        //lcd.print(S5 ? "F " : "E ");
       // lcd.setCursor(8, 1);
       // lcd.print("S6:");
       // lcd.print(S6 ? "F " : "E ");
   // }
//}

void loop() {
    // Read sensor data
    Read_Sensor();

    // Control the servo motor for both entry and exit
    ServoControl();

    // Display parking slot information on the LCD
    //DisplaySlots();

    // Send data to ThingSpeak every 15 seconds
    SendDataToThingSpeak();
    delay(15000); // 15 seconds delay
}