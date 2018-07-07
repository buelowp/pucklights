#define ON_BUTTON           D1
#define OFF_BUTTON          D5
#define MOTION_DETECT       D2
#define LIGHT_DETECT        A1
#define VERSION             49

#define FIVE_MINUTES        (1000 * 60 * 5)
#define TWENTY_SECONDS      (1000 * 20)
#define ONE_MINUTE          (1000 * 60 * 1)

bool g_lightsOn;
unsigned long g_millis;
int g_lux;
int g_version;
bool g_motionDetected;
unsigned long g_timeOut;
unsigned long g_detectRemain;

/*
 * Turn the lights on. This will simply reset the current
 * time off timer each time it observes movement.
 */
int turnLightsOn(String)
{
    if (!g_lightsOn) {
        digitalWrite(ON_BUTTON, HIGH);
        delay(100);
        digitalWrite(ON_BUTTON, LOW);
        g_lightsOn = true;
    }
    g_timeOut = millis() + FIVE_MINUTES;
    return 1;
}

/*
 * Turn the lights off
 * This includes resetting the lights are on flag, setting the turn off
 * timer to 0, and starting the lockout timer which keeps stray events
 * from happening as the 433Mh radio tends to activate the motion sensor.
 */
int turnLightsOff(String)
{
    digitalWrite(OFF_BUTTON, HIGH);
    delay(100);
    digitalWrite(OFF_BUTTON, LOW);
    g_lightsOn = false;
    g_timeOut = 0;
    g_detectRemain = millis() + TWENTY_SECONDS;
    return 0;
}

void setup()
{
    pinMode(ON_BUTTON, OUTPUT);
    pinMode(OFF_BUTTON, OUTPUT);
    pinMode(MOTION_DETECT, INPUT_PULLDOWN);
    pinMode(LIGHT_DETECT, INPUT);

    digitalWrite(ON_BUTTON, LOW);
    digitalWrite(OFF_BUTTON, LOW);

    Particle.function("On", turnLightsOn);
    Particle.function("Off", turnLightsOff);
    Particle.variable("lux", g_lux);
    Particle.variable("version", g_version);

    g_lightsOn = false;
    g_timeOut = 0;
    g_motionDetected = false;
    g_millis = millis();
    g_detectRemain = 0;
    g_version = VERSION;

    turnLightsOff(String());
    g_lux = analogRead(LIGHT_DETECT);
}

void loop()
{
    g_lux = analogRead(LIGHT_DETECT);
    g_millis = millis();
    // Give the PIR about a minute to settle down to avoid
    // false positives.
    if (g_millis < ONE_MINUTE)
        return;

    if (g_detectRemain < millis()) {
        g_detectRemain = 0;
    }

    if (g_timeOut < millis() && g_lightsOn) {
        turnLightsOff(String());
        return;
    }

    if (digitalRead(MOTION_DETECT) == HIGH) {
        g_motionDetected = true;
        if ((g_lux < 50) && (g_detectRemain == 0)) {
            turnLightsOn(String());
        }
    }
    else {
        g_motionDetected = false;
    }
}
