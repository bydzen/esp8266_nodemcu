# esp8266_nodemcu
All project about ESP8266/NodeMCU Microcontroller🤖

## dht11-led-automation
The DHT11 and LED automation. If the temperature reach 30°C then the Red LED will turn on.
Each 10 minutes the humidity and temperature data send to firebase. If there any data changes
on DHT11 it will be updated to Blynk and write the data to virtual pin.


## dht11-led-buzzer-automation
LED and Buzzer automation based on DHT11 inputs. The data send to Firebase and send to Blynk.
Can be use automation with Google Colab with python and Firebase library/packages.
