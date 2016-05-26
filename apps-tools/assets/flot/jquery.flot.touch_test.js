(function($) {
  var isReady = false;
  var options = {
    touch: {
      pan: 'xy',
      scale: null,
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
    var lastTouchDistance = 0;
    var relativeOffset = { x: 0, y: 0};
    var relativeScale = 1.0;
    var scaleOrigin = { x: 50, y: 50 };
    
    var lastTouchPos = {
      t1: lastTouchPosition,
      t2: lastTouchPosition
    };
    
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
    
    function scale(delta, scale) {
      var placeholder = plot.getPlaceholder();
      var options = plot.getOptions();
      var container = placeholder.children('div.flot-touch-container');
      
      scale = 'x'; //(scale === undefined ? options.touch.scale.toLowerCase() : scale);
      relativeScale *= 1 + (delta / 100);
      
      switch (scale) {
        case 'x':
          container.css({
            'transform': 'scaleX(' + relativeScale + ')',
            '-webkit-transform': 'scaleX(' + relativeScale + ')'
          });
          break;
        case 'y':
          container.css({
            'transform': 'scaleY(' + relativeScale + ')',
            '-webkit-transform': 'scaleY(' + relativeScale + ')'
          });
          break;
        default:
          container.css({
            'transform': 'scale(' + relativeScale + ')',
            '-webkit-transform': 'scale(' + relativeScale + ')'
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
      
      if (touches.length === 1) {
        isPanning = true;
        lastTouchPosition = {
          x: touches[0].pageX,
          y: touches[0].pageY
        };
        lastTouchDistance = 0;
      }
      
      else if (touches.length === 2) {
        isZooming = true;
        options.scale = null;
        lastTouchPos = {
          t1: {
            x: touches[0].pageX,
            y: touches[0].pageY
          },
          t2: {
            x: touches[1].pageX,
            y: touches[1].pageY
          }
        };
        lastTouchPosition = {
          x: (touches[0].pageX + touches[1].pageX) / 2,
          y: (touches[0].pageY + touches[1].pageY) / 2
        };
        lastTouchDistance = Math.sqrt(Math.pow(touches[1].pageX - touches[0].pageX, 2) + Math.pow(touches[1].pageY - touches[0].pageY, 2));
      }
      
      var offset = placeholder.offset();
      var rect = {
        x: offset.left,
        y: offset.top,
        width: placeholder.width(),
        height: placeholder.height()
      };
      
      var normalizedTouchPosition = {
        x: lastTouchPosition.x,
        y: lastTouchPosition.y
      };
      
      if (normalizedTouchPosition.x < rect.x) {
        normalizedTouchPosition.x = rect.x;
      } else if (normalizedTouchPosition.x > rect.x + rect.width) {
        normalizedTouchPosition.x = rect.x + rect.width;
      }

      if (normalizedTouchPosition.y < rect.y) {
        normalizedTouchPosition.y = rect.y;
      } else if (normalizedTouchPosition.y > rect.y + rect.height) {
        normalizedTouchPosition.y = rect.y + rect.height;
      }
      
      scaleOrigin = {
        x: Math.round((normalizedTouchPosition.x / rect.width) * 100),
        y: Math.round((normalizedTouchPosition.y / rect.height) * 100)
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
      var position, distance, delta;
      
      if (isPanning && touches.length === 1) {
        position = {
          x: touches[0].pageX,
          y: touches[0].pageY
        };
        delta = {
          x: lastTouchPosition.x - position.x,
          y: lastTouchPosition.y - position.y
        };
        
        // Transform via the delta
        pan(delta);
        
        lastTouchPosition = position;
        lastTouchDistance = 0;
      }
      
      else if (isZooming && touches.length === 2) {
        distance = Math.sqrt(Math.pow(touches[1].pageX - touches[0].pageX, 2) + Math.pow(touches[1].pageY - touches[0].pageY, 2));
        position = {
          x: (touches[0].pageX + touches[1].pageX) / 2,
          y: (touches[0].pageY + touches[1].pageY) / 2
        };
        
        delta = distance - lastTouchDistance;
        
        if(! options.scale) {
          var xDistDiff = Math.abs((touches[0].pageX - touches[1].pageX) - (lastTouchPos.t1.x - lastTouchPos.t2.x));
          var yDistDiff = Math.abs((touches[0].pageY - touches[1].pageY) - (lastTouchPos.t1.y - lastTouchPos.t2.y));
          
          options.scale = (xDistDiff / plot.width() >= yDistDiff / plot.height() ? 'x' : 'y');
          
          //$('#debug').prepend(xDistDiff + ' | ' + yDistDiff + '<br>');
        }
        else {
          //$('#debug').prepend('scale: ' + options.scale + '<br>');
        }
        
        lastTouchPos = {
          t1: {
            x: touches[0].pageX,
            y: touches[0].pageY
          },
          t2: {
            x: touches[1].pageX,
            y: touches[1].pageY
          }
        };  
        
        // Scale via the delta
        scale(delta, options.scale);
        
        lastTouchPosition = position;
        lastTouchDistance = distance;
      }
    }
    
    function jqtTouchEnd(evt) {
      var placeholder = plot.getPlaceholder();
      var options = plot.getOptions();
      var container = placeholder.children('div.flot-touch-container');
      
      // Apply the pan.
      if (relativeOffset.x !== 0 || relativeOffset.y !== 0) {
        $.each(plot.getAxes(), function(index, axis) {
          if (axis.direction === options.touch.pan.toLowerCase() || options.touch.pan.toLowerCase() == 'xy') {
            var min = axis.c2p(axis.p2c(axis.min) - relativeOffset[axis.direction]);
            var max = axis.c2p(axis.p2c(axis.max) - relativeOffset[axis.direction]);
            
            axis.options.min = min;
            axis.options.max = max;
          }
        });
      }
      
      // Apply the scale.
      if (relativeScale !== 1.0) {
        var width = plot.width();
        var height = plot.height();
        var scaleOriginPixel = {
          x: Math.round((scaleOrigin.x / 100) * width),
          y: Math.round((scaleOrigin.y / 100) * height)
        };
        var range = {
          x: {
            min: scaleOriginPixel.x - (scaleOrigin.x / 100) * width / relativeScale,
            max: scaleOriginPixel.x + (1 - (scaleOrigin.x / 100)) * width / relativeScale
          },
          y: {
            min: scaleOriginPixel.y - (scaleOrigin.y / 100) * height / relativeScale,
            max: scaleOriginPixel.y + (1 - (scaleOrigin.y / 100)) * height / relativeScale
          }
        };
        
        $.each(plot.getAxes(), function(index, axis) {
          //if (axis.direction === options.touch.scale.toLowerCase() || options.touch.scale.toLowerCase() == 'xy') {
          if (axis.direction === options.scale) {
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

        //$('body').append('<div style="top: ' + lastTouchPosition.y + 'px; left: ' + lastTouchPosition.x + 'px; height: 3px; width: 3px; background-color: red; position: absolute; z-index: 9999" />');
      }
      
      plot.setupGrid();
      plot.draw();
      
      isPanning = false;
      isZooming = false;
      lastTouchPosition = { x: -1, y: -1 };
      lastTouchDistance = 0;
      relativeOffset = { x: 0, y: 0 };
      relativeScale = 1.0;
      scaleOrigin = { x: 50, y: 50 };
      options.scale = null;
      
      container.css({
        'transform': 'translate(' + relativeOffset.x + 'px,' + relativeOffset.y + 'px) scale(' + relativeScale + ')',
        'transform-origin': scaleOrigin.x + '% ' + scaleOrigin.y + '%',
        '-webkit-transform': 'translate(' + relativeOffset.x + 'px,' + relativeOffset.y + 'px) scale(' + relativeScale + ')',
        '-webkit-transform-origin': scaleOrigin.x + '% ' + scaleOrigin.y + '%'
      });
    }

    function bindEvents(plot, eventHolder) {
      var placeholder = plot.getPlaceholder();
      var container = $('<div class="flot-touch-container" style="background:#fff;"/>');
      
      placeholder.css({
        //'background': '#fff url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAAZQTFRF////mpqaPjL2kgAAABdJREFUeNpiYIQCBhgYIIEBth4mABBgAEUQAIEfdL0YAAAAAElFTkSuQmCC) repeat',
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

          $(canvas).attr('width', width * window.devicePixelRatio);
          $(canvas).attr('height', height * window.devicePixelRatio);
          $(canvas).css('width', width + 'px');
          $(canvas).css('height', height + 'px');

          context.scale(window.devicePixelRatio, window.devicePixelRatio);
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
          $.plot(placeholder, plot.getData(), plot.getOptions());
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
