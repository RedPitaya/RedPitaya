# Usage - iw must be run as root (suggestion: add as an alias in bashrc):
# iwlist wlan0 scan | awk -f scan.awk

function strip(s) {
  gsub(/^[ \t]+/,"",s)
  gsub(/[ \t]+$/,"",s)
  return s
}

# Parse input and collect info

BEGIN {
}

$4 == "Address:" {
    MAC = $5
    wifi[MAC]["enc"] = "Open"
    wifi[MAC]["SSID"] = "Hidden" # Assume hidden
}

substr($1, 0, 5) == "ESSID" {
    split($0,a,":")
    wifi[MAC]["SSID"] = strip(a[2])
}

substr($2, 0, 6) == "Signal" {
    split($3,res , "=")
    split(res[2],res2 , "/")
    wifi[MAC]["sig"] = res2[1] - 100
}

$1 == "Encryption" {
    if ($2 == "key:on") {
        wifi[MAC]["enc"] = "Yes"
    }
}

# Print collected info
END {
    t=""
    printf "{\"scan\": [\n"
    fmt = "  {\"SSID\": %s, \"sig\": \"%s\", \"enc\": \"%s\"}"
    for (w in wifi) {
        printf t
        printf fmt, wifi[w]["SSID"], wifi[w]["sig"], wifi[w]["enc"]
        t=",\n"
    }
    printf "\n]}"
}
