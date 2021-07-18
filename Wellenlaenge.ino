#include <math.h>  //Importieren aller benötigten librarys
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HCSR04.h>
#define enA 10
#define in1 2
#define in2 3
#define enB 11
#define in3 4
#define in4 5
UltraSonicDistanceSensor distanceSensor(6, 9);


LiquidCrystal_I2C lcd(0x3F, 16, 2);
double Mitte = 44.50; //Position Hauptmaximum
double abstand_laser_zu_Sensor = 0.443; //Abstand Gitter zum ersten Maxima
double gitterKonstante = 0.000002; //Gitterkonstante 500 Linien pro mm
int light[] = {0}; //liste Lichtwerte
double distance[] = {0}; //Liste Entfernungswerte
int counter = 0; //Zähler wie viele Maxima gemessen wurden
int j = 0;
int z = 0;
int pos1, pos2, pos3, pos4, pos5, pos6; //Maximal 6 Maxima können gemessen werden
int Max[] = {};
double abstan = 0;



void setup()
{
  pinMode(enA, OUTPUT);  //Pins für Abstandssensor und Lichtsensor
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  // initialize LCD,
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor (0, 0);
  lcd.print("Abstand: "); //gibt den Anfangs Abstandswert aus um zu schauen ob der Sensor richtig misst
  lcd.print(abstand());
  lcd.print("cm");
  analogWrite(enA, 200); // Signal zu den Motoren
  analogWrite(enB, 200);
  delay(3000); //3 Sekunden Warten

}

void loop()
{

  while (abstand() > 6.00) { //Solange Abstand zum Ende größer als 6 cm wird nach Lichtsignalen Gesucht

    while (analogRead(A0) > 100 && (abstand() < Mitte - 4 or abstand() > Mitte + 4) && abstand() > 6.00) { //Wenn der Lichtsensor einen Hohen wert hat und es nicht das Hauptmaximum ist werden Daten Gemessen
      light[counter] = analogRead(A0); //Lichtwerte werden in Liste eingetragen
      distance[counter] = abstand();   //Abstandswerte werden in Liste Eingetragen
      counter = counter + 1;           //Zähler für die Listenposition wird erhöht
    }

    if (counter != j) { //Wenn zähler unglich j ist wurden Daten gemessen (Da  j zu Begin = i war)
                        // z gibt an wie oft schon Daten von einem Lichtintervall gemessen wurden (zu Begin z = 0)
      if (z == 5) {     //wurden z.B. schon 5 mal Daten gemessen:
        z = 6;          // wird z erhöht
        j = counter;
        pos6 = counter; // Die Position in den Listen (distance[] und light[]) wo das 6. Mal Daten Gemessen wurden wird in pos6 gespeichert
      }
      if (z == 4) {
        z = 5;
        j = counter;
        pos5 = counter;
      }
      if (z == 3) {
        z = 4;
        j = counter;
        pos4 = counter;
      }

      if (z == 2) {
        z = 3;
        j = counter;
        pos3 = counter;
      }

      if (z == 1) {
        z = 2;
        j = counter;
        pos2 = counter;
      }
      if (z == 0) {
        z = 1;
        j = counter;
        pos1 = counter;
      }


    }
  } // Ende While Schleife = ABstand zum Ende < 6 cm
  analogWrite(enA, 0); // Motoren aus
  analogWrite(enB, 0);

  if (z != 0) {
    Max[0] = distance[Maximum(0, pos1)]; //Erstes Gemessenes Maximum (Max[0]) bekommte den Abstandswert vom MaximalLichtwert(Maximum()) des ersten Gemessen LichtIntervall (0,pos1)
    if (z > 1) { //wird für die Anzahl an gemessenen Lichtintervalen (=z) wiederholt
      Max[1] = distance[Maximum(pos1 + 1, pos2)];
    }
    if (z > 2) {
      Max[2] = distance[Maximum(pos2 + 1, pos3)];
    }
    if (z > 3) {
      Max[3] = distance[Maximum(pos3 + 1, pos4)];
    }
    if (z > 4) {
      Max[4]  = distance[Maximum(pos4 + 1, pos5)];
    }
    if (z > 5) {
      Max[5] = distance[Maximum(pos5 + 1, pos6)];
    }
  }
  lcd.clear();
  lcd.setCursor (0, 0);
  lcd.print("Wellenlaenge: ");
  lcd.setCursor(0, 1);
  lcd.print((Wellenlaenge(firstMax()) + Wellenlaenge(firstMaxOp())) / 2);
  //Lcd gibt die berechnete Wellenlänge des ersten Maximum aus (Wellenlänge für beide Seiten wird errechnet, addiert und durch 2 geteilt für einen Mittelwert)
  lcd.print("nm");



  delay(500000); //Programm ist vorbei, 500 Sekunden warten
}

double firstMax() {  //Position Erstes Maximum zum Hauptmaximum
  double c = 100;
  for (int k = 0; k < z; k++) { //Geht alle Max[] durch
    if ((abs(Max[k] - Mitte)) < c) { //Findet Maximum welches am Nächsten an der Mitte liegt
      c = abs(Max[k] - Mitte);
    }
  }
  return c; //Gibt  Abstand vom ersten Maximum zum Hauptmaximum wieder
}

double firstMaxOp() { //erstes Maximum andere Seite
  double m = firstMax();
  double c = 0;
  for (int k = 0; k < z; k++) {
    if (abs(Max[k] - Mitte) < c && abs(Max[k] - Mitte != m)) { //Findet Maximum welches Nach dem FirstMax() am nächsten bei der Mitte liegt (= erstes Maximum auf der anderen Seite)
      c = abs(Max[k] - Mitte);
    }
  }
  if (c == 0) { //Wenn kein zweitnächstes Maximum ermittelt werden kann wird das erste Maximum ausgegeben
    c = m;
  }
  return c;
}

double Wellenlaenge(double d) { //berechnet die Wellenlänge mit der aus der Theorie bestimmten Formel
  double h = gitterKonstante * sin(atan(d * 0.01 / abstand_laser_zu_Sensor)) * 1000000000; // x10^9 um Wellenlänge in nm auszugeben
  return h;
}

int Maximum(int p1, int p2) { //Ermittelt den Maximal Lichtwert eines Lichtintervalles [p1,p2]
  int a = 0; // wert des Lichtes maximal
  int b = 0; //position in der Liste
  for (int u = p1; u <= p2; u++) {
    if (light[u] > a) {
      a = light[u];
      b = u;
    }
  }
  return b;

}

double abstand() { //gibt den Abstand zum Ende Wieder
  double m = distanceSensor.measureDistanceCm();  //Abstandssensor misst Abstand zum Ende
  return m;
}
