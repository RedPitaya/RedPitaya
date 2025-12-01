#/bin/bash

for file in ./*.sr; do
  if [ -f "$file" ]; then
    file=$(basename "$file")
    file="${file%.*}"
    echo "Proccess " $file
    rm -rf ./$file 2> /dev/null
    mkdir -p ./$file
    cp ./$file.sr ./$file/$file.sr
    sigrok-cli -i ./$file/$file.sr --show > ./$file/$file.sr.info
    sigrok-cli -i ./$file/$file.sr -o ./$file/$file.sr.bin -O binary
    if grep -q "Channels: 8" ./$file/$file.sr.info; then
        echo "Detect 8 bit mode"
        la_rle_tool s8-r8 ./$file/$file.sr.bin ./$file/$file.bin
    fi
    if grep -q "Channels: 16" ./$file/$file.sr.info; then
        echo "Detect 16 bit mode"
        la_rle_tool s16-r8 ./$file/$file.sr.bin ./$file/$file.bin
    fi

    rm -rf ./$file/$file.sr.bin
  fi
done