#define ON_BUTTON           D1
#define OFF_BUTTON          D5
#define MOTION_DETECT       D2
#define LIGHT_DETECT        A1
#define APP_ID              65
#define DEFAULT_TRIGGER     25
#define ARRAY_SIZE          50

#define ONE_SECOND          (1000)
#define TWENTY_SECONDS      (ONE_SECOND * 20)
#define ONE_MINUTE          (ONE_SECOND * 60)
#define FOUR_MINUTES        (ONE_MINUTE * 4)
#define FIVE_MINUTES        (ONE_MINUTE * 5)
#define ONE_HOUR            (ONE_MINUTE * 60)
#define TWELVE_HOURS        (ONE_HOUR * 12)
#define CST_OFFSET          -6
#define DST_OFFSET          (CST_OFFSET + 1)
#define TIME_BASE_YEAR		2019

const uint8_t _usDSTStart[22] = { 10, 8,14,13,12,10, 9, 8,14,12,11,10, 9,14,13,12,11, 9};
const uint8_t _usDSTEnd[22]   = { 3, 1, 7, 6, 5, 3, 2, 1, 7, 5, 4, 3, 2, 7, 6, 5, 4, 2};

bool g_lightsOn;
bool g_syncDone;
int g_timeZone;
int g_lux;
int g_appid;
int g_trigger;
int g_turnOnLux;
int g_arrayIndex;
int g_average;
bool g_motionDetected;
unsigned long g_debounce;
unsigned long g_timeOut;
int g_luxArray[ARRAY_SIZE];

int currentTimeZone()
{
    g_timeZone = DST_OFFSET;
    if (Time.month() > 3 && Time.month() < 11) {
        return DST_OFFSET;
    }
    if (Time.month() == 3) {
        if ((Time.day() == _usDSTStart[Time.year() - TIME_BASE_YEAR]) && Time.hour() >= 2)
            return DST_OFFSET;
        if (Time.day() > _usDSTStart[Time.year() - TIME_BASE_YEAR])
            return DST_OFFSET;
    }
    if (Time.month() == 11) {
        if ((Time.day() == _usDSTEnd[Time.year() - TIME_BASE_YEAR]) && Time.hour() <=2)
            return DST_OFFSET;
        if (Time.day() < _usDSTEnd[Time.year() - TIME_BASE_YEAR])
            return DST_OFFSET;
    }
    g_timeZone = CST_OFFSET;
    return CST_OFFSET;
}

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
    g_timeOut = millis() + FOUR_MINUTES;
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

int averageLux(int lux)
{
    int average = 0;

    if (g_arrayIndex == (ARRAY_SIZE - 1))
        g_arrayIndex = 0;

    g_luxArray[g_arrayIndex++] = lux;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        average += g_luxArray[i];
    }
    g_average = average / ARRAY_SIZE;
    return g_average;
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
    g_arrayIndex = 0;
    g_lightsOn = false;
    g_timeOut = 0;
    g_motionDetected = false;
    g_debounce = 0;
    g_syncDone = true;

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
    Particle.variable("averagelux", g_average);

    Particle.syncTime();

    Time.zone(currentTimeZone());

    turnLightsOff(String());    // Set lights off if we restart
    g_lux = analogRead(LIGHT_DETECT); // Allow us to see the value before we start the loop
    delay(ONE_MINUTE);      // Let the motion detect circuit settle
}

void loop()
{
    if (Time.hour() == 3 && Time.minute() == 1 && Time.second() == 1) {
        if (!g_syncDone) {
            Particle.syncTime();
            Time.zone(currentTimeZone());
            g_syncDone = true;
        }
    }
    else {
        g_syncDone = false;
    }

    g_lux = analogRead(LIGHT_DETECT);
    averageLux(g_lux);

    if (Time.hour() >= 6 && Time.hour() <= 21)
        return;

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

        if (g_average < g_trigger) {
            g_turnOnLux = g_average;
            turnLightsOn(String());
        }
    }
    else {
        g_motionDetected = false;
    }
}
