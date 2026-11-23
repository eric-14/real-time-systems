# 🕒 Real-Time Systems: High-Frequency Clock Generation on ARM Cortex-M3

### Objective

Generate a stable 300 kHz square wave using General-Purpose Input/Output (GPIO) pins on the Arduino Due (AT91SAM3X8E microcontroller, ARM Cortex-M3 core), addressing timing constraints imposed by high-level abstractions and system interruptions.


## 📉 Initial Limitations with Arduino API

When using the standard Arduino API (e.g., digitalWrite()), the achievable square-wave frequency was capped at < 100 kHz, primarily due to:


    Software overhead: digitalWrite() executes multiple instructions (typically > 2 CPU cycles), limiting switching speed.

    Inflexible clocking: The Arduino Tone() or PWM APIs lack nanosecond-level precision, making them unsuitable for deterministic high-frequency generation.


These limitations arise from the abstraction layer added by the Arduino core, which trades performance and determinism for portability and ease of use.


## 🔧 Optimizations Implemented




Optimization
	Technical Approach
	Benefit




1. Direct Register Access
	Bypassed Arduino API; used port register manipulation (PIO->SODR, PIO->CODR) with bitwise masking for ±1-cycle pin toggling.
	Reduced toggle latency to ~2 CPU cycles (at 84 MHz, ≈24 ns per operation).


2. Enhanced Clock Source
	Leveraged the Debug Clock (DCK) or Master Clock (MCK) divided via the Generic Clock (GCLK) subsystem for precise timing control (where applicable).
	Enabled sub-microsecond period control — essential for reaching ≥300 kHz targets.


3. Interrupt Disabling
	Disabled all non-critical interrupts (especially SysTick, USART, USB) during critical timing loops using __disable_irq() / __enable_irq().
	Eliminated interrupt-induced jitter (up to ~10–50 µs latency on Due), ensuring jitter < 1 cycle (≈12 ns).




## ✅ Results


    Achieved Frequency: 300.0 kHz (target: 300 kHz ±0.1% tolerance)

    Jitter: < 12 ns (single-cycle variation)

    Duty Cycle: Adjustable via loop unrolling (e.g., 50% with symmetric timing)

    Signal Purity: Clean square wave (no dead time, minimal ringing with proper load)



    Verification: Measured using oscilloscope (1 GHz bandwidth) and logic analyzer (400 MSa/s sampling).



## 📚 Key Takeaways (for RT Systems Design)


    High-level abstractions (e.g., Arduino digitalWrite()) are unsuitable for deterministic real-time tasks.

    For sub-microsecond precision, register-level programming and intelligent interrupt management are essential.

    The ARM Cortex-M3’s clocking architecture provides flexible clock sources — but they must be configured directly for time-critical GPIO tasks.



## 🛠️ Hardware/Software Stack


    Board: Arduino Due (AT91SAM3X8E, ARM Cortex-M3 @ 84 MHz)

    Toolchain: Arduino IDE (C/C++), Atmel Studio optional for deeper peripheral access

    Libraries Used: None (pure CMSIS-style register programming)

    Dependencies: None beyond Arduino core (but avoided for this module)

