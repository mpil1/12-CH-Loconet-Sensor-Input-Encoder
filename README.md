# 12-CH-Loconet-Sensor-Input-Encoder
Arduino based 12 Channel Loconet Sensor Input Encoder
The 12 Channel Loconet Sensor Input Encoder monitors the voltage at 12 discrete inputs (0-18VDC, schematic to come!!...) and sends 
 Sensor nnnn ON or Sensor nnnn OFF messages over Loconet. (nnnn being a number from 1 to 4095).  The device has its start address 
 assgined over Loconet by sending a Switch Thrown command for a number corresponding to the desired start address.
