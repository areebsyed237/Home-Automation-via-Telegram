# Home Automation via Telegram

### Introduction
This project uses an __esp32-cam__ developer board to build a system that allows the user to control electric devices, request a photo, control flash-LED and get temperature readings via WiFi, interfaced through a __telegram bot__ API.<br>
Further modifications can be added to include _PIR Sensor_ triggered photo capture, _LDR_ controlled lighting, staircase switching for manual over-ride and many more such features.<br>
### Components
* ESP32-CAM Development Board
* FTDI Serial Adapter Module (USB to TTL)
* DS18B20 Temperature Sensor
* 5V Relay Module 
* Power Supply Module
* 12V DC Adapter
* LED & Resistors (1k & 4.7k ohm)
* Breadboard & Jumper Wires

### Circuit Diagram & Setup
<table style="margin: 0 auto;">
  <tr>
    <td>
        <img src="jarvis_schematic.png" alt="schematic" width="500"/>
        <p style="font-size: 14px; text-align: center;"> Circuit Schematic Diagram  </p>
    </td>
    <td style="font-size: 32px; text-align: center; vertical-align: middle;">&rarr;</td>
    <td>
        <img src="project_setup.jpg" alt="setup" width="312"/>
        <p style="font-size: 14px; text-align: center;"> Actual Setup </p>
    </td>
  </tr>
</table>

### Telegram Integration
<table style="margin: 0 auto;">
  <tr>
    <td><img src="Screenshots/001.jpg" alt="img1" width="200"/></td>
    <td><img src="Screenshots/002.jpg" alt="img2" width="200"/></td>
    <td><img src="Screenshots/003.jpg" alt="img3" width="200"/></td>
    <td><img src="Screenshots/004.jpg" alt="img4" width="200"/></td>
  </tr>
  </table>