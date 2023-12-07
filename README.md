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
#For IPC2100,IPC2600,HMI2100 series
dtoverlay=ed-pca953x,ipc2110,addr=0x20
#For IPC2200,HMI2200 series
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
./lvd
```

Turn off the 12V main power supply of the device, you will obtain the following output:
```sh
Low voltage
LVD trigger callback script
```
**NOTE: You can modify the macro `LVD_ HOOK_ EXEC` specifies a custom callback script**

At this point,the output of `sudo cat /sys/kernel/debug/gpio`:
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
