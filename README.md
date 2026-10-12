# HỆ THỐNG ĐIỀU KHIỂN MÁY GIẶT TỰ ĐỘNG (FSM WASHING MACHINE)
**Môn học:** Hệ thống nhúng (Embedded Systems - CO3053)  
**Nền tảng:** Vi điều khiển ARM Cortex-M3 STM32F103C6  
**Framework:** STM32Cube HAL  
**Môi trường phát triển:** PlatformIO / VS Code  
**Môi trường mô phỏng:** Proteus 8 Professional  

---

## 1. Giới thiệu tổng quan (Introduction)

Dự án hiện thực hóa bộ điều khiển trung tâm cho **Máy giặt tự động trả tiền xu (Coin-operated Washing Machine)** dựa trên mô hình **Máy trạng thái hữu hạn (Finite State Machine - FSM)** kết hợp với **Hệ điều hành hợp tác (Cooperative Task Scheduler - Delta List)** chạy trên vi điều khiển **STM32F103C6**.

Hệ thống điều khiển toàn bộ quy trình:
- Nhận tiền xu tích lũy (10 cent, 20 cent, 50 cent).
- Quản lý các trạng thái hoạt động: Chờ nạp tiền (`STANDBY`), Sẵn sàng giặt (`READY`), Đang giặt (`RUNNING`), Tạm dừng (`PAUSED`), và Báo lỗi (`ERROR`).
- Đếm lùi chu trình giặt (chế độ mô phỏng 30 giây hoặc thời gian thực 30 phút).
- Xử lý dừng khẩn cấp / reset máy khi người dùng nhấn đúp nút STOP (Double-click detection).
- Hiển thị trực quan qua LED báo trạng thái 2 màu (RLED, BLED, Heartbeat LED) và khối 2 LED 7 đoạn quét xen kẽ (Multiplexed 7-Segment Displays) hiển thị số dư và thời gian đếm ngược.
- Tùy biến linh hoạt qua preprocessor option `#define USE_SEG_LED` để dùng hoặc không dùng LED 7 đoạn mà không làm thay đổi logic FSM.

---

## 2. Kiến trúc hệ thống (System Architecture)

Hệ thống được thiết kế theo mô hình phân tầng mô-đun hóa cao (Modular Layered Architecture):

```
+-------------------------------------------------------------------------+
|                           APPLICATION LAYER                             |
|       main.cpp : Khởi tạo hệ thống & Đăng ký tác vụ vào Scheduler       |
+-------------------------------------------------------------------------+
             |                                              |
             v                                              v
+--------------------------+                   +--------------------------+
|    FSM ENGINE (fsm.cpp)  |                   |   SCHEDULER ENGINE       |
| - Ma trận chuyển trạng   |                   |   (scheduler.cpp)        |
|   thái (Transition Table)|                   | - Hàng đợi delta-list    |
| - 5 Trạng thái hệ thống  |                   | - Bộ điều phối tác vụ    |
| - Bộ đếm lùi chu trình   |                   | - Base Tick 10ms (TIM2)  |
+--------------------------+                   +--------------------------+
       |             |                                      ^
       v             v                                      |
+--------------+ +--------------------+                     |
| BUTTON DRIVER| | LED DISPLAY DRIVER |                     |
| (button.cpp) | | (led_display.cpp)  |                     |
| - Chống rung | | - RLED / BLED      |                     |
| - FallingEdge| | - 7-Segment Mux    |                     |
| - DoubleClick| | - Option USE_SEG   |                     |
+--------------+ +--------------------+                     |
       |                   |                                |
       +-------------------+--------------------------------+
                           |
                           v
+-------------------------------------------------------------------------+
|                  HARDWARE ABSTRACTION LAYER (HAL / BSP)                 |
|       helper.cpp / helper.h : Cấu hình GPIO, Timer 2, Clock 8MHz        |
+-------------------------------------------------------------------------+
```

---

## 3. Máy trạng thái hữu hạn (FSM Specification)

