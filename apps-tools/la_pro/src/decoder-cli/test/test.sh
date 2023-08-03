./sigrok-cli -i xfp.sr -P i2c -A i2c | grep '[A-Za-z]' | sed -e '/\(Read\|Write\)/d' > sigrok_output
./decoder-cli -vi i2c --scl 1 --sda 2 logic res | sed -e '1,6d' > decoder_output
diff sigrok_output decoder_output
