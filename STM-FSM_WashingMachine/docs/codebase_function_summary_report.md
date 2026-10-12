# BÁO CÁO PHÂN TÍCH CHI TIẾT CẤU TRÚC & CHỨC NĂNG TỪNG HÀM TRONG PROJECT
## Dự án: STM-FSM_WashingMachine (CO3053 - Embedded Systems)

---

Chào bạn đồng hành (co-partner)! Để chuẩn bị cho việc hiện thực hóa các chức năng của máy giặt một cách chính xác, tài liệu này sẽ mổ xẻ và tóm tắt cặn kẽ **nội dung, cấu trúc dữ liệu và chức năng của từng hàm** trong toàn bộ các file mã nguồn hiện tại của dự án.

---

## 1. Tổng quan Kiến trúc Dự án (Architecture Overview)

Dự án được xây dựng theo mô hình **Hệ điều hành hợp tác (Cooperative Preemptive-like Task Scheduler)** kết hợp với **Máy trạng thái hữu hạn (FSM)** trên vi điều khiển **STM32F103C6**:

```
+------------------------------------------------------------------------+
|                          MAIN APPLICATION                              |
|                             (main.cpp)                                 |
|         - Khởi tạo HAL, Timer, GPIO                                     |
|         - Đăng ký các task định kỳ vào Scheduler                       |
|         - Vòng lặp chính gọi Scheduler_Dispatch()                      |
+------------------------------------------------------------------------+
           |                                                |
           v                                                v
+-----------------------------+          +-------------------------------+
|     FSM CONTROL ENGINE      |          |      SCHEDULER ENGINE         |
|     (fsm.h / fsm.cpp)       |          |  (scheduler.h / scheduler.cpp)|
|  - Quản lý các trạng thái:  |          |  - Hàng đợi thời gian Delta   |
|    STANDBY, READY, RUNNING, |          |  - Hàng đợi Ready Queue       |
|    PAUSED, ERROR            |          |  - Dispatcher thực thi tác vụ |
|  - Bảng chuyển trạng thái   |          +-------------------------------+
+-----------------------------+                         ^
           |                                            | Tick mỗi 10ms
           +--------------------+-----------------------+
                                |
                                v
+------------------------------------------------------------------------+
|                      HARDWARE ABSTRACTION LAYER (HAL/BSP)              |
|                             (helper.h / helper.cpp)                    |
|  - Cấu hình chân GPIO: 2 LED, 3 Nút chức năng, 3 Nút tiền xu          |
|  - Cấu hình Timer TIM2: Ngắt định kỳ mỗi 10ms                          |
|  - Hàm xử lý ngắt: SysTick_Handler(), TIM2_IRQHandler()                |
+------------------------------------------------------------------------+
```

---

## 2. Nhóm Hardware & Ngoại vi: `include/helper.h` & `src/helper.cpp`

File này đóng vai trò là tầng **Board Support Package (BSP)**, thiết lập cấu hình chân phần cứng, ngắt vi điều khiển và cấu hình phần cứng cơ bản.

> [!NOTE]
> Ghi chú ở đầu file: `/* Không được động k là máy nổ :D */` cho thấy đây là file cấu hình phần cứng chuẩn được cung cấp sẵn cho đề bài hoặc mô phỏng Proteus.

### 2.1. Phân tích `include/helper.h`
* **Mục đích:** Định nghĩa địa chỉ các chân GPIO của LED, các nút bấm và nguyên mẫu các hàm khởi tạo phần cứng.
* **Các định nghĩa chân (Macros):**
  * `PIN_LED_BLINK` (`GPIO_PIN_15` - Port B): Đèn LED nhấp nháy kiểm tra nhịp tim hệ thống (Heartbeat LED).
  * `PIN_LED_RED` (`GPIO_PIN_13` - Port B): Đèn LED Đỏ (RLED) chỉ báo chế độ STANDBY / ERROR.
  * `PIN_LED_BLUE` (`GPIO_PIN_14` - Port B): Đèn LED Xanh (BLED) chỉ báo chế độ READY / RUNNING / PAUSED.
  * `PIN_BUTTON_RUN` (`GPIO_PIN_0` - Port B): Nút bấm RUN.
  * `PIN_BUTTON_PAUSE` (`GPIO_PIN_1` - Port B): Nút bấm PAUSE.
  * `PIN_BUTTON_STOP` (`GPIO_PIN_2` - Port B): Nút bấm STOP.
  * `PIN_BUTTON_10_CENTS` (`GPIO_PIN_1` - Port A): Nút mô phỏng nạp đồng xu 10 cent.
  * `PIN_BUTTON_20_CENTS` (`GPIO_PIN_2` - Port A): Nút mô phỏng nạp đồng xu 20 cent.
  * `PIN_BUTTON_50_CENTS` (`GPIO_PIN_15` - Port A): Nút mô phỏng nạp đồng xu 50 cent.