### 3.1. Các trạng thái hoạt động (States)
| Trạng thái | Mã định danh | Chỉ báo LED Đỏ (RLED) | Chỉ báo LED Xanh (BLED) | Hiển thị LED 7 đoạn | Mô tả hoạt động |
| :--- | :--- | :---: | :---: | :---: | :--- |
| **STANDBY** | `WM_STATE_STANDBY` | **SÁNG (ON)** | TẮT (OFF) | Số xu tích lũy (00 - 40) | Chờ nạp tiền xu, máy rảnh sẵn sàng phục vụ. |
| **READY** | `WM_STATE_READY` | TẮT (OFF) | **SÁNG (ON)** | TẮT (OFF) | Đã nạp đủ $\ge 50$ xu, chờ người dùng bấm RUN. |
| **RUNNING** | `WM_STATE_RUNNING` | TẮT (OFF) | **NHÁY (BLINK)** | Đếm ngược (30s $\rightarrow$ 0s) | Đang thực hiện chu trình giặt, thời gian đếm lùi. |
| **PAUSED** | `WM_STATE_PAUSED` | TẮT (OFF) | **SÁNG (ON)** | Đếm ngược (vẫn đếm lùi) | Đang tạm dừng giặt, đồng hồ vẫn tiếp tục đếm lùi. |
| **ERROR** | `WM_STATE_ERROR` | **NHÁY (BLINK)** | TẮT (OFF) | TẮT (OFF) | Phát hiện sự cố/lỗi, khóa máy chờ reset. |

> **Quy ước nhấp nháy LED:** Tần số chớp tắt là $1\text{ Hz}$ ($500\text{ms}$ Sáng / $500\text{ms}$ Tắt).

---

### 3.2. Các sự kiện kích hoạt (Events)
| Sự kiện | Mã định danh | Nguồn kích hoạt |
| :--- | :--- | :--- |
| **COIN** | `WM_EV_COIN` | Tích lũy đủ $\ge 50$ xu từ các nút nạp tiền (10c, 20c, 50c). |
| **BTN_RUN** | `WM_EV_BTN_RUN` | Nhấn nút **RUN** (chuyển sang giặt hoặc tiếp tục giặt khi đang pause). |
| **BTN_PAUSE** | `WM_EV_BTN_PAUSE` | Nhấn nút **PAUSE** khi đang trong trạng thái `RUNNING`. |
| **BTN_STOP_TWICE** | `WM_EV_BTN_STOP_TWICE` | Nhấn đúp nút **STOP** (2 lần trong vòng $1.5\text{ giây}$) để dừng khẩn cấp / reset lỗi. |
| **TIMEOUT** | `WM_EV_TIMEOUT` | Hết thời gian giặt (đồng hồ đếm lùi về 0). |
| **FAULT** | `WM_EV_FAULT` | Sự cố phần cứng / lỗi ngoại vi (có thể kích hoạt bằng nhấn đúp PAUSE để demo). |

---

## 4. Sơ đồ nối dây phần cứng (Hardware Pinout Mapping)

### 4.1. Bảng phân bổ chân STM32F103C6

| Thiết bị / Linh kiện | Ký hiệu chân | Cổng GPIO | Chế độ cấu hình | Mức Logic tích cực |
| :--- | :--- | :---: | :---: | :--- |
| **Heartbeat LED (SysLED)** | `PIN_LED_BLINK` | **PB15** | Output Push-Pull | Active-HIGH (0V: Tắt, 3.3V: Sáng) |
| **Đèn LED Đỏ (RLED)** | `PIN_LED_RED` | **PB13** | Output Push-Pull | **Active-LOW** (0V: Sáng, 3.3V: Tắt) |
| **Đèn LED Xanh (BLED)** | `PIN_LED_BLUE` | **PB14** | Output Push-Pull | **Active-LOW** (0V: Sáng, 3.3V: Tắt) |
| **Nút bấm RUN** | `PIN_BUTTON_RUN` | **PB0** | Input Pull-Up | Nhấn: Nối GND (`RESET`), Thả: `SET` |
| **Nút bấm PAUSE** | `PIN_BUTTON_PAUSE` | **PB1** | Input Pull-Up | Nhấn: Nối GND (`RESET`), Thả: `SET` |
| **Nút bấm STOP** | `PIN_BUTTON_STOP` | **PB2** | Input Pull-Up | Nhấn: Nối GND (`RESET`), Thả: `SET` |
| **Nút nạp 10 Cents** | `PIN_BUTTON_10_CENTS` | **PA1** | Input Pull-Up | Nhấn: Nối GND (`RESET`), Thả: `SET` |
| **Nút nạp 20 Cents** | `PIN_BUTTON_20_CENTS` | **PA2** | Input Pull-Up | Nhấn: Nối GND (`RESET`), Thả: `SET` |
| **Nút nạp 50 Cents** | `PIN_BUTTON_50_CENTS` | **PA15** | Input Pull-Up | Nhấn: Nối GND (`RESET`), Thả: `SET` |
| **Transistor Kích Hàng chục (Q1)**| `PIN_EN0` | **PA4** | Output Push-Pull | **Active-LOW** (PNP: 0V dẫn, 3.3V ngắt) |
| **Transistor Kích Hàng đơn vị (Q2)**| `PIN_EN1` | **PA5** | Output Push-Pull | **Active-LOW** (PNP: 0V dẫn, 3.3V ngắt) |
| **Thanh LED a, b, c, d, e, f, g** | `PIN_SEG0..SEG6` | **PB5 $\rightarrow$ PB11** | Output Push-Pull | **Active-LOW** (0V: Thanh sáng, 3.3V: Tắt) |

