#/bin/bash


for path in ./can/*/; do
    dir_name=$(basename "$path")
    python ./test_can.py $path $dir_name > $path/test.out
    diff_output=$(diff "$path/sigrock.out" "$path/test.out")
    if [ -z "$diff_output" ]; then
        echo "[CAN] $dir_name [OK]"
        rm $path/test.out
    else
        echo "[CAN] $dir_name [ERROR]"
    fi
done