---

### 2.2. Phân tích các hàm trong `src/helper.cpp`

#### 1. `SysTick_Handler(void)`
* **Prototype:** `extern "C" void SysTick_Handler(void)`
* **Chức năng:** Trình phục vụ ngắt hệ thống (System Tick Interrupt Service Routine) của nhân ARM Cortex-M.
* **Nguyên lý hoạt động:** Được gọi tự động mỗi 1ms khi SysTick đếm hết chu kỳ. Hàm gọi `HAL_IncTick()` để tăng biến đếm thời gian hệ thống `uwTick` của thư viện STM32 HAL.
* **Caller:** Phần cứng NVIC ngắt SysTick.

#### 2. `TIM2_IRQHandler(void)`
* **Prototype:** `extern "C" void TIM2_IRQHandler(void)`
* **Chức năng:** Trình phục vụ ngắt toàn cục cho bộ định thời phần cứng Timer 2 (TIM2 Global Interrupt Handler).
* **Nguyên lý hoạt động:** Khi TIM2 tràn bộ đếm (Counter Overflow), cờ ngắt được kích hoạt, hàm này bắt ngắt và chuyển tiếp xử lý cho thư viện HAL qua hàm `HAL_TIM_IRQHandler(&htim2)`.
* **Caller:** Phần cứng NVIC khi xảy ra sự kiện ngắt từ TIM2.

#### 3. `SetupPinMode()`
* **Prototype:** `void SetupPinMode()`
* **Chức năng:** Khởi tạo cấu hình vào/ra (I/O Mode) cho tất cả các chân GPIO sử dụng trong dự án.
* **Nguyên lý hoạt động:**
  1. Bật xung nhịp cấp clock cho hai ngoại vi Port A và Port B: `__HAL_RCC_GPIOB_CLK_ENABLE()`, `__HAL_RCC_GPIOA_CLK_ENABLE()`.
  2. Cấu hình các chân LED (`PB13`, `PB14`, `PB15`): Chế độ `GPIO_MODE_OUTPUT_PP` (Output Push-Pull), tốc độ thấp `GPIO_SPEED_FREQ_LOW`. Sau đó reset toàn bộ chân về mức thấp (0V).
  3. Cấu hình các nút điều khiển (`PB0`, `PB1`, `PB2`): Chế độ `GPIO_MODE_INPUT` kết hợp điện trở kéo lên nội `GPIO_PULLUP`. (Mặc định không nhấn là mức HIGH (1), khi nhấn nút sẽ nối xuống GND kéo về mức LOW (0)).
  4. Cấu hình các nút nạp tiền xu (`PA1`, `PA2`, `PA15`): Chế độ `GPIO_MODE_INPUT` kết hợp điện trở kéo lên nội `GPIO_PULLUP`.
* **Caller:** Gọi 1 lần duy nhất trong `main()` trước khi vào vòng lặp chính.

#### 4. `InitTIM()`
* **Prototype:** `void InitTIM()`
* **Chức năng:** Khởi tạo và kích hoạt bộ định thời phần cứng TIM2 tạo ngắt chu kỳ cố định 10ms làm nhịp cơ sở (Base Tick) cho hệ thống Scheduler.
* **Nguyên lý hoạt động:**
  1. Kích hoạt clock cho TIM2 (`__HAL_RCC_TIM2_CLK_ENABLE()`).
  2. Thiết lập cấu hình bộ chia tần và chu kỳ đếm:
     * `Prescaler = 7999`: Với xung nhịp HSI tiêu chuẩn của STM32F103 là 8 MHz, tần số sau bộ chia là $f = \frac{8.000.000}{7999 + 1} = 1.000 \text{ Hz}$ (tương ứng chu kỳ $1 \text{ ms}$ mỗi tick đếm).
     * `Period = 9`: Bộ đếm đếm từ 0 đến 9 (tổng cộng 10 chu kỳ $1 \text{ ms}$), do đó chu kỳ ngắt tràn là $10 \text{ ms}$ (tần số $100 \text{ Hz}$).
  3. Thiết lập ưu tiên ngắt trong NVIC (`HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0)`) và cho phép ngắt (`HAL_NVIC_EnableIRQ(TIM2_IRQn)`).
  4. Khởi động Timer chế độ ngắt: `HAL_TIM_Base_Start_IT(&htim2)`.
