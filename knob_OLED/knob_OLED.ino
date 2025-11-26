#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const unsigned char ART_1 [] PROGMEM = {
	// 'pxArt, 24x24px
	0x04, 0x00, 0x0c, 0x80, 0x1c, 0x40, 0xfd, 0x20, 0xfc, 0xa0, 0xfc, 0xa0, 0xfc, 0xa0, 0xfd, 0x20, 
	0x1c, 0x40, 0x0c, 0x80, 0x04, 0x00
};

const unsigned char ART_2 [] PROGMEM = {
	// 'Layer 4, 11x11px
	0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x40, 0x40, 0x91, 0x20, 0xba, 0xa0, 0x91, 0x20, 0x40, 0x40, 
	0x3f, 0x80, 0x00, 0x00, 0x00, 0x00
};

const unsigned char ART_3 [] PROGMEM = {
	// 'music, 11x11px
	0x1c, 0x00, 0x1e, 0x00, 0x17, 0x80, 0x11, 0xc0, 0x70, 0xc0, 0xf0, 0x40, 0xf0, 0x40, 0x61, 0xc0, 
	0x03, 0xc0, 0x03, 0xc0, 0x01, 0x80
};

const unsigned char ART_4 [] PROGMEM = {
	// 'comms, 11x11px
	0x00, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xc0, 0x00, 0x60, 0x00, 0x60, 0x00, 0x30, 0x00, 
	0x1f, 0x80, 0x0f, 0xc0, 0x03, 0x80
};

const unsigned char ART_5 [] PROGMEM = {
	// 'mic, 11x11px
	0x0e, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0x2e, 0x80, 0x2e, 0x80, 0x2e, 0x80, 0x1b, 0x00, 
	0x04, 0x00, 0x0e, 0x00, 0x00, 0x00
};

// Array of pointers to your bitmaps so we can loop through them
const unsigned char* iconArray[5] = {ART_1, ART_2, ART_3, ART_4, ART_5};

const int NUM_SLIDERS = 5;
const int NUM_OF_LAYERS = 1;
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A7};

//make a buffer to avoid jitter
const int BUFFER_SIZE = 4;
int analogBuffer[NUM_SLIDERS][BUFFER_SIZE];


int displayVolume[NUM_OF_LAYERS][NUM_SLIDERS];
int analogSliderValues[NUM_SLIDERS];
String analogSliderNames[NUM_SLIDERS] = {"Main", "Game", "Chat", "Music", "Mic"};//you can change the names displayed here

const unsigned long sleepAfter = 1000; // this value will change how long the oled will display until turning off.
unsigned long startTime;
unsigned long currTime;
bool standby = 0;

uint8_t currentLayer = 0;
bool inhibitReads = false;

// int lastActiveSlider = -1;

//see http://javl.github.io/image2cpp/ for how to make these


void setup() { 
  //Serial.begin(9600);
  Serial.begin(115200);
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }
  for (int i = 0; i < NUM_SLIDERS; i++) {//fist read
    for (int j = 0; j < BUFFER_SIZE; j++) {
      analogBuffer[i][j] = analogRead(analogInputs[i]);
    }
  }
  Wire.begin();
  Wire.setClock(400000);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); 
  startTime = millis();
}

void loop() {
  updateSliderValues();
  sendSliderValues(); // Actually send data (all the time)
  // printSliderValues(); // For debug
  currTime = millis();
   if (standby == 0 && (currTime - startTime >= sleepAfter)) {
    alwayson();
    display.display();
    standby = 1;
    startTime = currTime;
   }
  delay(1);
}

void updateSliderValues() {
  bool screenNeedsUpdate = false;

  for (int i = 0; i < NUM_SLIDERS; i++) {

    // Read analog value of slider
    analogRead(analogInputs[i]);
    // Remove buffer delay
    // delayMicroseconds(100);
    int rawValue = analogRead(analogInputs[i]);
    //rawValue = (rawValue - 1023) * -1;

    
    // Shift the buffer
    for(int j = BUFFER_SIZE - 1; j > 0; j--) {
      analogBuffer[i][j] = analogBuffer[i][j-1];
    }
    analogBuffer[i][0] = rawValue;
 
    // Use average value instead of raw value
    long sum = 0;
    for (int j = 0; j < BUFFER_SIZE; j++){
      sum += analogBuffer[i][j];
    }
    int averageValue = sum / BUFFER_SIZE;

    // Temporal calibrated value
    // int calibratedValue = averageValue;
    // Calibrate extreme values
    int calibratedValue = map(averageValue, 12, 1017, 0, 1023);

    // Constrain to avoid negative numbers
    calibratedValue = constrain(calibratedValue, 0, 1023);

    // Update Slider value if change is great
    if (abs(calibratedValue - displayVolume[currentLayer][i]) > 2 && inhibitReads == false) {
      displayVolume[currentLayer][i] = calibratedValue;
      displayVol(i);
      // Don't display yet
      //display.display();
      screenNeedsUpdate = true;

      startTime = currTime;
      standby = 0;
    }
  }

  if (screenNeedsUpdate){
      display.display();
    }
}

void sendSliderValues() {
  String builtString = String(""); 

 for(uint8_t j = 0; j < NUM_OF_LAYERS; j++){
    for (int i = 0; i < NUM_SLIDERS; i++) {
      builtString += String((int)displayVolume[j][i]);
      if (i < NUM_SLIDERS - 1 && j < NUM_OF_LAYERS  + 1) {
        builtString += String("|");
      }
    }
  }
  Serial.println(builtString);
}

