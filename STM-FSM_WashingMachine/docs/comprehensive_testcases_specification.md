# TÀI LIỆU KỊCH BẢN KIỂM THỬ TOÀN DIỆN (COMPREHENSIVE TEST CASES SPECIFICATION)
## Dự án: STM-FSM_WashingMachine (CO3053 - Embedded Systems)

---

## 1. Mục đích & Phạm vi Kiểm thử

Tài liệu này cung cấp ma trận các trường hợp kiểm thử (**Test Cases Matrix**) chi tiết và đầy đủ nhất nhằm xác minh tính đúng đắn của phần mềm điều khiển Máy giặt FSM trên vi điều khiển **STM32F103C6** (chạy thực tế hoặc mô phỏng trên **Proteus**).

Mọi trường hợp kiểm thử đều bám sát 100% các tiêu chí trong đặc tả đề bài (**CO3053 Assignment**):
* **Inputs:** 3 nút điều khiển (`RUN`, `PAUSE`, `STOP`) và 3 nút nạp tiền (`10c`, `20c`, `50c`).
* **Outputs:** 2 đèn LED chỉ báo (`RLED` - PB13, `BLED` - PB14).
* **FSM States:** `STANDBY`, `READY`, `RUNNING`, `PAUSED`, `ERROR`.
* **Quy tắc:** Nạp tối thiểu 50c; không hoàn tiền thừa khi bắt đầu chạy; timer 30 phút vẫn đếm khi Pause; hết giờ hoặc nhấn đúp STOP thì dừng; báo lỗi và phục hồi.

---

## 2. Bảng Quy ước Phần cứng & Trạng thái Đầu ra

| Tín hiệu | Chân vi điều khiển | Mức tích cực | Ý nghĩa kiểm tra |
| :--- | :--- | :--- | :--- |
| **Nút RUN** | `GPIOB - PIN 0` | Mức LOW (Pull-up) | Bắt đầu giặt từ READY, hoặc tiếp tục từ PAUSED |
| **Nút PAUSE** | `GPIOB - PIN 1` | Mức LOW (Pull-up) | Tạm dừng khi đang RUNNING |
| **Nút STOP** | `GPIOB - PIN 2` | Mức LOW (Pull-up) | Nhấn đúp: Force Stop; Nhấn 1 lần trong ERROR: Reset |
| **Nút 10 CENTS** | `GPIOA - PIN 1` | Mức LOW (Pull-up) | Nạp thêm 10 cents |
| **Nút 20 CENTS** | `GPIOA - PIN 2` | Mức LOW (Pull-up) | Nạp thêm 20 cents |
| **Nút 50 CENTS** | `GPIOA - PIN 15` | Mức LOW (Pull-up) | Nạp thêm 50 cents |
| **RLED (Đỏ)** | `GPIOB - PIN 13` | Mức HIGH (Output) | **Sáng tĩnh (ON):** Standby; **Chớp (BLINK):** Error |
| **BLED (Xanh)**| `GPIOB - PIN 14` | Mức HIGH (Output) | **Sáng tĩnh (ON):** Ready / Paused; **Chớp (BLINK):** Running |

---

## 3. Ma trận Kịch bản Kiểm thử Chi tiết (Detailed Test Cases)

### Nhóm 1: Khởi động Hệ thống (System Boot & Standby)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-01** | Khởi động nguồn mặc định | Cấp nguồn cho vi điều khiển / Chạy Proteus | 1. Bật nguồn mô phỏng.<br>2. Quan sát 2 đèn LED.<br>3. Kiểm tra biến nội bộ. | - **RLED sáng tĩnh (ON)**.<br>- **BLED tắt (OFF)**.<br>- FSM ở trạng thái `WM_STATE_STANDBY`.<br>- Số tiền `current_money = 0`. | **Critical** |
| **TC-02** | Nhấn phím RUN khi ở Standby | Máy đang ở `STANDBY` (`current_money = 0`) | 1. Nhấn nút `RUN` 1 lần.<br>2. Quan sát LED và trạng thái máy. | - Máy **KHÔNG chạy**.<br>- RLED vẫn sáng, BLED vẫn tắt.<br>- Không có chuyển trạng thái nào xảy ra. | **High** |
| **TC-03** | Nhấn phím PAUSE khi ở Standby | Máy đang ở `STANDBY` | 1. Nhấn nút `PAUSE`.<br>2. Quan sát LED. | - Máy giữ nguyên ở `STANDBY`.<br>- RLED ON, BLED OFF. | **Medium** |
| **TC-04** | Nhấn phím STOP khi ở Standby | Máy đang ở `STANDBY` | 1. Nhấn nút `STOP` (1 lần hoặc 2 lần). | - Máy giữ nguyên ở `STANDBY`.<br>- Không có phản ứng bất thường. | **Medium** |

