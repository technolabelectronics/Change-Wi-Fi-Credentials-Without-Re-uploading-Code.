#include <WiFi.h>
#include <WebServer.h>

const char* apSSID = "technolab_Config";
const char* apPassword = "12345678";

WebServer server(80);

// Store SSID and password
String ssid = "";
String password = "";

// GPIO for the EN button
const int enButtonPin = 0; // GPIO0
bool apMode = false;

void setup() {
  pinMode(enButtonPin, INPUT_PULLUP);
  Serial.begin(115200);

  // Try connecting to the saved Wi-Fi credentials
  WiFi.begin(ssid.c_str(), password.c_str());
  int retries = 20;
  while (WiFi.status() != WL_CONNECTED && retries-- > 0) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed!");
  }
}

void loop() {
  // Check if the EN button is held for 5 seconds
  if (digitalRead(enButtonPin) == LOW) {
    delay(5000);
    if (digitalRead(enButtonPin) == LOW) {
      enterAccessPointMode();
    }
  }

  if (apMode) {
    server.handleClient();
  }
}

void enterAccessPointMode() {
  Serial.println("Entering Access Point Mode...");
  apMode = true;

  // Set up Access Point
  WiFi.softAP(apSSID, apPassword);
  Serial.println("Access Point started.");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define routes
  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);

  server.begin();
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <body>
      <h2>Wi-Fi Configuration</h2>
      <form action="/submit">
        SSID:<br>
        <input type="text" name="ssid"><br>
        Password:<br>
        <input type="password" name="password"><br><br>
        <input type="submit" value="Submit">
      </form>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleSubmit() {
  ssid = server.arg("ssid");
  password = server.arg("password");

  Serial.println("Received Wi-Fi credentials:");
  Serial.println("SSID: " + ssid);
  Serial.println("Password: " + password);

  // Stop AP mode
  WiFi.softAPdisconnect(true);
  apMode = false;
  server.stop();

  // Connect to the provided Wi-Fi credentials
  WiFi.begin(ssid.c_str(), password.c_str());
  int retries = 20;
  while (WiFi.status() != WL_CONNECTED && retries-- > 0) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed!");
  }

  // Respond to the client
  server.send(200, "text/html", "<h2>Credentials received. Trying to connect...</h2>");
}
