#define ON_BUTTON           D1
#define OFF_BUTTON          D5
#define MOTION_DETECT       D2
#define LIGHT_DETECT        A1
#define APP_ID              66
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
int g_highCount;
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

void turnLightsOn()
{
    if (!g_lightsOn) {
        digitalWrite(ON_BUTTON, HIGH);
        delay(100);
        digitalWrite(ON_BUTTON, LOW);
        g_lightsOn = true;
    }
    g_timeOut = millis() + FOUR_MINUTES;
}

/*
 * Turn the lights on. This will simply reset the current
 * time off timer each time it observes movement.
 */
int turnLightsOnNet(String)
{
    turnLightsOn();
    return 1;
}

void turnLightsOff()
{
    digitalWrite(OFF_BUTTON, HIGH);
    delay(100);
    digitalWrite(OFF_BUTTON, LOW);
    g_lightsOn = false;
    g_timeOut = 0;
    g_debounce = millis() + TWENTY_SECONDS;
}

/*
 * Turn the lights off
 * This includes resetting the lights are on flag, setting the turn off
 * timer to 0, and starting the lockout timer which keeps stray events
 * from happening as the 433Mh radio tends to activate the motion sensor.
 */
int turnLightsOffNet(String)
{
    turnLightsOff();
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
    g_highCount = 0;

    digitalWrite(ON_BUTTON, LOW);
    digitalWrite(OFF_BUTTON, LOW);

    Particle.function("On", turnLightsOnNet);
    Particle.function("Off", turnLightsOffNet);
    Particle.function("SetTrigger", setLuxTriggerValue);
    Particle.variable("lux", g_lux);
    Particle.variable("version", g_appid);
    Particle.variable("trigger", g_trigger);
    Particle.variable("state", g_lightsOn);
    Particle.variable("turnonlux", g_turnOnLux);
    Particle.variable("averagelux", g_average);

    Particle.syncTime();

    Time.zone(currentTimeZone());

    turnLightsOff();    // Set lights off if we restart
    g_lux = analogRead(LIGHT_DETECT); // Allow us to see the value before we start the loop
    delay(ONE_MINUTE);      // Let the motion detect circuit settle
}

void loop()
{
    static int lastHour = 25;

    if (Time.hour() != lastHour) {
        Particle.syncTime();
        waitUntil(Particle.syncTimeDone);
        Time.zone(currentTimeZone());
        lastHour = Time.hour();
    }

    g_lux = analogRead(LIGHT_DETECT);
    averageLux(g_lux);

    if (Time.hour() >= 6 && Time.hour() <= 21)
        return;

    if (g_timeOut >= millis())
        return;

    if (g_lightsOn) {
        turnLightsOff();
        return;
    }

    if (digitalRead(MOTION_DETECT) == HIGH) {
        if (g_debounce >= millis())     // don't turn on within 20 seconds of turning off
            return;
        else
            g_debounce = 0;             // reset so we can turn on now

        /**
         * We need to be both below the trigger, and to have seen
         * 100 straight events. This should avoid false triggers based on one
         * off events from the sensor
         */
        if ((g_average < g_trigger) && (g_highCount++ >= 100)) {
            g_turnOnLux = g_average;
            g_highCount = 0;
            turnLightsOn();
        }
    }
    else {
        g_motionDetected = false;
        g_highCount = 0;
    }
}
