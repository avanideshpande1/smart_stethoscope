#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

unsigned long channelID = 1234567;
const char* writeKey = "YOUR_WRITE_API_KEY";

const int MIC_PIN = A0;
const int LED_PIN = LED_BUILTIN;

// record 20 sec like in our project poster
const int SAMPLE_RATE = 50;
const int RECORD_SEC = 20;
const int BUFFER_SIZE = SAMPLE_RATE * RECORD_SEC;
const int SAMPLE_MS = 1000 / SAMPLE_RATE;

// simple moving average window (same idea as our validation graph)
const int SMA_WINDOW = 10;

// normalized amplitude threshold - red dashed line in our chart was at 0.5
const float THRESHOLD = 0.5;

// don't count two beats too close together
const int MIN_BEAT_GAP = 25;

uint16_t rawSamples[BUFFER_SIZE];
float filteredSamples[BUFFER_SIZE];

WiFiClient client;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("WiFi lost, reconnecting...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected Successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void recordHeartbeat() {
  Serial.println("Recording 20 sec - hold stethoscope on chest");
  digitalWrite(LED_PIN, LOW);

  for (int i = 0; i < BUFFER_SIZE; i++) {
    rawSamples[i] = analogRead(MIC_PIN);
    delay(SAMPLE_MS);
  }

  digitalWrite(LED_PIN, HIGH);
  Serial.println("Recording finished");
}

void applySMA() {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    long sum = 0;
    int count = 0;

    for (int j = i - SMA_WINDOW + 1; j <= i; j++) {
      if (j < 0) {
        continue;
      }
      sum += rawSamples[j];
      count++;
    }

    filteredSamples[i] = (sum / (float)count) / 1023.0;
  }
}

int countBeats(int& maxRaw, int& avgRaw) {
  long sum = 0;
  maxRaw = 0;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    sum += rawSamples[i];
    if (rawSamples[i] > maxRaw) {
      maxRaw = rawSamples[i];
    }
  }

  avgRaw = sum / BUFFER_SIZE;

  int beats = 0;
  int lastBeat = -MIN_BEAT_GAP;
  bool aboveThreshold = false;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    float rawNorm = rawSamples[i] / 1023.0;

    if (!aboveThreshold && rawNorm > THRESHOLD && (i - lastBeat) >= MIN_BEAT_GAP) {
      beats++;
      lastBeat = i;
      aboveThreshold = true;
    }

    if (rawNorm < THRESHOLD - 0.05) {
      aboveThreshold = false;
    }
  }

  return beats;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(MIC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.println();
  Serial.print("Connecting to Network: ");
  Serial.println(ssid);

  connectWiFi();
  ThingSpeak.begin(client);
}

void loop() {
  connectWiFi();

  recordHeartbeat();
  applySMA();

  int maxRaw, avgRaw;
  int beats = countBeats(maxRaw, avgRaw);
  int bpm = 0;

  if (beats >= 1) {
    bpm = (beats * 60) / RECORD_SEC;
  }

  Serial.print("Microphone Sensor Value: ");
  Serial.println(avgRaw);
  Serial.print("Beats detected: ");
  Serial.println(beats);
  Serial.print("Heart rate (BPM): ");
  Serial.println(bpm);
  Serial.print("Peak amplitude: ");
  Serial.println(maxRaw);

  // send results to thingspeak for doctor to check later
  ThingSpeak.setField(1, bpm);
  ThingSpeak.setField(2, beats);
  ThingSpeak.setField(3, avgRaw);
  ThingSpeak.setField(4, maxRaw);

  int httpCode = ThingSpeak.writeFields(channelID, writeKey);

  if (httpCode == 200) {
    Serial.println("ThingSpeak Channel update successful.");
  } else {
    Serial.print("Problem updating channel. HTTP error code: ");
    Serial.println(httpCode);
  }

  Serial.println("(Waiting for next reading...)");
  delay(15000);
}
