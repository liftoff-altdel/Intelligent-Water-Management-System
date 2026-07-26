#define TdsSensorPin 34
#define TurbiditySensorPin 35

float tdsVoltage = 0, turbidityVoltage = 0;
float tdsValue = 0, turbidityNTU = 0;

// Calibration line: (1.19V,0NTU) → (1.07V,5NTU)
const float m = -41.67;
const float b = 49.6;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 - TDS & Turbidity Sensor (Increase till 0.81V, 0 below)");
  Serial.println("--------------------------------------------------");
}

void loop() {
  // ---------- TDS Sensor ----------
  int tdsADC = analogRead(TdsSensorPin);
  tdsVoltage = (tdsADC * 3.3) / 4095.0;
  tdsValue = (133.42 * tdsVoltage * tdsVoltage * tdsVoltage
             - 255.86 * tdsVoltage * tdsVoltage
             + 857.39 * tdsVoltage) * 0.5;

  // ---------- Turbidity Sensor ----------
  int turbidityADC = analogRead(TurbiditySensorPin);
  turbidityVoltage = (turbidityADC * 3.3) / 4095.0;

  // --- Turbidity Calibration Logic ---
  if (turbidityVoltage > 1.19) {
    // Above calibration range
    turbidityNTU = 0.0;
  }
  else if (turbidityVoltage >= 0.81 && turbidityVoltage <= 1.19) {
    // Continue linear rise down to 0.81V
    turbidityNTU = m * turbidityVoltage + b;
    if (turbidityNTU < 0) turbidityNTU = 0.0;
  }
  else {
    // 0.80V or below = 0 NTU
    turbidityNTU = 0.0;
  }

  // ---------- Display ----------
  Serial.print("TDS Voltage: ");
  Serial.print(tdsVoltage, 3);
  Serial.print(" V | TDS: ");
  Serial.print(tdsValue, 2);
  Serial.println(" ppm");

  Serial.print("Turbidity Voltage: ");
  Serial.print(turbidityVoltage, 3);
  Serial.print(" V | Turbidity: ");
  Serial.print(turbidityNTU, 2);
  Serial.println(" NTU");

  Serial.println("--------------------------------------------------\n");

  delay(2000);
}
