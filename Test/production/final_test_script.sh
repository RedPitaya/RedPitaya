#!/bin/bash

# Get input
read_user_input () {
    read -p "Enter RP URL or use QR-scanner to get it: " result
    echo $result
}

# Parse URL
parse_url () {
    if [[ $1 =~ (rp-[A-Za-z0-9]+\.local) ]]; then
        url=${BASH_REMATCH[1]}
        echo $url
    else
        echo 0
    fi
}

# Check WEB connection
check_connection () {
    if curl -s --head --request GET $1 | grep "200 OK" > /dev/null; then 
        status="OK"
    else
        status="FAILED"
    fi
    dt=$(date '+%d/%m/%Y %H:%M:%S');
    echo "$dt $1 - $status"
}

# Log to file
log_to_file () {
    echo $1 >> log.txt
}


# Main function
main () {
    # Enter URL
    user_input=$(read_user_input)

    # Parse URL
    rp_url=$(parse_url $user_input)
    
    # Check URL
    if ! [[ $rp_url == 0 ]]; then
        # Check WEB is up
        connection_result=$(check_connection $rp_url)

        # Log to file result
        log_to_file "$connection_result"

        echo $connection_result
    else
        echo "Unable to parse Red Pitaya URL"
    fi
}

# Start
main
