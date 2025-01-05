#include <Servo.h>
#include <Keypad.h>
#include <DHT.h>

Servo servo; //initialize the servo motor object

//defining the pins for components
#define DHTTYPE DHT11
#define tempPin 11
#define enablePin 9
#define RGBred A0
#define RGBblue 12
#define RGBgreen 13
#define buzzerPin A5
#define LEDPin A2
#define LDRPin A1
#define PIRPin A4
#define mServo A3

//keypad identification
const byte NUMrows = 4;
const byte NUMcols = 4;
char keyMap[NUMrows][NUMcols] =
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[NUMrows] = {10, 8, 7, 6};
byte colPins[NUMcols] = {5, 4, 3, 2};

//initialize the keypad class
Keypad mKeypad = Keypad(makeKeymap(keyMap), rowPins, colPins, NUMrows, NUMcols);

//variables for sensor readings
int LDRread = 0;
int PIRread;
float celsius;

//variables for password and commands
int authenticated = 0;
char key;
String inputPassword = "";
char lastCommand = '0';
bool commandReceived = false; //flag for receiving commands from the CV code

//initialize dht sensor
DHT dht(tempPin, DHTTYPE);


void setup()
{
    Serial.begin(9600);
   // Serial.setTimeout(100);

    //setup I/O pins
    pinMode(RGBred, OUTPUT);
    pinMode(RGBblue, OUTPUT);
    pinMode(RGBgreen, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(LEDPin, OUTPUT);
    pinMode(PIRPin, INPUT);
    pinMode(tempPin, INPUT);
    pinMode(enablePin, OUTPUT);

    //attach the servo motor to its pin
    servo.attach(mServo);
    servo.write(0); //set the motor to closed position
    dht.begin();
}

void loop()
{
    //serial and keypad input handling
    handleCommands();
    if(lastCommand=='3')
    {
        handleKeypad();
    }

    //sensor readings and control systems while the face is recognised and/or the password is correctly entered
    if (authenticated)
    {
        manageSensors();
    }
}

//function to handle commands
void handleCommands()
{
    if (Serial.available())
    {
      char incomingChar=Serial.read();
        if(incomingChar!='\n'&&incomingChar!='\r')
          {
        lastCommand = incomingChar;
        commandReceived = true;
          }
        
    }

    if (commandReceived)
    {
        processCommand();
        commandReceived = false; //reset the flag
    }
}

//function to perform actions according to commands
void processCommand()
{
    switch (lastCommand)
    {
    case'0':
            wrongPassword();
        authenticated=0;
        break;
    case '1':  //open the door
        openDoor();
        authenticated = 1;
        break;
    case '4':  //send sensor readings
        if (authenticated)
        {
            sensorsReadings();
        }
        else
        {
          	authenticated=1;
          	manageSensors();
            sensorsReadings();
        }
        break;
    case '2':  //disable sensors
        disableSensors();
        break;
    case '3':  //enter password
        Serial.println("Enter the password on the keypad.");
        break;
    case '5':
    digitalWrite(enablePin,LOW);
  Serial.println("trun off fan");
        break;
    case '6':
        digitalWrite(enablePin,HIGH);
  Serial.println("trun on fan");
        break;
    default:
        Serial.println("Invalid command.");
        break;
    }
}

//function to handle keypad input
void handleKeypad()
{
    key = mKeypad.getKey();
    if (key)
    {
        //if number is pressed, add it to the string
        if (key >= '0' && key <= '9')
        {
            inputPassword += key;
            Serial.print("Key Pressed: ");
            Serial.println(key);
        }

        //if 'A' is pressed, check the password through sending it to the CV code and recieve the response
        if (key == 'A')
        {
            Serial.print("Password Entered: ");
            Serial.println(inputPassword);
            inputPassword = "";  // Clear input after checking
        }

        //if '*' is pressed, clear the input
        if (key == '*')
        {
            inputPassword = "";
            Serial.println("Password cleared.");
        }
    }
}

//function to manage sensors
void manageSensors()
{
    //light system
    int ldr = analogRead(LDRPin);
    if (ldr > 500)
    {
        digitalWrite(LEDPin,HIGH);
    }
    else
    {
        digitalWrite(LEDPin, LOW);
    }
     float celsius=dht.readTemperature();
   
    if (celsius < 20)
    {
      
       // analogWrite(enablePin, 200); //turn off fan
        analogWrite(RGBblue, 255); //blue light on
        analogWrite(RGBred, 0);
        analogWrite(RGBgreen, 0);
    }
    else if (celsius >= 20 && celsius <= 30)
    {
      
       // analogWrite(enablePin, 150); //adjust fan speed
        analogWrite(RGBgreen, 255); //green light on
        analogWrite(RGBblue, 0);
        analogWrite(RGBred, 0);
    }
    else
    {
      
       // analogWrite(enablePin, 255); //maximum fan speed
        analogWrite(RGBred, 255); //red light on
        analogWrite(RGBblue, 0);
        analogWrite(RGBgreen, 0);
    }

    //PIR sensor for motion detection
    PIRread = digitalRead(PIRPin);
    if (PIRread == HIGH)
    {
        Serial.println("Someone Inside The House");
        delay(1000);
    }
}

//function to open the door


void openDoor()
{
    servo.write(180);  //open door
    delay(2000);
    servo.write(0);   //close door
    Serial.println("Door is open.");
}


//function to disable all the sensors
void disableSensors()
{
    digitalWrite(LEDPin, LOW);
    //analogWrite(enablePin, 0);
    analogWrite(RGBred, 0);
    analogWrite(RGBgreen, 0);
    analogWrite(RGBblue, 0);
    digitalWrite(buzzerPin, LOW);
    authenticated=0;  //to prevent the loop from opening the sensor again before being notified
}

//function to read sensor data and send to GUI
void sensorsReadings()
{
    int ldr = analogRead(LDRPin);
    celsius=dht.readTemperature();
    int pir = digitalRead(PIRPin);
    String data = "LDR:" + String(ldr) + ",Temp:" + String(celsius) + ",PIR:" + String(pir);
    //create and send formatted sensor data to be displayed on the GUI
   
    Serial.println(data);
}

//function to handle wrong passwords and send warning
void wrongPassword()
{
    digitalWrite(buzzerPin, HIGH);
    delay(3000);
    digitalWrite(buzzerPin, LOW);
    Serial.println("Wrong Password!");
}
void turnOnFan()
{
  digitalWrite(enablePin,HIGH);
  Serial.println("trun on fan");
}
