#include <Servo.h>
#include <Ultrasonic.h>
#include "pitches.h"
#include <Wire.h>
#include "rgb_lcd.h"
/////////////////////////////////////////////////////////////////////////////

#define PLEIN_POUBELLE  5
#define ERREUR_POUBELLE_PLEINE 7

////////////////////////////////////////////////////////////////////////////


/////////////////////////// DEBUT DEFINTION CLASSES ////////////////

class UltrasonicSensor {
  private:
    Ultrasonic ultrasonic;  
    int distance;           // Variable pour stocker la distance
    Ultrasonic *pointer;

  public:
    UltrasonicSensor* operator->() {
        return this; //redéfintion de l'opérateur d'accès aux éléments de la classe UltrasonicSensor
    }

    UltrasonicSensor(int trigPin) : ultrasonic(trigPin) {} // Constructeur pour initialiser le capteur 

    int getDistance() { // Méthode pour lire la distance en cm
      distance = ultrasonic.read();  // Lire la distance en centimètres
      return distance;
    }
};


class BinUltrasonicSensor : public UltrasonicSensor { // Classe dérivée pour le capteur ultrason de la poubelle
  public:
    BinUltrasonicSensor(int trig) : UltrasonicSensor(trig) {}
    long dist = getDistance();
};


class ServoControl { // Classe pour la gestion du servo moteur
  private:
    Servo myServo;
    int pin;
    
  public:
    ServoControl(int pin) : pin(pin) {
      myServo.attach(pin);
    }

    void open() {
      myServo.write(0);  // Ouvre à 90°
    }

    void close() {
      myServo.write(180);  // Ferme à 0°
    }
};


class Melody {
  private:
    int pin; // Pin auquel le buzzer est connecté
    int melody[3] = {NOTE_C4, NOTE_G3, NOTE_A3};
    int noteDurations[3] = {10, 10, 10};

  public:
    
    Melody(int pin) {  // Constructeur pour initialiser le buzzer à un pin spécifique
      this->pin = pin;
      pinMode(pin, OUTPUT);  // Définir le pin comme une sortie
    }
    void playMelody() {

      for (int thisNote = 0; thisNote < 3; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote]; // Calculer la durée de chaque note (en ms)
        tone(pin, melody[thisNote], noteDuration); // Jouer la note
        int pauseBetweenNotes = noteDuration * 1.30; // Pause entre les notes, avec un peu de délai pour éviter que les notes se chevauchent
        delay(pauseBetweenNotes);
        noTone(pin);// Arrêter le son de la note

    }
  }
};



class Button {
private:
    int pin;
    int buttonState;
    int lastButtonState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;
public:
    Button(int pin) : pin(pin), buttonState(0), lastButtonState(0), lastDebounceTime(0), debounceDelay(50) {
        pinMode(pin, HIGH);
    }

    bool isPressed() {
        unsigned long currentTime = millis();
        buttonState = digitalRead(pin);
        if (buttonState == LOW && lastButtonState == HIGH && (currentTime - lastDebounceTime) > debounceDelay) {
            lastDebounceTime = currentTime;
            lastButtonState = buttonState;
            return true;
        }
        lastButtonState = buttonState;
        return false;
    }
};

//////////////////////////////// FIN DEFINITON CLASSES /////////////




/////////////////////////////////////////// DECLARATION OBJETS ET PINs /////////////////
Button button(D3); //Bouton pour le changement de couleur
UltrasonicSensor Sensor(D6); // Capteur de mouvement 
BinUltrasonicSensor binSensor(D5); // Capteur ultrason pour l'intétieur de la poubelle 
ServoControl servo(D8); // Servo sur le pin 9
Melody melodyPlayer(D7);
//////////////////////////////////////////// FIN DECLARATION OBJETS ET PINs




//////////////////////////////////// DECLARATION VARIABLES POUR COULEURS LCD/////////////////////

// déclarations pour la gestion des couleurs LCD
rgb_lcd lcd;
int colors[6][3] = { // tableau qui contient les couleurs
  {255, 0, 0},    // Rouge
  {0, 255, 0},    // Vert
  {0, 0, 255},    // Bleu
  {255,255,0},    // ...
  {0,255,255},
  {255,0,255},     
};

int currentColorIndex_ouverture = 0;
int currentColorIndex_plein = 0;
int currentColorIndex_fermeture = 0;

int colorR ;
int colorG ;
int colorB ;


int color1; // pour ouverture
int color2;
int color3;

int color4; // pour plein
int color5;
int color6;

int color7; // pour fermeture
int color8;
int color9;

int color_ouverture_changed = false;
int color_fermeture_changed = false;
int color_plein_changed = false;


int distance;
int distance_plein;
//////////////////////////////////// FIN DÉCLARATION VARIABLES POUR COULEURS LCD /////////////////////




