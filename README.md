# i.MX 93 and STM32 Sensor Communication
FRDM i.MX93 runs Linux and connects through UART to Nucleo L476RG. Sends commands that are interpreted by STM32 and response is sent back. STM32 is connected with I2C to BME280.

Hardware used:
* FRDM i.MX 93
* STM32 Nucleo L476RG
* BME280

Each board subfolder discusses indepth their respective sections.
frdm-imx93 - UART configuration in device tree using Yocto, 
stm32* - UART and I2C configuration, commands implementation 
*NOTE - this part is based on my older project, original can be found here. Some modifications were introduced.

```
    Linux                  No OS                Sensor
+-----------+           +---------+           +--------+
|  i.MX 93  |           |  STM32  |    I2C    | BME280 |
|           |           |  PB9 SDA|-----------|SDA     |
|           |   UART    |  PB8 SCL|-----------|SCL     |
| GPIO_14 TX|-----------|RX PA3   |           +--------+
| GPIO_15 RX|-----------|TX PA2   |
+-----------+           +---------+

```

