#!/bin/bash
set -euo pipefail

PROJECT_FPGA=$1
REPORT_FILE=$2
MODE=$3

# Escape &, <, > so git log / commit info can't break the HTML.
html_escape() {
    sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g'
}

write_header() {
    local title=$1
    cat > "$REPORT_FILE" <<EOF
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>$title</title>
<style>
  body { font-family: sans-serif; font-size: 13px; }
  h2 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 4px; }
  table { border-collapse: collapse; width: 100%; }
  td, th { text-align: left; padding: 4px 8px; vertical-align: top; }
  th { color: blue; white-space: nowrap; }
  td.value { color: black; }
  pre { white-space: pre-wrap; margin: 0; font-family: monospace; font-size: 12px; }
  a { color: blue; }
</style>
</head>
<body>
<h2>$title</h2>
<table>
EOF
}

write_footer() {
    cat >> "$REPORT_FILE" <<EOF
</table>
</body>
</html>
EOF
}

write_row() {
    local name=$1 link=$2 detail=$3
    local name_esc link_esc detail_esc
    name_esc=$(printf '%s' "$name" | html_escape)
    link_esc=$(printf '%s' "$link" | html_escape)
    detail_esc=$(printf '%s' "$detail" | html_escape)

    cat >> "$REPORT_FILE" <<EOF
<tr>
  <th>$name_esc</th>
  <td class="value"><a href="$link_esc">$link_esc</a></td>
</tr>
<tr>
  <td colspan="2"><pre>$detail_esc</pre></td>
</tr>
EOF
}

if [[ "$MODE" == "FPGA" ]]; then
    write_header "FPGA/$PROJECT_FPGA"

    for f in build/fpga/"$PROJECT_FPGA"/*; do
        if [ -d "$f" ]; then
            DIR_NAME=$(basename "$f")
            INFO=$(cat "$f/git_info.txt")
            COMMIT=$(awk 'NR==2 {print $2}' "$f/git_info.txt")
            write_row "$DIR_NAME" "https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-fpga/-/commit/$COMMIT" "$INFO"
        fi
    done

    write_footer
fi

if [[ "$MODE" == "KERNEL" ]]; then
    write_header "Kernel"

    BRANCH=$(git show -s --pretty=%D HEAD | awk '{gsub("origin/",""); print $2}')
    LOG=$(git log -n 1)
    COMMIT=$(git log -n 1 | awk 'NR==1 {print $2}')
    write_row "$BRANCH" "https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-public/-/commit/$COMMIT" "$LOG"

    write_footer
fi

if [[ "$MODE" == "ECO" ]]; then
    write_header "Ecosystem"

    BRANCH=$(git branch --contains HEAD)
    LOG=$(git log -n 1)
    COMMIT=$(git log -n 1 | awk 'NR==1 {print $2}')
    write_row "$BRANCH" "https://gitlab.redpitaya.com/redpitaya-3.0/redpitaya-public/-/commit/$COMMIT" "$LOG"

    write_footer
fi