#/bin/bash


for path in ./spi/*/; do
    dir_name=$(basename "$path")
    python ./test_spi.py $path $dir_name > $path/test.out
    diff_output=$(diff "$path/sigrock.out" "$path/test.out")
    if [ -z "$diff_output" ]; then
        echo "[SPI] $dir_name [OK]"
        rm $path/test.out
    else
        echo "[SPI] $dir_name [ERROR]"
    fi
done