* **Caller:** Gọi 1 lần duy nhất trong `main()`.

---

## 3. Nhóm Bộ lập lịch Tác vụ: `include/scheduler.h` & `src/scheduler.cpp`

Module này hiện thực một **Time-triggered Cooperative Task Scheduler** sử dụng cấu trúc **Delta Queue (Hàng đợi chênh lệch thời gian)**. Đây là kỹ thuật lập lịch kinh điển trong Hệ thống nhúng thời gian thực (RTOS/Embedded Systems) giúp tiết kiệm tài nguyên CPU tối đa.

### 3.1. Phân tích `include/scheduler.h`
* **Mục đích:** Định nghĩa hằng số kích thước hàng đợi và nguyên mẫu các API của bộ lập lịch.
* **Các hằng số:**
  * `TABLE_SIZE 16`: Số lượng tác vụ tối đa có thể quản lý đồng thời trong bảng tác vụ (`task_table`).
  * `QUEUE_SIZE 32`: Kích thước của hàng đợi các tác vụ đã sẵn sàng thực thi (`ready_queue`).
  * `NONE 0xff`: Giá trị chỉ số đặc biệt tương đương con trỏ `NULL` (đánh dấu kết thúc danh sách liên kết).
* **Các hàm công khai (APIs):**
  * `void Scheduler_Dispatch();`
  * `void Scheduler_AddTask(void (*task)(), uint32_t delay, uint32_t period);`
  * `void Scheduler_Step();`

---

### 3.2. Cấu trúc dữ liệu trong `src/scheduler.cpp`

```cpp
struct SchedularEntry {
    void (*task_function)() = nullptr; // Con trỏ tới hàm tác vụ cần chạy
    int32_t timer = 0;                  // Thời gian trễ tương đối (delta ticks)
    int32_t period = 0;                 // Chu kỳ lặp lại (0: chạy một lần; >0: chạy định kỳ)
    EntryIndex next_entry = NONE;       // Chỉ số của node tiếp theo trong delta-linked-list
};
```

* **Khái niệm Delta Queue:** Thay vì mỗi task lưu thời gian tuyệt đối và trong ngắt phải duyệt giảm timer của tất cả 16 task, hệ thống sắp xếp các task thành một danh sách liên kết. Node đầu tiên (`head_task`) lưu thời gian còn lại cho đến khi task đầu tiên kích hoạt. Mỗi node phía sau chỉ lưu **khoảng thời gian chênh lệch** so với node liền trước nó. Nhờ đó, trong ngắt `Scheduler_Step()`, CPU **chỉ cần giảm timer của duy nhất một task ở đầu danh sách (`head_task`)**!

---

### 3.3. Phân tích các hàm trong `src/scheduler.cpp`

#### 1. `InsertDeltaTask(uint8_t index, int32_t delay)`
* **Prototype:** `static void InsertDeltaTask(uint8_t index, int32_t delay)`
* **Chức năng:** Chèn một tác vụ có chỉ số `index` vào danh sách liên kết Delta Queue với thời gian trễ `delay` (đơn vị: tick 10ms).
* **Nguyên lý hoạt động:**
  1. Duyệt qua danh sách bắt đầu từ `head_task`.
  2. So sánh `delay` với `task_table[curr].timer`: Nếu `delay >= timer` của node hiện tại, trừ bớt khoảng thời gian của node đó và tiến đến node tiếp theo (`curr = next_entry`).
  3. Khi tìm được vị trí thích hợp:
     * Gán `task_table[index].timer = delay;`
     * Nối node mới vào danh sách (`next_entry = curr`).
     * Nếu có node phía sau (`curr != NONE`), giảm `timer` của node sau đi một lượng bằng `delay` của node mới (duy trì tính tương đối của Delta Queue).
     * Cập nhật `head_task` nếu node mới được chèn vào đầu hàng.
* **Caller:** Được gọi bên trong `Scheduler_AddTask()` và `Scheduler_Step()`.

