# Pengenalan-RTOS-dan-Embedded-Toolchain
Hands-on Mentoring Karir Embedded Software Engineering MENTARI HME

## Project Pertemuan #2

Spesifikasi : 

1. Input push button dan Output LED red, green, blue
2. Red LED default OFF, blue LED default ON, green LED blink continuously 300 ms cycle
3. Short press button to toggle blue and red LED state
4. Long press button to reboot blue and red LED state
5. Button, red LED, blue LED, and green LED masing-masing thread terpisah
6. Button pakai polling 1kHz
7. Tidak menggunakan external library selain bawaan ESP

Notes :

1. Awalnya pakai xQueuesend dan xQueuereceive buat passing info dari task button ke task led apakah lagi short press atau long press. Tapi jadinya switch statenya ga barengan, sekali press hanya satu LED yang switch state. Terus baca baca terus coba pakai eventgroup and it works.
2. wawawiwawawa
3. help sampe skrg polling 100 Hz, kalo 1kHz keluar warning ini apaan

E (40336) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:  
E (40336) task_wdt:  - IDLE (CPU 1)  
E (40336) task_wdt: Tasks currently running:  
E (40336) task_wdt: CPU 0: IDLE  
E (40336) task_wdt: CPU 1: Button Task  
E (40336) task_wdt: Print CPU 1 backtrace  