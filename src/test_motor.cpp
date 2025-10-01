// Test básico del motor sin AccelStepper
#include <Arduino.h>

// Pines del motor
#define MOTOR_PIN1 2
#define MOTOR_PIN2 3
#define MOTOR_PIN3 4
#define MOTOR_PIN4 10

// Secuencia de pasos para el 28BYJ-48
int stepSequence[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

void testMotorDirect() {
    Serial.println("[TEST] Iniciando test directo del motor");

    // Configurar pines como salida
    pinMode(MOTOR_PIN1, OUTPUT);
    pinMode(MOTOR_PIN2, OUTPUT);
    pinMode(MOTOR_PIN3, OUTPUT);
    pinMode(MOTOR_PIN4, OUTPUT);

    // Hacer 100 pasos
    for(int i = 0; i < 100; i++) {
        int step = i % 8;

        digitalWrite(MOTOR_PIN1, stepSequence[step][0]);
        digitalWrite(MOTOR_PIN2, stepSequence[step][1]);
        digitalWrite(MOTOR_PIN3, stepSequence[step][2]);
        digitalWrite(MOTOR_PIN4, stepSequence[step][3]);

        // Los LEDs del ULN2003 deberían encenderse con este patrón
        if(i % 10 == 0) {
            Serial.print("[TEST] Step ");
            Serial.print(i);
            Serial.print(" - LEDs: ");
            Serial.print(stepSequence[step][0]);
            Serial.print(stepSequence[step][1]);
            Serial.print(stepSequence[step][2]);
            Serial.println(stepSequence[step][3]);
        }

        delay(10); // 10ms entre pasos
    }

    // Apagar todos los pines
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);

    Serial.println("[TEST] Test directo completado");
}