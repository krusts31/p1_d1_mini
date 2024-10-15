# USES ESP8266 LOLINMINI D1
```sh
#find your connected ESP8266
sudo dmesg | grep tty
#find the path to it
ls /dev/
sudo docker build -t esp8266 .
#--device give the path to esp8266
sudo docker run -ti --device=/dev/ttyUSB0 esp8266 bash
#that is it
```
##### DEMO:
https://youtu.be/gyAwr1sEQN0



if on windows
to add https://learn.microsoft.com/en-us/windows/wsl/connect-usb#attach-a-usb-device.
