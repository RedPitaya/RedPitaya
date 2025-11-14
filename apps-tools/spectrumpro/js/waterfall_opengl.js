/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

class GLWaterfall {
    constructor() {
        this.width = undefined;
        this.height = undefined;

        this.buffer_draw_canvas = undefined;
        this.buffer_draw_canvas_gl_ctx = undefined;
        this.buffer_draw_canvas_program = undefined;
        this.buffer_draw_canvas_program_info = undefined;

        this.shiftFramebuffer = undefined;
        this.shiftTexture = undefined;
        this.shiftProgram = undefined;
        this.shiftProgramInfo = undefined;

        this.isInit = false;

        this.vs_points_lines = `
            attribute float a_index;
            attribute vec3 a_color;
            uniform vec2 u_resolution;
            varying vec3 v_color;

            void main() {
                float x = a_index;
                float y = 0.0;
                vec2 zeroToOne = vec2(x / u_resolution.x, y / u_resolution.y);
                vec2 zeroToTwo = zeroToOne * 2.0;
                vec2 clipSpace = zeroToTwo - 1.0;
                gl_Position = vec4(clipSpace * vec2(1, -1), 0, 1);
                v_color = a_color;
            }
        `;

        this.fs_points_lines = `
            precision mediump float;
            varying vec3 v_color;

            void main() {
                gl_FragColor = vec4(v_color, 1.0);
            }
        `;

        this.vs_shift = `
            attribute vec2 a_position;
            attribute vec2 a_texCoord;
            varying vec2 v_texCoord;

            void main() {
                gl_Position = vec4(a_position, 0.0, 1.0);
                v_texCoord = a_texCoord;
            }
        `;

        this.fs_shift = `
            precision mediump float;
            uniform sampler2D u_previousFrame;
            uniform vec2 u_resolution;
            varying vec2 v_texCoord;

            void main() {
                vec2 shiftedCoord = v_texCoord + vec2(0.0, 1.0 / u_resolution.y);

                if (shiftedCoord.y < 0.0) {
                    gl_FragColor = vec4(0, 0, 0, 1);
                } else {
                    gl_FragColor = texture2D(u_previousFrame, shiftedCoord);
                }
            }
        `;
    }

    destroy() {
        this.releaseWGL();
        this.releaseShiftResources();

        this.width = undefined;
        this.height = undefined;
        this.isInit = false;

        this.vs_points_lines = null;
        this.fs_points_lines = null;
        this.vs_shift = null;
        this.fs_shift = null;

        console.log('GLWaterfall destroyed');
    }

    initShiftResources() {
        if (!this.buffer_draw_canvas_gl_ctx) return false;

        const gl = this.buffer_draw_canvas_gl_ctx;

        this.shiftProgram = twgl.createProgramFromSources(
            gl,
            [this.vs_shift, this.fs_shift]
        );

        if (!this.shiftProgram) {
            console.error('Failed to create shift program');
            return false;
        }

        gl.useProgram(this.shiftProgram);

        this.shiftProgramInfo = {
            u_resolution: gl.getUniformLocation(this.shiftProgram, "u_resolution"),
            u_previousFrame: gl.getUniformLocation(this.shiftProgram, "u_previousFrame"),
            a_position: gl.getAttribLocation(this.shiftProgram, "a_position"),
            a_texCoord: gl.getAttribLocation(this.shiftProgram, "a_texCoord")
        };

        this.shiftTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, this.shiftTexture);
        if (this.width != undefined && this.height != undefined){
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.width, this.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        }
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

