//-------------------------------------------------
//      Redpitaya browser detector
//      Created by Artem Kokos
//-------------------------------------------------

(function(BrowserChecker, $) {
    // Constants
    BrowserChecker.newestChrome = 49;
    BrowserChecker.newestMozilla = 45;

    // Variables
    BrowserChecker.browserName = "";
    BrowserChecker.browserVer = 0;


    BrowserChecker.checkName = function() {
        BrowserChecker.browserName = $.browser.name;
        // alert(JSON.stringify($.browser));
        if ($.browser.platform == "iphone" || $.browser.platform == "ipad")
            return true;

        if (window.chrome || $.browser.chrome)
            return true;
        if (BrowserChecker.browserName != "chrome" && BrowserChecker.browserName != "mozilla")
            return false;

        return true;
    }

    BrowserChecker.checkVer = function() {
        BrowserChecker.browserVer = $.browser.versionNumber;

        if (BrowserChecker.browserName == "chrome") {
            if (BrowserChecker.browserVer < (BrowserChecker.newestChrome - 5)) {
                return false;
            }
        } else if (BrowserChecker.browserName == "mozilla") {
            if (BrowserChecker.browserVer < (BrowserChecker.newestMozilla - 5))
                return false;
        }

        return true;
    }

    $(document).ready(function($) {
        var nameResult = BrowserChecker.checkName();
        var verResult = BrowserChecker.checkVer();

        if (!nameResult) {
            var htmlText = "<p id='browser_detect_text'>You are using unsupported browser.<br>Please, install Mozilla Firefox or Google Chrome.<br>\
        	<a href='http://www.google.com/chrome/browser/desktop/index.html'><img src='../assets/images/gchrome_logo.png' style='width: 10%;'/></a>\
        	<a href='http://www.mozilla.org/download'><img src='../assets/images/mfirefox_logo.png' style='width: 10%;'/></a></p>";

            PopupStack.add(htmlText);
        } else if (!verResult) {
            var imgHtml = (BrowserChecker.browserName == "chrome") ?
                "<a href='http://www.google.com/chrome/browser/desktop/index.html'><img src='../assets/images/gchrome_logo.png' style='width: 10%;'/></a>" :
                "<a href='http://www.mozilla.org/download'><img src='../assets/images/mfirefox_logo.png' style='width: 10%;'/></a>";

            var htmlText = "<p id='browser_detect_text'>Your browser is too old. Please, update your browser.<br>" + imgHtml + "</p>";

            PopupStack.add(htmlText);
        }
    });

})(window.BrowserChecker = window.BrowserChecker || {}, jQuery);