---

### Nhóm 2: Nạp tiền & Ngưỡng Kích hoạt (Coin Accumulation & Threshold)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-05** | Nạp chưa đủ tiền (< 50 cents) | Máy ở `STANDBY` (`money = 0`) | 1. Nhấn nút `10 CENTS` 1 lần (tiền = 10c).<br>2. Nhấn nút `20 CENTS` 1 lần (tiền = 30c).<br>3. Nhấn nút `RUN`. | - Sau mỗi lần nhấn: tiền cộng dồn chính xác (10c $\rightarrow$ 30c).<br>- RLED vẫn ON, BLED vẫn OFF (vẫn ở `STANDBY`).<br>- Bấm RUN không có tác dụng. | **Critical** |
| **TC-06** | Nạp vừa đủ 50 cents bằng đồng 50c | Máy ở `STANDBY` (`money = 0`) | 1. Nhấn nút `50 CENTS` 1 lần. | - Tiền đạt 50c $\rightarrow$ FSM chuyển sang `WM_STATE_READY`.<br>- **RLED TẮT**.<br>- **BLED SÁNG TĨNH (ON)**. | **Critical** |
| **TC-07** | Nạp đủ 50 cents bằng nhiều đồng lẻ | Máy ở `STANDBY` (`money = 0`) | 1. Nhấn `20 CENTS` lần 1 (20c).<br>2. Nhấn `20 CENTS` lần 2 (40c).<br>3. Nhấn `10 CENTS` (đủ 50c). | - Ở 20c và 40c: RLED vẫn ON, BLED OFF.<br>- Vừa nhấn đủ 50c: RLED tắt, **BLED bật sáng tĩnh (ON)**. | **High** |
| **TC-08** | Nạp dư tiền (> 50 cents) | Máy ở `STANDBY` | 1. Nhấn `50 CENTS` (vào `READY`, BLED ON).<br>2. Nhấn thêm `20 CENTS` (tổng 70c).<br>3. Nhấn thêm `10 CENTS` (tổng 80c). | - Máy vẫn ở trạng thái `READY`.<br>- BLED vẫn sáng tĩnh (ON), RLED tắt.<br>- Tổng tiền lưu trữ là 80 cents (cho phép nhận tiền dư). | **High** |

---

### Nhóm 3: Khởi động Chu trình Giặt & Xóa Tiền thừa (Run & Redundancy Clear)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-09** | Nhấn RUN khi đủ tiền (50c chuẩn) | Máy ở `READY` (`money = 50c`, BLED ON) | 1. Nhấn nút `RUN` 1 lần. | - FSM chuyển sang `WM_STATE_RUNNING`.<br>- **BLED bắt đầu NHẤP NHÁY (BLINK)** (~500ms chu kỳ).<br>- RLED vẫn TẮT.<br>- Tiền bị xóa về 0 (`current_money = 0`).<br>- Bộ đếm thời gian được nạp giá trị chu trình (30s hoặc 1800s) và bắt đầu đếm lùi. | **Critical** |
| **TC-10** | Nhấn RUN khi nạp dư tiền (Không thối lại) | Máy ở `READY` (`money = 70c`, nạp dư 20c) | 1. Nhấn nút `RUN` 1 lần. | - Máy chuyển sang `RUNNING`, BLED chớp.<br>- Toàn bộ tiền dư bị xóa trắng về 0 (`money = 0`).<br>- Không có cơ chế trả tiền thừa (*"without returning redundancies"*). | **Critical** |
| **TC-11** | Nạp tiền khi máy đang chạy giặt | Máy đang ở `RUNNING` | 1. Bấm các nút `10c`, `20c`, `50c` khi máy đang giặt. | - Máy tiếp tục giặt bình thường, BLED vẫn chớp.<br>- Timer vẫn tiếp tục đếm, tiền xu nạp vào không ảnh hưởng chu trình. | **Medium** |

---

