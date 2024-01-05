// 3641BS
// http://www.xlitx.com/datasheet/3641BS.pdf

#define DIG1_PIN 2                      // D2
#define DIG2_PIN 3                      // D3
#define DIG3_PIN 4                      // D4
#define DIG4_PIN 5                      // D5

// 74HC595

#define LATCH_PIN 8                     // ST_CP <- D8
#define CLOCK_PIN 12                    // SH_CP <- D12
#define DATA_PIN 11                     // DS    <- D11

#define DIGIT_COUNT 4

#define MAKE_7SEG_BYTE(A, B, C, D, E, F, G, DP)                 \
    (((A) << 7) | ((B) << 6) | ((C) << 5) | ((D) << 4) |        \
     ((E) << 3) | ((F) << 2) | ((G) << 1) | (DP))

// Table: (see: Common Anode)
// https://www.electronicshub.org/7-segment-display-pinout

static const uint8_t g_DigitSegments[10] PROGMEM = {
    MAKE_7SEG_BYTE(0, 0, 0, 0, 0, 0, 1, 1),     // 0
    MAKE_7SEG_BYTE(1, 0, 0, 1, 1, 1, 1, 1),     // 1
    MAKE_7SEG_BYTE(0, 0, 1, 0, 0, 1, 0, 1),     // 2
    MAKE_7SEG_BYTE(0, 0, 0, 0, 1, 1, 0, 1),     // 3
    MAKE_7SEG_BYTE(1, 0, 0, 1, 1, 0, 0, 1),     // 4
    MAKE_7SEG_BYTE(0, 1, 0, 0, 1, 0, 0, 1),     // 5
    MAKE_7SEG_BYTE(0, 1, 0, 0, 0, 0, 0, 1),     // 6
    MAKE_7SEG_BYTE(0, 0, 0, 1, 1, 1, 1, 1),     // 7
    MAKE_7SEG_BYTE(0, 0, 0, 0, 0, 0, 0, 1),     // 8
    MAKE_7SEG_BYTE(0, 0, 0, 0, 1, 0, 0, 1),     // 9
};

static const uint8_t g_DigitPins[DIGIT_COUNT] PROGMEM = {
    DIG1_PIN, DIG2_PIN, DIG3_PIN, DIG4_PIN
};

static uint16_t g_CachedValue = 0; // latest read value

// 10000 - 1 = 9999 (if DIGIT_COUNT == 4)
static const size_t g_MaxValue = pow(10, DIGIT_COUNT) - 1;

static void display(uint8_t segments, uint8_t digitNo)
{
    // Set the latch pin low to start sending data
    digitalWrite(LATCH_PIN, LOW);
    // Shift out the segment value
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, segments);
    // Set the latch pin high to stop sending data
    digitalWrite(LATCH_PIN, HIGH);

    // Turn on the digit pin for a short time
    digitalWrite(pgm_read_byte(&g_DigitPins[digitNo]), HIGH);
    delayMicroseconds(250);
    digitalWrite(pgm_read_byte(&g_DigitPins[digitNo]), LOW);
}

static void displayNumber(uint16_t number)
{
    uint8_t i = DIGIT_COUNT - 1;

    do {
        uint8_t digit = number % 10;
        display(pgm_read_byte(&g_DigitSegments[digit]), i--);
    } while (number /= 10);
}

static void displayError()
{
    display(MAKE_7SEG_BYTE(0, 1, 1, 0, 0, 0, 0, 1), 0); // E
    display(MAKE_7SEG_BYTE(1, 1, 1, 1, 0, 1, 0, 1), 1); // r
    display(MAKE_7SEG_BYTE(1, 1, 1, 1, 0, 1, 0, 1), 2); // r
}

void setup()
{
    Serial.begin(9600);

    for (uint8_t i = 0; i < DIGIT_COUNT; ++i)
        pinMode(pgm_read_byte(&g_DigitPins[i]), OUTPUT);

    // Set the latch, clock, and data pins as outputs
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
}

void loop()
{
    // uint16_t == uint16_t == 2 bytes

    if (Serial.available() > 0)
        Serial.readBytes((uint8_t*) &g_CachedValue, 2);

    if (g_CachedValue > g_MaxValue)
        displayError();
    else
        displayNumber(g_CachedValue);
}