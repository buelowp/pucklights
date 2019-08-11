#define ON_BUTTON           D1
#define OFF_BUTTON          D5
#define MOTION_DETECT       D2
#define LIGHT_DETECT        A1
#define APP_ID             58
#define DEFAULT_TRIGGER     25

#define ONE_SECOND          (1000)
#define TWENTY_SECONDS      (ONE_SECOND * 20)
#define ONE_MINUTE          (ONE_SECOND * 60)
#define FIVE_MINUTES        (ONE_MINUTE * 5)
#define ONE_HOUR            (ONE_MINUTE * 60)
#define TWELVE_HOURS        (ONE_HOUR * 12)

bool g_lightsOn;
int g_lux;
int g_appid;
int g_trigger;
int g_turnOnLux;
bool g_motionDetected;
unsigned long g_debounce;
unsigned long g_timeOut;

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
    g_debounce = millis() + TWENTY_SECONDS;
    return 0;
}

int setLuxTriggerValue(String lux)
{
    g_trigger = lux.toInt();
    return g_trigger;
}

void setup()
{
    pinMode(ON_BUTTON, OUTPUT);
    pinMode(OFF_BUTTON, OUTPUT);
    pinMode(MOTION_DETECT, INPUT_PULLDOWN);
    pinMode(LIGHT_DETECT, INPUT);

    g_appid = APP_ID;
    g_trigger = DEFAULT_TRIGGER;
    g_turnOnLux = 0;

    digitalWrite(ON_BUTTON, LOW);
    digitalWrite(OFF_BUTTON, LOW);

    Particle.function("On", turnLightsOn);
    Particle.function("Off", turnLightsOff);
    Particle.function("SetTrigger", setLuxTriggerValue);
    Particle.variable("lux", g_lux);
    Particle.variable("version", g_appid);
    Particle.variable("trigger", g_trigger);
    Particle.variable("state", g_lightsOn);
    Particle.variable("turnonlux", g_turnOnLux);

    g_lightsOn = false;
    g_timeOut = 0;
    g_motionDetected = false;
    g_debounce = 0;

    turnLightsOff(String());    // Set lights off if we restart
    g_lux = analogRead(LIGHT_DETECT); // Allow us to see the value before we start the loop
    delay(ONE_MINUTE);      // Let the motion detect circuit settle
}

void loop()
{
    g_lux = analogRead(LIGHT_DETECT);

    if (g_timeOut >= millis())
        return;

    if (g_lightsOn) {
        turnLightsOff(String());
        return;
    }

    if (digitalRead(MOTION_DETECT) == HIGH) {
        g_motionDetected = true;
        if (g_debounce >= millis())
            return;
        else
            g_debounce = 0;

        if (g_lux < g_trigger) {
            g_turnOnLux = g_lux;
            turnLightsOn(String());
        }
    }
    else {
        g_motionDetected = false;
    }
}
