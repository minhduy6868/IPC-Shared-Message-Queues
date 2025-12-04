# 🚀 IPC Smart Message Queue System

## 📚 Lý Thuyết Cơ Bản

### 🔹 **Inter-Process Communication (IPC)**
IPC là cơ chế cho phép các tiến trình (processes) giao tiếp và trao đổi dữ liệu với nhau trong hệ điều hành. Linux cung cấp nhiều phương pháp IPC, trong đó **Message Queues** là một trong những cơ chế phổ biến nhất.

### 🔹 **Message Queues - Hàng Đợi Thông Điệp**

**Định nghĩa:**
- Message Queue là một danh sách liên kết các thông điệp được lưu trữ trong kernel
- Mỗi message có một **type** (mtype) và **data** (mtext)
- Processes có thể gửi (send) và nhận (receive) messages thông qua queue ID

**Đặc điểm:**
- ✅ **Bất đồng bộ (Asynchronous):** Sender không cần đợi receiver nhận message
- ✅ **FIFO theo type:** Messages cùng type được xử lý theo thứ tự First-In-First-Out
- ✅ **Persistent:** Message tồn tại trong kernel cho đến khi được đọc hoặc xóa
- ✅ **Type-based filtering:** Receiver có thể chọn lọc message theo mtype

**System Calls chính:**
```c
ftok()     // Tạo unique key từ file path
msgget()   // Tạo hoặc truy cập message queue
msgsnd()   // Gửi message vào queue
msgrcv()   // Nhận message từ queue
msgctl()   // Kiểm soát và xóa queue
```

---

## 🏗️ Kiến Trúc Hệ Thống

```
┌─────────────────────────────────────────────────────────────┐
│                    LINUX KERNEL SPACE                       │
│                                                             │
│    ┌───────────────────────────────────────────────┐       │
│    │      MESSAGE QUEUE (Queue ID: xxxxx)          │       │
│    │  ┌─────────────────────────────────────────┐  │       │
│    │  │ Type=1 │ User Request: "get_weather"   │  │       │
│    │  ├─────────────────────────────────────────┤  │       │
│    │  │ Type=2 │ Bot Response: "Temp: 25°C..." │  │       │
│    │  └─────────────────────────────────────────┘  │       │
│    └───────────────────────────────────────────────┘       │
│            ▲                           │                    │
└────────────┼───────────────────────────┼────────────────────┘
             │ msgsnd(mtype=1)           │ msgrcv(mtype=2)
             │                           ▼
┌────────────┴────────────┐    ┌────────────────────────┐
│   USER PROCESS          │    │   BOT PROCESS          │
│   (user.c)              │    │   (bot.c)              │
│                         │    │                        │
│ 1. Hiển thị menu        │    │ 1. Chờ request         │
│ 2. Nhận input           │    │ 2. Xử lý logic         │
│ 3. Gửi request (mtype=1)│    │ 3. Gọi API/System      │
│ 4. Chờ response         │    │ 4. Gửi response        │
│ 5. Nhận kết quả(mtype=2)│    │    (mtype=2)           │
│ 6. Hiển thị cho user    │    │ 5. Logging & Stats     │
└─────────────────────────┘    └────────────────────────┘
```

---

## 📦 Phân Tích 3 Chức Năng Chi Tiết

### 1️⃣ **System Information & Top Processes**

**Lý thuyết:**
- Sử dụng `sysinfo()` system call để lấy thông tin hệ thống từ kernel
- Sử dụng `uname()` để lấy thông tin OS và kernel version
- Parse `/proc` filesystem để lấy danh sách processes
- Sử dụng `popen()` để chạy shell command `ps aux --sort=-%cpu`

**Luồng xử lý:**
```
User → "get_system_info" → Bot → sysinfo() + uname() + ps command
                                → Format output với box drawing
                                → Response về User
```

**Cấu trúc dữ liệu:**
```c
struct sysinfo {
    long uptime;        // Thời gian hệ thống chạy (seconds)
    unsigned long totalram;  // Tổng RAM
    unsigned long freeram;   // RAM trống
    // ... các trường khác
};
```