#### 2. `Scheduler_AddTask(...)`
* **Prototype:** `void Scheduler_AddTask(void (*task)(), uint32_t delay, uint32_t period)`
* **Tham số:**
  * `task`: Con trỏ hàm của tác vụ cần thực thi.
  * `delay`: Thời gian chờ trước lần chạy đầu tiên (tính theo đơn vị tick 10ms).
  * `period`: Chu kỳ chạy lặp lại (tính theo đơn vị tick 10ms). Nếu `period == 0`, tác vụ là one-shot (chạy 1 lần rồi xóa).
* **Chức năng:** Đăng ký một tác vụ mới vào hệ thống lập lịch.
* **Nguyên lý hoạt động:**
  1. Quét tìm slot trống đầu tiên trong `task_table` (nơi `task_function == nullptr`).
  2. Lưu con trỏ hàm `task` và giá trị `period`.
  3. Gọi `InsertDeltaTask(index, delay)` để đưa tác vụ vào hàng đợi thời gian.
  4. Nếu bảng tác vụ đầy (`TABLE_SIZE`), tác vụ sẽ bị bỏ qua (silently dropped).
* **Caller:** Thường được gọi từ `main()` khi khởi tạo ứng dụng hoặc từ một tác vụ khác muốn lên lịch tác vụ mới.

#### 3. `Scheduler_Step()`
* **Prototype:** `void Scheduler_Step()`
* **Chức năng:** Bộ đếm nhịp thời gian của Scheduler (Tick Engine).
* **Nguyên lý hoạt động:**
  1. Kiểm tra nếu danh sách rỗng (`head_task == NONE`) thì thoát ngay.
  2. Giảm biến đếm thời gian của tác vụ ở đầu hàng: `--task_table[head_task].timer`.
  3. Nếu `timer == 0` (đã đến giờ thực thi):
     * Rút node đó ra khỏi Delta Queue.
     * Đưa chỉ số của task vào hàng đợi chờ thực thi `ready_queue` (vòng tròn FIFO: `queue_back = (queue_back + 1) & (QUEUE_SIZE - 1)`).
     * Nếu tác vụ có chu kỳ lặp (`period != 0`), gọi `InsertDeltaTask()` để tái xếp lịch cho tác vụ vào chu kỳ kế tiếp.
     * Vòng lặp `while` tiếp tục kiểm tra nếu các tác vụ tiếp theo cũng có `timer == 0` (chạy đồng thời) để chuyển hết vào `ready_queue`.
* **Caller:** Được gọi từ hàm callback ngắt Timer `HAL_TIM_PeriodElapsedCallback()` trong `main.cpp` (chạy mỗi 10ms trong ngữ cảnh ngắt ISR).

#### 4. `Scheduler_Dispatch()`
* **Prototype:** `void Scheduler_Dispatch()`
* **Chức năng:** Bộ điều phối thực thi tác vụ (Task Dispatcher).
* **Nguyên lý hoạt động:**
  1. Kiểm tra số lượng tác vụ trong hàng đợi sẵn sàng (`ready_count > 0`).
  2. Lấy chỉ số tác vụ từ đầu hàng đợi `ready_queue` tại vị trí `queue_front` và tăng con trỏ tròn `queue_front`.
  3. Thực thi hàm tác vụ: `func = task_table[index].task_function; if (func) func();`.
  4. Nếu tác vụ là loại one-shot (`period == 0`), giải phóng slot bằng cách gán `task_table[index].task_function = nullptr`.
* **Caller:** Được gọi liên tục trong vòng lặp vô tận `while(1)` trong hàm `main()`. Chạy ở ngữ cảnh Thread (non-ISR), giúp các tác vụ thực thi an toàn mà không làm gián đoạn ngắt.

---

## 4. Nhóm Máy trạng thái Hữu hạn: `include/fsm.h` & `src/fsm.cpp`

Module này là khối não bộ điều khiển (Control Unit) chính của máy giặt, hiện thực máy trạng thái hữu hạn (FSM) dựa trên mô hình bảng chuyển trạng thái (Table-driven State Machine) kết hợp con trỏ hàm (Function Pointers).

### 4.1. Phân tích `include/fsm.h`

