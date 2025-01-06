/*
 Name:		Nano_kabelstripper.ino
 Created:	12/31/2024 2:14:46 PM
 Author:	daanw 518080
*/

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Stepper.h>

#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

#define STEPS 100
Stepper stepper(STEPS, 8, 10, 9, 11);

RF24 radio(7, 8); // CE, CSN
const byte address[][6] = { "00001","00002" };

void setup() {
    radio.begin();
    radio.openWritingPipe(address[1]); // 00001
    radio.openReadingPipe(1, address[0]); // 00002
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();

    pinMode(A0, INPUT_PULLUP);

    // Set S0 - S3 as outputs
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);

    // Set Sensor output as input
    pinMode(sensorOut, INPUT);

    // Set Frequency scaling to 20%
    digitalWrite(S0, HIGH);
    digitalWrite(S1, LOW);
}

// Calibration Values   
int redMin = 30; // Red minimum value
int redMax = 350; // Red maximum value
int greenMin = 30; // Green minimum value
int greenMax = 350; // Green maximum value
int blueMin = 30; // Blue minimum value
int blueMax = 350; // Blue maximum value

// Variables for Color Pulse Width Measurements
int redPW = 0;
int greenPW = 0;
int bluePW = 0;

// Variables for final Color values
int redValue;
int greenValue;
int blueValue;

//struct data pakket van radiocommunicatie
struct Data_Package {
    int p;//protecol
    int M3;
    int I1;
    bool S6;
};

Data_Package data;

// the loop function runs over and over again until power down or reset
void loop() {
    radio.read(&data, sizeof(Data_Package));

    double wit = 0;
    int kleur = getBluePW() + getGreenPW() + getRedPW();
    if (kleur >= 600) { wit = 1; }
    else { wit = 0; }
    data.I1 = wit;
    radio.write(&data, sizeof(Data_Package));

    if (data.M3 >= 0) {
        stepper.setSpeed(100);
        stepper.step(-4000);
        delay(1000*data.M3);
    }

    double wit = 0;
    int kleur = getBluePW() + getGreenPW() + getRedPW();
    if (kleur >= 600) { wit = 1; }
    else { wit = 0; }
    data.I1 = wit;
    radio.write(&data, sizeof(Data_Package));

    if (data.M3 <= 0) {
        stepper.setSpeed(100);
        stepper.step(4000);
        delay(-1000 * data.M3);
    }

    double wit = 0;
    int kleur = getBluePW() + getGreenPW() + getRedPW();
    if (kleur >= 600) { wit = 1; }
    else { wit = 0; }
    data.I1 = wit;
    radio.write(&data, sizeof(Data_Package));

}


// Function to read Red Pulse Widths
int getRedPW() {
    // Set sensor to read Red only
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    // Define integer to represent Pulse Width
    int PW;
    // Read the output Pulse Width
    PW = pulseIn(sensorOut, LOW);
    // Return the value
    return PW;
}

// Function to read Green Pulse Widths
int getGreenPW() {
    // Set sensor to read Green only
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    // Define integer to represent Pulse Width
    int PW;
    // Read the output Pulse Width
    PW = pulseIn(sensorOut, LOW);
    // Return the value
    return PW;
}

// Function to read Blue Pulse Widths
int getBluePW() {
    // Set sensor to read Blue only
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    // Define integer to represent Pulse Width
    int PW;
    // Read the output Pulse Width
    PW = pulseIn(sensorOut, LOW);
    // Return the value
    return PW;
}
