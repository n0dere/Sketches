#define POTENTIOMETER_A0 0

#define RGB_LED_RED_PIN 2               // D2
#define RGB_LED_GRN_PIN 3               // D3
#define RGB_LED_BLU_PIN 4               // D4

static short g_LastHueValue = 0;

static void setColorHSV(float h, float s, float v)
{
    // Convert HSV to RGB using the algorithm from
    // https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB

    float c = v * s;
    float h_prime = fmod(h / 60.0, 6);
    float x = c * (1 - fabs(fmod(h_prime, 2) - 1));
    float m = v - c;
    float r = 0, g = 0, b = 0;
    
    switch ((byte) h_prime) {
        case 0: r = c; g = x; b = 0; break;     // Red
        case 1: r = x; g = c; b = 0; break;     // Yellow
        case 2: r = 0; g = c; b = x; break;     // Green
        case 3: r = 0; g = x; b = c; break;     // Cyan
        case 4: r = x; g = 0; b = c; break;     // Blue
        case 5: r = c; g = 0; b = x; break;     // Magenta
    }

    // Write the RGB values to the LED pins
    analogWrite(RGB_LED_RED_PIN, (r + m) * 255);
    analogWrite(RGB_LED_GRN_PIN, (g + m) * 255);
    analogWrite(RGB_LED_BLU_PIN, (b + m) * 255);

    // NOTE:

    // This code is very slow.
    // Use https://www.vagrearg.org/content/hsvrgb instead
}

void setup()
{
    pinMode(RGB_LED_RED_PIN, OUTPUT);
    pinMode(RGB_LED_GRN_PIN, OUTPUT);
    pinMode(RGB_LED_BLU_PIN, OUTPUT);
}

void loop()
{
    short potValue = analogRead(POTENTIOMETER_A0);
    short hueValue = map(potValue, 0, 1023, 0, 359);
    
    if (g_LastHueValue != hueValue) {
        setColorHSV(hueValue, 1 /* 100% */, 1 /* 100% */);
        g_LastHueValue = hueValue;
    }
}