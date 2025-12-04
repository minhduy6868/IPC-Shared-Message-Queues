# 🚀 IPC Smart Message Queue System

## 📋 Mô Tả Dự Án
Hệ thống IPC (Inter-Process Communication) nâng cao sử dụng **Linux Message Queues** để giao tiếp giữa User và Bot. Tích hợp **Gemini AI 2.0 Flash** cho tính năng tóm tắt văn bản thông minh.

---

## ✨ Tính Năng Nâng Cấp

### 🎨 **Giao Diện User (user.c)**
- ✅ **ASCII Art Banner** - Logo chuyên nghiệp với Unicode box drawing
- ✅ **Loading Animation** - Hiệu ứng spinner khi kết nối
- ✅ **Color-Coded Menu** - Menu phân màu dễ đọc với emoji icons
- ✅ **Progress Indicators** - Hiển thị trạng thái gửi/nhận real-time
- ✅ **Smart File Naming** - Tự động đặt tên file với timestamp
- ✅ **Enhanced Error Messages** - Thông báo lỗi rõ ràng và hữu ích
- ✅ **Response Boxing** - Format kết quả trong khung đẹp mắt

### 🤖 **Bot Backend (bot.c)**
- ✅ **Activity Logging** - Ghi log tất cả hoạt động vào `bot_activity.log`
- ✅ **Statistics Tracking** - Theo dõi số lượng request theo từng loại
- ✅ **Session Summary** - Hiển thị thống kê khi thoát
- ✅ **Formatted Responses** - Output có khung và icon cho mỗi chức năng
- ✅ **Timestamp Logging** - Mỗi request có timestamp chi tiết
- ✅ **Better Error Handling** - Xử lý lỗi chi tiết hơn với logging

---

## 📦 3 Chức Năng Chính

### 1️⃣ **System Info + Top Processes**
```
📊 Hiển thị:
- OS và kernel version
- CPU load percentage
- RAM usage (used/total)
- System uptime
- Top 10 processes theo CPU
```

### 2️⃣ **Weather Forecast**
```
🌤️ Tính năng:
- Lấy thông tin thời tiết từ OpenWeatherMap API
- Hiển thị nhiệt độ và mô tả thời tiết
- Mặc định: Turan (có thể thay đổi)
```

### 3️⃣ **AI File Summarizer**
```
🤖 Powered by Gemini AI 2.0:
- Đọc nội dung file (max 1MB)
- Tóm tắt ngắn gọn bằng tiếng Việt
- Tùy chọn lưu kết quả vào file .txt
- Format file: summary_<filename>_HHMMSS.txt
```

---

## 🔧 Biên Dịch & Chạy

### Yêu Cầu:
```bash
sudo apt-get install libcurl4-openssl-dev
```

### Compile:
```bash
# Compile bot
gcc bot.c -o bot -lcurl -lm

# Compile user
gcc user.c -o user
```

### Chạy:
```bash
# Terminal 1: Khởi động bot trước
./bot

# Terminal 2: Chạy user client
./user
```

---

## 📊 Log File Format

File `bot_activity.log` ghi lại:
```
[2025-12-04 14:30:15] BOT_START: IPC Bot initialized successfully
[2025-12-04 14:30:22] REQUEST: get_system_info
[2025-12-04 14:30:22] SYSTEM_INFO: Generated system report
[2025-12-04 14:30:22] RESPONSE: Sent back to user
[2025-12-04 14:31:05] REQUEST: get_weather:Hanoi
[2025-12-04 14:31:06] WEATHER: Hanoi
[2025-12-04 14:31:06] RESPONSE: Sent back to user
```

---

## 🎯 Các Cải Tiến So Với Bản Gốc

| Tính Năng | Bản Cũ | Bản Mới ⭐ |
|-----------|--------|-----------|
| **Giao Diện** | Text đơn giản | ASCII art + màu sắc + emoji |
| **Error Handling** | Cơ bản | Chi tiết với màu và icon |
| **Logging** | Không có | File log đầy đủ với timestamp |
| **Statistics** | Không có | Theo dõi và tổng kết session |
| **Response Format** | Plain text | Boxed layout với icons |
| **File Naming** | Tĩnh | Động với timestamp |
| **Loading Feedback** | Không có | Spinner + progress dots |
| **User Experience** | Functional | Professional & Modern |

---

## 📸 Screenshots Mô Phỏng

### User Interface:
```
  ██╗██████╗  ██████╗     ███████╗███╗   ███╗ █████╗ ██████╗ ████████╗
  ██║██╔══██╗██╔════╝     ██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝
  ██║██████╔╝██║          ███████╗██╔████╔██║███████║██████╔╝   ██║   
  ██║██╔══╝  ██║          ╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║   
  ██║██║     ╚██████╗     ███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║   
  ╚═╝╚═╝      ╚═════╝     ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   
               ╔══════════════════════════════════════╗
               ║  Message Queue Communication System ║
               ║  Powered by Linux IPC & Gemini AI  ║
               ╚══════════════════════════════════════╝

  ╔════════════════════════════════════════════════════╗
  ║              🤖 MAIN MENU 🤖                   ║
  ╟────────────────────────────────────────────────────╢
  ║  [1] 🖥️  System Info + Top Processes          ║
  ║  [2] 🌤️  Weather Forecast (Default: Turan)   ║
  ║  [3] 📄 AI File Summarizer (Gemini 2.0)    ║
  ║  [0] 🚪 Exit Program                        ║
  ╚════════════════════════════════════════════════════╝
  ➤ Your choice: 
```

### Bot Output:
```
╔═══════════════════════════════════════════════════════╗
║  Status: ONLINE                                     ║
║  Queue ID: 12345                                    ║
║  Log file: bot_activity.log                         ║
║  Protocol: User(mtype=1) → Bot → User(mtype=2)     ║
╚═══════════════════════════════════════════════════════╝

▶️ Waiting for requests...

[↓ 14:30:22] Request #1: 'get_system_info'
[✓] Processed: get_system_info
[↑] Sent response (458 chars)

╔═══════════════════════════════════════════════════════╗
║               SESSION STATISTICS                    ║
╟───────────────────────────────────────────────────────╢
║  Total Requests:        5                        ║
║  System Info:           2                        ║
║  Weather Queries:       2                        ║
║  File Summaries:        1                        ║
║  Runtime:               3m 45s                     ║
╚═══════════════════════════════════════════════════════╝
```

---

## 🎓 Ý Nghĩa Học Tập

Dự án này minh họa:
- ✅ **IPC Messaging** - Message Queues trong Linux
- ✅ **Process Synchronization** - Đồng bộ giữa User/Bot
- ✅ **API Integration** - Gemini AI & OpenWeatherMap
- ✅ **Error Handling** - Xử lý lỗi trong môi trường đa tiến trình
- ✅ **Logging & Monitoring** - Theo dõi hoạt động hệ thống
- ✅ **User Experience Design** - Giao diện terminal chuyên nghiệp

---

## 👨‍💻 Author
**Dự án bài tập lớn IPC - Linux System Programming**

---

## 📝 License
Educational Project - Free to use and modify
