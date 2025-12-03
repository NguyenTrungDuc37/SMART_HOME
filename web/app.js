const socket = io();

// ============================================
// REFERENCES CÁC PHẦN TỬ HTML
// ============================================
const connStatus = document.getElementById("connStatus");
const tempEl = document.getElementById("temp");
const humidityEl = document.getElementById("humidity");
const gasEl = document.getElementById("gas");
const gasSensorEl = document.getElementById("gasSensor");
const gasWarningEl = document.getElementById("gasWarning");
const lastUpdateEl = document.getElementById("lastUpdate");

// Progress bars
const tempBar = document.getElementById("tempBar");
const humidityBar = document.getElementById("humidityBar");
const gasBar = document.getElementById("gasBar");

// Trạng thái thiết bị
const lightStatusEl = document.getElementById("lightStatus");
const fanStatusEl = document.getElementById("fanStatus");
const doorStatusEl = document.getElementById("doorStatus");

// Nút điều khiển
const btnLightOn = document.getElementById("btnLightOn");
const btnLightOff = document.getElementById("btnLightOff");
const btnFanOn = document.getElementById("btnFanOn");
const btnFanOff = document.getElementById("btnFanOff");
const btnDoorOpen = document.getElementById("btnDoorOpen");
const btnDoorClose = document.getElementById("btnDoorClose");

// ============================================
// XỬ LÝ KẾT NỐI WEBSOCKET
// ============================================
socket.on("connect", () => {
  connStatus.classList.remove("disconnected");
  connStatus.classList.add("connected");
  connStatus.querySelector(".status-text").innerText = "Đã kết nối";
});

socket.on("disconnect", () => {
  connStatus.classList.remove("connected");
  connStatus.classList.add("disconnected");
  connStatus.querySelector(".status-text").innerText = "Mất kết nối";

  tempEl.innerText = "--";
  humidityEl.innerText = "--";
  gasEl.innerText = "--";
  tempBar.style.width = "0%";
  humidityBar.style.width = "0%";
  gasBar.style.width = "0%";
});

// ============================================
// NHẬN DỮ LIỆU CẢM BIẾN
// ============================================
socket.on("sensor-update", (data) => {
  if (data.temp !== undefined && data.temp !== "--") {
    const temp = parseFloat(data.temp);
    tempEl.innerText = temp.toFixed(1);
    tempBar.style.width = Math.min((temp / 50) * 100, 100) + "%";
  }

  if (data.humidity !== undefined && data.humidity !== "--") {
    const humidity = parseFloat(data.humidity);
    humidityEl.innerText = humidity.toFixed(1);
    humidityBar.style.width = humidity + "%";
  }

  if (data.gas !== undefined && data.gas !== "--") {
    const gasValue = parseFloat(data.gas);
    gasEl.innerText = gasValue.toFixed(0);
    gasBar.style.width = Math.min((gasValue / 1000) * 100, 100) + "%";

    // CẢNH BÁO KHÍ GAS (ngưỡng 300 ppm)
    if (gasValue > 300) {
      gasWarningEl.classList.remove("hidden");
      gasSensorEl.classList.add("warning-gas");
    } else {
      gasWarningEl.classList.add("hidden");
      gasSensorEl.classList.remove("warning-gas");
    }
  }

  if (data.lastUpdate) {
    const time = new Date(data.lastUpdate).toLocaleTimeString("vi-VN");
    lastUpdateEl.innerText = "Cập nhật: " + time;
  }
});

// ============================================
// NHẬN TRẠNG THÁI THIẾT BỊ
// ============================================
socket.on("device-status", (status) => {
  if (status.light) {
    lightStatusEl.innerText = status.light === "ON" ? "Đang bật" : "Tắt";
    lightStatusEl.style.color = status.light === "ON" ? "#10b981" : "#94a3b8";
  }

  if (status.fan) {
    fanStatusEl.innerText = status.fan === "ON" ? "Đang bật" : "Tắt";
    fanStatusEl.style.color = status.fan === "ON" ? "#10b981" : "#94a3b8";
  }

  if (status.door) {
    doorStatusEl.innerText = status.door === "OPEN" ? "Đang mở" : "Đóng";
    doorStatusEl.style.color = status.door === "OPEN" ? "#3b82f6" : "#94a3b8";
  }
});

// ============================================
// ĐIỀU KHIỂN ĐÈN
// ============================================
btnLightOn.addEventListener("click", () => {
  socket.emit("light-on");
  lightStatusEl.innerText = "Đang xử lý...";
  btnLightOn.disabled = true;
  setTimeout(() => (btnLightOn.disabled = false), 1000);
});

btnLightOff.addEventListener("click", () => {
  socket.emit("light-off");
  lightStatusEl.innerText = "Đang xử lý...";
  btnLightOff.disabled = true;
  setTimeout(() => (btnLightOff.disabled = false), 1000);
});

// ============================================
// ĐIỀU KHIỂN QUẠT
// ============================================
btnFanOn.addEventListener("click", () => {
  socket.emit("fan-on");
  fanStatusEl.innerText = "Đang xử lý...";
  btnFanOn.disabled = true;
  setTimeout(() => (btnFanOn.disabled = false), 1000);
});

btnFanOff.addEventListener("click", () => {
  socket.emit("fan-off");
  fanStatusEl.innerText = "Đang xử lý...";
  btnFanOff.disabled = true;
  setTimeout(() => (btnFanOff.disabled = false), 1000);
});

// ============================================
// ĐIỀU KHIỂN CỬA
// ============================================
btnDoorOpen.addEventListener("click", () => {
  socket.emit("door-open");
  doorStatusEl.innerText = "Đang xử lý...";
  btnDoorOpen.disabled = true;
  setTimeout(() => (btnDoorOpen.disabled = false), 1000);
});

btnDoorClose.addEventListener("click", () => {
  socket.emit("door-close");
  doorStatusEl.innerText = "Đang xử lý...";
  btnDoorClose.disabled = true;
  setTimeout(() => (btnDoorClose.disabled = false), 1000);
});
