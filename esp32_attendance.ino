#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPIFFS.h>
#include <time.h>
#include <map>

const char* ssid = "VAC";
const char* password = "sonuSameshitha";

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

#define TOTAL_STUDENTS 40
int totalPresent = 0;

const char* attendanceFile = "/trialattendance.csv";

struct AttendanceRecord {
  time_t loginTime = 0;
  time_t logoutTime = 0;
};

std::map<String, AttendanceRecord> attendanceRecords;

String getNameByUSN(String usn) {
  if (usn == "1AT24CD001") return "Raghu";
  if (usn == "1AT24CD002") return "Chetan Achar";
  if (usn == "1AT24CD003") return "Yathin";
  if (usn == "1AT24CD004") return "Chinna";
  if (usn == "1AT24CD005") return "Navya";
  if (usn == "1AT24CD006") return "Pooja";
  if (usn == "1AT24CD007") return "Akshay";
  if (usn == "1AT24CD008") return "Niranjan";
  if (usn == "1AT24CD009") return "Sneha";
  if (usn == "1AT24CD010") return "Shankaline";
  if (usn == "1AT24CD011") return "Harish";
  if (usn == "1AT24CD012") return "Hema";
  if (usn == "1AT24CD013") return "Chinmay";
  if (usn == "1AT24CD014") return "Praveen";
  if (usn == "1AT24CD015") return "Keerthana";
  if (usn == "1AT24CD016") return "Likitha";
  if (usn == "1AT24CD017") return "Jashwanth";
  if (usn == "1AT24CD018") return "Ganesh";
  if (usn == "1AT24CD019") return "Neeraj";
  if (usn == "1AT24CD020") return "Yash";
  if (usn == "1AT24CD021") return "Nithin";
  if (usn == "1AT24CD022") return "Pooja";
  if (usn == "1AT24CD023") return "Siddharth";
  if (usn == "1AT24CD024") return "Laksh";
  if (usn == "1AT24CD025") return "Rahul";
  if (usn == "1AT24CD026") return "Sowmya";
  if (usn == "1AT24CD027") return "Rohan";
  if (usn == "1AT24CD028") return "Tanya";
  if (usn == "1AT24CD029") return "Isha";
  if (usn == "1AT24CD030") return "Vikram";
  if (usn == "1AT24CD031") return "Nandini";
  if (usn == "1AT24CD032") return "Anusha";
  if (usn == "1AT24CD033") return "Shruti";
  if (usn == "1AT24CD034") return "Vijay";
  if (usn == "1AT24CD035") return "Sanjay";
  return "Unknown";
}

void handleAttendance() {
  if (!server.hasArg("usn")) {
    lcd.clear();
    lcd.print("Missing USN");
    server.send(400, "text/plain", "Missing USN");
    return;
  }

  String usn = server.arg("usn");
  String name = getNameByUSN(usn);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(usn.length() > 16 ? usn.substring(0, 16) : usn);
  lcd.setCursor(0, 1);
  lcd.print(name.length() > 16 ? name.substring(0, 16) : name);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    server.send(500, "text/plain", "Time error");
    return;
  }

  time_t now = mktime(&timeinfo);
  time_t midnight = now - (timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec);

  AttendanceRecord &record = attendanceRecords[usn];

  File file = SPIFFS.open(attendanceFile, FILE_APPEND);
  if (!file) {
    server.send(500, "text/plain", "File error");
    return;
  }

  if (file.size() == 0) {
    file.printf("USN,Name,Type,Timestamp\n");
  }

  if (record.loginTime < midnight) {
    record.loginTime = now;
    record.logoutTime = 0;

    file.printf("%s,%s,Login,%ld\n", usn.c_str(), name.c_str(), now);
    Serial.printf("Login marked: %s %s\n", usn.c_str(), name.c_str());

    server.send(200, "text/plain", "Login recorded");
  } else if (record.logoutTime == 0) {
    record.logoutTime = now;

    file.printf("%s,%s,Logout,%ld\n", usn.c_str(), name.c_str(), now);
    Serial.printf("Logout marked: %s %s\n", usn.c_str(), name.c_str());

    server.send(200, "text/plain", "Logout recorded");
  } else {
    server.send(200, "text/plain", "Already logged today");
  }

  file.close();
}

void handleDownload() {
  File file = SPIFFS.open(attendanceFile, FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/csv");
  file.close();
}

void handleClear() {
  SPIFFS.remove(attendanceFile);
  server.send(200, "text/plain", "File cleared");
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    lcd.print("SPIFFS Fail");
    return;
  }

  WiFi.begin(ssid, password);
  lcd.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("Connected");

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");

  server.on("/attendance", handleAttendance);
  server.on("/download", handleDownload);
  server.on("/clear", handleClear);

  server.begin();
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.print("Ready to Scan");
}

void loop() {
  server.handleClient();
}
