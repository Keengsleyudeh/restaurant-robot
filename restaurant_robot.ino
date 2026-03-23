/*
  Arduino program for a restaurant robot (Project Group A)
  Features: Continuous sensor polling, non-blocking obstacle avoidance, 
  and reliable intersection detection.
*/

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Arduino.h>

// --- PIN DEFINITIONS ---
#define trigPin 12
#define echoPin 11

int left_track = A0;
int right_track = A1;
int mid_track = A2;
int tray1 = A3;
int battery = A4;
int led = 13;

#define left_back 10
#define left_front 7
#define right_back 8
#define right_front 9

#define enA 6
#define enB 5

// --- GLOBAL VARIABLES ---
int duration, distance;
int left, right, mid;
int tray, battery_state;
char command;

// Speed settings (0 - 255)
const int BASE_SPEED = 160;  // Normal driving speed (reduced to prevent spilling)
const int TURN_SPEED = 200;  // Speed for turning 90/180 degrees

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(left_back, OUTPUT);
  pinMode(right_back, OUTPUT);
  pinMode(left_front, OUTPUT);
  pinMode(right_front, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Mechatronics Eng.");
  lcd.setCursor(0, 1);
  lcd.print("Robot Waiter");
  delay(3000);
  lcd.clear();
}

// ==========================================
// SENSOR FUNCTION
// ==========================================
void updateSensors() {
  // Read Line Sensors (white = 1, black = 0)
  left = digitalRead(left_track);
  mid = digitalRead(mid_track);
  right = digitalRead(right_track);
  
  // Read Tray Sensor
  tray = digitalRead(tray1);

  // Read Ultrasonic Sensor (with timeout to prevent freezing)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 25000); // 25ms timeout
  if (duration == 0) {
    distance = 999; // If no echo, assume path is clear
  } else {
    distance = (duration / 2) / 29.1;
  }
}