---

### 2️⃣ **Weather Forecast (API Integration)**

**Lý thuyết:**
- HTTP REST API call sử dụng thư viện **libcurl**
- OpenWeatherMap API endpoint: `api.openweathermap.org/data/2.5/weather`
- JSON parsing thủ công (không dùng thư viện JSON)
- URL encoding cho city name

**Luồng xử lý:**
```
User input city → Bot → curl_easy_init()
                      → HTTP GET request với API key
                      → Nhận JSON response
                      → Parse "temp" và "description" fields
                      → Format output
                      → Response về User
```

**CURL callbacks:**
```c
write_cb()  // Callback function để nhận data từ HTTP response
            // Được gọi mỗi khi có chunk data mới
```

**JSON Response Example:**
```json
{
  "main": {"temp": 25.5},
  "weather": [{"description": "clear sky"}]
}
```

---

### 3️⃣ **AI File Summarizer (Gemini AI Integration)**

**Lý thuyết:**
- Đọc file binary-safe với `fread()`
- Giới hạn file size (1MB) để tránh overflow
- REST API call đến Google Gemini AI API
- JSON prompt engineering cho tóm tắt tiếng Việt
- Escape sequence handling (`\n` trong JSON)

**Luồng xử lý:**
```
User nhập path → Bot → fopen() + fread()
                     → Kiểm tra file size
                     → Tạo JSON prompt với nội dung file
                     → POST request đến Gemini API
                     → Parse response JSON
                     → Xử lý escape sequences
                     → Response về User
                     → User có thể lưu vào file
```

**Gemini API Request Format:**
```json
{
  "contents": [{
    "parts": [{
      "text": "Tóm tắt ngắn gọn bằng tiếng Việt, dưới 100 từ:\n\n<nội dung file>"
    }]
  }]
}
```

**File I/O:**
```c
fseek(f, 0, SEEK_END);  // Di chuyển đến cuối file
long len = ftell(f);     // Lấy kích thước file
rewind(f);               // Quay về đầu file
fread(buf, 1, len, f);   // Đọc toàn bộ file
```

---

## 🔐 Cơ Chế Đồng Bộ Hóa

### **Key Generation với ftok()**
```c
key_t key = ftok("keyfile", 'a');
```
- Tạo unique key từ file path và project ID
- Đảm bảo User và Bot sử dụng cùng một queue
- File "keyfile" phải tồn tại trước khi chạy

### **Message Type Protocol**
```
mtype = 1: User Request  (User → Bot)
mtype = 2: Bot Response  (Bot → User)
```

### **Blocking vs Non-blocking**
```c
msgrcv(msgid, &msg, size, 1, 0);  // Block cho đến khi có message type=1
msgrcv(msgid, &msg, size, 2, IPC_NOWAIT);  // Non-blocking (không dùng trong code này)
```

---

## 📊 Logging & Statistics

### **Activity Logging**
```c
log_activity(const char *event, const char *details);
```
- Ghi tất cả hoạt động vào `bot_activity.log`
- Format: `[YYYY-MM-DD HH:MM:SS] EVENT: Details`
- Theo dõi: Khởi động, Request, Response, Lỗi, Thoát

### **Statistics Tracking**
```c
typedef struct {
    int total_requests;      // Tổng số request
    int system_info_count;   // Số lần gọi system info
    int weather_count;       // Số lần gọi weather
    int summarize_count;     // Số lần tóm tắt file
    time_t start_time;       // Thời điểm khởi động
} BotStats;
```

---

## 🎨 Giao Diện & UX Enhancements

### **ASCII Art Banner**
- Sử dụng Unicode Box Drawing Characters (U+2500 - U+257F)
- ANSI Color Codes cho màu sắc terminal

### **ANSI Color Codes**
```c
\033[1;32m  // Green Bold
\033[1;31m  // Red Bold
\033[0m     // Reset
```

### **Loading Animation**
```c
const char *spinner = "|/-\\";  // 4 frames animation
usleep(100000);  // 100ms delay per frame
```