> [!NOTE]
> Chân `PA15` được cấu hình giải phóng hoàn toàn khỏi JTAG bằng hàm `__HAL_AFIO_REMAP_SWJ_NOJTAG()` trong `SetupPinMode()`, giúp bảo toàn chuẩn SWD (PA13/PA14) mà vẫn sử dụng `PA15` làm chân GPIO Input có Pull-Up ổn định.

---

### 4.2. Cấu hình tùy chọn LED 7 đoạn (`USE_SEG_LED`)

Trong file [`include/led_display.h`](include/led_display.h):
```c
// Configuration: 1 = enable 7-segment display, 0 = disable
#ifndef USE_SEG_LED
#define USE_SEG_LED             1
#endif
```
- Khi `#define USE_SEG_LED 1`: Bật tính năng quét 2 LED 7 đoạn (PA4, PA5, PB5..PB11), hiển thị số xu nạp và đếm lùi chu trình giặt.
- Khi `#define USE_SEG_LED 0`: Toàn bộ các chân GPIO kích transistor và thanh LED 7 đoạn được bỏ qua không khởi tạo; thuật toán quét xen kẽ không thực thi; các hàm hiển thị trở thành hàm inline rỗng giúp tiết kiệm **~460 bytes Flash** mà không làm thay đổi bất kỳ dòng code nào trong FSM.

---

## 5. Cấu trúc mã nguồn (Project Structure)

```
STM-FSM_WashingMachine/
├── platformio.ini              # File cấu hình PlatformIO (Board, Framework, Flags)
├── include/
│   ├── button.h                # Khai báo enum nút bấm và API nhận diện nhấn đơn/nhấn đúp
│   ├── fsm.h                   # Khai báo State, Event, ma trận chuyển trạng thái
│   ├── helper.h                # Định nghĩa ánh xạ chân phần cứng, clock, timer
│   ├── led_display.h           # Khai báo module LED báo trạng thái, LED 7 đoạn, macro USE_SEG_LED
│   └── scheduler.h             # Khai báo bộ điều phối tác vụ hợp tác Delta-List
├── src/
│   ├── button.cpp              # Xử lý chống rung, lọc cạnh rơi (falling edge), bắt double-click 1.5s
│   ├── fsm.cpp                 # Hiện thực hóa ma trận FSM, timer đếm ngược, logic tiền xu
│   ├── helper.cpp              # Khởi tạo GPIO, AFIO Remap, ngắt phần cứng Timer 2 (10ms base tick)
│   ├── led_display.cpp         # Điều khiển Active-Low LEDs và thuật toán multiplexing 7 đoạn
│   ├── main.cpp                # Hàm main(), đăng ký task vào Scheduler theo kỹ thuật Phase-Shifting
│   └── scheduler.cpp           # Thuật toán hàng đợi Delta-List và Dispatcher
└── .pio/build/genericSTM32F103C6/
    └── firmware.hex            # File nhị phân dùng để nạp vào Proteus mô phỏng
```

---

## 6. Hướng dẫn cài đặt & Biên dịch (Build & Simulation Guide)

### 6.1. Yêu cầu môi trường
1. **Visual Studio Code** cài đặt extension **PlatformIO IDE**.
2. **Proteus Professional** (khuyên dùng bản 8.9 trở lên).
3. Toolchain ARM GNU Toolchain (`arm-none-eabi-gcc`) tự động tải qua PlatformIO.

### 6.2. Các bước biên dịch
1. Mở thư mục dự án `STM-FSM_WashingMachine` trong VS Code.
2. Mở Terminal và gõ lệnh build:
   ```powershell
   pio run
   ```
   Hoặc bấm biểu tượng **PlatformIO: Build (✔)** ở thanh trạng thái dưới cùng.
3. Khi biên dịch thành công, file `firmware.hex` được tự động tạo tại:
   ```
   .pio/build/genericSTM32F103C6/firmware.hex
   ```