// ==========================================
// MOTOR FUNCTIONS
// ==========================================
void stop() {
  digitalWrite(left_front, LOW);
  digitalWrite(right_front, LOW);
  digitalWrite(left_back, LOW);
  digitalWrite(right_back, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

void forward() {
  digitalWrite(left_front, HIGH);
  digitalWrite(right_front, HIGH);
  digitalWrite(left_back, LOW);
  digitalWrite(right_back, LOW);
  analogWrite(enA, BASE_SPEED);
  analogWrite(enB, BASE_SPEED);
}

void backward() {
  digitalWrite(left_front, LOW);
  digitalWrite(right_front, LOW);
  digitalWrite(left_back, HIGH);
  digitalWrite(right_back, HIGH);
  analogWrite(enA, BASE_SPEED);
  analogWrite(enB, BASE_SPEED);
}

// Hard turns for intersections
void turn_left() {
  digitalWrite(left_front, LOW);
  digitalWrite(right_front, HIGH);
  digitalWrite(left_back, HIGH);
  digitalWrite(right_back, LOW);
  analogWrite(enA, TURN_SPEED);
  analogWrite(enB, TURN_SPEED);
  delay(1200); // Adjust this delay to achieve a perfect 90-degree turn
  stop();
}

void turn_right() {
  digitalWrite(left_front, HIGH);
  digitalWrite(right_front, LOW);
  digitalWrite(left_back, LOW);
  digitalWrite(right_back, HIGH);
  analogWrite(enA, TURN_SPEED);
  analogWrite(enB, TURN_SPEED);
  delay(1200); // Adjust this delay to achieve a perfect 90-degree turn
  stop();
}

// Soft turns for line correction while driving
void drift_left() {
  digitalWrite(left_front, LOW);
  digitalWrite(right_front, HIGH);
  digitalWrite(left_back, LOW);
  digitalWrite(right_back, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, BASE_SPEED);
}

void drift_right() {
  digitalWrite(left_front, HIGH);
  digitalWrite(right_front, LOW);
  digitalWrite(left_back, LOW);
  digitalWrite(right_back, LOW);
  analogWrite(enA, BASE_SPEED);
  analogWrite(enB, 0);
}


// ==========================================
// NAVIGATION LOGIC
// ==========================================

// This function drives the robot forward, following the line, 
// until it detects an intersection (all sensors read black).
void followLineToIntersection() {
  delay(500); // Small delay to clear the previous intersection
  
  while (true) {
    updateSensors();

    // 1. Obstacle Avoidance
    if (distance > 0 && distance < 60) {
      stop();
      lcd.setCursor(0, 2);
      lcd.print("Biko kpuo n' uzo"); // "Please move"
      
      // Wait here until the obstacle is removed
      while (distance > 0 && distance < 60) {
        updateSensors();
        delay(100);
      }
      lcd.setCursor(0, 2);
      lcd.print("                "); // Clear the message
    }

    // 2. Intersection Detected (0 = black)
    if (left == 0 && mid == 0 && right == 0) {
      stop();
      break; // Exit the loop, we arrived!
    }

    // 3. Line Following Logic
    if (mid == 0) {
      forward(); // Centered on line
    } else if (left == 0) {
      drift_left(); // Drifting right, correct left
    } else if (right == 0) {
      drift_right(); // Drifting left, correct right
    } else {
      forward(); // All white (1 1 1), keep inching forward to find line
    }
  }
}

// Function to handle waiting at the table
void deliverFood() {
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Enjoy your meal!");
  Serial.println("Waiting for tray to be removed...");
  
  // Wait until the tray sensor changes (assuming 1 = empty, 0 = food)
  // Update this logic based on how your specific tray sensor works
  updateSensors();
  while (tray == 0) {
    updateSensors();
    delay(200);
  }
  
  delay(2000); // Give customer 2 seconds to get hands clear
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Returning to base");
}


// ==========================================
// TABLE ROUTINES
// ==========================================
void table_1() {
  lcd.clear();
  lcd.print("Order to Table 1");

  // Go to main intersection
  followLineToIntersection();
  
  // Turn to table
  turn_left();
  
  // Go to table intersection
  followLineToIntersection();
  
  // Deliver
  deliverFood();
  
  // Turn around (180 degrees)
  turn_left();
  turn_left();
  
  // Return to main intersection
  followLineToIntersection();
  
  // Turn back toward kitchen
  turn_right();
  
  // Drive back to kitchen/start point
  followLineToIntersection();
  
  lcd.clear();
  lcd.print("Ready for order");
}

void table_2() {
  lcd.clear();
  lcd.print("Order to Table 2");

  followLineToIntersection();
  turn_right();
  followLineToIntersection();
  
  deliverFood();
  
  turn_right();
  turn_right();
  followLineToIntersection();
  turn_left();
  followLineToIntersection();
  
  lcd.clear();
  lcd.print("Ready for order");
}

// Duplicate the logic for Table 3 and 4 based on your physical track layout
void table_3() { table_1(); } // Placeholder
void table_4() { table_2(); } // Placeholder


// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  updateSensors();

  // Update Battery Display every loop
  battery_state = analogRead(battery);
  battery_state = map(battery_state, 0, 1023, 0, 100);
  
  lcd.setCursor(0, 0);
  lcd.print("Proj Group A  ");
  lcd.setCursor(14, 0);
  lcd.print("B:");
  lcd.print(battery_state);
  lcd.print("% ");

  // Check for obstacles in idle mode
  if (distance > 0 && distance < 60) {
    stop();
    lcd.setCursor(0, 2);
    lcd.print("Biko kpuo n' uzo");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("                "); // Clear message
  }

  // Bluetooth / Serial Commands
  if (Serial.available() > 0) {
    command = Serial.read();
    
    switch (command) {
      case '1': table_1(); break;
      case '2': table_2(); break;
      case '3': table_3(); break; // Update these once you map the track
      case '4': table_4(); break; // Update these once you map the track
      case 'A': forward(); delay(500); stop(); break; // Manual nudge forward
      case 'B': backward(); delay(500); stop(); break;
      case 'C': turn_left(); break;
      case 'D': turn_right(); break;
      case 'E': stop(); break;
      default: break;
    }
  }
  
  delay(50); // Small loop delay for stability
}