* **Enum `FSM_State`:** Định nghĩa 5 trạng thái vận hành của máy giặt:
  * `WM_STATE_STANDBY = 0`: Chế độ chờ phục vụ (RLED sáng, số tiền < 50c).
  * `WM_STATE_READY = 1`: Đã nạp đủ $\ge 50$ cents, chờ người dùng nhấn nút RUN (BLED sáng).
  * `WM_STATE_RUNNING = 2`: Đang thực hiện chu trình giặt, đếm ngược 30 phút (BLED chớp).
  * `WM_STATE_PAUSED = 3`: Tạm dừng giặt, **timer 30 phút vẫn tiếp tục đếm ngược**.
  * `WM_STATE_ERROR = 4`: Trạng thái máy bị lỗi (RLED chớp), chờ nhấn STOP để reset về STANDBY.
* **Enum `FSM_Event`:** Định nghĩa 6 sự kiện tác động vào hệ thống:
  * `WM_EV_COIN = 0`: Sự kiện nạp tiền xu vào máy.
  * `WM_EV_BTN_RUN = 1`: Sự kiện nhấn nút RUN.
  * `WM_EV_BTN_PAUSE = 2`: Sự kiện nhấn nút PAUSE.
  * `WM_EV_BTN_STOP = 3`: Sự kiện nhấn nút STOP.
  * `WM_EV_TIMEOUT = 4`: Sự kiện hết thời gian 30 phút chu trình giặt.
  * `WM_EV_FAULT = 5`: Sự kiện phát hiện lỗi phần cứng/hệ thống.
* **Macro tra cứu chỉ số ma trận `GET_TRANSITION_INDEX(state, event)`:**
  * Hiện tại: `((state) * STATE_COUNT + (event))`
  * *Lưu ý kỹ thuật:* Như đã phân tích ở báo cáo trước, macro này đang có bug (nhân với `STATE_COUNT` thay vì `EVENT_COUNT`).
* **Hàm công khai:** `void Task_FSM();`

---

### 4.2. Các biến tĩnh nội bộ trong `src/fsm.cpp`
* `state_function_table[STATE_COUNT]`: Bảng con trỏ hàm lưu địa chỉ các hàm xử lý tương ứng với từng trạng thái.
* `state_transition_table[STATE_COUNT * EVENT_COUNT]`: Bảng ma trận 2D được phẳng hóa, lưu trạng thái tiếp theo dựa trên cặp `(state, event)`.
* `current_state`: Biến lưu trạng thái hiện tại của FSM (kiểu `FSM_State`).
* `current_state_function`: Con trỏ hàm trỏ tới hàm xử lý của trạng thái hiện tại.
* `init`: Cờ boolean đánh dấu lần chạy đầu tiên để thực hiện khởi tạo hệ thống.

---

### 4.3. Phân tích các hàm trong `src/fsm.cpp`

#### 1. `Task_FSM()`
* **Prototype:** `void Task_FSM()`
* **Chức năng:** Tác vụ FSM chính được Scheduler gọi định kỳ.
* **Nguyên lý hoạt động:**
  1. Kiểm tra cờ `init`: nếu là lần chạy đầu tiên, gọi `StateFunction_INIT()` để nạp bảng hàm và cấu hình ma trận chuyển trạng thái ban đầu, sau đó đặt `init = false`.
  2. Thực thi hàm xử lý của trạng thái hiện tại thông qua con trỏ hàm `current_state_function()`.
* **Caller:** Được Scheduler gọi định kỳ từ `main.cpp`.

#### 2. `EnterNextState(FSM_Event event)`
* **Prototype:** `static void EnterNextState(FSM_Event event)`
* **Tham số:** `event` - Sự kiện xảy ra cần xử lý chuyển trạng thái.
* **Chức năng:** Tra cứu bảng `state_transition_table` để tìm trạng thái kế tiếp và chuyển hướng con trỏ hàm xử lý.
* **Nguyên lý hoạt động:**
  * Tính index: `idx = GET_TRANSITION_INDEX(current_state, event)`.
  * Tìm trạng thái tiếp theo: `next_state = state_transition_table[idx]`.
  * Gán lại con trỏ hàm: `current_state_function = state_function_table[next_state]`.
  * *Lưu ý kỹ thuật:* Hiện tại hàm thiếu dòng gán `current_state = next_state;` dẫn đến biến `current_state` không được cập nhật.
* **Caller:** Được gọi từ các hàm trạng thái khi phát sinh sự kiện chuyển đổi.

