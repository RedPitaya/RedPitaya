class GLMode {
    constructor() {
        this.width = undefined;
        this.height = undefined;

        this.buffer_draw_canvas = undefined;
        this.buffer_draw_canvas_gl_ctx = undefined;
        this.buffer_draw_canvas_program = undefined;
        this.buffer_draw_canvas_program_info = undefined;

        this.isInit = false;

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
                gl_PointSize = 3.0;
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

    destroy() {
        this.releaseWGL();

        this.width = undefined;
        this.height = undefined;
        this.isInit = false;

        this.vs_points_lines = null;
        this.fs_points_lines = null;

        console.log('GLMode destroyed');
    }

    hexToFloatArray(hex) {
        hex = hex.replace('#', '');

        const r = parseInt(hex.substring(0, 2), 16) / 255;
        const g = parseInt(hex.substring(2, 4), 16) / 255;
        const b = parseInt(hex.substring(4, 6), 16) / 255;
        const a = parseInt(hex.substring(7, 9), 16) / 255;
        if (!isNaN(a))
            return [r, g, b, a];
        return [r, g, b, 1.0];
    }

    initWGL() {
        this.releaseWGL();
        this.buffer_draw_canvas = document.createElement('canvas');

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

        this.buffer_draw_canvas_program = twgl.createProgramFromSources(
            this.buffer_draw_canvas_gl_ctx,
            [this.vs_points_lines, this.fs_points_lines]
        );

        if (!this.buffer_draw_canvas_program) {
            this.releaseWGL();
            return false;
        }

        this.buffer_draw_canvas_gl_ctx.useProgram(this.buffer_draw_canvas_program);

        let program_info = this.buffer_draw_canvas_program_info;
        const program = this.buffer_draw_canvas_program;
        const gl = this.buffer_draw_canvas_gl_ctx;

        if (program_info == undefined) program_info = {};
        program_info.u_resolution = gl.getUniformLocation(program, "u_resolution");
        program_info.u_color = gl.getUniformLocation(program, "u_color");
        program_info.a_y = gl.getAttribLocation(program, "a_y");
        program_info.a_index = gl.getAttribLocation(program, "a_index");

        program_info.point_buffer = gl.createBuffer();
        program_info.index_buffer = gl.createBuffer();

        this.buffer_draw_canvas_program_info = program_info;
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
        const gl_d = this.buffer_draw_canvas_gl_ctx;

        if (gl_d && pi && pi.gl_texture) {
            gl_d.deleteBuffer(pi.point_buffer);
            gl_d.deleteBuffer(pi.index_buffer);
        }

        this.buffer_draw_canvas_program_info = undefined;

        if (this.buffer_draw_canvas_program && gl_d) {
            gl_d.deleteProgram(this.buffer_draw_canvas_program);
            this.buffer_draw_canvas_program = undefined;
        }

        this.buffer_draw_canvas_gl_ctx = undefined;
        this.buffer_draw_canvas = undefined;
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
            if (this.buffer_draw_canvas) {
                this.buffer_draw_canvas.width = w;
                this.buffer_draw_canvas.height = h;
            }

            const gl = this.buffer_draw_canvas_gl_ctx;
            const pi = this.buffer_draw_canvas_program_info;

            if (gl && pi) {
                gl.uniform2f(pi.u_resolution, w, h);
                gl.viewport(0, 0, w, h);
                this.width = w;
                this.height = h;
            }
            return true;
        }
        return false;
    }

    clearBuffers() {
        if (this.buffer_draw_canvas_gl_ctx) {
            this.buffer_draw_canvas_gl_ctx.clearColor(0, 0, 0, 0);
            this.buffer_draw_canvas_gl_ctx.clear(this.buffer_draw_canvas_gl_ctx.COLOR_BUFFER_BIT);
        }
    }

    setNewSizeWGL(w, h) {
        if (this.setSizeWGL(w, h)) {
            this.clearBuffers();
        }
    }

    drawSeriesPoints(gl, programInfo, series, points_x, points_y) {
        if (!points_x || points_x.length === 0) return;
        if (!points_y || points_y.length === 0) return;

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.point_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, points_y, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_y);
        gl.vertexAttribPointer(programInfo.a_y, 1, gl.FLOAT, false, 0, 0);

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.index_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, points_x, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_index);
        gl.vertexAttribPointer(programInfo.a_index, 1, gl.FLOAT, false, 0, 0);

        gl.uniform4fv(programInfo.u_color, this.hexToFloatArray(series.color));
        gl.drawArrays(gl.POINTS, 0, points_x.length);
    }

    drawSeriesLines(gl, programInfo, series, points_x, points_y) {
        if (!points_x || points_x.length < 2) return;
        if (!points_y || points_y.length < 2) return;

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.point_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, points_y, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_y);
        gl.vertexAttribPointer(programInfo.a_y, 1, gl.FLOAT, false, 0, 0);

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.index_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, points_x, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_index);
        gl.vertexAttribPointer(programInfo.a_index, 1, gl.FLOAT, false, 0, 0);

        gl.uniform4fv(programInfo.u_color, this.hexToFloatArray(series.color));
        gl.lineWidth(2.0);

        gl.drawArrays(gl.LINE_STRIP, 0, points_x.length);
    }

    draw(ctx, series) {
        if (this.isInit == false) {
            return;
        }

        const dc = this.buffer_draw_canvas;
        const dgl = this.buffer_draw_canvas_gl_ctx;
        const dpi = this.buffer_draw_canvas_program_info;

        if (series.data_gl != undefined && dpi && dgl) {
            if (series.lines.show_gl || series.points.show_gl) {

                if (dpi.indices == undefined) {
                    dpi.indices = new Float32Array(series.data_gl.length);
                } else {
                    if (dpi.indices.length != series.data_gl.length) {
                        dpi.indices = new Float32Array(series.data_gl.length);
                    }
                }

                for (let i = 0; i < dpi.indices.length; i++) {
                    dpi.indices[i] = series.xaxis.p2c(i);
                }

                this.clearBuffers();

                for (let i = 0; i < series.data_gl.length; i++) {
                    const y = series.data_gl[i];
                    if (isNaN(y)) {
                        series.data_gl[i] = NaN;
                    } else {
                        series.data_gl[i] = series.yaxis.p2c(y);
                    }
                }

                if (series.lines.show_gl) {
                    this.drawSeriesLines(dgl, dpi, series, dpi.indices, series.data_gl);
                }
                if (series.points.show_gl) {
                    this.drawSeriesPoints(dgl, dpi, series, dpi.indices, series.data_gl);
                }

                ctx.drawImage(dc, 0, 0);
            }
        }
    }

    drawXY(ctx, series) {
        if (this.isInit == false) {
            return;
        }

        const dc = this.buffer_draw_canvas;
        const dgl = this.buffer_draw_canvas_gl_ctx;
        const dpi = this.buffer_draw_canvas_program_info;

        if (series.data_gl != undefined && dpi && dgl) {
            if (series.lines.show_gl || series.points.show_gl) {
                let l = series.data_gl.length
                if (dpi.indices == undefined) {
                    dpi.indices = new Float32Array(l);
                } else {
                    if (dpi.indices.length != l) {
                        dpi.indices = new Float32Array(l);
                    }
                }

                if (dpi.y_values == undefined) {
                    dpi.y_values = new Float32Array(l);
                } else {
                    if (dpi.y_values.length != l) {
                        dpi.y_values = new Float32Array(l);
                    }
                }

                for (let i = 0; i < l; i++) {
                    const x = series.data_gl[i][0];
                    const y = series.data_gl[i][1];
                    dpi.indices[i] = series.xaxis.p2c(x);
                    dpi.y_values[i] = series.yaxis.p2c(y);
                }

                this.clearBuffers();

                if (series.lines.show_gl) {
                    this.drawSeriesLines(dgl, dpi, series, dpi.indices, dpi.y_values);
                }
                if (series.points.show_gl) {
                    this.drawSeriesPoints(dgl, dpi, series, dpi.indices, dpi.y_values);
                }

                ctx.drawImage(dc, 0, 0);
            }
        }
    }
}