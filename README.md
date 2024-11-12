# Usage for LVD


## Update dtoverlay
```sh
cd lvd/dtoverlay
dtc -@ -I dts -O dtb -o ed-pca953x.dtbo ed-pca953x-overlay.dts
sudo cp ./ed-pca953x.dtbo /boot/overlays/
```

## Modify config.txt
```sh
sudo nano /boot/config.txt
#Modify dtoverlay=ed-pca953x according to the following parameters
#For IPC21xx,IPC26xx,HMI21xx series
dtoverlay=ed-pca953x,ipc2110,addr=0x20
#For IPC22xx,HMI22xx series
dtoverlay=ed-pca953x,ipc2210,addr=0x20
```
Reboot the device after completing the modifications.

## Install gpiod library

```sh
sudo apt-get -y install libgpiod-dev
sudo apt-get install gpiod
```

## Compile and Run
```sh
cd lvd
make
```
You will get an executable file `lvd`.

### Manual testing

```sh
./lvd
```

Turn off the 12V main power supply of the device, you will obtain the following output:
```sh
Low voltage
LVD trigger callback script
```
**NOTE: You can modify the macro `LVD_ HOOK_ EXEC` specifies a custom callback script**


### Using the lvd-detect.service
```sh
sudo cp lvd /usr/sbin/
sudo cp lvd-callback.sh /usr/sbin/
sudo cp service/lvd-en.sh /usr/sbin/
sudo cp service/lvd-detect.service /lib/systemd/system/

sudo systemctl enable lvd-detect.service
sudo systemctl start lvd-detect.service
```
The output of `sudo cat /sys/kernel/debug/gpio`:
```sh
gpiochip2: GPIOs 488-503, parent: i2c/10-0020, 10-0020, can sleep:
 gpio-488 (5V_GOOD             )
 gpio-489 (LVD                 |falling edge        ) in  hi IRQ
 gpio-490 (BUZZER_EN           )
 gpio-491 (4G_RST              )
 gpio-492 (4G_LED              )
 gpio-493 (USER_LED            )

```
**NOTE: For IPC2110 and IPC2210, the LVD signal is connected to the Pin1 of gpiochip2**

Turn off the 12V main power supply of the device, run the following command line
```sh
tail /var/log/messages
```
You will obtain the following output:
```sh
raspberrypi root: LVD trigger callback script
```
**NOTE: You can modify the content of the script `lvd-callback.sh` according to the needs of your application**


### LVD GPIO
Add read config: /etc/lvd/config.ini
```ini
[gpio]
gpiochip=0
line=11
```

use [iniparser](https://github.com/ndevilla/iniparser)
```bash
# Compile iniparser
mkdir build && cd build
cmake .. -DBUILD_DOCS=off  && make
```