### 6.3. Nạp và chạy mô phỏng trên Proteus
1. Khởi chạy file mô phỏng Proteus: `Proteus/simulation.pdsprj`.
2. Click đúp chuột trái vào chip vi điều khiển **STM32F103C6 (U1)**.
3. Tại ô **Program File**, bấm nút thư mục và duyệt chọn đến file `firmware.hex` vừa biên dịch.
4. Tại ô **Crystal Frequency**, đảm bảo thiết lập **`8MHz`**.
5. Bấm nút **Play (F12)** ở góc dưới bên trái Proteus để bắt đầu mô phỏng.

---

## 7. Hướng dẫn vận hành & Kịch bản kiểm thử (User Guide & Scenarios)

### Kịch bản 1: Khởi động & Nạp tiền (STANDBY $\rightarrow$ READY)
1. Khi vừa nhấn Play:
   - **RLED sáng tĩnh (ON)**, BLED tắt.
   - LED 7 đoạn hiển thị `00`.
   - Heartbeat LED (SysLED) nhấp nháy nhịp tim đều đặn mỗi $1\text{s}$.
2. Bấm nút **10 CENT**: Số tiền tăng lên `10`, 7 đoạn hiện `10`.
3. Bấm tiếp nút **20 CENT**: Số tiền tăng lên `30`, 7 đoạn hiện `30`.
4. Bấm tiếp nút **20 CENT**: Số tiền tích lũy đạt `50`:
   - RLED tắt, **BLED bật sáng tĩnh (ON)** thông báo máy đã sẵn sàng (`READY`).
   - Khối LED 7 đoạn tự động tắt.
5. *(Tùy chọn)* Nạp thêm tiền ở trạng thái này: Máy tiếp tục nhận tiền nhưng không hoàn tiền thừa theo đúng quy tắc đề bài.

### Kịch bản 2: Bắt đầu giặt & Đếm ngược tự động (READY $\rightarrow$ RUNNING $\rightarrow$ STANDBY)
1. Ở trạng thái `READY` (BLED sáng tĩnh), bấm nút **RUN**:
   - Máy chuyển sang `RUNNING`: **BLED bắt đầu nhấp nháy (Blink)** nhịp $500\text{ms}$.
   - Số tiền được xóa về 0.
   - LED 7 đoạn bắt đầu đếm ngược thời gian giặt từ `30` giây về `00`.
2. Khi đếm về `00` (Timeout):
   - Chu trình giặt hoàn tất.
   - BLED tắt, **RLED bật sáng tĩnh (ON)**, máy tự động quay về `STANDBY`.

### Kịch bản 3: Tạm dừng và tiếp tục giặt (RUNNING $\leftrightarrow$ PAUSED)
1. Khi máy đang giặt (`RUNNING` - BLED đang nháy):
   - Bấm nút **PAUSE**: Máy chuyển sang `PAUSED`.
   - **BLED chuyển từ nhấp nháy sang sáng tĩnh (ON)**.
   - Đồng hồ đếm lùi trên LED 7 đoạn **vẫn tiếp tục đếm ngược** (đúng theo yêu cầu đặc tả).
2. Khi muốn tiếp tục:
   - Bấm nút **RUN**: BLED nhấp nháy trở lại, máy trở về trạng thái `RUNNING`.

### Kịch bản 4: Dừng khẩn cấp bằng nhấn đúp STOP (Force Stop)
1. Khi máy đang ở trạng thái `RUNNING` hoặc `PAUSED`:
   - Nhấn đúp nút **STOP** (2 lần click trong vòng $1.5\text{ giây}$):
   - Máy lập tức hủy chu trình giặt, tắt BLED, xóa đồng hồ đếm và trở về `STANDBY` (RLED sáng).
   - *Ghi chú:* Nhấn đơn nút STOP một lần sẽ bị bỏ qua để tránh thao tác chạm nhầm.

### Kịch bản 5: Báo lỗi & Khôi phục sự cố (ERROR $\rightarrow$ STANDBY)
1. Khi máy gặp sự cố (`WM_EV_FAULT`):
   - **RLED nhấp nháy liên tục (Blink $500\text{ms}$)**.
   - BLED tắt, LED 7 đoạn tắt. Toàn bộ các nút nạp tiền và nút RUN bị khóa.
2. Để reset lỗi:
   - Nhấn đúp nút **STOP** (2 lần trong $1.5\text{s}$):
   - Máy xóa cờ lỗi và quay trở lại trạng thái `STANDBY` (RLED sáng tĩnh) sẵn sàng hoạt động.

> [!NOTE]
> Bật cờ FAULT_CASE trong file [include/fsm.h](include/fsm.h) để mô phỏng sự cố vận hành (cụ thể, máy hỏng sau 5s chạy).
---