### Nhóm 4: Tạm dừng, Tiếp tục & Timer ngầm (Pause, Resume & Countdown)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-12** | Nhấn PAUSE khi đang giặt | Máy đang ở `RUNNING` (BLED đang chớp) | 1. Nhấn nút `PAUSE` 1 lần. | - FSM chuyển sang `WM_STATE_PAUSED`.<br>- **BLED ngừng chớp, chuyển sang SÁNG TĨNH (ON)**.<br>- RLED vẫn TẮT. | **Critical** |
| **TC-13** | Kiểm tra Timer khi đang PAUSED | Máy đang ở `PAUSED` | 1. Quan sát biến `wash_time_remaining` trong 5–10 giây. | - **Thời gian VẪN TIẾP TỤC ĐẾM LÙI** (*"timer is still counting down"*).<br>- Timer không bị dừng/đóng băng khi Pause. | **Critical** |
| **TC-14** | Nhấn RUN để tiếp tục từ PAUSED | Máy đang ở `PAUSED` (BLED sáng tĩnh) | 1. Nhấn nút `RUN` 1 lần. | - FSM quay lại `WM_STATE_RUNNING`.<br>- **BLED tiếp tục NHẤP NHÁY (BLINK)**.<br>- Timer tiếp tục đếm từ giá trị thời gian còn lại. | **Critical** |
| **TC-15** | Nhấn liên tục PAUSE khi đang PAUSED | Máy đang ở `PAUSED` | 1. Nhấn nút `PAUSE` thêm nhiều lần. | - Máy vẫn giữ nguyên ở `PAUSED`, BLED ON.<br>- Không bị lỗi trạng thái. | **Low** |

---

### Nhóm 5: Dừng Cưỡng bức (Force Stop - Single Click vs Double Click STOP)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-16** | Nhấn STOP 1 LẦN khi đang RUNNING | Máy đang ở `RUNNING` (BLED chớp) | 1. Nhấn nút `STOP` đúng 1 lần.<br>2. Đợi quá 2 giây. | - **MÁY KHÔNG ĐƯỢC DỪNG**!<br>- BLED vẫn chớp, chu trình giặt tiếp tục bình thường. | **Critical** |
| **TC-17** | Nhấn STOP 2 LẦN nhanh khi đang RUNNING | Máy đang ở `RUNNING` (BLED chớp) | 1. Nhấn nút `STOP` lần 1.<br>2. Trong vòng 1.5 giây, nhấn `STOP` lần 2. | - Máy kích hoạt **Force Stop**!<br>- FSM chuyển ngay về `WM_STATE_STANDBY`.<br>- BLED TẮT, **RLED SÁNG TĨNH (ON)**.<br>- Chu trình giặt bị hủy hoàn toàn. | **Critical** |
| **TC-18** | Nhấn STOP 1 LẦN khi đang PAUSED | Máy đang ở `PAUSED` (BLED ON) | 1. Nhấn nút `STOP` đúng 1 lần.<br>2. Đợi quá 2 giây. | - Máy **vẫn ở `PAUSED`**, BLED vẫn ON.<br>- Timer vẫn tiếp tục đếm ngược. | **High** |
| **TC-19** | Nhấn STOP 2 LẦN nhanh khi đang PAUSED | Máy đang ở `PAUSED` (BLED ON) | 1. Nhấn nút `STOP` đúp 2 lần trong 1.5s. | - Máy kích hoạt **Force Stop**!<br>- FSM về `WM_STATE_STANDBY` (RLED ON, BLED OFF). | **High** |
| **TC-20** | Nhấn 2 lần STOP quá chậm (> 2 giây) | Máy đang ở `RUNNING` | 1. Nhấn `STOP` lần 1.<br>2. Đợi 3 giây (cửa sổ double-click hết hạn).<br>3. Nhấn `STOP` lần 2. | - Máy hiểu là 2 lần nhấn đơn độc lập.<br>- **Máy KHÔNG dừng**, vẫn tiếp tục giặt. | **Medium** |

---

### Nhóm 6: Tự động Kết thúc Hết giờ (Cycle Timeout Normal & In Pause)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-21** | Giặt xong bình thường khi đang RUNNING | Máy ở `RUNNING` (`wash_time_remaining = 3s`) | 1. Để máy chạy tự do đến khi thời gian đếm về 0. | - Sự kiện `WM_EV_TIMEOUT` được kích hoạt.<br>- FSM tự động chuyển về `WM_STATE_STANDBY`.<br>- BLED TẮT, **RLED BẬT SÁNG TĨNH (ON)**. | **Critical** |
| **TC-22** | Hết giờ khi máy đang bị PAUSED | Máy ở `PAUSED` (`wash_time_remaining = 3s`) | 1. Bấm PAUSE khi còn 3s.<br>2. Không bấm gì thêm, để thời gian tự đếm về 0. | - Khi timer về 0: FSM tự động kết thúc chu trình và chuyển về `WM_STATE_STANDBY`.<br>- BLED TẮT, **RLED SÁNG TĨNH (ON)**. | **Critical** |