void alwayson(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int rowHeight = 12; // Fits 5 items in 64px

  for (int i = 0; i < NUM_SLIDERS; i++) {
    int y = i * rowHeight; 

    // Icon
    display.drawBitmap(0, y + 1, iconArray[i], 11, 11, WHITE);

    // Frame
    display.drawRect(13, y + 2, 80, 8, WHITE);

    // Fill
    int barWidth = map(displayVolume[currentLayer][i], 0, 1023, 0, 76);
    display.fillRect(15, y + 4, barWidth, 4, WHITE);

    int percent = map(displayVolume[currentLayer][i], 0, 1023, 0, 100);

    // Text
    // display.setCursor(98, y + 2);
    // display.print(displayVolume[currentLayer][i] / 10); // Print %

    if (percent == 100) display.setCursor(95, y + 2);
    else if (percent < 10) display.setCursor(107, y + 2);
    else display.setCursor(101, y + 2);

    display.print(percent);
    display.print("%");
  }

  // display.clearDisplay();
  // display.setTextSize(2);
  // display.setTextColor(SSD1306_WHITE);

  // display.setCursor(92, 0);
  // display.print((analogRead(A0)) / 10.3 , 0);
  // display.fillRect(1, 2, ((analogRead(A0)) / 13.6), 13, WHITE);  
  // display.drawBitmap(0, 2, ART_1, 11, 11, WHITE);
  // display.setCursor(116, 0);
  // display.print("%");
  // display.drawFastHLine(12, 0, 76, WHITE);
  // display.drawFastHLine(12, 14, 76, WHITE);
  // display.drawFastVLine(12, 0, 14, WHITE);
  // display.drawFastVLine(88, 0, 14, WHITE);

  // display.setCursor(92, 16);
  // display.print((analogRead(A1)) / 10.3 , 0);
  // display.fillRect(13, 17, ((analogRead(A1)) / 13.6), 13, WHITE);  
  // display.setCursor(0, 16); 
  // display.drawBitmap(0, 18, ART_2, 11, 11, WHITE);
  // display.setCursor(116, 16);
  // display.print("%");
  // display.drawFastHLine(12, 16, 76, WHITE);
  // display.drawFastHLine(12, 29, 76, WHITE);
  // display.drawFastVLine(12, 16, 14, WHITE);
  // display.drawFastVLine(88, 16, 14, WHITE); 
  
  // display.setCursor(92, 33);
  // display.print((analogRead(A2)) / 10.3 , 0);
  // display.fillRect(13, 34, ((analogRead(A2)) / 13.6), 13, WHITE);  
  // display.setCursor(0, 33);
  // display.drawBitmap(0, 34, ART_3, 11, 11, WHITE);
  // display.setCursor(116, 32);
  // display.print("%");
  // display.drawFastHLine(12, 33, 76, WHITE);
  // display.drawFastHLine(12, 46, 76, WHITE);
  // display.drawFastVLine(12, 33, 14, WHITE);
  // display.drawFastVLine(88, 33, 14, WHITE);

  // display.setCursor(92, 50);
  // display.print((analogRead(A3)) / 10.3 , 0);
  // display.fillRect(13, 51, ((analogRead(A3)) / 13.6), 13, WHITE);    
  // display.setCursor(0, 50);
  // display.drawBitmap(0, 52, ART_4, 11, 11, WHITE);
  // display.setCursor(116, 50);
  // display.print("%");
  // display.drawFastHLine(12, 50, 76, WHITE);
  // display.drawFastHLine(12, 63, 76, WHITE);
  // display.drawFastVLine(12, 50, 14, WHITE);
  // display.drawFastVLine(88, 50, 14, WHITE);
}

void displayVol(int i){
  int percentage = percentage_volume(displayVolume[currentLayer][i]);

  display.clearDisplay();

  // 1. Draw Name
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 17);
  display.println(analogSliderNames[i]);

  // 2. Draw Bar
  display.fillRect(0, 0, percentage * 1.28, 16, SSD1306_WHITE);
  
  // 3. Draw Number
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 40);
  display.println(percentage);

  // if (i != lastActiveSlider){
  //   display.clearDisplay();

  //   display.setTextSize(3);
  //   display.setTextColor(SSD1306_WHITE);
  //   display.setCursor(0, 16);
  //   display.println(analogSliderNames[i]);

  //   lastActiveSlider = i;
  // }
  // else {
  //   display.fillRect(0, 0, 126, 16, SSD1306_BLACK);

  //   display.fillRect(0, 40, 128, 24, SSD1306_BLACK);
  // }

  // display.fillRect(0, 0, percentage*1.28, 16, WHITE);
  // display.setTextSize(3);             //pixel scale
  // display.setTextColor(SSD1306_WHITE);        // Draw white text
  // display.setCursor(0, 40);
  // display.println(percentage);
}

int percentage_volume(int actual_value){
  return (actual_value) / 10.22;
}

void printSliderValues() {
  for(int j = 0; j < NUM_OF_LAYERS; j++){
    for (int i = 0; i < NUM_SLIDERS; i++) {
      String printedString = String("Slider #") + String(i + 1) + String(": ") + String(displayVolume[j][i]/4) + String(" mV");
      Serial.write(printedString.c_str());

      if (i < NUM_SLIDERS - 1 && j < NUM_OF_LAYERS + 1) {
        Serial.write(" | ");
      } else {
        Serial.write("\n");
      }
    }
  }
}
