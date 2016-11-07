(function(window) {
    'use strict';
    var Modulator = function(params) {

        if (!params)
            params = new Object();

        // this odd construct is for safari compatibility
        if (!("audioContext" in window))
            window.audioContext = new(window.AudioContext || window.webkitAudioContext)();

        this.encoder = new FskEncoder(44100); //window.audioContext.sampleRate);

        // Create a "script node" that will actually generate audio samples.
        this.script_node = Modulator.prototype.script_node;

        if (!this.script_node) {
            Modulator.prototype.script_node = window.audioContext.createScriptProcessor(4096, 2, 2);

            // If the script node has an invalid buffer size, force one with a nonzero buffer.
            if (!Modulator.prototype.script_node.bufferSize) {

                // IE with a polyfill exhibits this problem, and crashes when you try to stop.
                Modulator.prototype.script_node = window.audioContext.createScriptProcessor(4096, 2, 2);
                this.prototype.can_stop = false;
            }
            this.script_node = Modulator.prototype.script_node;
        }

        // Start out in a not-playing state
        this.playing = false;
    }

    Modulator.prototype = {
        audioCtx: null, // AudioContext object
        encoder: null, // FskEncoder object
        outputAudioBuffer: null, // AudioBuffer object
        uiCallback: null, // UI object for callback
        scriptNode: null, // Re-used script node object for audio generation
        can_stop: true, // Whether we can stop (usually we can)

        // modulate a single packet. The data to modulate should be Uint8 format
        // This function allocates an audio buffer based on the length of the data and the sample rate
        // It then calls the fsk modulator, and shoves the returned floating point array
        // into the audio context for later playback
        modulate: function(data) {
            var bufLen = Math.ceil(data.length * 8 * this.encoder.samplesPerBit());
            this.outputAudioBuffer = window.audioContext.createBuffer(1, bufLen, 44100); //window.audioContext.sampleRate);

            var timeStart = performance.now();

            var outputFloatArray = this.outputAudioBuffer.getChannelData(0);
            this.encoder.modulate(data, outputFloatArray); // writes outputFloatArray in-place

            // How far into the outputAudioBuffer we are.
            this.script_node_offset = 0;

            var timeEnd = performance.now();
            var timeElapsed = timeEnd - timeStart;
            console.log("Rendered " + data.length + " data bytes in " +
                timeElapsed.toFixed(2) + "ms");
        },

        silence: function(msecs) {
            var bufLen = Math.ceil(window.audioContext.sampleRate / (1000.0 / msecs));
            this.outputAudioBuffer = window.audioContext.createBuffer(1, bufLen, window.audioContext.sampleRate);
            var outputFloatArray = this.outputAudioBuffer.getChannelData(0);
            for (var i = 0; i < outputFloatArray.length; i++)
                outputFloatArray[i] = 0;
            this.script_node_offset = 0;
        },

        // draw the waveform to the canvas, assuming the proper UI element is provided
        // for debug, of course
        drawWaveform: function(canvas) {
            // comment out now for performonce
            var b = this.outputAudioBuffer.getChannelData(0);
            this.drawWaveformToCanvas(b, 0, canvas);
        },

        processAudio: function(ev) {
            var outl = ev.outputBuffer.getChannelData(0);
            var outr = ev.outputBuffer.getChannelData(1);

            // If we're not playing, but still being called, just fill the channel with silence.
            if (!this.playing) {
                for (var i = 0; i < outl.length; i++)
                    outl[i] = outr[i] = 0;

                // Some browsers crash when you stop playing
                if (this.can_stop)
                    this.script_node.disconnect();
                return;
            }

            var outputFloatArray = this.outputAudioBuffer.getChannelData(0);

            for (var i = 0; i < outl.length; i++) {
                if (this.script_node_offset >= outputFloatArray.length) {
                    // If there's more data to play, reset the output float array.
                    if (this.get_more_data())
                        outputFloatArray = this.outputAudioBuffer.getChannelData(0);

                    // Otherwise, fill the buffer with 0s, and we'll stop playing on the next iteration.
                    else {
                        for (var j = 0; j < outputFloatArray.length; j++)
                            outputFloatArray[j] = 0;
                    }
                    this.script_node_offset = 0;
                }

                outl[i] = outr[i] = outputFloatArray[this.script_node_offset++];
            }
        },

        // immediately play the modulated audio exactly once. Useful for debugging single packets
        playBuffer: function(obj, func) {
            console.log("-- playAudioBuffer --");
            var bufferNode = window.audioContext.createBufferSource();
            bufferNode.buffer = this.outputAudioBuffer;
            bufferNode.connect(window.audioContext.destination); // Connect to speakers
            bufferNode.addEventListener("ended", function() {
                var playTimeEnd = performance.now();
                var timeElapsed = playTimeEnd - this.playTimeStart;
                console.log("got audio ended event after " + timeElapsed.toFixed(2) + "ms");
                if (obj && func)
                    func.call(obj);
            }.bind(this));
            this.playTimeStart = performance.now();
            bufferNode.start(0); // play immediately
        },

        // Plays through an entire file. You need to set the callback so once
        // a single audio packet is finished, the next can start. The index
        // tells where to start playing. You could, in theory, start modulating
        // part-way through an audio stream by setting index to a higher number on your
        // first call.
        playLoop: function(obj, end_func, param) {

            /*
                        this.get_more_data = function() {
                            if (!end_func.call(obj, param)) {
                                this.playing = false;
                                return false;
                            }
                            return true;
                        };

                        this.script_node.onaudioprocess = function(ev) {
                            Modulator.prototype.processAudio.call(this, ev);
                        }.bind(this);

                        if (!this.playing) {
                            this.playing = true;
                            this.script_node.connect(window.audioContext.destination);
                        }
                        */
            var a = document.getElementById('a');


            var inBuffer = this.outputAudioBuffer.getChannelData(0);
            var outBuffer = new Array(inBuffer.length * 2);
            for (var i = 0; i < inBuffer.length; i++) {
                // Map -1 .. 1 to -32767 .. 32768
                var sample = Math.round((inBuffer[i]) * 32767);
                outBuffer[(i * 2) + 0] = Math.round(sample & 0xff);
                outBuffer[(i * 2) + 1] = Math.round((sample >> 8) & 0xff);
            }

            var pcmObj = new pcm({
                channels: 1,
                rate: 44100,
                depth: 16
            }).toWav(outBuffer);
            a.src = pcmObj.encode();
            //pcmObj.play();
            a.play();

            window.setTimeout(function() {
                if (!end_func.call(obj, param)) {
                    this.playing = false;
                    return false;
                }
                return true;
            }, (inBuffer.length * 1000) / 44100);

        },


        drawWaveformToCanvas: function(buffer, start, canvas) {
            if (!canvas || !canvas.getContext)
                return;

            var strip = canvas.getContext('2d');

            // Resize the canvas to be the window size.
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;

            var h = strip.canvas.height;
            var w = strip.canvas.width;
            strip.clearRect(0, 0, w, h);

            var y;
            // Draw scale lines at 10% interval
            strip.lineWidth = 1.0;
            strip.strokeStyle = "#55a";
            strip.beginPath();
            y = 1 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 2 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 3 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 4 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 5 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 6 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 7 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 8 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            y = 9 * (h / 10);
            strip.moveTo(0, y);
            strip.lineTo(w, y);
            strip.stroke();

            strip.strokeStyle = "#fff";
            strip.lineWidth = 1.0;

            var b = start;
            var lastSample = (buffer[b++] + 1) / 2; // map -1..1 to 0..1

            for (var x = 1; x < canvas.width; x++) {
                var sample = (buffer[b++] + 1) / 2;
                if (b > buffer.length)
                    break;
                strip.beginPath();
                strip.moveTo(x - 1, h - lastSample * h);
                strip.lineTo(x, h - sample * h);
                strip.stroke();
                lastSample = sample;
            }
        }
    };

    /* Set up the constructor, so we can do "new Modulator()" */
    window.Modulator = function(params) {
        return new Modulator(params);
    };
}(this));