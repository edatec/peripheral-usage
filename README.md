# Usage for LVD


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
