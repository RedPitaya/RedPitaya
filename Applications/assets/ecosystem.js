//-------------------------------------------------
//		Quick Pager jquery plugin
//		Created by dan and emanuel @geckonm.com
//		www.geckonewmedia.com
//
//
//		18/09/09 * bug fix by John V - http://blog.geekyjohn.com/
//		1.2 - allows reloading of pager with new items
//-------------------------------------------------

(function($) {
    var isOnline = false;
    var default_apps = [{ id: "marketplace", name: "Marketplace", description: "Provides access to new applications", url: "http://bazaar.redpitaya.com/", image: "http://moball.tv/wp-content/plugins/moball_app_factory/images/sofit-app-icon.png" },
        { id: "appstore", name: "Store", description: "Provides access to new hardware", url: "http://store.redpitaya.com/", image: "http://moball.tv/wp-content/plugins/moball_app_factory/images/sofit-app-icon.png" },
        { id: "github", name: "Github", description: "Our github", url: "https://github.com/redpitaya", image: "http://moball.tv/wp-content/plugins/moball_app_factory/images/sofit-app-icon.png" },
        { id: "visualprogramming", name: "Visual Programming", description: "Perfect tool for newcomers to have fun while learning and putting their ideas into practice", url: "http://account.redpitaya.com/try-visual-programming.php", image: "images/img_visualprog.png" },
    ];

    var apps = [];

    placeElements = function() {
        var elemWidth = $('.app-item').outerWidth(true);
        var containerWidth = $('#list-container').width();
        var elemsInRow = Math.floor(containerWidth / elemWidth);
        elemsInRow = (elemsInRow == 0) ? 1 : elemsInRow;

        var elemHeight = $('.app-item').outerHeight(true);
        var containerHeight = $('#main-container').height();
        var elemsInCol = Math.floor(containerHeight / elemHeight);
        elemsInCol = (elemsInCol == 0) ? 1 : elemsInCol;

        $("ul.paging").quickPager({
            pageSize: elemsInRow * elemsInCol
        });
    }

    var prepareOffline = function() {
        Offline.options = {
            checks: {
                xhr: {
                    url: '/check_inet'
                }
            },
            checkOnLoad: false,
        };
        Offline.on('up', function() {
            isOnline = true;
        });
        Offline.on('down', function() {
            isOnline = false;
        });
        Offline.check();
        var run = function() {
            Offline.check();
        }
        setInterval(run, 3000);
    }

    var refillList = function() {
        $('.app-item').unbind('click');

        $('#list-container').empty();
        for (var i = 0; i < apps.length; i++) {
            var txt = '<li class="app-item" key="' + i + '" >';
            txt += '<a href="#" class="app-link"><img class="app-icon" src="' + apps[i]['image'] + '"><span class="app-name">' + apps[i]['name'] + '</span></a>';
            txt += '</li>';
            $('#list-container').append(txt);
        }
        $('.app-item').click(clickApp);
    }

    var GetListOfApps = function() {
        $.ajax({
            url: 'bazaar?apps=',
            cache: false,
            async: true
        }).done(function(result) {
            var url_arr = window.location.href.split("/");;
            var url = url_arr[0] + '//' + url_arr[2] + '/';
            apps = default_apps.slice();

            $.each(result, function(key, value) {
                var obj = {};
                obj['id'] = key;
                obj['name'] = value['name'];
                obj['description'] = value['description'];
                obj['url'] = url + key + '/?type=run';
                obj['image'] = url + key + '/info/icon.png';
                apps.push(obj);
            });

            refillList();
            placeElements();
        }).fail(function(msg) { /*GetListOfApps();*/ });
    }

    var clickApp = function() {
        var key = parseInt($(this).attr('key')) * 1;
        window.open(apps[key].url, '_blank');
    }

    $(document).ready(function($) {
        GetListOfApps();
        prepareOffline();
    });

    $(window).resize(function($) {
        refillList();
        placeElements();
    });
})(jQuery);
