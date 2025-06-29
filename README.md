# Share Emoji 😄📲

Effortlessly share your favorite emojis between devices!  
**Share Emoji** is built for the GUITION 1.8" 360x360 ESP32-S3-JC3636W518, making emoji communication fun, fast, and wireless via WebSocket ([sockethook.ericbetts.dev](https://sockethook.ericbetts.dev/)). 🚀✨

---

## ✨ Features

- **Instant Emoji Sharing:** Send and display emojis at the tap of a finger!
- **WebSocket Powered:** Real-time emoji sync across devices. 🌐
- **Easy Setup:** Simple WiFi configuration with AutoConnectAP.
- **Vibrant Display:** Optimized for the 360x360 color screen. 📺
- **Open Source & Modular:** Build, extend, or integrate with your own emoji projects.

---

## 🛠️ Usage

1. **Power Up:** 🔌 Plug in the device.  
   _First time? Connect to **AutoConnectAP** to configure WiFi._ 📶
2. **Select Emoji:** 👉 Tap an emoji on the ring to choose it.
3. **See the Magic:** 💬 The selected emoji appears at the center when a WebSocket event is received. 😃

> _Tip: If the emoji doesn't appear in the center after selection, check your WiFi or WebSocket connection. ❌_

---

## 📦 Project Structure

- `src/` – Core firmware (C/C++)
- `include/` – Header files
- `build/` – Compiled binaries
- `Makefile` – Build configuration

---

## 🚀 Getting Started

### Requirements

- ESP32-S3-JC3636W518 board
- GCC or compatible C/C++ compiler
- PlatformIO or Make
- WiFi access
- SD Card with emoji

### Build & Flash

1. **Clone the repo:**
    ```sh
    git clone https://github.com/bettse/share_emoji.git
    cd share_emoji
    ```
2. **Build with Make:**
    ```sh
    make
    ```

3. **Flash to device** (check your board's method):
    ```sh
    make upload
    ```

---

## 🧩 Based on

- [Skynerz's ESP32_Display_Panel-Guition_JC3636W518](https://github.com/Skynerz/ESP32_Display_Panel-Guition_JC3636W518)
- PlatformIO ⚙️

---

> _Share your smile, one emoji at a time!_ 😁🚀
