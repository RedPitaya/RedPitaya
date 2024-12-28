#/bin/bash


for path in ./uart/*/; do
    dir_name=$(basename "$path")
    python ./test_uart.py $path $dir_name > $path/test.out
    diff_output=$(diff "$path/sigrock.out" "$path/test.out")
    if [ -z "$diff_output" ]; then
        echo "$dir_name [OK]"
        rm $path/test.out
    else
        echo "$dir_name [ERROR]"
    fi
done
