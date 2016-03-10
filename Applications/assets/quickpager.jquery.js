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

    $.fn.quickPager = function(options) {

        var defaults = {
            pageSize: 10,
            currentPage: 1,
            holder: null,
            pagerLocation: "after"
        };

        var options = $.extend(defaults, options);


        return this.each(function() {


            var selector = $(this);
            var pageCounter = 1;

            selector.wrap("<div class='simplePagerContainer'></div>");

            selector.parents(".simplePagerContainer").find("ul.simplePagerNav").remove();

            selector.children().each(function(i) {

                if (i < pageCounter * options.pageSize && i >= (pageCounter - 1) * options.pageSize) {
                    $(this).addClass("simplePagerPage" + pageCounter);
                } else {
                    $(this).addClass("simplePagerPage" + (pageCounter + 1));
                    pageCounter++;
                }

            });

            // show/hide the appropriate regions
            selector.children().hide();
            selector.children(".simplePagerPage" + options.currentPage).show();

            if (pageCounter <= 1) {
                return;
            }

            //Build pager navigation
            var pageNav = "<ul class='simplePagerNav'>";
            for (i = 1; i <= pageCounter; i++) {
                if (i == options.currentPage) {
                	pageNav += "<li class='currentPage simplePageNav" + i + "'><a rel='" + i + "' href='#'><div id='dot' class='active-dot'></div></a></li>";
                } else {
                    pageNav += "<li class='simplePageNav" + i + "'><a rel='" + i + "' href='#'><div id='dot' class='dot'></div></a></li>";
                }
            }
            pageNav += "</ul>";

            if (!options.holder) {
                switch (options.pagerLocation) {
                    case "before":
                        selector.before(pageNav);
                        break;
                    case "both":
                        selector.before(pageNav);
                        selector.after(pageNav);
                        break;
                    default:
                        selector.after(pageNav);
                }
            } else {
                $(options.holder).append(pageNav);
            }

            //pager navigation behaviour
            selector.parent().find(".simplePagerNav a").click(function() {

                //grab the REL attribute
                var clickedLink = $(this).attr("rel");
                options.currentPage = clickedLink;

                if (options.holder) {
                    $(this).parent("li").parent("ul").parent(options.holder).find("li.currentPage").find('#dot').addClass('dot');
                    $(this).parent("li").parent("ul").parent(options.holder).find("li.currentPage").find('#dot').removeClass('active-dot');
                    $(this).parent("li").parent("ul").parent(options.holder).find("li.currentPage").removeClass("currentPage");

                    $(this).parent("li").parent("ul").parent(options.holder).find("a[rel='" + clickedLink + "']").parent("li").addClass("currentPage");
                    $(this).parent("li").parent("ul").parent(options.holder).find("a[rel='" + clickedLink + "']").parent("li").find('#dot').addClass("active-dot");
                    $(this).parent("li").parent("ul").parent(options.holder).find("a[rel='" + clickedLink + "']").parent("li").find('#dot').removeClass("dot");
                } else {
                    //remove current current (!) page
                    $(this).parent("li").parent("ul").parent(".simplePagerContainer").find("li.currentPage").find('#dot').addClass("dot");
                    $(this).parent("li").parent("ul").parent(".simplePagerContainer").find("li.currentPage").find('#dot').removeClass("active-dot");
                    $(this).parent("li").parent("ul").parent(".simplePagerContainer").find("li.currentPage").removeClass("currentPage");

                    //Add current page highlighting
                    $(this).parent("li").parent("ul").parent(".simplePagerContainer").find("a[rel='" + clickedLink + "']").parent("li").addClass("currentPage");
                    $(this).parent("li").parent("ul").parent(".simplePagerContainer").find("a[rel='" + clickedLink + "']").parent("li").find('#dot').addClass("active-dot");
                    $(this).parent("li").parent("ul").parent(".simplePagerContainer").find("a[rel='" + clickedLink + "']").parent("li").find('#dot').removeClass("dot");
                }

                //hide and show relevant links
                selector.children().hide();
                selector.find(".simplePagerPage" + clickedLink).show();

                return false;
            });
        });
    }


})(jQuery);
