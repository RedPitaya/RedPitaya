import QtQuick

// The text styles of the web page: kind is text, title, cardTitle, sub, hint, label, value, mono, bad
Text {
    property string kind: "text"

    color: kind === "title" || kind === "cardTitle" ? Theme.title
         : kind === "sub" || kind === "hint" ? Theme.muted
         : kind === "value" || kind === "mono" ? Theme.value
         : kind === "bad" ? Theme.dangerText
         : Theme.text
    font.pixelSize: kind === "title" ? Theme.fsXl
                  : kind === "cardTitle" ? Theme.fsLg
                  : kind === "sub" || kind === "value" || kind === "mono" ? Theme.fsMd
                  : kind === "hint" || kind === "label" || kind === "bad" ? Theme.fsSm
                  : Theme.fsBase
    font.bold: kind === "cardTitle" || kind === "label" || kind === "value"
    font.family: kind === "mono" ? Theme.mono : Qt.application.font.family
    wrapMode: kind === "sub" || kind === "hint" || kind === "bad" ? Text.WordWrap : Text.NoWrap
    elide: wrapMode === Text.NoWrap ? Text.ElideRight : Text.ElideNone
    lineHeight: 1.1
}
