#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <WiFi.h>
#include <HTTPClient.h>

// Cấu hình WiFi
const char* ssid = "Hoang Van Thoan";
const char* password = "66778899";

// URL file cần tải từ Google Drive hoặc github
const char* fileURL = "https://raw.githubusercontent.com/thoan9k/Update_firmware_OTA/main/hello.txt";
//https://drive.google.com/file/d/1__VMmdXdNzLhVKFZ8mkK2Ql_RdlmFkZ7/view?usp=sharing
//https://drive.google.com/file/d/1XCSFpM0-HctHF3a1fdo9JJAzk0HE7GpR/view?usp=sharing

// Base URL cho GitHub API
const char* repoAPI = "https://api.github.com/repos/thoan9k/Update_firmware_OTA/commits/main";
const char* baseURL = "https://raw.githubusercontent.com/thoan9k/Update_firmware_OTA/";
const char* firmwarefile = "/hello.txt";
const char* versionfile = "/version.json";
// Phiên bản hiện tại của ESP32
const char* current_version = "1.0.2";

void checkForUpdate() {
   
  String commitHash = getLatestCommitHash();
  if (commitHash.length() > 0)
  {
    String latestURL = String(baseURL) + commitHash + versionfile;
    HTTPClient http;
    http.begin(latestURL);  // link JSON
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        const char* new_version = doc["version"];
        // const char* firmware_url = doc["firmware"];

        Serial.println("Version trên server: " + String(new_version));
        if (String(new_version) != current_version) {
          Serial.println("Phát hiện phiên bản mới! Đang tải firmware...");
          downloadLatestFile(firmwarefile);
          // performFirmwareUpdate(firmware_url);
        } else {
          Serial.println("Đã là phiên bản mới nhất.");
        }
      }
    } else {
      Serial.println("Lỗi tải version.json, mã lỗi: " + String(httpCode));
    }
    http.end();
  }
 
}
String getLatestCommitHash() {
  HTTPClient http;
  http.begin(repoAPI);
  http.addHeader("User-Agent", "ESP32-HTTPClient");
  
  int httpCode = http.GET();
  String commitHash = "";
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    // Tìm commit hash trong JSON response
    int shaIndex = payload.indexOf("\"sha\":\"");
    if (shaIndex != -1) {
      commitHash = payload.substring(shaIndex + 7, shaIndex + 47); // SHA có 40 ký tự
      Serial.print("Latest commit: ");
      Serial.println(commitHash);
    }
  }
  
  http.end();
  return commitHash;
}
void downloadLatestFile(const char* fileName) {
  String commitHash = getLatestCommitHash();
  
  if (commitHash.length() > 0) {
    String latestURL = String(baseURL) + commitHash + fileName;
    Serial.print("URL với commit hash: ");
    Serial.println(latestURL);
    
    // Tải file với URL mới
    HTTPClient http;
    http.begin(latestURL.c_str());
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String content = http.getString();
      Serial.println("=== NỘI DUNG FILE MỚI NHẤT ===");
      Serial.println(content);
      Serial.println("==============================");
    }
    
    http.end();
  } else {
    Serial.println("Không lấy được commit hash, dùng URL cũ...");
    downloadFile(); // Fallback
  }
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Bắt đầu kết nối WiFi...");
  
  // Kết nối WiFi
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi đã kết nối!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
  
  // Tải file
  // downloadFile();
  // downloadLatestFile();
}

void loop() {
  // Không làm gì trong loop
  delay(10000);
}

void downloadFile() {
  HTTPClient http;
  
  Serial.println("Bắt đầu tải file...");
  Serial.print("URL: ");
  Serial.println(fileURL);
  
  // Khởi tạo HTTP request
  http.begin(fileURL);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  // Thêm header nếu cần
  http.addHeader("User-Agent", "ESP32-HTTPClient");
  http.addHeader("Accept", "text/plain, */*");
  http.addHeader("Cache-Control", "no-cache");
  
  // Gửi GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    
    // In ra headers để debug
    Serial.println("=== RESPONSE HEADERS ===");
    Serial.print("Content-Type: ");
    Serial.println(http.header("Content-Type"));
    Serial.print("Content-Length: ");
    Serial.println(http.header("Content-Length"));
    Serial.print("Location: ");
    Serial.println(http.header("Location"));
    Serial.println("========================");
    
    if (httpResponseCode == HTTP_CODE_OK) {
      // Lấy độ dài nội dung
      int contentLength = http.getSize();
      Serial.print("Kích thước file: ");
      Serial.print(contentLength);
      Serial.println(" bytes");
      
      // Đọc và in nội dung file
      String payload = http.getString();
      Serial.println("=== NỘI DUNG FILE ===");
      Serial.println(payload);
      Serial.println("=== KẾT THÚC FILE ===");
      
      Serial.print("Đã tải thành công ");
      Serial.print(payload.length());
      Serial.println(" ký tự");
    } else {
      Serial.print("Lỗi HTTP: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.print("Lỗi kết nối: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }
  
  // Đóng kết nối
  http.end();
}

// Hàm tải file lớn với buffer (cho file kích thước lớn)
void downloadLargeFile(const char* url) {
  HTTPClient http;
  
  Serial.println("Bắt đầu tải file lớn...");
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();
    int len = http.getSize();
    
    Serial.print("Kích thước file: ");
    Serial.println(len);
    Serial.println("=== BẮT ĐẦU NỘI DUNG ===");
    
    uint8_t buff[128] = { 0 };
    int totalRead = 0;
    
    while (http.connected() && (len > 0 || len == -1)) {
      size_t size = stream->available();
      
      if (size) {
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        
        // In ra Serial
        for (int i = 0; i < c; i++) {
          Serial.write(buff[i]);
        }
        
        totalRead += c;
        
        if (len > 0) {
          len -= c;
        }
      }
      delay(1);
    }
    
    Serial.println("\n=== KẾT THÚC NỘI DUNG ===");
    Serial.print("Tổng cộng đã đọc: ");
    Serial.print(totalRead);
    Serial.println(" bytes");
  } else {
    Serial.printf("Lỗi HTTP: %d\n", httpCode);
  }
  
  http.end();
}