#### 3. `StateFunction_INIT()`
* **Prototype:** `static void StateFunction_INIT()`
* **Chức năng:** Thiết lập toàn bộ cấu hình ban đầu cho máy trạng thái FSM.
* **Nguyên lý hoạt động:**
  1. Đăng ký địa chỉ các hàm xử lý trạng thái vào mảng `state_function_table`:
     * Index 0 (`WM_STATE_STANDBY`) $\rightarrow$ `StateFunction_STANDBY`
     * Index 1 (`WM_STATE_READY`) $\rightarrow$ `StateFunction_READY`
     * Index 2 (`WM_STATE_RUNNING`) $\rightarrow$ `StateFunction_RUNNING`
     * Index 3 (`WM_STATE_PAUSED`) $\rightarrow$ `StateFunction_PAUSED`
     * Index 4 (`WM_STATE_ERROR`) $\rightarrow$ `StateFunction_ERROR`
  2. Cấu hình bảng chuyển trạng thái `state_transition_table`:
     * `(STANDBY, COIN)` $\rightarrow$ `READY`
     * `(READY, BTN_RUN)` $\rightarrow$ `RUNNING`
     * `(RUNNING, BTN_PAUSE)` $\rightarrow$ `PAUSED`
     * `(RUNNING, BTN_STOP)` $\rightarrow$ `STANDBY`
     * `(RUNNING, TIMEOUT)` $\rightarrow$ `STANDBY`
     * `(PAUSED, BTN_RUN)` $\rightarrow$ `RUNNING`
     * `(PAUSED, TIMEOUT)` $\rightarrow$ `STANDBY`
  3. Khởi tạo trạng thái ban đầu: `current_state = WM_STATE_STANDBY` và `current_state_function = StateFunction_STANDBY`.
* **Caller:** Gọi 1 lần duy nhất trong `Task_FSM()` khi cờ `init == true`.

#### 4. `StateFunction_STANDBY()`
* **Prototype:** `static void StateFunction_STANDBY()`
* **Chức năng:** Hàm xử lý nghiệp vụ khi máy ở trạng thái chờ (**STANDBY**).
* **Hiện trạng trong code:** Hiện tại đang có một lệnh test bật/tắt LED xanh `HAL_GPIO_TogglePin(GPIOB, PIN_LED_BLUE);` và các dòng chú thích `//TODO`.
* **Nhiệm vụ cần hoàn thiện (TODO):** Bật RLED sáng tĩnh, tắt BLED; lắng nghe sự kiện nạp tiền (10c, 20c, 50c), tích lũy biến tiền; nếu tổng tiền $\ge 50$ cents thì kích hoạt chuyển sang trạng thái `READY`.

#### 5. `StateFunction_READY()`
* **Prototype:** `static void StateFunction_READY()`
* **Chức năng:** Hàm xử lý nghiệp vụ khi máy đã đủ tiền và sẵn sàng giặt (**READY**).
* **Hiện trạng trong code:** Đang trống (`//TODO`).
* **Nhiệm vụ cần hoàn thiện (TODO):** Tắt RLED, bật BLED sáng tĩnh; tiếp tục nhận tiền nếu có (không thối lại); lắng nghe nút RUN: nếu nhấn RUN thì xóa biến tiền về 0 (`money = 0`), khởi động timer 30 phút và chuyển sang trạng thái `RUNNING`.

#### 6. `StateFunction_RUNNING()`
* **Prototype:** `static void StateFunction_RUNNING()`
* **Chức năng:** Hàm xử lý nghiệp vụ khi máy đang thực hiện chu trình giặt (**RUNNING**).
* **Hiện trạng trong code:** Đang trống (`//TODO`).
* **Nhiệm vụ cần hoàn thiện (TODO):** Điều khiển BLED nhấp nháy; đếm ngược timer 30 phút; lắng nghe nút PAUSE để chuyển sang `PAUSED`; lắng nghe nút STOP nhấn 2 lần liên tiếp để dừng cưỡng bức (Force Stop); bắt sự kiện hết giờ (Timeout) để hoàn thành chu trình giặt và quay về `STANDBY`.

#### 7. `StateFunction_PAUSED()`
* **Prototype:** `static void StateFunction_PAUSED()`
* **Chức năng:** Hàm xử lý nghiệp vụ khi máy đang tạm dừng giặt (**PAUSED**).
* **Hiện trạng trong code:** Đang trống (`//TODO`).
* **Nhiệm vụ cần hoàn thiện (TODO):** Giữ BLED sáng tĩnh; **tiếp tục đếm ngược timer 30 phút**; lắng nghe nút RUN để tiếp tục chu trình giặt (`RUNNING`); lắng nghe nút STOP nhấn 2 lần hoặc sự kiện Timeout để quay về `STANDBY`.

