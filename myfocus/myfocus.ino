/********************************************
/ Arduino/ULN2003/28BYJ48 based moto focuser
/ emulates Moonlite protocol
/
/ (c) 2021 Thx8411 GPL V2
/
/ Thanks to Fehlfarbe (https://github.com/fehlfarbe/arduino-motorfocus)
/
/ New features :
/    Backlash compensation
/    GO      : get temp offset
/    SL<XX>  : set backlash (steps)
/    GL      : get backlash (steps)
*/

#include <AccelStepper.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimerOne.h>
#include <SimpleTimer.h>

// minimal temp correction (steps)
#define MIN_CORRECTION	5

// stepper refresh period (us)
#define PERIOD		2000
// temp compensation period (ms)
#define COMP_PERIOD	30000
// temp refresh period (ms)
#define TEMP_PERIOD	10000

// pins locations
#define PIN_COIL1	5
#define PIN_COIL2	3
#define PIN_COIL3	4
#define PIN_COIL4	2
#define ONE_WIRE_BUS	6
#define SNS_POWER	7

// EEPROM @ mapping
#define	POS_ADDR		0	// unsigned long
#define DIR_ADDR		POS_ADDR+sizeof(long)	 // bool
#define TEMPC_ADDR		DIR_ADDR+sizeof(bool)	 // int
#define TEMPO_ADDR		TEMPC_ADDR+sizeof(int)	 // float
#define BACKLASH_ADDR		TEMPO_ADDR+sizeof(float) // int
#define LAST_ADDR               BACKLASH_ADDR+sizeof(int)

// directions
#define	FORWARD		true
#define BACKWARD	false

// stepper object
AccelStepper stepper(AccelStepper::FULL4WIRE, PIN_COIL1, PIN_COIL2,
		     PIN_COIL3, PIN_COIL4, false);

// simple timer
SimpleTimer compensation_update(COMP_PERIOD);
SimpleTimer temp_update(TEMP_PERIOD);

// temperature
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// multiplier of SPEEDMUX, currently max speed is 480.
int speedFactor = 16;
int speedFactorRaw = 2;
int speedMult = 15;

// temp compensation
int t_coeff = -2;
float t_offset = 0.0;
bool compensation_enabled = false;
bool focused = false;
float temperature = 0.0;
float last_temp = -100.0;

// positions, timing and status
long targetPosition = 0;
long lastSavedPosition = 0;
bool lastDirection = FORWARD;
long millisLastMove = 0;
const long millisDisableDelay = 15000;
int backlash=12;

// read commands
bool eoc = false;
String line;


//
// Interrupt handler
//
static void intHandler() {
    stepper.run();
}


//
// SETUP
//
void setup() {
    // init serial
    Serial.begin(9600);

    // initalize motor
    stepper.setMaxSpeed(speedFactor * speedMult);
    stepper.setAcceleration(100);
    millisLastMove = millis();

    // read saved position from EEPROM
    unsigned long position;
    EEPROM.get(POS_ADDR, position);
    // prevent negative values if EEPROM is empty
    position = max(0, position);
    stepper.setCurrentPosition(position);
    lastSavedPosition = position;
    // read saved direction
    EEPROM.get(DIR_ADDR, lastDirection);
    // read saved temp coefficient
    EEPROM.get(TEMPC_ADDR, t_coeff);
    // read saved temp offset
    EEPROM.get(TEMPO_ADDR, t_offset);
    // read backlash value
    EEPROM.get(BACKLASH_ADDR, backlash);
    backlash = max(0, backlash);

    // init temperature sensor
    pinMode(SNS_POWER, OUTPUT);
    digitalWrite(SNS_POWER, HIGH);
    sensors.begin();

    // init timer
    Timer1.initialize(PERIOD);
    Timer1.attachInterrupt(intHandler);

    // read first temp
    readTemp();
    last_temp=temperature;

    // assume moved a start
    millisLastMove=millis();
}

