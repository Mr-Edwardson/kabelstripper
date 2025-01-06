/*
 Name:		project_kabelstripper.ino
 Created:	12/13/2024 10:52:35 AM
 Author:	daanw 518080
*/

#define EN_M1    A5 //enable 
#define STEP_M1  3 //step
#define DIR_M1   2 //direction

#define EN_M2    A4 //enable 
#define STEP_M2  5 //step
#define DIR_M2   4 //direction

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
const byte address[][6] = {"00001","00002"};

void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(EN_M1, OUTPUT); // set the EN_PIN as an output
    digitalWrite(EN_M1, HIGH); // deactivate driver (LOW active)
    pinMode(DIR_M1, OUTPUT); // set the DIR_PIN as an output
    digitalWrite(DIR_M1, LOW); // set the direction pin to low
    pinMode(STEP_M1, OUTPUT); // set the STEP_PIN as an output
    digitalWrite(STEP_M1, LOW); // set the step pin to low

    digitalWrite(EN_M1, LOW); // activate driver

    pinMode(EN_M2, OUTPUT); // set the EN_PIN as an output
    digitalWrite(EN_M2, HIGH); // deactivate driver (LOW active)
    pinMode(DIR_M2, OUTPUT); // set the DIR_PIN as an output
    digitalWrite(DIR_M2, LOW); // set the direction pin to low
    pinMode(STEP_M2, OUTPUT); // set the STEP_PIN as an output
    digitalWrite(STEP_M2, LOW); // set the step pin to low

    digitalWrite(EN_M2, LOW); // activate driver

    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    pinMode(A2, INPUT_PULLUP);
    pinMode(A3, INPUT_PULLUP);

    radio.begin();
    radio.openWritingPipe(address[1]); // 00001
    radio.openReadingPipe(1, address[0]); // 00002
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();

}


//Actuatoren en indicatoren
void LCD(String a = "empty");
void M1(double a);
void M2(int a);
void M3(double a); //send

//struct data pakket van radiocommunicatie
struct Data_Package {
    int p;//protecol
    int M3;
    int I1;
    bool S6;
};

Data_Package data;

//Sensoren
int I1() {

    if (radio.available()) {
        radio.read(&data, sizeof(Data_Package));
        return data.I1;
    }
    else { return -1; }
};

bool S2() { return digitalRead(A0); };
bool S3() { return digitalRead(A1); };
bool S5() { return digitalRead(A2); };

bool S6() {
    if (radio.available()) {
        radio.read(&data, sizeof(Data_Package));
        return data.S6;
    }
    else { return -1; }
};

//maximale waardes
int kalibratielengtemax = 5;
int maxlengtekabelstrippen = 3000;