---

### Nhóm 7: Xử lý Sự cố & Phục hồi Lỗi (Error Handling & Reset)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-23** | Kích hoạt sự cố phần cứng (`FAULT`) | Máy ở bất kỳ trạng thái nào (`STANDBY`, `READY`, `RUNNING`) | 1. Mô phỏng sự kiện `WM_EV_FAULT`. | - FSM chuyển ngay sang `WM_STATE_ERROR`.<br>- BLED TẮT.<br>- **RLED NHẤP NHÁY (BLINK)** báo động. | **High** |
| **TC-24** | Nhấn nút chức năng khi đang ERROR | Máy đang ở `ERROR` (RLED chớp) | 1. Nhấn thử các nút: `RUN`, `PAUSE`, `10c`, `20c`. | - Máy từ chối mọi thao tác.<br>- RLED vẫn tiếp tục nhấp nháy báo lỗi. | **Medium** |
| **TC-25** | Reset lỗi về Standby bằng nút STOP | Máy đang ở `ERROR` (RLED chớp) | 1. Nhấn nút `STOP` (1 lần hoặc nhấn đúp). | - FSM chuyển về `WM_STATE_STANDBY`.<br>- **RLED ngừng chớp, chuyển sang SÁNG TĨNH (ON)**.<br>- Hệ thống an toàn trở lại. | **High** |

---

### Nhóm 8: Kiểm thử Biên & Thao tác Bất thường (Edge Cases & Stress Tests)

| ID | Tên Kịch bản | Tiền điều kiện | Các bước thực hiện (Steps) | Kết quả kỳ vọng (Expected Result) | Mức độ |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-26** | Nhấn giữ nút (Holding Button) | Máy ở `STANDBY` | 1. Click và giữ chuột chặt nút `10 CENTS` trong 2 giây trên Proteus.<br>2. Nhả chuột ra. | - Tiền **chỉ được cộng đúng 1 lần (10 cents)** do cơ chế bắt sườn xuống (Edge Detection).<br>- Tiền KHÔNG bị cộng dồn liên tục. | **High** |
| **TC-27** | Nhấn nhiều nút cùng lúc (Multi-button press) | Máy ở `STANDBY` | 1. Nhấn nhanh liên tiếp nút 10c và RUN. | - Hệ thống xử lý tuần tự theo nhịp quét 20ms, không bị treo (freeze/crash). | **Low** |
| **TC-28** | Chu trình đầy đủ liên tục (End-to-End Full Life Cycle) | Máy mới khởi động | 1. Nạp 10c + 20c + 20c (vào Ready).<br>2. Bấm RUN (vào Running).<br>3. Bấm PAUSE sau 5s (vào Pause).<br>4. Bấm RUN sau 3s (vào Running tiếp).<br>5. Chờ hết 30s (về Standby).<br>6. Lặp lại chu trình lần 2. | - Toàn bộ chu trình hoạt động trơn tru.<br>- Không bị rò rỉ bộ nhớ, không treo Scheduler.<br>- Máy sẵn sàng cho chu trình giặt tiếp theo. | **Critical** |

---

## 4. Biểu mẫu Ghi nhận Kết quả Kiểm thử (Test Sign-off Sheet)

Khi bạn chạy mô phỏng trên Proteus, hãy đánh dấu `[x]` vào các mục đã đạt:

- [ ] **TC-01 ➔ TC-04:** Khởi động nguồn, LED đỏ sáng tĩnh ở Standby.
- [ ] **TC-05 ➔ TC-08:** Nạp đủ 50c thì LED xanh sáng tĩnh, LED đỏ tắt.
- [ ] **TC-09 ➔ TC-11:** Bấm RUN thì xóa tiền về 0, LED xanh chớp, bắt đầu giặt.
- [ ] **TC-12 ➔ TC-15:** Bấm PAUSE thì LED xanh sáng tĩnh, timer vẫn đếm lùi.
- [ ] **TC-16 ➔ TC-20:** Bấm STOP 1 lần không dừng; bấm STOP 2 lần nhanh mới dừng.
- [ ] **TC-21 ➔ TC-22:** Hết giờ tự ngắt về Standby (LED đỏ sáng tĩnh).
- [ ] **TC-23 ➔ TC-25:** Trạng thái lỗi LED đỏ chớp, bấm STOP để reset.
- [ ] **TC-26 ➔ TC-28:** Nhấn giữ chuột không bị dội phím, chu trình chạy liên tục ổn định.
