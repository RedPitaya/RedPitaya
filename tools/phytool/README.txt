This is an application for changing registers in the lantiq chip.

Usage example:

read - ./phytool end0/1/0x17
write - ./phytool write end0/1/0x17 0xdc00


Get ethernet leds state:
read - ./phytool end0/1/0x1b

Enable ethernet leds:
write - ./phytool write end0/1/0x1b 0x0F00

Disable ethernet leds:
write - ./phytool write end0/1/0x1b 0x0
