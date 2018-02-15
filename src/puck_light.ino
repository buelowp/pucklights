#define ON_BUTTON   D2
#define OFF_BUTTON  D1
#define MOTION_DETECT   A0
#define LIGHT_DETECT    A2

#define FIVE_MINUTES    (1000 * 60 * 5)

bool g_lightsOff;
int g_millisToTurnOff;

bool isDark()
{
    int lux = analogRead(LIGHT_DETECT);

    if (lux > 500)
        return true;

    return false;
}

void turnLightsOn()
{
    digitalWrite(ON_BUTTON, 1);
    delay(1);
    digitalWrite(ON_BUTTON, 0);
    g_lightsOff = false;
    g_millisToTurnOff = millis();
}

void turnLightsOff()
{
    digitalWrite(OFF_BUTTON, 1);
    delay(1);
    digitalWrite(OFF_BUTTON, 0);
    g_lightsOff = true;
    g_millisToTurnOff = -1;
}

void setup()
{
    pinMode(ON_BUTTON, OUTPUT);
    pinMode(OFF_BUTTON, OUTPUT);
    pinMode(MOTION_DETECT, INPUT_PULLUP);
    pinMode(LIGHT_DETECT, INPUT);

    g_lightsOff = true;
    g_millisToTurnOff = -1;
}

void loop()
{
    if (digitalRead(MOTION_DETECT) == 0) {
        if (isDark() && g_lightsOff) {
            turnLightsOn();
        }
    }
    if (g_millisToTurnOff != -1) {
        if (millis() < g_millisToTurnOff) {
            turnLightsOff();    // Millis wrapper, so just flip the switch off
        }
        else if ((millis() - g_millisToTurnOff) > FIVE_MINUTES) {
            turnLightsOff();
        }
    }
}
