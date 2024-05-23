var helpListNM = 
{
  idle: [
    {
      Text: "Red Pitaya's Forum",
      URL: "http://forum.redpitaya.com/",
      Img: "star"
    }
  ]
};

$.fn.fI = function(e) { //Flash Item
  if (!e) { e = {} }
  if (this) { e.e = this }
  switch (e.f) {
      case 0:
          break;
      default:
          switch (e.css) {
              case 0:
                  e.d = 'background-color'
                  break;
              case undefined:
                  e.d = 'border-color'
                  break;
              default:
                  e.d = e.css
                  break;
          }
          if (!e.c1) { e.c1 = '#FF0000' }
          if (!e.c2) { e.c2 = '#A00000' }
          if (!e.p) { e.p = 200 }
          e.e.css(e.d, e.c1)
          setTimeout(function() {
              e.e.css(e.d, e.c2)
              setTimeout(function() {
                  e.e.css(e.d, e.c1)
                  setTimeout(function() {
                      e.e.css(e.d, e.c2)
                      setTimeout(function() {
                          e.e.css(e.d, '')
                      }, e.p)
                  }, e.p)
              }, e.p)
          }, e.p)
          break;
  }
  return this
}