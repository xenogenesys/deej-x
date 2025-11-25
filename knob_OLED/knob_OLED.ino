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
	// 'music, 11x11px
	0x1c, 0x00, 0x1e, 0x00, 0x17, 0x80, 0x11, 0xc0, 0x70, 0xc0, 0xf0, 0x40, 0xf0, 0x40, 0x61, 0xc0, 
	0x03, 0xc0, 0x03, 0xc0, 0x01, 0x80
};

const unsigned char ART_5 [] PROGMEM = {
	// 'music, 11x11px
	0x1c, 0x00, 0x1e, 0x00, 0x17, 0x80, 0x11, 0xc0, 0x70, 0xc0, 0xf0, 0x40, 0xf0, 0x40, 0x61, 0xc0, 
	0x03, 0xc0, 0x03, 0xc0, 0x01, 0x80
};

const int NUM_SLIDERS = 5;
const int NUM_OF_LAYERS = 1;
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A7};

//make a buffer to avoid jitter
const int BUFFER_SIZE = 5;
int analogBuffer[NUM_SLIDERS][BUFFER_SIZE];


int displayVolume[NUM_OF_LAYERS][NUM_SLIDERS];
int analogSliderValues[NUM_SLIDERS];
String analogSliderNames[NUM_SLIDERS] = {"Current","Game","Discord","Spotify", "Mic"};//you can change the names displayed here

const unsigned long sleepAfter = 1000; // this value will change how long the oled will display until turning off.
unsigned long startTime;
unsigned long currTime;
bool standby = 0;

uint8_t currentLayer = 0;
bool inhibitReads = false;


//see http://javl.github.io/image2cpp/ for how to make these


void setup() { 
  Serial.begin(9600);
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }
  for (int i = 0; i < NUM_SLIDERS; i++) {//fist read
    for (int j = 0; j < BUFFER_SIZE; j++) {
      analogBuffer[i][j] = analogRead(analogInputs[i]);
    }
  }
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
    standby = 1;
    display.display();
    startTime = currTime;
   }
  delay(10);
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    // Read analog value of slider
    int rawValue = analogRead(analogInputs[i]);
    //rawValue = (rawValue - 1023) * -1;

    // Update Slider value if change is great
    if (abs(rawValue - displayVolume[currentLayer][i]) > 10 && inhibitReads == false) {
      displayVolume[currentLayer][i] = rawValue;
      displayVol(i);
      display.display();
      startTime = currTime;
      standby = 0;
    }
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
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(92, 0);
  display.print((analogRead(A0)) / 10.3 , 0);
  display.fillRect(13, 1, ((analogRead(A0)) / 13.6), 13, WHITE);  
  display.drawBitmap(0, 2, ART_1, 11, 11, WHITE);
  display.setCursor(116, 0);
  display.print("%");
  display.drawFastHLine(12, 0, 76, WHITE);
  display.drawFastHLine(12, 14, 76, WHITE);
  display.drawFastVLine(12, 0, 15, WHITE);
  display.drawFastVLine(88, 0, 15, WHITE);

  display.setCursor(92, 16);
  display.print((analogRead(A1)) / 10.3 , 0);
  display.fillRect(13, 17, ((analogRead(A1)) / 13.6), 13, WHITE);  
  display.setCursor(0, 16); 
  display.drawBitmap(0, 18, ART_2, 11, 11, WHITE);
  display.setCursor(116, 16);
  display.print("%");
  display.drawFastHLine(12, 16, 76, WHITE);
  display.drawFastHLine(12, 29, 76, WHITE);
  display.drawFastVLine(12, 16, 14, WHITE);
  display.drawFastVLine(88, 16, 14, WHITE); 
  
  display.setCursor(92, 33);
  display.print((analogRead(A2)) / 10.3 , 0);
  display.fillRect(13, 34, ((analogRead(A2)) / 13.6), 13, WHITE);  
  display.setCursor(0, 33);
  display.drawBitmap(0, 34, ART_3, 11, 11, WHITE);
  display.setCursor(116, 32);
  display.print("%");
  display.drawFastHLine(12, 33, 76, WHITE);
  display.drawFastHLine(12, 46, 76, WHITE);
  display.drawFastVLine(12, 33, 14, WHITE);
  display.drawFastVLine(88, 33, 14, WHITE);

  display.setCursor(92, 50);
  display.print((analogRead(A3)) / 10.3 , 0);
  display.fillRect(13, 51, ((analogRead(A3)) / 13.6), 13, WHITE);    
  display.setCursor(0, 50);
  display.drawBitmap(0, 52, ART_4, 11, 11, WHITE);
  display.setCursor(116, 50);
  display.print("%");
  display.drawFastHLine(12, 50, 76, WHITE);
  display.drawFastHLine(12, 63, 76, WHITE);
  display.drawFastVLine(12, 50, 14, WHITE);
  display.drawFastVLine(88, 50, 14, WHITE);

  display.setCursor(92, 50);
  display.print((analogRead(A7)) / 10.3 , 0);
  display.fillRect(13, 51, ((analogRead(A3)) / 13.6), 13, WHITE);    
  display.setCursor(0, 50);
  display.drawBitmap(0, 52, ART_4, 11, 11, WHITE);
  display.setCursor(116, 50);
  display.print("%");
  display.drawFastHLine(12, 50, 76, WHITE);
  display.drawFastHLine(12, 63, 76, WHITE);
  display.drawFastVLine(12, 50, 14, WHITE);
  display.drawFastVLine(88, 50, 14, WHITE);
}

void displayVol(int i){
  int percentage = percentage_volume(displayVolume[currentLayer][i]);

  display.clearDisplay();
  display.fillRect(0, 0, percentage*1.28, 16, WHITE);
  display.setTextSize(3);             //pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 16);             // Start at top-left corner
  display.println(analogSliderNames[i]);
  display.setCursor(0, 40);
  display.setTextSize(3);
  display.println(percentage);
  display.display();
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
