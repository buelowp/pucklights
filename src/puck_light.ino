#define ON_BUTTON   D2
#define OFF_BUTTON  D1
#define MOTION_DETECT   A0
#define LIGHT_DETECT    A2

#define FIVE_MINUTES    (1000 * 60 * 5)

bool g_lightsOff;
unsigned long g_millisToTurnOff;

bool isDark()
{
    int lux = analogRead(LIGHT_DETECT);

    if (lux > 500)
        return true;

    return false;
}

int turnLightsOn(String)
{
    digitalWrite(ON_BUTTON, 1);
    delay(1);
    digitalWrite(ON_BUTTON, 0);
    g_lightsOff = false;
    g_millisToTurnOff = millis();
}

int turnLightsOff(String)
{
    digitalWrite(OFF_BUTTON, 1);
    delay(1);
    digitalWrite(OFF_BUTTON, 0);
    g_lightsOff = true;
    g_millisToTurnOff = 0;
}

void setup()
{
    pinMode(ON_BUTTON, OUTPUT);
    pinMode(OFF_BUTTON, OUTPUT);
    pinMode(MOTION_DETECT, INPUT_PULLUP);
    pinMode(LIGHT_DETECT, INPUT);

    Particle.function("On", turnLightsOn);
    Particle.function("Off", turnLightsOff);
    
    g_lightsOff = true;
    g_millisToTurnOff = 0;

    turnLightsOff(String());
}

void loop()
{
    if (digitalRead(MOTION_DETECT) == 0) {
        if (isDark() && g_lightsOff) {
            turnLightsOn(String());
        }
    }
    if (g_millisToTurnOff != 0) {
        if (millis() < g_millisToTurnOff) {
            turnLightsOff(String());    // Millis wrapper, so just flip the switch off
        }
        else if ((millis() - g_millisToTurnOff) > FIVE_MINUTES) {
            turnLightsOff(String());
        }
    }
}
