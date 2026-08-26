# Parse `iw dev wlan0 scan` into the JSON the Network Manager UI expects.
#
# Rewritten because the original:
#   * used FS=":" and took only $2 for the SSID, so any SSID containing a colon
#     ("Home: 5G") was truncated at the colon
#   * emitted SSIDs into JSON without escaping " or \, producing invalid JSON
#     that made the whole network list disappear in the browser
#   * reported every RSN network as "WPA2", so a WPA3-SAE / WPA2+WPA3-mixed AP
#     was indistinguishable from plain WPA2
#   * built the BSS key as $2$3$4$5$6 under FS=":", dropping the first MAC octet
#     and appending "(on wlan0)", so two APs differing only in the first octet
#     collided and one of them vanished from the list

function jesc(s) {
    gsub(/\\/, "\\\\", s)
    gsub(/"/,  "\\\"", s)
    gsub(/\t/, " ", s)
    return s
}

function strip(s) {
    sub(/^[ \t]+/, "", s)
    sub(/[ \t]+$/, "", s)
    return s
}

/^BSS[ \t]/ {
    line = $0
    sub(/^BSS[ \t]+/, "", line)
    sub(/[ \t]*\(.*$/, "", line)
    MAC = line
    order[++n] = MAC
    enc[MAC]  = "Open"
    ssid[MAC] = ""
    sig[MAC]  = ""
    sae[MAC]  = "No"
    next
}

# Take everything after "SSID: " verbatim - colons included.
/^[ \t]*SSID: / {
    l = $0
    sub(/^[ \t]*SSID: ?/, "", l)
    ssid[MAC] = l
    next
}

/^[ \t]*signal: / {
    l = $0
    sub(/^[ \t]*signal: ?/, "", l)
    split(strip(l), a, " ")
    sig[MAC] = a[1]
    next
}

/^[ \t]*WPA:/ { if (enc[MAC] == "Open") enc[MAC] = "WPA"; next }
/^[ \t]*RSN:/ { enc[MAC] = "WPA2"; next }

/Authentication suites:/ {
    if ($0 ~ /SAE/) {
        sae[MAC] = "Yes"
        enc[MAC] = ($0 ~ /PSK/) ? "WPA2/WPA3" : "WPA3"
    }
    next
}

END {
    printf "{\"scan\": [\n"
    t = ""
    for (i = 1; i <= n; i++) {
        m = order[i]
        if (ssid[m] == "") continue          # hidden network, nothing to show
        if (seen[ssid[m]]) continue          # collapse mesh/roaming duplicates
        seen[ssid[m]] = 1
        printf t
        printf "  {\"SSID\": \"%s\", \"sig\": \"%s\", \"enc\": \"%s\", \"sae\": \"%s\"}",
               jesc(ssid[m]), sig[m], enc[m], sae[m]
        t = ",\n"
    }
    printf "\n]}"
}
