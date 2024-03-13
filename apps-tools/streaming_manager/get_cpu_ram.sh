#!/bin/bash
memory_size=$(free -b | grep Mem | awk '{print $2}')
cpu=$(nproc)
max_cpu=$cpu
limit=$((1024 * 1024 * 150))
for ((i = max_cpu; i > 1; i--)); do
    result=$((memory_size / i))
    if [ $result -ge $limit ]; then
        break;
    fi
done
echo $i