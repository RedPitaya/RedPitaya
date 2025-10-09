(function(TA_MODE, $, undefined) {

    TA_MODE.width = {}
    TA_MODE.height = {}

    TA_MODE.buffer_accumulate_canvas = {}
    TA_MODE.buffer_accumulate_canvas_gl_ctx = {}
    TA_MODE.buffer_accumulate_program = {}
    TA_MODE.buffer_accumulate_program_info = {}

    TA_MODE.buffer_draw_canvas = {}
    TA_MODE.buffer_draw_canvas_gl_ctx = {}
    TA_MODE.buffer_draw_canvas_program = {}
    TA_MODE.buffer_draw_canvas_program_info = {}

    TA_MODE.data_points = {}
    TA_MODE.data_points_array = {}

    TA_MODE.isInit = false

    TA_MODE.color1 = {CH1 : "#3F00FF", CH2 : "#3F00FF", CH3 : "#3F00FF", CH4 : "#3F00FF"}
    TA_MODE.color2 = {CH1 : "#0000FF", CH2 : "#0000FF", CH3 : "#0000FF", CH4 : "#0000FF"}
    TA_MODE.color3 = {CH1 : "#00FF00", CH2 : "#00FF00", CH3 : "#00FF00", CH4 : "#00FF00"}
    TA_MODE.color4 = {CH1 : "#FF0000", CH2 : "#FF0000", CH3 : "#FF0000", CH4 : "#FF0000"}
    TA_MODE.inverted = {CH1 : 0, CH2 : 0, CH3 : 0, CH4 : 0}

    TA_MODE.vs = `
        attribute vec2 a_position;
        attribute vec2 a_texCoord;
        uniform vec2 u_resolution;
        varying vec2 v_texCoord;
        void main() {
            vec2 zeroToOne = a_position / u_resolution;
            vec2 zeroToTwo = zeroToOne * 2.0;
            vec2 clipSpace = zeroToTwo - 1.0;
            gl_Position = vec4(clipSpace * vec2(1, -1), 0, 1);
            v_texCoord = a_texCoord;
        }
    `;

    TA_MODE.fs = `
        precision mediump float;
        uniform vec3 u_colorA;
        uniform vec3 u_colorB;
        uniform vec3 u_colorC;
        uniform vec3 u_colorD;
        uniform int inverted;

        uniform sampler2D u_canvas1;
        varying vec2 v_texCoord;

        void main() {
            vec4 color1 = texture2D(u_canvas1, v_texCoord);
            if (color1.a  < 0.001) {
                gl_FragColor = vec4(0.0);
                return;
            }
            float gradientPos = color1.a;
            vec3 gradientColor;
            if(gradientPos < 0.33) {
                float factor = gradientPos * 3.0;
                gradientColor = mix(u_colorA, u_colorB, factor);
            } else if (gradientPos < 0.66){
                float factor = (gradientPos - 0.33) * 3.0;
                gradientColor = mix(u_colorB, u_colorC, factor);
            } else {
                float factor = (gradientPos - 0.66) * 3.0;
                gradientColor = mix(u_colorC, u_colorD, factor);
            }
            float a = 0.7 * (inverted == 1 ? (1.0 - color1.a) : color1.a);
            gl_FragColor = vec4(gradientColor, 0.3 + a);
        }
    `;

    TA_MODE.vs_points_lines = `
        attribute vec2 a_position;
        uniform vec2 u_resolution;
        void main() {
            vec2 zeroToOne = a_position / u_resolution;
            vec2 zeroToTwo = zeroToOne * 2.0;
            vec2 clipSpace = zeroToTwo - 1.0;
            gl_Position = vec4(clipSpace * vec2(1, -1), 0, 1);
            gl_PointSize = 2.0;
        }
    `;

    TA_MODE.fs_points_lines = `
        precision mediump float;
        uniform vec4 u_color;
        void main() {
            gl_FragColor = u_color;
        }
    `;

    TA_MODE.initWGL = function(channel){
        TA_MODE.releaseWGL(channel)

        TA_MODE.buffer_accumulate_canvas[channel] = document.createElement('canvas');
        TA_MODE.buffer_draw_canvas[channel] = document.createElement('canvas');

        if (TA_MODE.buffer_accumulate_canvas[channel]){
            TA_MODE.buffer_accumulate_canvas_gl_ctx[channel] = TA_MODE.buffer_accumulate_canvas[channel].getContext("webgl");
            if (!TA_MODE.buffer_accumulate_canvas_gl_ctx[channel]) {
                TA_MODE.releaseWGL(channel)
                return false;
            }
        }
        else{
            TA_MODE.releaseWGL(channel)
            return false
        }

        if (TA_MODE.buffer_draw_canvas[channel]){
            TA_MODE.buffer_draw_canvas_gl_ctx[channel] = TA_MODE.buffer_draw_canvas[channel].getContext("webgl");
            if (!TA_MODE.buffer_draw_canvas_gl_ctx[channel]) {
                TA_MODE.releaseWGL(channel)
                return false;
            }
        }
        else{
            TA_MODE.releaseWGL(channel)
            return false
        }

        TA_MODE.buffer_accumulate_program[channel] = twgl.createProgramFromSources(TA_MODE.buffer_accumulate_canvas_gl_ctx[channel], [TA_MODE.vs_points_lines, TA_MODE.fs_points_lines]);
        if (!TA_MODE.buffer_accumulate_program[channel]){
            TA_MODE.releaseWGL(channel)
            return false;
        }

        TA_MODE.buffer_draw_canvas_program[channel] = twgl.createProgramFromSources(TA_MODE.buffer_draw_canvas_gl_ctx[channel], [TA_MODE.vs, TA_MODE.fs]);
        if (!TA_MODE.buffer_draw_canvas_program[channel]){
            TA_MODE.releaseWGL(channel)
            return false;
        }

        TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].useProgram(TA_MODE.buffer_accumulate_program[channel]);
        TA_MODE.buffer_draw_canvas_gl_ctx[channel].useProgram(TA_MODE.buffer_draw_canvas_program[channel]);

        var program_info = TA_MODE.buffer_draw_canvas_program_info[channel]
        var program = TA_MODE.buffer_draw_canvas_program[channel]
        var gl = TA_MODE.buffer_draw_canvas_gl_ctx[channel]


        if (program_info == undefined) program_info = {}
        program_info.a_position = gl.getAttribLocation(program, "a_position");
        program_info.a_texCoord = gl.getAttribLocation(program, "a_texCoord");
        program_info.u_resolution = gl.getUniformLocation(program, "u_resolution");
        program_info.buf_position = gl.createBuffer()
        program_info.tex_buffer = gl.createBuffer()
        program_info.gl_texture = gl.createTexture();

        gl.enableVertexAttribArray(program_info.a_texCoord);
        gl.bindBuffer(gl.ARRAY_BUFFER, program_info.tex_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
            0.0,  0.0,
            1.0,  0.0,
            0.0,  1.0,
            0.0,  1.0,
            1.0,  0.0,
            1.0,  1.0]), gl.STATIC_DRAW);
        gl.vertexAttribPointer(program_info.a_texCoord, 2, gl.FLOAT, false, 0, 0);

        program_info.u_colorA = gl.getUniformLocation(program, "u_colorA")
        program_info.u_colorB = gl.getUniformLocation(program, "u_colorB")
        program_info.u_colorC = gl.getUniformLocation(program, "u_colorC")
        program_info.u_colorD = gl.getUniformLocation(program, "u_colorD")
        program_info.u_canvas1 = gl.getUniformLocation(program, "u_canvas1");
        program_info.inverted = gl.getUniformLocation(program, "inverted");
        TA_MODE.buffer_draw_canvas_program_info[channel] = program_info


        TA_MODE.buffer_accumulate_program_info[channel] = {}
        var api = TA_MODE.buffer_accumulate_program_info[channel]
        api.a_position = TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].getAttribLocation(TA_MODE.buffer_accumulate_program[channel], "a_position");
        api.u_resolution = TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].getUniformLocation(TA_MODE.buffer_accumulate_program[channel], "u_resolution");
        api.u_color = TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].getUniformLocation(TA_MODE.buffer_accumulate_program[channel], "u_color");

        TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].enable(TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].BLEND);
        TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].blendFunc(TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].ONE, TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].ONE);

        TA_MODE.buffer_draw_canvas_gl_ctx[channel].enable(TA_MODE.buffer_draw_canvas_gl_ctx[channel].BLEND);
        TA_MODE.buffer_draw_canvas_gl_ctx[channel].blendFunc(TA_MODE.buffer_draw_canvas_gl_ctx[channel].SRC_ALPHA, TA_MODE.buffer_draw_canvas_gl_ctx[channel].ONE_MINUS_SRC_ALPHA);

        TA_MODE.clearBuffers(channel)

        return true
    }

    TA_MODE.init = function(channels) {
        var f = true
        for(var i = 1; i <= channels; i++){
            f = TA_MODE.initWGL('CH'+i)
            if (f == false)
                break
        }

        if (f == false){
            TA_MODE.isInit = false
            for(var i = 1; i <= channels; i++){
                TA_MODE.releaseWGL('CH'+i)
            }
            return
        }

        TA_MODE.isInit = true
    }

    TA_MODE.releaseWGL = function(channel){
        var pi = TA_MODE.buffer_draw_canvas_program_info[channel]
        var gl_a = TA_MODE.buffer_accumulate_canvas_gl_ctx[channel]
        var gl_d = TA_MODE.buffer_draw_canvas_gl_ctx[channel]
        if (gl_d && pi && pi.gl_texture){
            gl_d.deleteTexture(pi.gl_texture)
            gl_d.deleteBuffer(pi.buf_position)
            gl_d.deleteBuffer(pi.tex_buffer)
            pi.gl_texture = null
        }
        TA_MODE.buffer_draw_canvas_program_info[channel] = undefined

        TA_MODE.buffer_accumulate_program_info[channel] = undefined

        if (TA_MODE.buffer_accumulate_program[channel] && gl_a){
            gl_a.deleteProgram(TA_MODE.buffer_accumulate_program[channel])
            TA_MODE.buffer_accumulate_program[channel] = undefined
        }

        if (TA_MODE.buffer_draw_canvas_program[channel] && gl_d){
            gl_d.deleteProgram(TA_MODE.buffer_draw_canvas_program[channel])
            TA_MODE.buffer_draw_canvas_program[channel] = undefined
        }

        TA_MODE.buffer_accumulate_canvas_gl_ctx[channel] = undefined
        TA_MODE.buffer_accumulate_canvas[channel] = undefined

        TA_MODE.buffer_draw_canvas_gl_ctx[channel] = undefined
        TA_MODE.buffer_draw_canvas[channel] = undefined

    }
    TA_MODE.setGLRectangle = function (gl, x, y, width, height) {
        var x1 = x;
        var x2 = x + width;
        var y1 = y;
        var y2 = y + height;
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
            x1, y1,
            x2, y1,
            x1, y2,
            x1, y2,
            x2, y1,
            x2, y2]), gl.STATIC_DRAW);
    }

    TA_MODE.setSizeWGL = function(channel, w,h){

        // init GL canvas size
        if (TA_MODE.width[channel] != w || TA_MODE.height[channel] != h) {

            TA_MODE.width[channel] = w
            TA_MODE.height[channel] = h

            if (TA_MODE.buffer_accumulate_canvas[channel]){
                TA_MODE.buffer_accumulate_canvas[channel].width =  w
                TA_MODE.buffer_accumulate_canvas[channel].height = h
            }

            if (TA_MODE.buffer_draw_canvas[channel]){
                TA_MODE.buffer_draw_canvas[channel].width =  w
                TA_MODE.buffer_draw_canvas[channel].height = h
            }

            var gl = TA_MODE.buffer_draw_canvas_gl_ctx[channel]
            var pi = TA_MODE.buffer_draw_canvas_program_info[channel]
            if (gl && pi) {
                gl.uniform2f(pi.u_resolution, w, h);
                gl.viewport(0, 0, w, h);
            }

            var agl = TA_MODE.buffer_accumulate_canvas_gl_ctx[channel]
            var api = TA_MODE.buffer_accumulate_program_info[channel]
            if (agl && api) {
                agl.uniform2f(api.u_resolution, w, h);
                agl.viewport(0, 0, w, h);
            }
            return true
        }
        return false

    }

    TA_MODE.clearBuffers = function(channel) {
        if (TA_MODE.buffer_accumulate_canvas_gl_ctx[channel]){
            TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].clearColor(0, 0, 0, 0);
            TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].clear(TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].COLOR_BUFFER_BIT);
        }
        if (TA_MODE.buffer_draw_canvas_gl_ctx[channel]){
            TA_MODE.buffer_draw_canvas_gl_ctx[channel].clearColor(0, 0, 0, 0);
            TA_MODE.buffer_draw_canvas_gl_ctx[channel].clear(TA_MODE.buffer_accumulate_canvas_gl_ctx[channel].COLOR_BUFFER_BIT);
        }
    }

    TA_MODE.setNewSizeWGL = function(w,h){
        for(var ch in TA_MODE.buffer_draw_canvas){
            var ret = TA_MODE.setSizeWGL(ch, w,h)
            if (ret){
                TA_MODE.clearBuffers(ch)
            }
        }
    }


    TA_MODE.setupGLTexture = function (gl, canvas, pi, gradientParams,inverted) {

        const hexToFloatArray = function (hex) {
            hex = hex.replace('#', '');

            const r = parseInt(hex.substring(0, 2), 16) / 255;
            const g = parseInt(hex.substring(2, 4), 16) / 255;
            const b = parseInt(hex.substring(4, 6), 16) / 255;

            return [r, g, b];
        }

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, pi.gl_texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas)

        // Set the parameters so we can render any size image.
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

        gl.uniform1i(pi.u_canvas1, 0);

        gl.uniform3fv(pi.u_colorA, hexToFloatArray(gradientParams.colorA));
        gl.uniform3fv(pi.u_colorB, hexToFloatArray(gradientParams.colorB));
        gl.uniform3fv(pi.u_colorC, hexToFloatArray(gradientParams.colorC));
        gl.uniform3fv(pi.u_colorD, hexToFloatArray(gradientParams.colorD));

        gl.uniform1i(pi.inverted, inverted);

    }

    TA_MODE.drawSeriesPoints = function(gl, programInfo, series, points) {
        if (!points || points.length === 0) return;

        const positions = [];
        for (let i = 0; i < points.length; i++) {
            const x = i;
            const y = points[i];
            if(isNaN(y)) continue;
            const xC = series.xaxis.p2c(x);
            const yC = series.yaxis.p2c(y);

            positions.push(xC, yC);
        }

        const buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);

        gl.enableVertexAttribArray(programInfo.a_position);
        gl.vertexAttribPointer(programInfo.a_position, 2, gl.FLOAT, false, 0, 0);
        gl.uniform4f(programInfo.u_color, 0.0, 0.0, 0.0, 0.01);
        gl.drawArrays(gl.POINTS, 0, positions.length / 2);
        gl.deleteBuffer(buffer);
    };

    TA_MODE.drawSeriesLines = function(gl, programInfo, series, points) {
        if (!points || points.length < 2) return;

        const positions = [];
        for (let i = 1; i < points.length; i++) {
            const x1 = i - 1, y1 = points[i - 1];
            const x2 = i, y2 = points[i];

            const x1C = series.xaxis.p2c(x1);
            const y1C = series.yaxis.p2c(y1);
            const x2C = series.xaxis.p2c(x2);
            const y2C = series.yaxis.p2c(y2);

            positions.push(x1C, y1C, x2C, y2C);
        }

        const buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);

        gl.enableVertexAttribArray(programInfo.a_position);
        gl.vertexAttribPointer(programInfo.a_position, 2, gl.FLOAT, false, 0, 0);

        gl.uniform4f(programInfo.u_color, 0.0, 0.0, 0.0, 0.01);
        gl.lineWidth(2.0);
        gl.drawArrays(gl.LINES, 0, positions.length / 2);
        gl.deleteBuffer(buffer);
    };


    TA_MODE.draw = function(plot,ctx, series) {

        if (TA_MODE.isInit == false) {
            return
        }

        const canvas = plot.getCanvas()

        var ac = TA_MODE.buffer_accumulate_canvas[series.channel]
        var agl = TA_MODE.buffer_accumulate_canvas_gl_ctx[series.channel]
        var api = TA_MODE.buffer_accumulate_program_info[series.channel]

        var dc = TA_MODE.buffer_draw_canvas[series.channel]
        var dgl = TA_MODE.buffer_draw_canvas_gl_ctx[series.channel]
        var dpi = TA_MODE.buffer_draw_canvas_program_info[series.channel]

        var data = TA_MODE.data_points[series.channel]
        if (data != undefined && api && agl){

            for(var i = 0; i < data.length; i++){
                if (series.lines.show) {
                    TA_MODE.drawSeriesLines(agl, api, series, data[i].points);
                }
                if (series.points.show) {
                    TA_MODE.drawSeriesPoints(agl, api, series, data[i].points);
                }
            }
            //  ctx.drawImage(ac,0,0)
        }

        TA_MODE.data_points[series.channel] = []

        if (agl && dpi && dgl && ac){

            dgl.clearColor(0, 0, 0, 0);
            dgl.clear(dgl.COLOR_BUFFER_BIT);
            dgl.enableVertexAttribArray(dpi.a_position);
            dgl.bindBuffer(dgl.ARRAY_BUFFER, dpi.buf_position);
            TA_MODE.setGLRectangle(dgl,0,0,canvas.width,canvas.height)
            dgl.vertexAttribPointer(dpi.a_position, 2, dgl.FLOAT, false, 0, 0);

            TA_MODE.setupGLTexture(dgl, ac, dpi,
                {
                    colorA: TA_MODE.color1[series.channel],
                    colorB: TA_MODE.color2[series.channel],
                    colorC: TA_MODE.color3[series.channel],
                    colorD: TA_MODE.color4[series.channel]
                },
                TA_MODE.inverted[series.channel]
            );

            dgl.drawArrays(dgl.TRIANGLES, 0, 6);
            ctx.drawImage(dc,0,0)
        }
    }

    TA_MODE.resetData = function() {
        TA_MODE.data_points = {}
    }

    TA_MODE.setPoints = function(channel, points, ts, decimation, tScale){
        var ch = 'CH' + (channel + 1)
        console.log('Data',ch, decimation, tScale)
        if (TA_MODE.decimation != decimation || TA_MODE.tScale != tScale){
            TA_MODE.data_points[ch] = []
            TA_MODE.clearBuffers(ch)
            TA_MODE.decimation = decimation
            TA_MODE.tScale = tScale
            console.log('Data reset',ch, decimation, tScale)
        }
        if (TA_MODE.data_points[ch] == undefined){
            TA_MODE.data_points[ch] = []
        }
        TA_MODE.data_points[ch].push({ts: ts, points, points})
    }

    TA_MODE.setColor = function(ch,colnum,color) {
        if (colnum == 1){
            TA_MODE.color1[ch] = color
        }
        if (colnum == 2){
            TA_MODE.color2[ch] = color
        }
        if (colnum == 3){
            TA_MODE.color3[ch] = color
        }
        if (colnum == 4){
            TA_MODE.color4[ch] = color
        }
    }

    TA_MODE.setInverted = function(ch,state) {
        TA_MODE.inverted[ch] = state
    }


}(window.TA_MODE = window.TA_MODE || {}, jQuery));


// Page onload event handler
$(function() {

})