#### 8. `StateFunction_ERROR()`
* **Prototype:** `static void StateFunction_ERROR()`
* **Chức năng:** Hàm xử lý nghiệp vụ khi hệ thống gặp sự cố (**ERROR**).
* **Hiện trạng trong code:** Đang trống (`//TODO`).
* **Nhiệm vụ cần hoàn thiện (TODO):** Bật RLED nhấp nháy báo động, tắt BLED; lắng nghe nút STOP để reset lỗi và đưa máy về trạng thái an toàn `STANDBY`.

---

## 5. Nhóm Điểm khởi đầu Ứng dụng: `src/main.cpp`

File này là điểm xuất phát chính (Entry Point) của toàn bộ chương trình vi điều khiển.

### 5.1. Phân tích các hàm trong `src/main.cpp`

#### 1. `Task_BlinkLED()`
* **Prototype:** `void Task_BlinkLED()`
* **Chức năng:** Tác vụ nhấp nháy đèn LED nhịp tim hệ thống.
* **Nguyên lý hoạt động:** Gọi hàm `HAL_GPIO_TogglePin(GPIOB, PIN_LED_BLINK)` để đảo trạng thái chân PB15.
* **Caller:** Được Scheduler gọi định kỳ mỗi 1 giây (100 ticks $\times$ 10ms). Giúp lập trình viên quan sát hệ thống còn sống hay đã bị treo (Heartbeat signal).

#### 2. `main(void)`
* **Prototype:** `int main(void)`
* **Chức năng:** Hàm khởi động chính của vi điều khiển.
* **Nguyên lý hoạt động:**
  1. `HAL_Init()`: Khởi tạo thư viện STM32 HAL, cấu hình Flash prefetch và vector ngắt.
  2. `SetupPinMode()`: Cấu hình tất cả các chân GPIO LED và Button.
  3. `InitTIM()`: Cấu hình Timer TIM2 tạo ngắt cơ sở 10ms.
  4. Đăng ký các task vào Scheduler:
     * `Scheduler_AddTask(Task_BlinkLED, 100, 100)`: Chạy sau 1s, lặp lại mỗi 1s.
     * `Scheduler_AddTask(Task_FSM, 1, 100)`: Chạy sau 10ms, lặp lại mỗi 1s.
  5. Vòng lặp vô tận `while (1)`:
     * Liên tục gọi `Scheduler_Dispatch()` để lấy các tác vụ đã sẵn sàng trong `ready_queue` ra thực thi.
     * Gọi lệnh ASM `__WFI()` (Wait For Interrupt): Đưa nhân Cortex-M vào chế độ Sleep tiết kiệm điện năng cho đến khi có ngắt tiếp theo (chú thích trong code: *"Cho Proteus đỡ bùng nổ"* - giảm tải CPU máy tính khi mô phỏng trên Proteus).

#### 3. `HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)`
* **Prototype:** `void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)`
* **Chức năng:** Hàm callback tiêu chuẩn của thư viện STM32 HAL được gọi khi một bộ định thời xảy ra sự kiện tràn chu kỳ đếm.
* **Nguyên lý hoạt động:** Kiểm tra con trỏ ngoại vi nếu nguồn ngắt đúng là Timer 2 (`htim->Instance == TIM2`), hàm sẽ gọi `Scheduler_Step()` để cập nhật Delta Queue và kích hoạt các task đã đến hạn vào `ready_queue`.
* **Caller:** Được gọi từ hàm `HAL_TIM_IRQHandler(&htim2)` bên trong `TIM2_IRQHandler()`.

---

## 6. Nhóm Script Cấu hình Build: `platformio.ini` & `extra_script.py`

### 6.1. `platformio.ini`
* **Mục đích:** File cấu hình cho môi trường build PlatformIO.
* **Nội dung:**
  * Target board: `genericSTM32F103C6` (dòng vi điều khiển STM32 Blue Pill giá rẻ, 32KB Flash, 10KB RAM).
  * Framework: `stm32cube` (sử dụng thư viện chuẩn STM32Cube HAL do ST cung cấp).
  * `build_flags = -g`: Kèm thông tin debug khi biên dịch.
  * `extra_scripts = post:extra_script.py`: Khai báo script Python chạy tự động sau khi biên dịch xong để sinh file HEX.

