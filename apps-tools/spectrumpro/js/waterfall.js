/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */
(function($) {

    function Waterfall(placeholder, width, height) {

        // Waterfall object
        var waterfall = this;

        // Variables
        var newCanvas = $('<canvas/>',{'class':'waterfall_canvas'});
        placeholder.append(newCanvas);
        if (width === 0){
            width = 100
        }
        waterfall.canvas = newCanvas[0];
        waterfall.canvas_context = waterfall.canvas.getContext('2d');
        waterfall.canvas_context.canvas.width = width;
        waterfall.canvas_context.canvas.height = height;
        waterfall.width = width;
        waterfall.height = height;
        waterfall.data = [];
        waterfall.gl = undefined


        waterfall.draw = function() {
            waterfall.gl.draw(this.canvas_context);
        };


        waterfall.addData = function(plot, x,y) {
            this.gl.addLine(plot,x,y)
        };


        waterfall.setSize = function(width, height) {

            if (this.width !== width || this.height  !== height){
                this.canvas_context.canvas.width = width;
                this.canvas_context.canvas.height = height;
                this.width = width;
                this.height = height;
                this.gl.setNewSizeWGL(width,height)
            }
        };

    }



    $.createWaterfall = function(placeholder, width, height) {
        var waterfall = new Waterfall($(placeholder), width, height);
        waterfall.gl = new GLWaterfall()
        waterfall.gl.init()
        return waterfall;
    };

})(jQuery);