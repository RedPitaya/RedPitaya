(function($) {
  var isReady = false;
  var options = {
    touch: {
      pan: 'xy',
      scale: 'xy',
      autoWidth: true,
      autoHeight: true
    }
  };

  function init(plot) {
  
    if(window.ontouchstart === undefined) {
      return;
    }
  
    var isPanning = false;
    var isZooming = false;
    var lastTouchPosition = { x: -1, y: -1 };
    
    var startTouchDistance = { x: -1, y: -1 };
    var startTouchPosition = { x: -1, y: -1 };
    
    var relativeOffset = { x: 0, y: 0};
    
    var scaleArray = { x: 1.0, y: 1.0};
    var scaleOrigin = { x: 0, y: 0};

    function pan(delta) {
      var placeholder = plot.getPlaceholder();
      var options = plot.getOptions();
      
      relativeOffset.x -= delta.x;
      relativeOffset.y -= delta.y;
      
      switch (options.touch.pan.toLowerCase()) {
        case 'x':
          placeholder.children('div.flot-touch-container').css({
            'transform': 'translateX(' + relativeOffset.x + 'px)',
            '-webkit-transform': 'translateX(' + relativeOffset.x + 'px)'
          });
          break;
        case 'y':
          placeholder.children('div.flot-touch-container').css({
            'transform': 'translateY(' + relativeOffset.y + 'px)',
            '-webkit-transform': 'translateY(' + relativeOffset.y + 'px)'
          });
          break;
        default:
          placeholder.children('div.flot-touch-container').css({
            'transform': 'translate(' + relativeOffset.x + 'px,' + relativeOffset.y + 'px)',
            '-webkit-transform': 'translate(' + relativeOffset.x + 'px,' + relativeOffset.y + 'px)'
          });
          break;
      }
    }
    
    function scale(delta, trans) {
      var placeholder = plot.getPlaceholder();
      var options = plot.getOptions();
      var container = placeholder.children('div.flot-touch-container');
      
      switch (options.touch.scale.toLowerCase()) {
        case 'x':
          container.css({
            'transform': 'translateX(' + trans.x + 'px) scaleX(' + delta.x + ')',
            '-webkit-transform': 'translateX(' + trans.x + 'px) scaleX(' + delta.x + ')'
          });
          break;
        case 'y':
          container.css({
            'transform': 'translateY(' + trans.y + 'px) scaleY(' + delta.y + ')',
            '-webkit-transform': 'translateY(' + trans.y + 'px) scaleY(' + delta.y + ')'
          });
          break;
        default:
          container.css({
            'transform': 'translate(' +trans.x + 'px, ' + trans.y + 'px) scale(' + delta.x + ', ' + delta.y + ')',
            '-webkit-transform': 'translate(' +trans.x + 'px, ' + trans.y + 'px) scale(' + delta.x + ', ' + delta.y + ')'
          });
          break;
      }
    }

    function processOptions(plot, options) {      
      var placeholder = plot.getPlaceholder();

      if (options.touch.autoWidth) {
        placeholder.css('width', '100%');
      }
      
      if (options.touch.autoHeight) {
        var placeholderParent = placeholder.parent();
        var height = 0;
        
        placeholderParent.siblings().each(function() {
          height -= $(this).outerHeight();
        });
        
        height -= parseInt(placeholderParent.css('padding-top'), 10);
        height -= parseInt(placeholderParent.css('padding-bottom'), 10);
        height += window.innerHeight;
        
        placeholder.css('height', (height <= 0) ? 100 : height + 'px');
      }
    }
    
    function jqtTouchStart(evt) {
      var touches = evt.originalEvent.touches;
      var placeholder = plot.getPlaceholder();
      var container = placeholder.children('div.flot-touch-container');
      
      var offset = placeholder.offset();
      var rect = {
        x: offset.left,
        y: offset.top,
        width: placeholder.width(),
        height: placeholder.height()
      };

      if (touches.length === 1) {
        isPanning = true;
        lastTouchPosition = {
          x: touches[0].pageX - rect.x,
          y: touches[0].pageY - rect.y
        };
      }
      
      else if (touches.length === 2) {
        isZooming = true;

        lastTouchPosition = {
          x: (touches[0].pageX + touches[1].pageX) / 2 - rect.x,
          y: (touches[0].pageY + touches[1].pageY) / 2 - rect.y
        };

        startTouchDistance = {
          x: (touches[0].pageX - touches[1].pageX),
          y: (touches[0].pageY - touches[1].pageY)
        };
      }

      startTouchPosition = {
        x: lastTouchPosition.x,
        y: lastTouchPosition.y
      };      
      
      scaleOrigin = {
        x: Math.round((startTouchPosition.x / rect.width) * 100),
        y: Math.round((startTouchPosition.y / rect.height) * 100)
      };

      container.css({
        'transform-origin': scaleOrigin.x + '% ' + scaleOrigin.y + '%',
        '-webkit-transform-origin': scaleOrigin.x + '% ' + scaleOrigin.y + '%'
      });

      // Return false to prevent touch scrolling.
      return false;
    }
    
    function jqtTouchMove(evt) {
      var touches = evt.originalEvent.touches;
      var position, delta;

      var placeholder = plot.getPlaceholder();
      var container = placeholder.children('div.flot-touch-container');
      
      var offset = placeholder.offset();
      var rect = {
        x: offset.left,
        y: offset.top,
        width: placeholder.width(),
        height: placeholder.height()
      };

      if (isPanning && touches.length === 1) {
        position = {
          x: touches[0].pageX - rect.x,
          y: touches[0].pageY - rect.y
        };
        delta = {
          x: lastTouchPosition.x - position.x,
          y: lastTouchPosition.y - position.y
        };
        
        // Transform via the delta
        pan(delta);
        
        lastTouchPosition = position;
      }
      
      else if (isZooming && touches.length === 2) {
        position = {
          x: (touches[0].pageX + touches[1].pageX) / 2 - rect.x,
          y: (touches[0].pageY + touches[1].pageY) / 2 - rect.y
        };
        var tempDistance = {
          x: (touches[0].pageX - touches[1].pageX),
          y: (touches[0].pageY - touches[1].pageY)
        };

        var deltaArray = {
          x: position.x - startTouchPosition.x,
          y: position.y - startTouchPosition.y
        };

        scaleArray = {
          x: Math.min(Math.max(0.1, tempDistance.x / startTouchDistance.x), 5.0),
          y: Math.min(Math.max(0.1, tempDistance.y / startTouchDistance.y), 5.0)
        };        

        if (Math.abs(scaleArray.x - 1) < 0.2) {
          scaleArray.x = 1.0;
        };

        if (Math.abs(scaleArray.y - 1) < 0.2) {
          scaleArray.y = 1.0;
        };

        if (Math.abs(startTouchDistance.x) < 50) {
          scaleArray.x = 1.0;
        };

        if (Math.abs(startTouchDistance.y) < 50) {
          scaleArray.y = 1.0;
        };

        scale(scaleArray, deltaArray);

        relativeOffset = deltaArray;

        lastTouchPosition = position;
      }
    }
    
    function jqtTouchEnd(evt) {
      var placeholder = plot.getPlaceholder();
      var options = plot.getOptions();
      var container = placeholder.children('div.flot-touch-container');

      // Apply the pan.
      if ((isPanning && !isZooming) && (relativeOffset.x !== 0 || relativeOffset.y !== 0)) {
        $.each(plot.getAxes(), function(index, axis) {
          if (axis.direction === options.touch.pan.toLowerCase() || options.touch.pan.toLowerCase() === 'xy') {
            var min = axis.c2p(axis.p2c(axis.min) - relativeOffset[axis.direction]);
            var max = axis.c2p(axis.p2c(axis.max) - relativeOffset[axis.direction]);
            
            axis.options.min = min;
            axis.options.max = max;
          }
        });
      }

      // Apply the scale.
      if (isZooming) {
        var width = plot.width();
        var height = plot.height();
        
        var scaleOriginPixel = {
          x: Math.round((scaleOrigin.x / 100) * width),
          y: Math.round((scaleOrigin.y / 100) * height)
        };
        
        var range = {
          x: {
            min: scaleOriginPixel.x - relativeOffset.x/scaleArray.x - (scaleOrigin.x / 100) * width / scaleArray.x,
            max: scaleOriginPixel.x - relativeOffset.x/scaleArray.x + (1 - (scaleOrigin.x / 100)) * width / scaleArray.x
          },
          y: {
            min: scaleOriginPixel.y - relativeOffset.y/scaleArray.y - (scaleOrigin.y / 100) * height / scaleArray.y,
            max: scaleOriginPixel.y - relativeOffset.y/scaleArray.y + (1 - (scaleOrigin.y / 100)) * height / scaleArray.y
          }
        };
        
        $.each(plot.getAxes(), function(index, axis) {
          if (axis.direction === options.touch.scale.toLowerCase() || options.touch.scale.toLowerCase() == 'xy') {

            var min = axis.c2p(range[axis.direction].min);
            var max = axis.c2p(range[axis.direction].max);

            if (min > max) {
              var temp = min;
              min = max;
              max = temp;
            }
            
            axis.options.min = min;
            axis.options.max = max;
          }
        });
      }
      
      plot.setupGrid();
      plot.draw();
      
      isPanning = false;
      isZooming = false;
      lastTouchPosition = { x: -1, y: -1 };
      relativeOffset = { x: 0, y: 0 };
      relativeScale = 1.0;
      scaleOrigin = { x: 50, y: 50 };

      container.css({
        'transform': 'translate(' + relativeOffset.x + 'px,' + relativeOffset.y + 'px) scale(' + relativeScale + ')',
        'transform-origin': scaleOrigin.x + '% ' + scaleOrigin.y + '%',
        '-webkit-transform': 'translate(' + relativeOffset.x + 'px,' + relativeOffset.y + 'px) scale(' + relativeScale + ')',
        '-webkit-transform-origin': scaleOrigin.x + '% ' + scaleOrigin.y + '%'
      });
    }

    function bindEvents(plot, eventHolder) {
      var placeholder = plot.getPlaceholder();
      var container = $('<div class="flot-touch-container" style="background:#fff;height:100%"/>');
      
      placeholder.css({
        //'background': '#fff url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAAZQTFRF////mpqaPjL2kgAAABdJREFUeNpiYIQCBhgYIIEBth4mABBgAEUQAIEfdL0YAAAAAElFTkSuQmCC) repeat',
        //'opacity': 0.99,     // Hack for Firefox on Android: http://jbkflex.wordpress.com/2013/04/04/css3-transformations-showing-content-outside-overflowhidden-region-in-firefoxandroid/
        'overflow': 'hidden'
      }).children('canvas').wrapAll(container);
      
      placeholder.bind('touchstart', jqtTouchStart)
                 .bind('touchmove', jqtTouchMove)
                 .bind('touchend', jqtTouchEnd);
    }
    
    function processDatapoints(plot, series, datapoints) {
      if (window.devicePixelRatio) {
        var placeholder = plot.getPlaceholder();
        
        placeholder.children('canvas').each(function(index, canvas) {
          var context  = canvas.getContext('2d');
          var width = $(canvas).attr('width');
          var height = $(canvas).attr('height');

          //$(canvas).attr('width', width * window.devicePixelRatio);
          //$(canvas).attr('height', height * window.devicePixelRatio);
          //$(canvas).css('width', width + 'px');
          //$(canvas).css('height', height + 'px');

          //$(canvas).attr('width', width);
          //$(canvas).attr('height', height);
          //$(canvas).css('width', width + 'px');
          //$(canvas).css('height', height + 'px');

          //$(canvas).attr('width', 500);
          //$(canvas).attr('height', 460);
          //$(canvas).css('width', '500px');
          //$(canvas).css('height', '460px');

          //context.scale(window.devicePixelRatio, window.devicePixelRatio);
          //alert(window.devicePixelRatio);
          context.scale(1, 1);
        });
      }
    }

    function shutdown(plot, eventHolder) {
      var placeholder = plot.getPlaceholder();
      
      placeholder.unbind('touchstart', jqtTouchStart).unbind('touchmove', jqtTouchMove).unbind('touchend', jqtTouchEnd);
    }

    plot.hooks.processOptions.push(processOptions);
    plot.hooks.bindEvents.push(bindEvents);
    plot.hooks.processDatapoints.push(processDatapoints);
    plot.hooks.shutdown.push(shutdown);

    if (!isReady) {
      $(document).bind('ready orientationchange', function(evt) {
        window.scrollTo(0, 1);
        
        setTimeout(function() {
          $.plot(plot.getPlaceholder(), plot.getData(), plot.getOptions());
        }, 50);
      });
      
      isReady = true;
    }
  }

  $.plot.plugins.push({
    init: init,
    options: options,
    name: 'touch',
    version: '1.0'
  });
})(jQuery);
