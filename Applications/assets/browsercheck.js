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

		if(BrowserChecker.browserName != "chrome" && BrowserChecker.browserName != "mozilla")
			return false;

		return true;
	}

	BrowserChecker.checkVer = function() {
		BrowserChecker.browserVer = $.browser.versionNumber;

		if(BrowserChecker.browserName == "chrome") {
			if(BrowserChecker.browserVer < (BrowserChecker.newestChrome - 5)) {
				return false;
			}
		}
		else if(BrowserChecker.browserName == "mozilla") {
			if(BrowserChecker.browserVer < (BrowserChecker.newestMozilla - 5))
				return false;
		}

		return true;
	}

	$(document).ready(function($) {
		var nameResult = BrowserChecker.checkName();
		var verResult = BrowserChecker.checkVer();

		if(!nameResult) {
			// Show dialog with unsupported browser
			$('#browser_detect_text').html("You are using unsupported browser.<br>Please, install Mozilla Firefox or Google Chrome.<br>\
			<a href='http://www.google.com/chrome/browser/desktop/index.html'><img src='images/gchrome_logo.png' style='width: 10%;'/></a>\
			<a href='http://www.mozilla.org/download'><img src='images/mfirefox_logo.png' style='width: 10%;'/></a>");
        	$('#browser_detect_dialog').show();
		}
		else if(!verResult) {
			// Show update browser dialog
			var imgHtml = (BrowserChecker.browserName == "chrome") ?
			 "<a href='http://www.google.com/chrome/browser/desktop/index.html'><img src='images/gchrome_logo.png' style='width: 10%;'/></a>" :
			 "<a href='http://www.mozilla.org/download'><img src='images/mfirefox_logo.png' style='width: 10%;'/></a>";
			 
			$('#browser_detect_text').html("Your browser is too old. Please, update your browser." + "<br>" + imgHtml);
        	$('#browser_detect_dialog').show();
		}
	});

})(window.BrowserChecker = window.BrowserChecker || {}, jQuery);

// Page event handler
$(function() {
	$('#browser_detect_close').on('click', function(ev) {
		$('#browser_detect_dialog').hide();
	});
});
