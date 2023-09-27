#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <CustomServo.h>
#include <L298N_ArduinoMega.h>
#include <Adafruit_NeoPixel.h>
#include <Lighting.h>

#define DHTTYPE DHT22 // DHT22 sensor

namespace Pins
{
  constexpr int LED_STRIP_FRONT = 8;
  constexpr int LED_STRIP_BACK = 7;
  constexpr int DHTPIN = 25;

  constexpr int REAR_ENA = 29;
  constexpr int REAR_IN1 = 31;
  constexpr int REAR_IN2 = 33;
  constexpr int REAR_IN3 = 35;
  constexpr int REAR_IN4 = 37;
  constexpr int REAR_ENB = 39;

  constexpr int FRONT_ENA = 28;
  constexpr int FRONT_IN1 = 30;
  constexpr int FRONT_IN2 = 32;
  constexpr int FRONT_IN3 = 34;
  constexpr int FRONT_IN4 = 36;
  constexpr int FRONT_ENB = 38;

  constexpr int CAM_YAW_SERVO = 2;
  constexpr int CAM_PITCH_SERVO = 3;
}

L298N rearMotors(Pins::REAR_ENA, Pins::REAR_IN1, Pins::REAR_IN2, Pins::REAR_ENB, Pins::REAR_IN3, Pins::REAR_IN4);
L298N frontMotors(Pins::FRONT_ENA, Pins::FRONT_IN1, Pins::FRONT_IN2, Pins::FRONT_ENB, Pins::FRONT_IN3, Pins::FRONT_IN4);

CustomServo camPitchServo, camYawServo;

Lighting lights = Lighting(Pins::LED_STRIP_FRONT, Pins::LED_STRIP_BACK);

DHT dht(Pins::DHTPIN, DHTTYPE);

void handleDHT22()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if any readings failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("humidity:");
  Serial.print(humidity);
  Serial.print("%|");
  Serial.print("temperature:");
  Serial.print(temperature);
  Serial.println("c");
}

void servoControl(String buffer)
{
  int separatorIndex = buffer.indexOf(":");
  String command = buffer.substring(separatorIndex + 1);

  separatorIndex = command.indexOf(":");
  float angle = command.substring(separatorIndex + 1).toFloat();
  command = command.substring(0, separatorIndex);

  Serial.println(command);
  Serial.println(angle);

  if (command == "cam_pitch")
  {
    Serial.println("ACK");
    camPitchServo.ChangeServoAngleLinear(angle);
  }
  else if (command == "cam_yaw")
  {
    Serial.println("ACK");
    camYawServo.ChangeServoAngleLinear(angle);
  }
}

void motorControl(String buffer)
{
  int separatorIndex = buffer.indexOf(":");
  String command = buffer.substring(separatorIndex + 1);
  Serial.println(command);
  if (command == "forward")
  {
    Serial.println("ACK MOTOR");
    rearMotors.moveForward();
    frontMotors.moveForward();
  }
  else if (command == "backward")
  {
    Serial.println("ACK MOTOR");
    frontMotors.moveBackward();
    rearMotors.moveBackward();
  }
  else if (command == "left")
  {
    Serial.println("ACK MOTOR");
    rearMotors.moveLeft();
    frontMotors.moveLeft();
  }
  else if (command == "right")
  {
    Serial.println("ACK MOTOR");
    rearMotors.moveRight();
    frontMotors.moveRight();
  }
  else if (command == "stop")
  {
    Serial.println("ACK MOTOR");
    rearMotors.stopMotors();
    frontMotors.stopMotors();
  }
  else if (command.startsWith("speed"))
  {
    int separatorIndex = command.indexOf(":");
    int speed = command.substring(separatorIndex + 1).toInt();
    Serial.println("ACK MOTOR");
    rearMotors.setSpeed(speed);
    frontMotors.setSpeed(speed);
  }
}

void setup()
{

  Serial.begin(115200);

  delay(2000);

  camPitchServo.Setup(Pins::CAM_PITCH_SERVO, 90.0);
  camYawServo.Setup(Pins::CAM_YAW_SERVO, 90.0);

  dht.begin();

  lights.startupAnimation();
}

void loop()
{
  if (Serial.available() > 0)
  {
    String buffer = Serial.readStringUntil('\n');
    if (buffer.startsWith("motor"))
    {
      motorControl(buffer);
    }
    else if (buffer.startsWith("servo"))
    {
      servoControl(buffer);
    }
    else if (buffer.startsWith("lights"))
    {
      lights.control(buffer);
    }
    else if (buffer.startsWith("dht:read"))
    {
      handleDHT22();
    }
  }

  lights.update();
}