        this.shiftFramebuffer = gl.createFramebuffer();
        gl.bindFramebuffer(gl.FRAMEBUFFER, this.shiftFramebuffer);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.shiftTexture, 0);

        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, null);


        this.shiftProgramInfo.positionBuffer = gl.createBuffer();
        this.shiftProgramInfo.texCoordBuffer = gl.createBuffer();

        this.shiftFramebuffer2 = gl.createFramebuffer();
        this.shiftTexture2 = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, this.shiftTexture2);
        if (this.width != undefined && this.height != undefined){
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.width, this.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        }
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, this.shiftFramebuffer2);
        gl.framebufferTexture2D(gl.DRAW_FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.shiftTexture2, 0);

        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, null);

        return true;
    }

    releaseShiftResources() {
        const gl = this.buffer_draw_canvas_gl_ctx;
        if (!gl) return;

        if (this.shiftProgramInfo) {
            if (this.shiftProgramInfo.positionBuffer) {
                gl.deleteBuffer(this.shiftProgramInfo.positionBuffer);
            }
            if (this.shiftProgramInfo.texCoordBuffer) {
                gl.deleteBuffer(this.shiftProgramInfo.texCoordBuffer);
            }
        }

        if (this.shiftTexture) {
            gl.deleteTexture(this.shiftTexture);
            this.shiftTexture = undefined;
        }

        if (this.shiftFramebuffer) {
            gl.deleteFramebuffer(this.shiftFramebuffer);
            this.shiftFramebuffer = undefined;
        }

        if (this.shiftTexture2) {
            gl.deleteTexture(this.shiftTexture2);
            this.shiftTexture2 = undefined;
        }

        if (this.shiftFramebuffer2) {
            gl.deleteFramebuffer(this.shiftFramebuffer2);
            this.shiftFramebuffer2 = undefined;
        }

        if (this.shiftProgram) {
            gl.deleteProgram(this.shiftProgram);
            this.shiftProgram = undefined;
        }

        this.shiftProgramInfo = undefined;
    }

    shiftFrameBufferDown() {
        if (!this.isInit || !this.shiftProgram || !this.shiftFramebuffer) {
            console.warn('Shift resources not initialized');
            return false;
        }

        if (this.width == undefined || this.height == undefined){
            return false
        }

        if (this.width <= 0 || this.height <= 0) {
            console.warn('Invalid canvas size:', this.width, this.height);
            return false;
        }

        const gl = this.buffer_draw_canvas_gl_ctx;
        const spi = this.shiftProgramInfo;




        gl.bindFramebuffer(gl.FRAMEBUFFER, this.shiftFramebuffer);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.shiftTexture, 0);

        const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
        if (status !== gl.FRAMEBUFFER_COMPLETE) {
            console.error('Framebuffer is not complete:', this.getFramebufferStatus(status));
            gl.bindFramebuffer(gl.FRAMEBUFFER, null);
            return false;
        }

        gl.bindFramebuffer(gl.READ_FRAMEBUFFER, null);
        gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, this.shiftFramebuffer);

        gl.blitFramebuffer(
            0, 0, this.width, this.height,
            0, 0, this.width, this.height,
            gl.COLOR_BUFFER_BIT, gl.NEAREST
        );

        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        gl.useProgram(this.shiftProgram);

        gl.uniform2f(spi.u_resolution, this.width, this.height);
        gl.uniform1i(spi.u_previousFrame, 0);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, this.shiftTexture);

        this.renderFullScreenQuad(spi);

        return true;
    }

    renderFullScreenQuad(programInfo) {
        const gl = this.buffer_draw_canvas_gl_ctx;

        const positions = new Float32Array([
            -1.0, -1.0,
             1.0, -1.0,
            -1.0,  1.0,
             1.0,  1.0
        ]);

        const texCoords = new Float32Array([
            0.0, 0.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 1.0
        ]);

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.positionBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_position);
        gl.vertexAttribPointer(programInfo.a_position, 2, gl.FLOAT, false, 0, 0);

        gl.bindBuffer(gl.ARRAY_BUFFER, programInfo.texCoordBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, texCoords, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(programInfo.a_texCoord);
        gl.vertexAttribPointer(programInfo.a_texCoord, 2, gl.FLOAT, false, 0, 0);

        gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    }


    initWGL() {
        this.releaseWGL();
        this.buffer_draw_canvas = document.createElement('canvas');

        if (this.buffer_draw_canvas) {
            this.buffer_draw_canvas_gl_ctx = this.buffer_draw_canvas.getContext("webgl2");
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
        program_info.a_color = gl.getAttribLocation(program, "a_color");
        program_info.a_index = gl.getAttribLocation(program, "a_index");

        program_info.index_buffer = gl.createBuffer();
        program_info.color_buffer = gl.createBuffer();

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

        if (!this.initShiftResources()) {
            console.warn('Failed to initialize shift resources');
        }

        this.isInit = true;
    }

    releaseWGL() {
        this.releaseShiftResources();

        const pi = this.buffer_draw_canvas_program_info;
        const gl_d = this.buffer_draw_canvas_gl_ctx;

        if (gl_d && pi && pi.gl_texture) {
            gl_d.deleteBuffer(pi.index_buffer);
            gl_d.deleteBuffer(pi.color_buffer);
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
                gl.useProgram(this.buffer_draw_canvas_program);
                gl.uniform2f(pi.u_resolution, w, h);
                gl.viewport(0, 0, w, h);
                this.width = w;
                this.height = h;

                if (this.shiftTexture) {
                    gl.bindTexture(gl.TEXTURE_2D, this.shiftTexture);
                    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
                    gl.bindTexture(gl.TEXTURE_2D, null);
                }

                if (this.shiftTexture2) {
                    gl.bindTexture(gl.TEXTURE_2D, this.shiftTexture2);
                    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
                    gl.bindTexture(gl.TEXTURE_2D, null);
                }
            }
            return true;
        }
        return false;
    }

    clearBuffers() {
        if (this.buffer_draw_canvas_gl_ctx) {
            this.buffer_draw_canvas_gl_ctx.clearColor(0, 0, 0, 1);
            this.buffer_draw_canvas_gl_ctx.clear(this.buffer_draw_canvas_gl_ctx.COLOR_BUFFER_BIT);
        }
    }

    setNewSizeWGL(w, h) {
        if (this.setSizeWGL(w, h)) {
            this.clearBuffers();
        }
    }

    hue2rgb(p, q, t){
        if(t < 0) t += 1;
        if(t > 1) t -= 1;
        if(t < 1/6) return p + (q - p) * 6 * t;
        if(t < 1/2) return q;
        if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
        return p;
    }

    addLine(plot, x, y) {
        if (this.isInit == false) {
            return;
        }

        const dgl = this.buffer_draw_canvas_gl_ctx;
        const dpi = this.buffer_draw_canvas_program_info;

        if (dpi && dgl) {
            let l = x.length
            if (dpi.indices == undefined || dpi.indices.length != l) {
                dpi.indices = new Float32Array(l);
            }

            if (dpi.colors == undefined || dpi.colors.length != l * 3) {
                dpi.colors = new Uint8Array(l * 3);
            }
            const axes = plot.getAxes();
            let min_y = undefined
            let max_y = undefined
            for (let i = 0; i < l; i++) {
                dpi.indices[i] = axes.xaxis.p2c(x[i]);
                if (x[i] >= axes.xaxis.min && x[i] <= axes.xaxis.max){
                    min_y = min_y == undefined  || min_y > y[i] ? y[i] : min_y
                    max_y = max_y == undefined  || max_y < y[i] ? y[i] : max_y
                }
            }
            let range = max_y - min_y;
            for (let i = 0; i < l; i++) {
                let r = 0
                let g = 0
                let b = 0
                if (x[i] >= axes.xaxis.min && x[i] <= axes.xaxis.max){
                    let percent = range > 0 ? (y[i] - min_y) / range : 0.5;

                    let h = 0.7 - percent * 0.7;
                    let s = 1.0;
                    let l = 0.5;

                    let q = l < 0.5 ? l * (1 + s) : l + s - l * s;
                    let p = 2 * l - q;

                    r = Math.round(this.hue2rgb(p, q, h + 1/3) * 255);
                    g = Math.round(this.hue2rgb(p, q, h)       * 255);
                    b = Math.round(this.hue2rgb(p, q, h - 1/3) * 255);
                }
                dpi.colors[i * 3] = r
                dpi.colors[i * 3 + 1] = g
                dpi.colors[i * 3 + 2] = b
            }

            this.shiftFrameBufferDown()

            dgl.useProgram(this.buffer_draw_canvas_program);

            dgl.bindBuffer(dgl.ARRAY_BUFFER, dpi.index_buffer);
            dgl.bufferData(dgl.ARRAY_BUFFER, dpi.indices, dgl.STATIC_DRAW);
            dgl.enableVertexAttribArray(dpi.a_index);
            dgl.vertexAttribPointer(dpi.a_index, 1, dgl.FLOAT, false, 0, 0);

            dgl.bindBuffer(dgl.ARRAY_BUFFER, dpi.color_buffer);
            dgl.bufferData(dgl.ARRAY_BUFFER, dpi.colors, dgl.STATIC_DRAW);
            dgl.enableVertexAttribArray(dpi.a_color);
            dgl.vertexAttribPointer(dpi.a_color, 3, dgl.UNSIGNED_BYTE, true, 0, 0);

            dgl.lineWidth(1.0);
            dgl.drawArrays(dgl.LINE_STRIP, 0, l);

            dgl.disableVertexAttribArray(dpi.a_index);
            dgl.disableVertexAttribArray(dpi.a_color);
        }
    }

    draw(ctx){
        if (this.isInit == false) {
            return;
        }

        const dc = this.buffer_draw_canvas;
        ctx.drawImage(dc, 0, 0);
    }
}