### 6.2. `extra_script.py`
* **Hàm `make_hex(source, target, env)`:**
  * **Chức năng:** Trích xuất file thực thi nhị phân `.hex` từ file đối tượng `.elf`.
  * **Nguyên lý:** Gọi công cụ `arm-none-eabi-objcopy -O ihex -R .eeprom <file.elf> <file.hex>`.
  * **Ý nghĩa thực tế:** File `.hex` là định dạng chuẩn bắt buộc cần nạp vào phần mềm mô phỏng mạch điện tử **Proteus** để chạy thử nghiệm.

---

## 7. Bảng Tổng hợp Toàn diện (Cheat-Sheet Ma trận Hàm)

| Tên File | Tên Hàm | Kiểu trả về | Tham số | Ngữ cảnh thực thi | Trạng thái hiện tại |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `helper.cpp` | `SysTick_Handler` | `void` | `void` | Ngắt ISR (1ms) | Hoàn thiện |
| `helper.cpp` | `TIM2_IRQHandler` | `void` | `void` | Ngắt ISR (10ms) | Hoàn thiện |
| `helper.cpp` | `SetupPinMode` | `void` | `void` | Khởi tạo (Thread) | Hoàn thiện |
| `helper.cpp` | `InitTIM` | `void` | `void` | Khởi tạo (Thread) | Hoàn thiện |
| `scheduler.cpp`| `InsertDeltaTask` | `void` | `uint8_t index, int32_t delay` | Nội bộ Scheduler | Hoàn thiện |
| `scheduler.cpp`| `Scheduler_AddTask`| `void` | `void (*task)(), uint32_t delay, uint32_t period` | Khởi tạo / Thread | Hoàn thiện |
| `scheduler.cpp`| `Scheduler_Step` | `void` | `void` | Ngắt ISR (10ms) | Hoàn thiện |
| `scheduler.cpp`| `Scheduler_Dispatch`| `void`| `void` | Vòng lặp chính `main()`| Hoàn thiện |
| `fsm.cpp` | `Task_FSM` | `void` | `void` | Tác vụ Scheduler | Khung cơ bản |
| `fsm.cpp` | `EnterNextState` | `void` | `FSM_Event event` | Nội bộ FSM | ⚠️ Cần sửa (thiếu cập nhật `current_state`) |
| `fsm.cpp` | `StateFunction_INIT` | `void` | `void` | Khởi tạo FSM | ⚠️ Cần sửa bảng chuyển trạng thái |
| `fsm.cpp` | `StateFunction_STANDBY`| `void`| `void` | Tác vụ FSM | ⚠️ Đang TODO (chỉ có code test) |
| `fsm.cpp` | `StateFunction_READY` | `void`| `void` | Tác vụ FSM | ⚠️ Đang TODO |
| `fsm.cpp` | `StateFunction_RUNNING`| `void`| `void` | Tác vụ FSM | ⚠️ Đang TODO |
| `fsm.cpp` | `StateFunction_PAUSED` | `void`| `void` | Tác vụ FSM | ⚠️ Đang TODO |
| `fsm.cpp` | `StateFunction_ERROR` | `void`| `void` | Tác vụ FSM | ⚠️ Đang TODO |
| `main.cpp` | `Task_BlinkLED` | `void` | `void` | Tác vụ Scheduler | Hoàn thiện |
| `main.cpp` | `main` | `int` | `void` | Khởi động hệ thống | Hoàn thiện |
| `main.cpp` | `HAL_TIM_PeriodElapsedCallback`| `void`| `TIM_HandleTypeDef *htim`| Ngắt ISR (10ms) | Hoàn thiện |
| `extra_script.py`| `make_hex` | `void` | `source, target, env` | Build Hook | Hoàn thiện |

---

> [!TIP]
> Tài liệu này cung cấp bức tranh toàn cảnh về cách các thành phần trong codebase liên kết với nhau. Khi bắt tay vào code, chúng ta chỉ cần tập trung hoàn thiện các hàm FSM trong [src/fsm.cpp](file:///c:/Users/ADMIN/Documents/ES-WashingMachine_FSM_Implementation/STM-FSM_WashingMachine/src/fsm.cpp) và bổ sung module đọc nút bấm/điều khiển LED mà không cần phải can thiệp hay làm xáo trộn cấu trúc của bộ Scheduler và HAL đã ổn định.
