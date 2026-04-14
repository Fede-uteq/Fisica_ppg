#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuración de la Pantalla OLED 0.91"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pines y Variables del Sensor
const int ppgPin = 0;       // Pin ADC (GPIO 0 / ADC1_CH0)
int sensorValue = 0;
int threshold = 2800;       // Valor inicial. Ajustar según lo que veas en el serial.
bool pulseDetected = false;

// Variables de Tiempo y BPM
unsigned long lastBeatTime = 0;
float bpm = 0;
const int samples = 5;      // Para suavizar el cálculo del BPM
int bpmList[samples];
int bpmIndex = 0;

void setup() {
  // Inicialización del Serial con retardo para estabilización
  Serial.begin(115200);
  delay(2000); 

  // Inicialización I2C con los pines que te funcionaron
  Wire.begin(5, 6);

  // Iniciar Pantalla en 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 no encontrado"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 10);
  display.println("SISTEMA PPG LISTO");
  display.display();
  delay(1500);
}

void loop() {
  sensorValue = analogRead(ppgPin);

  // --- 1. Mandar informacion a labview ---
  Serial.print(sensorValue);
  Serial.print(","); 
  Serial.println(bpm);

  // --- 2. Detección de Pulso (Lógica de Umbral con Histéresis) ---
  if (sensorValue > threshold && !pulseDetected) {
    unsigned long currentTime = millis();
    unsigned long duration = currentTime - lastBeatTime;

    // Filtro de tiempo: mínimo 350ms entre latidos (~170 BPM máximo)
    if (duration > 350) {
      int instantBpm = 60000 / duration;
      
      // Promedio simple para estabilidad
      bpmList[bpmIndex] = instantBpm;
      bpmIndex = (bpmIndex + 1) % samples;
      
      float sum = 0;
      for(int i=0; i<samples; i++) sum += bpmList[i];
      bpm = sum / samples;

      lastBeatTime = currentTime;
      pulseDetected = true;
      
      // Actualizar pantalla en cada latido detectado
      updateOLED();
    }
  }

  // Resetear detección cuando la señal baja del umbral (con margen de ruido)
  if (sensorValue < (threshold - 150)) {
    pulseDetected = false;
  }

  delay(10); // Frecuencia de muestreo 100Hz aprox.
}

void updateOLED() {
  display.clearDisplay();
  
  // Dibujar indicador de latido (un pequeño cuadro)
  display.fillRect(0, 0, 5, 5, SSD1306_WHITE);

  // Mostrar BPM
  display.setTextSize(2);
  display.setCursor(15, 8);
  display.print("BPM: ");
  display.print((int)bpm);
  
  display.display();
}