void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  //lcd.setRGB(colorR, colorG, colorB);
}



void loop() {

  lcd.setCursor(0, 0);
  distance = Sensor.operator->()->getDistance(); //utilisation de la Surcharge de l'opérateur -> dans la classe UltrasonicSensor !
  
  if (distance < 10 ) {
    try {
      distance_plein = binSensor.operator->()->getDistance(); //utilisation de la Surcharge de l'opérateur -> dans la classe UltrasonicSensor !
      if (distance_plein < PLEIN_POUBELLE) { // si la distance entre les ordures et le capteur est <10, la poubelle est pleine
        throw ERREUR_POUBELLE_PLEINE; // Lancer une exception personnalisée si la poubelle est pleine
    }
      
      else{
          /////////////////////////// cas ouverture //////////////////////
          if (color_ouverture_changed == false) {
            lcd.setRGB(0, 255, 0);
          }
          
          else {
            lcd.setRGB(color1,color2,color3);
          }

          lcd.print(" Ouverture !    ");           
          melodyPlayer.playMelody();                  
          servo.open();

          
          while( distance < 10 ){   // tant que je suis devant la poubelle, la poubelle reste ouverte !
            distance = Sensor.operator->()->getDistance();
            servo.open();             
            if (button.isPressed()) {
              color_ouverture_changed = true ;
              currentColorIndex_ouverture = (currentColorIndex_ouverture + 1) % 6;  // Passer à l'index suivant, en revenant à 0 après 5 (le reste de la divison par 6 peut aller de 0 à 5)
              lcd.setRGB(colors[currentColorIndex_ouverture][0], colors[currentColorIndex_ouverture][1], colors[currentColorIndex_ouverture][2]);// Change la couleur à chaque appuie sur le bouton
            }
          }
          color1 = colors[currentColorIndex_ouverture][0]; // Sauvegarder la nouvelle couleur après changement
          color2 = colors[currentColorIndex_ouverture][1];
          color3 = colors[currentColorIndex_ouverture][2];
          lcd.clear();
      }
    }

        catch (int a) {
          ///////////////////////////////// cas poubelle pleine //////////////
          if (color_plein_changed == false) {
            lcd.setRGB(255, 255, 0);
          }
          else {
            lcd.setRGB(color4,color5,color6);
          }
          
          if (button.isPressed()) {
              color_plein_changed = true;
              currentColorIndex_plein = (currentColorIndex_plein + 1) % 6;
              lcd.setRGB(colors[currentColorIndex_plein][0], colors[currentColorIndex_plein][1], colors[currentColorIndex_plein][2]);// Change la couleur à chaque appuie sur le bouton
          }
                   
          color4 = colors[currentColorIndex_plein][0];  // Sauvegarder la nouvelle couleur après changement
          color5 = colors[currentColorIndex_plein][1];
          color6 = colors[currentColorIndex_plein][2];;
      lcd.print(" Poubelle Pleine ");
    }

  }


  else {
      if (color_fermeture_changed == false) {
          lcd.setRGB(255, 0, 0);
        }
        
        else {
          lcd.setRGB(color7,color8,color9);
        }

        lcd.print(" Poubelle fermee  ");                            
        servo.close();



      while( distance > 10 ){   // tant que je suis devant la poubelle, la poubelle reste ouverte !
        Serial.println(distance);
        distance = Sensor.operator->()->getDistance();
        int distance_plein = binSensor.operator->()->getDistance();
        servo.close();
        lcd.setCursor(0, 1); // Positionner le curseur à la deuxième ligne
        if (distance_plein < 20){
          lcd.print("Remplissage: ");
          lcd.print(map(distance_plein, 5, 30, 100, 0)); // Afficher le pourcentage
          lcd.print("%"); // Ajouter le symbole pourcentage
        }
        else{
          lcd.print("Remplissage: 0");
          lcd.print("%"); // Ajouter le symbole pourcentage        
        }      
        if (button.isPressed()) {
          Serial.println("nhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh");
          color_fermeture_changed = true ;
          currentColorIndex_fermeture = (currentColorIndex_fermeture + 1) % 6;  // Passer à l'index suivant, en revenant à 0 après 5 (le reste de la divison par 6 peut aller de 0 à 5)
          lcd.setRGB(colors[currentColorIndex_fermeture][0], colors[currentColorIndex_fermeture][1], colors[currentColorIndex_fermeture][2]);// Change la couleur à chaque appuie sur le bouton
        }
    }
     Serial.println("nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn");
      color7 = colors[currentColorIndex_fermeture][0]; // Sauvegarder la nouvelle couleur après changement
      color8 = colors[currentColorIndex_fermeture][1];
      color9 = colors[currentColorIndex_fermeture][2];

  }

}


