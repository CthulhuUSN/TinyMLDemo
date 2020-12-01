#include "output_handler.h"
#include "Arduino.h"

// Track whether the function has run at least once
bool initialized = false;

// Animates a dot across the screen to represent the current x and y values
void HandleOutput(tflite::ErrorReporter* error_reporter, float y_value) {
  // Do this only once
  if (!initialized) {
    pinMode(14, OUTPUT); //Green LED
    pinMode(16, OUTPUT); //Red LED  
    }
    initialized = true;

  // Set the output light based on the results of the inference
  if(y_value >= 0.5){
    digitalWrite(16, HIGH);
    Serial.println("Attack Detected!");
    delay(2000);
    digitalWrite(16, LOW);
  } else {
    digitalWrite(14, HIGH);
    Serial.println("All Clear.");
    delay(2000);
    digitalWrite(14, LOW);
  }

  // Log the current y output value for display in the Arduino plotter
  error_reporter->Report("%d", y_value);
}