// the loop function runs over and over again forever
void loop() {

    //Stadia Boot
    LCD("Power on"); //Status aangeven

    int stadia = 0;

        while (S5() == true) { //terwijl startknop ingedrukt is...
            if (S2() == true) { // wordt er gecontroleerd of er een kabel aanwezig is
                stadia = 1; //Dan kan het door naar volgende stadia mist vorige 2 waar bleken
            }
            else {  //als de kabel niet is gedetecteerd maar wel op start is gedrukt
                LCD("Cabel not correctly attached");
                throw 1; //gooi foutmelding gezien de kabel er niet of nietgoed in zit
            }
        }

        int afgelegtaxiaal = 0;                   //aanmaken maximale check

        //Stadia Kalibratie
        if (stadia == 1) {
            LCD("Calibration");                   //Status aangeven

            //hub vooraan zetten
            M3(-2);                               //Mes stijgt maximaal
            while (S3 == false) { M2(-1); }       //zet de hub naar het begin
            M2(1);                                //haal hub los van de nul knop

            int afgelegtrond = 0;                 //aanmaken maximale check 


            //einde van witte gedeelte zoeken
            while (I1() > 1) {                                                  //zolang er wit wordt gedetecteerd
                M1(2);                                                          //Hub 2 graden laten draaien
                afgelegtrond += 2;                                              //bijhouden hoeveel de hub heeft gedraait
                if (afgelegtrond == 360) {                                      //als de hub 1 rondje heeft afgelegt
                    M2(+1);                                                     //schuif 1 mesbreedte op
                    afgelegtrond = 0;                                           //reset de waarde voor bijhouden rondje
                    afgelegtaxiaal + 1;                                         //bijhouden afgelegde axiale afstand
                    if (afgelegtaxiaal > kalibratielengtemax) { throw 2; }      //bij teveel afgelegde afstand foutmelding gooien                              
                }
            }
            if (afgelegtrond != 360 || afgelegtrond != 0) { M1(360 - afgelegtrond); } //rondje afdraaien

            stadia = 2;
        }

        int grovediepte = 0;
        if (stadia == 2) {                              //grove dieptebepaling
            LCD("Dethp determining...");                //Status aangeven

            bool wit = 0;                               //maakt wit bool aan en zet hem op 0
            while (!wit) {                              //kijkt of afgelopen rondje volledig wit was
                M3(1);                                  // zet het mes dieper
                grovediepte++;
                for (int i = 0; i < 180; i++) {         //herhaald stukje code 180x
                    wit = 1;                            //zet wit bool op 1
                    if (I1() < 1) { wit = 0; }          //als zwart wordt gedetecteerd zet hij 
                    M2(2);                              //laat het mes 2 graden draaien
                }
            }
            M3(-2);
            grovediepte = grovediepte - 2;

            stadia = 3;
        }

        if (stadia == 3) {
            LCD("Dethp determend, cutting...");

            M2(maxlengtekabelstrippen);

            //hub vooraan zetten
            M3(-2);                               //Mes stijgt maximaal
            while (S3 == false) { M2(-1); }       //zet de hub naar het begin
            M2(afgelegtaxiaal);                   //haal hub los van de nul knop

            stadia = 4;
        }

        if (stadia == 4) {
            LCD("presice stripping");
            int afstand = 0;

            for (int i = 0; i < maxlengtekabelstrippen; i++) {

                bool wit = 0;                               //maakt wit bool aan en zet hem op 0

                while (!wit) {                              //kijkt of afgelopen rondje volledig wit was
                    M3(1);                                  // zet het mes dieper
                    for (int i = 0; i < 180; i++) {         //herhaald stukje code 180x
                        wit = 1;                            //zet wit bool op 1
                        if (I1() < 1) { wit = 0; }          //als zwart wordt gedetecteerd zet hij wit op 0
                        M2(2);                              //laat het mes 2 graden draaien
                    }
                }
                M3(-3);
                M2(1);
            }

            //hub vooraan zetten
            M3(-2);                               //Mes stijgt maximaal
            while (S3 == false) { M2(-1); }       //zet de hub naar het begin
            M2(1);                                //haal hub los van de nul knop

            LCD("Done");
        }

        
}

//rondraaien
void M1(double steps) {
    steps = steps * 1000;
    // Set the motor direction
    if (steps < 0) { digitalWrite(DIR_M1, 1); }
    else { digitalWrite(DIR_M1, 0); }

    // Step the motor
    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_M1, HIGH);
        delayMicroseconds(90);
        digitalWrite(STEP_M1, LOW);
        delayMicroseconds(90);
    }
}

//liniair bewegen
void M2(int steps) {
    steps = steps * 500;
    // Set the motor direction
    if (steps < 0) { digitalWrite(DIR_M2, 1); }
    else { digitalWrite(DIR_M2, 0); }

    // Step the motor
    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_M2, HIGH);
        delayMicroseconds(90);
        digitalWrite(STEP_M2, LOW);
        delayMicroseconds(90);
    }
}

void M3(double a) {

    data.M3 = a;
    radio.write(&data, sizeof(Data_Package));
    delay(500);
    data.M3 = 0;
    radio.write(&data, sizeof(Data_Package));
}



