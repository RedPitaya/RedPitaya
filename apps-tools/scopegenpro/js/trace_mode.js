class TAMode {
    constructor() {
        this.width = undefined;
        this.height = undefined;

        this.buffer_accumulate_canvas = undefined;
        this.buffer_accumulate_canvas_gl_ctx = undefined;
        this.buffer_accumulate_program = undefined;
        this.buffer_accumulate_program_info = undefined;

        this.buffer_draw_canvas = undefined;
        this.buffer_draw_canvas_gl_ctx = undefined;
        this.buffer_draw_canvas_program = undefined;
        this.buffer_draw_canvas_program_info = undefined;

        this.data_points = [];
        this.data_points_array = undefined;

        this.isInit = false;

        this.color1 = "#3F00FF";
        this.color2 = "#0000FF";
        this.color3 = "#00FF00";
        this.color4 = "#FF0000";
        this.inverted = 0;

        this.vs = `
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

        this.fs = `
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

        this.vs_points_lines = `
            attribute float a_y;
            attribute float a_index;
            uniform vec2 u_resolution;
            void main() {
                float x = a_index;
                float y = a_y;
                vec2 zeroToOne = vec2(x / u_resolution.x, y / u_resolution.y);
                vec2 zeroToTwo = zeroToOne * 2.0;
                vec2 clipSpace = zeroToTwo - 1.0;
                gl_Position = vec4(clipSpace * vec2(1, -1), 0, 1);
                gl_PointSize = 2.0;
            }
        `;

        this.fs_points_lines = `
            precision mediump float;
            uniform vec4 u_color;
            void main() {
                gl_FragColor = u_color;
            }
        `;
    }

    initWGL() {
        this.releaseWGL();

        this.buffer_accumulate_canvas = document.createElement('canvas');
        this.buffer_draw_canvas = document.createElement('canvas');

        if (this.buffer_accumulate_canvas) {
            this.buffer_accumulate_canvas_gl_ctx = this.buffer_accumulate_canvas.getContext("webgl");
            if (!this.buffer_accumulate_canvas_gl_ctx) {
                this.releaseWGL();
                return false;
            }
        } else {
            this.releaseWGL();
            return false;
        }

        if (this.buffer_draw_canvas) {
            this.buffer_draw_canvas_gl_ctx = this.buffer_draw_canvas.getContext("webgl");
            if (!this.buffer_draw_canvas_gl_ctx) {
                this.releaseWGL();
                return false;
            }
        } else {
            this.releaseWGL();
            return false;
        }

        this.buffer_accumulate_program = twgl.createProgramFromSources(
            this.buffer_accumulate_canvas_gl_ctx,
            [this.vs_points_lines, this.fs_points_lines]
        );
        if (!this.buffer_accumulate_program) {
            this.releaseWGL();
            return false;
        }

        this.buffer_draw_canvas_program = twgl.createProgramFromSources(
            this.buffer_draw_canvas_gl_ctx,
            [this.vs, this.fs]
        );
        if (!this.buffer_draw_canvas_program) {
            this.releaseWGL();
            return false;
        }

        this.buffer_accumulate_canvas_gl_ctx.useProgram(this.buffer_accumulate_program);
        this.buffer_draw_canvas_gl_ctx.useProgram(this.buffer_draw_canvas_program);

        let program_info = this.buffer_draw_canvas_program_info;
        const program = this.buffer_draw_canvas_program;
        const gl = this.buffer_draw_canvas_gl_ctx;

        if (program_info == undefined) program_info = {};
        program_info.a_position = gl.getAttribLocation(program, "a_position");
        program_info.a_texCoord = gl.getAttribLocation(program, "a_texCoord");
        program_info.u_resolution = gl.getUniformLocation(program, "u_resolution");
        program_info.buf_position = gl.createBuffer();
        program_info.tex_buffer = gl.createBuffer();
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

        program_info.u_colorA = gl.getUniformLocation(program, "u_colorA");
        program_info.u_colorB = gl.getUniformLocation(program, "u_colorB");
        program_info.u_colorC = gl.getUniformLocation(program, "u_colorC");
        program_info.u_colorD = gl.getUniformLocation(program, "u_colorD");
        program_info.u_canvas1 = gl.getUniformLocation(program, "u_canvas1");
        program_info.inverted = gl.getUniformLocation(program, "inverted");

        this.buffer_draw_canvas_program_info = program_info;

        this.buffer_accumulate_program_info = {};
        const api = this.buffer_accumulate_program_info;
        api.a_y = this.buffer_accumulate_canvas_gl_ctx.getAttribLocation(this.buffer_accumulate_program, "a_y");
        api.a_index = this.buffer_accumulate_canvas_gl_ctx.getAttribLocation(this.buffer_accumulate_program, "a_index");
        api.u_resolution = this.buffer_accumulate_canvas_gl_ctx.getUniformLocation(this.buffer_accumulate_program, "u_resolution");
        api.u_color = this.buffer_accumulate_canvas_gl_ctx.getUniformLocation(this.buffer_accumulate_program, "u_color");
        api.point_buffer = this.buffer_accumulate_canvas_gl_ctx.createBuffer();
        api.index_buffer = this.buffer_accumulate_canvas_gl_ctx.createBuffer();

        this.buffer_accumulate_canvas_gl_ctx.enable(this.buffer_accumulate_canvas_gl_ctx.BLEND);
        this.buffer_accumulate_canvas_gl_ctx.blendFunc(this.buffer_accumulate_canvas_gl_ctx.ONE, this.buffer_accumulate_canvas_gl_ctx.ONE);

        this.buffer_draw_canvas_gl_ctx.enable(this.buffer_draw_canvas_gl_ctx.BLEND);
        this.buffer_draw_canvas_gl_ctx.blendFunc(this.buffer_draw_canvas_gl_ctx.SRC_ALPHA, this.buffer_draw_canvas_gl_ctx.ONE_MINUS_SRC_ALPHA);

        this.clearBuffers();
        return true;
    }

    init() {
        const f = this.initWGL();
        if (f == false) {
            this.isInit = false;
            this.releaseWGL();
            return;
        }

        this.isInit = true;
    }

    releaseWGL() {
        const pi = this.buffer_draw_canvas_program_info;
        const api = this.buffer_accumulate_program_info;
        const gl_a = this.buffer_accumulate_canvas_gl_ctx;
        const gl_d = this.buffer_draw_canvas_gl_ctx;

        if (gl_d && pi && pi.gl_texture) {
            gl_d.deleteTexture(pi.gl_texture);
            gl_d.deleteBuffer(pi.buf_position);
            gl_d.deleteBuffer(pi.tex_buffer);
            pi.gl_texture = null;
        }

        if (gl_a && api) {
            gl_a.deleteBuffer(api.point_buffer);
            gl_a.deleteBuffer(api.index_buffer);
        }

        this.buffer_draw_canvas_program_info = undefined;
        this.buffer_accumulate_program_info = undefined;

        if (this.buffer_accumulate_program && gl_a) {
            gl_a.deleteProgram(this.buffer_accumulate_program);
            this.buffer_accumulate_program = undefined;
        }

        if (this.buffer_draw_canvas_program && gl_d) {
            gl_d.deleteProgram(this.buffer_draw_canvas_program);
            this.buffer_draw_canvas_program = undefined;
        }

        this.buffer_accumulate_canvas_gl_ctx = undefined;
        this.buffer_accumulate_canvas = undefined;

        this.buffer_draw_canvas_gl_ctx = undefined;
        this.buffer_draw_canvas = undefined;
    }

    destroy() {
        this.releaseWGL();

        this.width = undefined;
        this.height = undefined;
        this.isInit = false;

        this.data_points = [];
        this.data_points_array = undefined;

        this.color1 = "#3F00FF";
        this.color2 = "#0000FF";
        this.color3 = "#00FF00";
        this.color4 = "#FF0000";
        this.inverted = 0;

        this.vs = null;
        this.fs = null;
        this.vs_points_lines = null;
        this.fs_points_lines = null;

        console.log('TAMode destroyed');
    }

    setGLRectangle(gl, x, y, width, height) {
        const x1 = x;
        const x2 = x + width;
        const y1 = y;
        const y2 = y + height;

        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
            x1, y1,
            x2, y1,
            x1, y2,
            x1, y2,
            x2, y1,
            x2, y2]), gl.STATIC_DRAW);
    }

    setSizeWGL(w, h) {
        if (this.width != w || this.height != h) {
            if (this.buffer_accumulate_canvas) {
                this.buffer_accumulate_canvas.width = w;
                this.buffer_accumulate_canvas.height = h;
            }

            if (this.buffer_draw_canvas) {
                this.buffer_draw_canvas.width = w;
                this.buffer_draw_canvas.height = h;
            }

            const gl = this.buffer_draw_canvas_gl_ctx;
            const pi = this.buffer_draw_canvas_program_info;
            if (gl && pi) {
                gl.enableVertexAttribArray(pi.a_position);
                gl.bindBuffer(gl.ARRAY_BUFFER, pi.buf_position);
                this.setGLRectangle(gl, 0, 0, w, h);
                gl.vertexAttribPointer(pi.a_position, 2, gl.FLOAT, false, 0, 0);
                gl.uniform2f(pi.u_resolution, w, h);
                gl.viewport(0, 0, w, h);
            }

            const agl = this.buffer_accumulate_canvas_gl_ctx;
            const api = this.buffer_accumulate_program_info;
            if (agl && api) {
                agl.uniform2f(api.u_resolution, w, h);
                agl.viewport(0, 0, w, h);
                this.width = w;
                this.height = h;
            }
            return true;
        }
        return false;
    }

    clearBuffers() {
        if (this.buffer_accumulate_canvas_gl_ctx) {
            this.buffer_accumulate_canvas_gl_ctx.clearColor(0, 0, 0, 0);
            this.buffer_accumulate_canvas_gl_ctx.clear(this.buffer_accumulate_canvas_gl_ctx.COLOR_BUFFER_BIT);
        }
        if (this.buffer_draw_canvas_gl_ctx) {
            this.buffer_draw_canvas_gl_ctx.clearColor(0, 0, 0, 0);
            this.buffer_draw_canvas_gl_ctx.clear(this.buffer_accumulate_canvas_gl_ctx.COLOR_BUFFER_BIT);
        }
    }

    setNewSizeWGL(w, h) {
        const ret = this.setSizeWGL(w, h);
        if (ret) {
            this.clearBuffers();
        }
    }

    setupGLTexture(gl, canvas, pi, gradientParams, inverted) {
        const hexToFloatArray = function (hex) {
            hex = hex.replace('#', '');

            const r = parseInt(hex.substring(0, 2), 16) / 255;
            const g = parseInt(hex.substring(2, 4), 16) / 255;
            const b = parseInt(hex.substring(4, 6), 16) / 255;

            return [r, g, b];
        };

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, pi.gl_texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);

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

    drawSeriesPoints(gl, programInfo, series, points) {
        if (!points || points.length === 0) return;

        if (programInfo.indices == undefined) {
            programInfo.indices = new Float32Array(points.length);
        } else {
            if (programInfo.indices.length != points.length) {
                programInfo.indices = new Float32Array(points.length);
            }
        }

        if (programInfo.a_y == undefined) {
            programInfo.a_y = new Float32Array(points.length);
        } else {
            if (programInfo.a_y.length != points.length) {
                programInfo.a_y = new Float32Array(points.length);
            }
        }

        for (let i = 0; i < points.length; i++) {
            programInfo.indices[i] = series.xaxis.p2c(i);
            programInfo.a_y[i] = points[i];
        }

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.point_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, programInfo.a_y, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_y);
        gl.vertexAttribPointer(programInfo.a_y, 1, gl.FLOAT, false, 0, 0);

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.index_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, programInfo.indices, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_index);
        gl.vertexAttribPointer(programInfo.a_index, 1, gl.FLOAT, false, 0, 0);

        gl.uniform4f(programInfo.u_color, 0.0, 0.0, 0.0, 0.01);
        gl.drawArrays(gl.POINTS, 0, points.length);
    }

    drawSeriesLines(gl, programInfo, series, points) {
        if (!points || points.length < 2) return;

        if (programInfo.indices == undefined) {
            programInfo.indices = new Float32Array(points.length);
        } else {
            if (programInfo.indices.length != points.length) {
                programInfo.indices = new Float32Array(points.length);
            }
        }

        if (programInfo.a_y == undefined) {
            programInfo.a_y = new Float32Array(points.length);
        } else {
            if (programInfo.a_y.length != points.length) {
                programInfo.a_y = new Float32Array(points.length);
            }
        }

        for (let i = 0; i < points.length; i++) {
            programInfo.indices[i] = series.xaxis.p2c(i);
            programInfo.a_y[i] = points[i];
        }

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.point_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, programInfo.a_y, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_y);
        gl.vertexAttribPointer(programInfo.a_y, 1, gl.FLOAT, false, 0, 0);

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.index_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, programInfo.indices, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_index);
        gl.vertexAttribPointer(programInfo.a_index, 1, gl.FLOAT, false, 0, 0);

        gl.uniform4f(programInfo.u_color, 0.0, 0.0, 0.0, 0.01);
        gl.lineWidth(2.0);
        gl.drawArrays(gl.LINE_STRIP, 0, points.length);
    }

    draw(plot, ctx, series) {
        if (this.isInit == false) {
            return;
        }

        const canvas = plot.getCanvas();

        const ac = this.buffer_accumulate_canvas;
        const agl = this.buffer_accumulate_canvas_gl_ctx;
        const api = this.buffer_accumulate_program_info;

        const dc = this.buffer_draw_canvas;
        const dgl = this.buffer_draw_canvas_gl_ctx;
        const dpi = this.buffer_draw_canvas_program_info;

        const data = this.data_points;
        if (data != undefined && api && agl) {
            for (let i = 0; i < data.length; i++) {
                const points = data[i];
                for (let j = 0; j < points.length; j++) {
                    const y = points[j];
                    if (isNaN(y)) {
                        points[j] = NaN;
                    } else {
                        points[j] = series.yaxis.p2c(y);
                    }
                }

                if (series.lines.show || series.lines.show_gl) {
                    this.drawSeriesLines(agl, api, series, points);
                }
                if (series.points.show || series.points.show_gl) {
                    this.drawSeriesPoints(agl, api, series, points);
                }
            }
        }

        this.data_points = [];

        if (agl && dpi && dgl && ac) {
            dgl.clearColor(0, 0, 0, 0);
            dgl.clear(dgl.COLOR_BUFFER_BIT);

            this.setupGLTexture(dgl, ac, dpi,
                {
                    colorA: this.color1,
                    colorB: this.color2,
                    colorC: this.color3,
                    colorD: this.color4
                },
                this.inverted
            );

            dgl.drawArrays(dgl.TRIANGLES, 0, 6);
            ctx.drawImage(dc, 0, 0);
        }
    }

    resetData() {
        this.data_points = [];
    }

    setPoints(points, decimation, tScale) {
        if (this.decimation != decimation || this.tScale != tScale) {
            this.data_points = [];
            this.clearBuffers();
            this.decimation = decimation;
            this.tScale = tScale;
        }
        if (this.data_points == undefined) {
            this.data_points = [];
        }
        this.data_points.push(points);
    }

    setColor(colnum, color) {
        if (colnum == 1) {
            this.color1 = color;
        }
        if (colnum == 2) {
            this.color2 = color;
        }
        if (colnum == 3) {
            this.color3 = color;
        }
        if (colnum == 4) {
            this.color4 = color;
        }
    }

    setInverted(state) {
        this.inverted = state;
    }
}