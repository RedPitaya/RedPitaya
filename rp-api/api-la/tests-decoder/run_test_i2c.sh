#/bin/bash


for path in ./i2c/*/; do
    dir_name=$(basename "$path")
    python ./test_i2c.py $path $dir_name > $path/test.out
    diff_output=$(diff "$path/sigrock.out" "$path/test.out")
    if [ -z "$diff_output" ]; then
        echo "[I2C] $dir_name [OK]"
        rm $path/test.out
    else
        echo "[I2C] $dir_name [ERROR]"
    fi
done
