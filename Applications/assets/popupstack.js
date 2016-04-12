//-------------------------------------------------
//      Redpitaya popup window stack controller
//      Created by Artem Kokos
//-------------------------------------------------

(function(PopupStack, $) {
    // Constants
    PopupStack.popupBottomOffset = 20;
    PopupStack.popupHeight = 100;

    // Variables
    PopupStack.lastAdded = 0;
    PopupStack.nextBottomOffset = PopupStack.popupBottomOffset;
    PopupStack.array = [];


    PopupStack.removeFromArray = function(key) {
        for (var i = 0; i < PopupStack.array.length; i++) {
            if (PopupStack.array[i] == key) {
                PopupStack.array.splice(i, 1);
            }
        }
    }

    PopupStack.add = function(html) {
        var popupHtml = "<div class='shadow_box browser_detect_dialog' key='" + PopupStack.lastAdded + "' style='bottom: " + PopupStack.nextBottomOffset + "px'>" +
            "<div class='close_d popup browser_detect_close' style='background-color: whitesmoke;'>X</div>" + html + "</div>";
        PopupStack.array.push(PopupStack.lastAdded);
        PopupStack.lastAdded++;
        PopupStack.nextBottomOffset += PopupStack.popupBottomOffset + PopupStack.popupHeight;

        $('body').append(popupHtml);

        $('.browser_detect_close').on('click', function(ev) {
            var key = $(this).parent().attr('key');
            PopupStack.remove(key);
        });
    }

    PopupStack.remove = function(key) {
        $('.browser_detect_dialog[key=' + key + ']').remove();
        PopupStack.removeFromArray(key);
        PopupStack.nextBottomOffset = PopupStack.popupBottomOffset;

        for (var i = 0; i < PopupStack.array.length; i++) {
            $('.browser_detect_dialog[key=' + PopupStack.array[i] + ']').css('bottom', PopupStack.nextBottomOffset);
            PopupStack.nextBottomOffset += PopupStack.popupBottomOffset + PopupStack.popupHeight;
        }
    }

})(window.PopupStack = window.PopupStack || {}, jQuery);