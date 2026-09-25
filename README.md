# ⚡ Auto-Meca Firmware | ESP32-S3 Mecanum Motion Controller

<div align="center">

[![Board](https://img.shields.io/badge/Board-ESP32--S3-red?style=for-the-badge&logo=espressif&logoColor=white)](https://www.espressif.com)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-orange?style=for-the-badge&logo=platformio&logoColor=white)](https://platformio.org)
[![Communication](https://img.shields.io/badge/Protocol-UDP%20Socket-blue?style=for-the-badge)](https://en.wikipedia.org/wiki/User_Datagram_Protocol)
[![Android Station](https://img.shields.io/badge/Paired%20With-Auto--Meca%20Android-green?style=for-the-badge&logo=android&logoColor=white)](https://github.com/thanhquan123hi1/auto-meca)

**Firmware điều khiển phần cứng cho xe 4 bánh Mecanum tự hành chạy trên vi điều khiển ESP32-S3, hỗ trợ phát Wi-Fi SoftAP, nhận lệnh điều hướng qua UDP, tích hợp cảm biến siêu âm HC-SR04 chống va chạm và cơ chế Failsafe tự động.**

[🔗 Liên kết Android Station](#-lien-ket-app-android-thi-giac) • [🔌 Sơ đồ chân Pinout](#-so-do-chan-pinout--phan-cung) • [📡 Giao thức mạng](#-cau-hinh-mang--giao-thuc-udp) • [🛡️ Cơ chế an toàn Failsafe](#-co-che-an-toan-failsafe) • [🚀 Nạp chương trình](#-huong-dan-bien-dich--nap-firmware)

</div>

---

## 📌 Giới thiệu & Vai trò

Firmware này được thiết kế để chạy trên kit **ESP32-S3**, đóng vai trò là tầng chấp hành phần cứng (Low-Level Motion Execution) của hệ thống xe tự hành Auto-Meca:
1. **Phát mạng Wi-Fi SoftAP độc lập:** Tự tạo điểm truy cập mạng nội bộ `Mecanum-Car` để điện thoại Android và Laptop kết nối trực tiếp, không cần router Wi-Fi ngoài.
2. **Lắng nghe gói lệnh UDP:** Nhận gói tin điều khiển 1 byte ASCII tại cổng `4210` từ ứng dụng Android với tần suất `100ms/lần`.
3. **Điều khiển chuyển động Mecanum đa hướng:** Tính toán vận tốc và hướng quay của 4 động cơ độc lập (Front-Left, Front-Right, Rear-Left, Rear-Right).
4. **Smooth Ramp PWM:** Khởi động mềm (soft-start) và đổi hướng mềm giúp giảm sụt áp nguồn và chống giật xe làm rung camera điện thoại.
5. **Bảo vệ va chạm phần cứng (HC-SR04):** Tự động chặn lệnh tiến nếu gặp chướng ngại vật sát gần ($\le 30\text{ cm}$) và lùi khẩn cấp nếu quá gần ($\le 12\text{ cm}$).
6. **Watchdog Timeout:** Dừng toàn bộ động cơ nếu quá `600ms` không nhận được lệnh hợp lệ từ điện thoại.

---

## 🔗 Liên kết App Android Thị Giác

Firmware này nhận lệnh điều khiển mức cao từ ứng dụng thị giác máy tính chạy trên điện thoại Android:

| Dự án | Link Kho mã nguồn | Trách nhiệm |
| :--- | :--- | :--- |
| **Android Vision Station** | [**thanhquan123hi1/auto-meca**](https://github.com/thanhquan123hi1/auto-meca) (hoặc `../auto-meca`) | CameraX, AI YOLO né vật cản, OpenCV ArUco bắt Marker ID 0, tính toán quyết định và gửi UDP |
| **ESP32-S3 Firmware** | [**thanhquan123hi1/auto_meca_firmware**](https://github.com/thanhquan123hi1/auto_meca_firmware) | Driver động cơ 4 bánh, phát AP, Failsafe và cảm biến siêu âm |

---

## 🔌 Sơ đồ chân Pinout & Phần cứng

Cấu hình chân được quy định chi tiết trong file [`include/config.h`](include/config.h):

### 1. Điều khiển Động cơ (2x Dual H-Bridge Driver)

| Bánh xe | Ký hiệu | Pin IN1 | Pin IN2 | Kênh PWM LEDC | Đảo chiều phần mềm |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Trước Trái (Front Left)** | `FL` | **GPIO 5** | **GPIO 4** | Kênh 0, 1 | `false` |
| **Trước Phải (Front Right)** | `FR` | **GPIO 17** | **GPIO 16** | Kênh 2, 3 | `false` |
| **Sau Trái (Rear Left)** | `RL` | **GPIO 18** | **GPIO 15** | Kênh 4, 5 | `true` |
| **Sau Phải (Rear Right)** | `RR` | **GPIO 14** | **GPIO 13** | Kênh 6, 7 | `true` |

*(Tần số PWM: 20kHz, độ phân giải 8-bit [0 - 255]).*

### 2. Cảm biến siêu âm HC-SR04 (Obstacle Guard)
- **Chân Trigger:** `GPIO 20`
- **Chân Echo:** `GPIO 12`
- **Chu kỳ đo:** `60ms/lần`, áp dụng bộ lọc trung vị (Median Filter 5 mẫu).

---

## 📡 Cấu hình mạng & Giao thức UDP

### 1. Thông số Wi-Fi SoftAP
```text
SSID:             Mecanum-Car
Mật khẩu:         12345678
Kênh Wi-Fi:       Channel 1 (2.4GHz)
Địa chỉ IP ESP32: 192.168.4.1
Cổng UDP nhận:    4210
```

### 2. Tập lệnh ASCII nhận từ Android
Mỗi gói tin UDP chỉ chứa **1 ký tự ASCII**:

| Ký tự | Lệnh | Hành động thực tế của 4 bánh xe |
| :---: | :--- | :--- |
| **`F`** | Forward | Cả 4 bánh quay tiến cùng chiều |
| **`B`** | Backward | Cả 4 bánh quay lùi |
| **`Q`** | Rotate Left | Bánh bên trái quay lùi, bánh bên phải quay tiến |
| **`E`** | Rotate Right | Bánh bên trái quay tiến, bánh bên phải quay lùi |
| **`S`** | Stop | Ngắt toàn bộ xung PWM, dừng xe ngay lập tức |
| `L` | Strafe Left | Trượt ngang sang trái (FL lùi, FR tiến, RL tiến, RR lùi) |
| `R` | Strafe Right | Trượt ngang sang phải (FL tiến, FR lùi, RL lùi, RR tiến) |

---

## 🛡️ Cơ chế an toàn Failsafe

Để đảm bảo xe không bao giờ chạy ngoài tầm kiểm soát:

1. **Watchdog Timeout (`600ms`):**
   Nếu mạng Wi-Fi bị ngắt quãng hoặc app Android bị treo quá `600ms`, vi điều khiển sẽ kích hoạt hàm dừng khẩn cấp `stopMotorsImmediate()`.
2. **Cảm biến siêu âm chặn phần cứng (Hardware Interlock):**
   - Khoảng cách $\le 30\text{ cm}$: Chặn lệnh `F` (tiến), xe sẽ tự động chuyển thành lệnh dừng `S`.
   - Khoảng cách $\le 12\text{ cm}$: Xe tự động kích hoạt lùi khẩn cấp trong `300ms` (`EMERGENCY_REVERSE_MS`) để tạo cự ly an toàn.
3. **Khởi động an toàn:** Khi vừa bật nguồn hoặc reset vi điều khiển, xe luôn ở trạng thái dừng `S`.

---

## 🚀 Hướng dẫn biên dịch & Nạp Firmware

### 1. Chuẩn bị
- Cài đặt [PlatformIO IDE](https://platformio.org/) trên VSCode hoặc cài [PlatformIO Core CLI](https://docs.platformio.org/en/latest/core/index.html).
- Cáp Type-C kết nối cổng USB của kit ESP32-S3 với máy tính.

### 2. Dòng lệnh PlatformIO

```bash
# 1. Di chuyển vào thư mục firmware
cd auto_meca_firmware

# 2. Biên dịch mã nguồn
pio run

# 3. Nạp firmware lên ESP32-S3 (tự động nhận cổng COM)
pio run --target upload

# 4. Mở Serial Monitor xem log (Baudrate 115200)
pio device monitor --baud 115200
```

---

## 📂 Cấu trúc thư mục

```text
auto_meca_firmware/
├── platformio.ini         # Cấu hình board esp32-s3-devkitc-1, framework arduino
├── include/
│   ├── config.h           # Toàn bộ cấu hình chân GPIO, Wi-Fi AP, tốc độ, timeout
│   ├── motor_control.h    # Khai báo hàm điều khiển động cơ và PWM ramp
│   ├── obstacle_guard.h   # Khai báo lớp bảo vệ vật cản siêu âm
│   ├── ultrasonic_sensor.h# Driver đọc cảm biến HC-SR04
│   └── telemetry_udp.h    # Gửi telemetry về Android
├── src/
│   ├── main.cpp           # Khởi tạo Wi-Fi AP, UDP Server, vòng lặp Failsafe
│   ├── motor_control.cpp  # Hiện thực hóa chuyển động Mecanum và LEDC PWM
│   ├── obstacle_guard.cpp # Logic phanh khẩn cấp theo khoảng cách
│   ├── ultrasonic_sensor.cpp # Lọc nhiễu cảm biến siêu âm
│   └── telemetry_udp.cpp  # Đóng gói dữ liệu cảm biến gửi ngược về điện thoại
└── docs/                  # Tài liệu chi tiết giao thức và phân tích hệ thống
```

---

<div align="center">
  <sub>Phối hợp cùng dự án Android Station • <a href="https://github.com/thanhquan123hi1/auto-meca">Auto-Meca Android</a></sub>
</div>