//
// LOOP
//
void loop() {
    // process the command we got
    if (eoc) {
	if (line.startsWith("2")) {
	    // remove first character and parse command
	    line = line.substring(1);
	}

	String cmd, param;
	int len = line.length();
	// 1 char temp commands
	if (len == 1) {
	    cmd = line.substring(0, 1);
	}
	// other commands
	if (len >= 2) {
	    cmd = line.substring(0, 2);
	}
	// parameter if exists
	if (len > 2) {
	    param = line.substring(2);
	}

	line = "";
	eoc = false;

        // GB ////////////////////////////
	// LED backlight value, always return "00"
	if (cmd.equalsIgnoreCase("GB")) {
	    Serial.print("00#");
	}

        // GV ////////////////////////////
	// firmware value, always return "10"
	if (cmd.equalsIgnoreCase("GV")) {
	    Serial.print("10#");
	}

        // GP ////////////////////////////
	// get the current motor position
	if (cmd.equalsIgnoreCase("GP")) {
	    unsigned long position = stepper.currentPosition();
	    char tempString[6];
	    sprintf(tempString, "%04X", position);
	    Serial.print(tempString);
	    Serial.print("#");
	}

        // GN ////////////////////////////
	// get the new motor position (target)
	if (cmd.equalsIgnoreCase("GN")) {
	    char tempString[6];
	    sprintf(tempString, "%04X", targetPosition);
	    Serial.print(tempString);
	    Serial.print("#");
	}

        // GT ////////////////////////////
	// get the current temperature from DS1820 temperature sensor
	if (cmd.equalsIgnoreCase("GT")) {
	    long t_int = (long) roundf(temperature * 2.0);
	    char tempString[6];
	    sprintf(tempString, "%04lX", t_int);
	    Serial.print(tempString);
	    Serial.print('#');
	}

        // GC ////////////////////////////
	// get the temperature coefficient
	if (cmd.equalsIgnoreCase("GC")) {
	    char tempString[4];
	    sprintf(tempString, "%02X", t_coeff);
	    Serial.print(tempString);
	    Serial.print('#');
	}

        // SC ////////////////////////////
	// set the temperature coefficient
	if (cmd.equalsIgnoreCase("SC")) {
	    if (param.length() > 2) {
		param = param.substring(param.length() - 2);
	    }

	    if (param.startsWith("F")) {
		t_coeff =
		    (0xFF - (int) strtol(param.c_str(), NULL, 16)) - 1;
	    } else {
		t_coeff = (int) strtol(param.c_str(), NULL, 16);
	    }

	    // write new temp coeff. to EEPROM
	    EEPROM.put(TEMPC_ADDR, t_coeff);
	}

        // PO /////////////////////////////
	// set temperature offset
	if (cmd.equalsIgnoreCase("PO")) {
	    if (param.length() > 2) {
		param = param.substring(param.length() - 2);
	    }

	    if (param.startsWith("F")) {
		t_offset =
		    (float) ((0xFF -
			      (int) strtol(param.c_str(), NULL,
					   16)) - 1) / 2.0;
	    } else {
		t_offset = (float) strtol(param.c_str(), NULL, 16) / 2.0;
	    }

	    // write new temp coeff. to EEPROM
	    EEPROM.put(TEMPO_ADDR, t_offset);
	}

        // GO /////////////////////////////
        // get temperature offset
        if (cmd.equalsIgnoreCase("GO")) {
            char tempString[4];
            sprintf(tempString, "%02X", t_offset);
            Serial.print(tempString);
            Serial.print('#');
        }

        // GD ///////////////////////////////
	// get the current motor speed, only values of 02, 04, 08, 10, 20
	if (cmd.equalsIgnoreCase("GD")) {
	    char tempString[6];
	    sprintf(tempString, "%02X", speedFactorRaw);
	    Serial.print(tempString);
	    Serial.print("#");
	}

        // SD ///////////////////////////////
	// set speed, only acceptable values are 02, 04, 08, 10, 20
	if (cmd.equalsIgnoreCase("SD")) {
	    speedFactorRaw = hexstr2int(param);
	    // SpeedFactor: smaller value means faster
	    speedFactor = 32 / speedFactorRaw;
	    stepper.setMaxSpeed(speedFactor * speedMult);
	}

        // GH ///////////////////////////////
	// whether half-step is enabled or not, always return "00"
	if (cmd.equalsIgnoreCase("GH")) {
	    Serial.print("00#");
	}

        // GI ///////////////////////////////
	// motor is moving - 01 if moving, 00 otherwise
	if (cmd.equalsIgnoreCase("GI")) {
	    if (stepper.isRunning()) {
		Serial.print("01#");
	    } else {
		Serial.print("00#");
	    }
	}

        // SP ////////////////////////////////
	// set current motor position
	if (cmd.equalsIgnoreCase("SP")) {
	    unsigned long position = hexstr2long(param);
	    stepper.setCurrentPosition(position);
	}

        // SN ////////////////////////////////
	// set new motor position
	if (cmd.equalsIgnoreCase("SN")) {
	    targetPosition = hexstr2long(param);
	}

        // FG ////////////////////////////////
	// initiate a move
	if (cmd.equalsIgnoreCase("FG")) {
	    // should we move ?
	    if (targetPosition != stepper.currentPosition()) {
                move();
                focused = false;
	    }
	}

        // FQ //////////////////////////////////
	// stop a move
	if (cmd.equalsIgnoreCase("FQ")) {
	    stepper.stop();
	}

        // ///////////////////////////////////
        // set backlash
        if (cmd.equalsIgnoreCase("SL")) {
            if (param.length() > 2) {
                param = param.substring(param.length() - 2);
            }
            backlash = (int) strtol(param.c_str(), NULL, 16);

            // write new temp coeff. to EEPROM
            EEPROM.put(BACKLASH_ADDR, backlash);
        }

        // ///////////////////////////////
        // get current backlash
        if (cmd.equalsIgnoreCase("GL")) {
            char tempString[6];
            sprintf(tempString, "%02X", backlash);
            Serial.print(tempString);
            Serial.print("#");
        }

        // C ///////////////////////////////////
	// temp capture
	if (cmd.equalsIgnoreCase("C")) {
	    readTemp();
	}

        // + ///////////////////////////////////
	// enable temp compensation
	if (cmd.equals("+")) {
	    compensation_enabled = true;
	}

        // - ///////////////////////////////////
	// disable temp compensation
	if (cmd.equals("-")) {
	    compensation_enabled = false;
	}

    }
    // record last date or save last position if needed
    if (stepper.isRunning()) {
	millisLastMove = millis();
    } else {
	stepper.disableOutputs();
	if (millis() - millisLastMove > millisDisableDelay) {
	    // Save current location in EEPROM
	    unsigned long position = stepper.currentPosition();
	    if (lastSavedPosition != position) {
		EEPROM.put(POS_ADDR, position);
		lastSavedPosition = position;
		EEPROM.put(DIR_ADDR, lastDirection);
	    }
            if(!focused) {
                // we assume focus is reached
                focused = true;
                last_temp = temperature;
            }
	}
    }

    // temp refresh
    if (temp_update.isReady()) {
        readTemp();
        temp_update.reset();
    }

    // temp compensation loop
    if (compensation_update.isReady()) {
	tempCompensation();
        compensation_update.reset();
    }
}