---

## 🛠️ Memory Management & Security

### **Buffer Overflow Prevention**
```c
#define MAX_TEXT 4096
strncpy(msg.mtext, request, MAX_TEXT - 1);
msg.mtext[MAX_TEXT - 1] = '\0';  // Null-termination
```

### **Dynamic Memory Allocation**
```c
char *content = malloc(len + 1);  // +1 cho null terminator
// ... sử dụng content
free(content);  // Giải phóng bộ nhớ
```

### **File Size Limit**
```c
#define MAX_FILE_SIZE (1024 * 1024)  // 1MB
if (len > MAX_FILE_SIZE) {
    // Từ chối đọc file quá lớn
}
```

---

## ⚙️ Hướng Dẫn Sử Dụng

### **Yêu Cầu Hệ Thống:**
- Linux OS (Ubuntu, Debian, CentOS, etc.)
- GCC Compiler
- libcurl development library

### **Cài Đặt Dependencies:**
```bash
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev
```

### **Biên Dịch:**
```bash
# Compile Bot Backend
gcc bot.c -o bot -lcurl -lm

# Compile User Client
gcc user.c -o user
```

### **Chạy Chương Trình:**
```bash
# Terminal 1: Khởi động Bot trước (phải chạy trước User)
./bot

# Terminal 2: Chạy User Client
./user
```

### **Xóa Message Queue thủ công (nếu cần):**
```bash
# Xem các queue đang tồn tại
ipcs -q

# Xóa queue theo ID
ipcrm -q <queue_id>
```

---

## 🔍 Debugging & Troubleshooting

### **Lỗi thường gặp:**

**1. "msgget failed"**
```
Nguyên nhân: File "keyfile" không tồn tại
Giải pháp: Bot tự động tạo keyfile khi khởi động
```

**2. "Connection failed"**
```
Nguyên nhân: Bot chưa được khởi động
Giải pháp: Chạy ./bot trước, sau đó mới chạy ./user
```

**3. "CURL error"**
```
Nguyên nhân: Thiếu thư viện libcurl
Giải pháp: sudo apt-get install libcurl4-openssl-dev
```

### **Kiểm tra Log:**
```bash
# Xem log realtime
tail -f bot_activity.log

# Xem toàn bộ log
cat bot_activity.log
```

---

## 📚 Kiến Thức Liên Quan

### **System Calls sử dụng:**
- `ftok()` - File to Key conversion
- `msgget()` - Message queue creation/access
- `msgsnd()` - Send message
- `msgrcv()` - Receive message
- `msgctl()` - Message queue control
- `sysinfo()` - System information
- `uname()` - System name information
- `popen()` - Process pipe
- `fopen()`, `fread()`, `fseek()` - File I/O

### **Libraries sử dụng:**
- **stdio.h** - Standard Input/Output
- **stdlib.h** - Memory allocation, process control
- **string.h** - String manipulation
- **sys/ipc.h** - IPC mechanisms
- **sys/msg.h** - Message queues
- **curl/curl.h** - HTTP client library
- **time.h** - Time and date functions

### **Concepts áp dụng:**
- ✅ Inter-Process Communication
- ✅ Message Passing
- ✅ REST API Integration
- ✅ JSON Parsing
- ✅ File I/O Operations
- ✅ Memory Management
- ✅ Error Handling
- ✅ Logging System
- ✅ Terminal UI Design

---

## 🎯 Kết Luận

Dự án này là một ví dụ hoàn chỉnh về:
- **IPC Message Queues** trong Linux System Programming
- Tích hợp **External APIs** (Weather, AI)
- **Process Synchronization** giữa Client-Server
- **User Experience** trong terminal applications
- **Production-ready features**: Logging, Statistics, Error Handling

Phù hợp làm bài tập lớn môn Hệ Điều Hành hoặc Linux System Programming.

---

## 👨‍💻 Tác Giả
**Dự án Bài Tập Lớn - Linux IPC Message Queues**

## 📝 License
Educational Project - Free to use and modify
