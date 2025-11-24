# 🔌 Circuit Connection Guide  
### For ESP8266 Telegram Ignition System  
**Author: Biswajit**  
> ✔ Works with Relay OR MOSFET ignition driver.

---

## 🟢 Beginner Wiring (Simple & Safe)

This version is easy and ideal for new users. Uses a relay module (already isolated and safe).

### 🧱 Components Required
- ESP8266 (NodeMCU / Wemos D1 Mini)
- 1-Channel Relay Module (Low Trigger Recommended)
- 5V Power Supply
- Jumper wires

---

### 🧩 Wiring Table

| ESP8266 Pin | → | Relay Module |
|------------|---|--------------|
| 5V (VIN)   | → | VCC |
| GND        | → | GND |
| D5 (GPIO14)| → | IN |

---

### 🔥 Ignition Load Wiring

| Relay Terminal | Connect To |
|---------------|------------|
| COM | Positive Power Source (+) |
| NO | Ignition Device Input (+) |
| Load Output | Ignition Device Output → Ground |

👉 When `/on` command is sent:  
Relay closes **NO → COM** and ignition fires.

---

### ⚠ Safety Notes

- Common **GND required** between ESP8266 and relay.
- Keep ignition output wires away from ESP circuits.

---

---

## 🟣 Advanced Wiring (MOSFET High-Speed Version)

This version gives **silent operation**, **fast response**, and supports **high current ignition coils**.

### 🧱 Components Required
- ESP8266
- Logic Level MOSFET (**IRLZ44N / AO3400 / IRLZ34** recommended)
- 1× 220Ω Resistor (Gate Input)
- 1× 10K Resistor (Gate Pull-down)
- 1× Diode (**1N4007** or **UF4007**) across load
- External power supply (5–36V depending load)

---

### 🧩 Wiring Table

| ESP8266 Pin | → | Component |
|------------|---|-----------|
| D5 (GPIO14) | → | 220Ω resistor → MOSFET Gate |
| GND | → | MOSFET Source |
| External Power (+) | → | Ignition Load → MOSFET Drain |

---

### 🔧 Circuit Details

```
 ESP D5 → 220Ω → Gate
 Gate → 10K → GND

 Load + → Power Supply +
 Load - → MOSFET Drain
 MOSFET Source → Ground

 Protection Diode: Across load (reverse direction)
```

---

### 🔥 Why MOSFET Version?

| Feature | Relay | MOSFET |
|--------|--------|--------|
| Noise | Click sound | 🔇 Silent |
| Speed | Slow | ⚡ Ultra-fast |
| Current Handling | Medium | 🔥 High Current |
| Lifetime | Mechanical wear | Infinite (solid state) |
| Reliability | Good | ⭐ Professional Grade |

---

## 🧠 Notes

- All grounds **MUST** be common.
- If using more than **5A**, use heat sink on MOSFET.
- For explosive ignition or high-voltage coil, use optocoupler isolation.

---

## 🏁 Conclusion

| Version | Best For |
|---------|----------|
| 🟢 Beginner Relay Version | New users, low current loads, safe setup |
| 🟣 Advanced MOSFET Version | High power ignition, silent fast control, professional use |

---

### ⭐ If this helped — don’t forget to give the repository a **Star ⭐ on GitHub!**
