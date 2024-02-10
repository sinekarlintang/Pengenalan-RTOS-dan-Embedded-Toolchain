# Pengenalan-RTOS-dan-Embedded-Toolchain
Hands-on Mentoring Karir Embedded Software Engineering MENTARI HME

## Project Pertemuan #2

Spesifikasi : 

1. Input push button dan Output LED red, green, blue ✔
2. Red LED default OFF, blue LED default ON, green LED blink continuously 300 ms cycle ✔
3. Short press button to toggle blue and red LED state ✔
4. Long press button to software reboot ✔
5. Button, red LED, blue LED, and green LED masing-masing thread terpisah ✔
6. Button pakai polling 1kHz 
7. Tidak menggunakan external library selain bawaan ESP ✔ 

Notes :

1. Awalnya pakai xQueuesend dan xQueuereceive buat passing info dari task button ke task led apakah lagi short press atau long press. Tapi jadinya switch statenya ga barengan, sekali press hanya satu LED yang switch state. Terus baca baca terus coba pakai eventgroup and it works.
2. Polling fixed to 1khz dengan naikin priority button task.

## Project Pertemuan #3

Spesifikasi : 

1. Add UART based command: ✔  
BLUE;ON -> turn blue led on  
BLUE;OFF -> turn blue led off  
RED;ON -> turn red led on  
RED;OFF -> turn red led off  
SWITCH -> switch led state  
DUMPLOG -> dump all stored event logs    
2. Add UART based event logs ✔
3. Add external ring buffer library https://github.com/MaJerle/lwrb ✔
4. Store 31 event logs to ring buffer, drop new logs when full. 

Notes :