//
// read the command until the terminating # character
//
void serialEvent() {
    // read the command until the terminating # character
    while (Serial.available() && !eoc) {
	char c = Serial.read();
	if (c != '#' && c != ':') {
	    line = line + c;
	} else {
	    if (c == '#') {
		eoc = true;
	    }
	}
    }
}


//
// helpers
//

// move to target with backlash compensation
void move() {
    // compute direction change for backlash compensation
    bool newDirection = (targetPosition - stepper.currentPosition()) > 0;
    if (newDirection != lastDirection) {
        // backward to forward
        if (newDirection) {
            stepper.setCurrentPosition(stepper.currentPosition() - backlash);
        // forward to backward
        } else {
            stepper.setCurrentPosition(stepper.currentPosition() + backlash);
        }
        lastDirection = newDirection;
    }
    stepper.enableOutputs();
    stepper.moveTo(targetPosition);
}

//
void readTemp() {
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    if (temperature > 100 || temperature < -50) {
	// error
	temperature = 0.0;
    }
    temperature += t_offset;
}

//
void tempCompensation() {
    if (compensation_enabled) {
	if (focused) {
	    if (temperature != last_temp) {
                long delta=(long)(round((temperature - last_temp) * t_coeff));
                if(abs(delta)!=MIN_CORRECTION) {
		    targetPosition=stepper.currentPosition() + delta;
                    move();
		    last_temp = temperature;
                }
	    }
	}
    }
}

//
// conversion tools
//

// hex char to int
int hexc2int(char c) {
    if (c >= '0' && c <= '9')
	return (c - '0');
    if (c >= 'A' && c <= 'F')
	return (c - 'A' + 10);
}


int hexstr2int(String line) {
    int ret = 0;
    ret += hexc2int(line.charAt(0)) * 16;
    ret += hexc2int(line.charAt(1));

    return (ret);
}

long hexstr2long(String line) {
    long ret = 0;
    ret += hexc2int(line.charAt(0)) * 16 * 16 * 16;
    ret += hexc2int(line.charAt(1)) * 16 * 16;
    ret += hexc2int(line.charAt(2)) * 16;
    ret += hexc2int(line.charAt(3));

    return (ret);
}
