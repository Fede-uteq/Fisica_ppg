#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuración OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pines
const int ppgPin = 0;       // Entrada sensor (ADC)
const int ledPin = 7;       // LED indicador de pulso

// Variables de control
int sensorValue = 0;
const int threshold = 2800; // Umbral fijo (ajusta según tu señal)
bool pulseDetected = false;

// Variables de BPM
unsigned long lastBeatTime = 0;
int bpm = 0;

void setup() {
  // 1. Iniciar Serial para LabVIEW
  Serial.begin(115200);

  // 2. Configurar LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 3. Iniciar I2C y Pantalla (Pines 5 y 6)
  Wire.begin(5, 6);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Si falla, el Serial nos avisará
    Serial.println("OLED_Error"); 
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 10);
    display.print("PPG MONITOR");
    display.display();
  }
  
  delay(1000);
}

void loop() {
  sensorValue = analogRead(ppgPin);

  // --- ENVÍO A LABVIEW (Formato: Señal,BPM) ---
  Serial.print(sensorValue);
  Serial.print(",");
  Serial.println(bpm);

  // --- LÓGICA DE DETECCIÓN DE PULSO ---
  if (sensorValue > threshold && !pulseDetected) {
    unsigned long currentTime = millis();
    unsigned long duration = currentTime - lastBeatTime;

    // Filtro de tiempo para evitar falsos positivos (>300ms = <200 BPM)
    if (duration > 300) {
      bpm = 60000 / duration;
      lastBeatTime = currentTime;
      pulseDetected = true;
      
      // Acciones rápidas en el latido
      digitalWrite(ledPin, HIGH);
      updateDisplay(); 
    }
  }

  // Histéresis simple para resetear el pulso
  if (sensorValue < (threshold - 100)) {
    pulseDetected = false;
    digitalWrite(ledPin, LOW);
  }

  // Muestreo estable (100Hz)
  delay(10); 
}

void updateDisplay() {
  // Solo se llama una vez por latido para no saturar el bus I2C
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 8);
  display.print("BPM: ");
  display.print(bpm);
  display.display();
}