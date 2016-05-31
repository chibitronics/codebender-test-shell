(function (window) {
    'use strict';
    var Modulator = function(controller) {

        // this odd construct is for safari compatibility
        if (!("audioContext" in window))
            window.audioContext = new (window.AudioContext || window.webkitAudioContext)();

        this.samplerate = window.audioContext.sampleRate;

        console.log("speakerSampleRate is " + this.samplerate);

        this.encoder = new FskEncoder(this.samplerate);
        this.ui = controller;
    }

    Modulator.prototype = {
        audioCtx: null,  // AudioContext object
        samplerate: 48000, // updated by the initializer
        encoder: null,  // FskEncoder object
        outputAudioBuffer: null,  // AudioBuffer object
        uiCallback: null,  // UI object for callback
        loopCallback: null, // loop callback
        loopIndex: null, // loop index on callback

        // modulate a single packet. The data to modulate should be Uint8 format
        // This function allocates an audio buffer based on the length of the data and the sample rate
        // It then calls the fsk modulator, and shoves the returned floating point array
        // into the audio context for later playback
        modulate: function(data) {
            var bufLen = Math.ceil(data.length * 8 * this.encoder.samplesPerBit());
            this.outputAudioBuffer = window.audioContext.createBuffer(1, bufLen, this.samplerate);

            var timeStart = performance.now();

            var outputFloatArray = this.outputAudioBuffer.getChannelData(0);
            this.encoder.modulate(data, outputFloatArray); // writes outputFloatArray in-place

            var timeEnd = performance.now();
            var timeElapsed = timeEnd - timeStart;
            console.log("Rendered " + data.length + " data bytes in " +
                        timeElapsed.toFixed(2) + "ms");
        },

        // draw the waveform to the canvas, assuming the proper UI element is provided
        // for debug, of course
        drawWaveform: function(canvas) {
            // comment out now for performonce
            var b = this.outputAudioBuffer.getChannelData(0);
            this.drawWaveformToCanvas(b, 0, canvas);
        },

        // immediately play the modulated audio exactly once. Useful for debugging single packets
        playBuffer: function(callBack) {
            if (callBack)
                this.uiCallback = callBack;
            console.log("-- playAudioBuffer --");
            var bufferNode = window.audioContext.createBufferSource();
            bufferNode.buffer = this.outputAudioBuffer;
            bufferNode.connect(window.audioContext.destination); // Connect to speakers
            bufferNode.addEventListener("ended", function() {
                var playTimeEnd = performance.now();
                var timeElapsed = playTimeEnd - this.playTimeStart;
                console.log("got audio ended event after " + timeElapsed.toFixed(2) + "ms");
                if (this.uiCallback)
                    this.uiCallback.audioEndCB();
            }.bind(this));
            this.playTimeStart = performance.now();
            bufferNode.start(0); // play immediately
        },

        // Plays through an entire file. You need to set the callback so once
        // a single audio packet is finished, the next can start. The index
        // tells where to start playing. You could, in theory, start modulating
        // part-way through an audio stream by setting index to a higher number on your
        // first call.
        playLoop: function(callBack, index) {
            var loopCallback;
            var loopIndex;

            if (callBack) {
                loopCallback = callBack;
                loopIndex = index;
            }
            var bufferNode = window.audioContext.createBufferSource();
            bufferNode.buffer = this.outputAudioBuffer;
            bufferNode.connect(window.audioContext.destination); // Connect to speakers

            // our callback to trigger the next packet
            bufferNode.onended = function audioLoopEnded() {
                if (loopCallback) {
                    if (((loopIndex - 2) * 256) < this.ui.byteArray.length) {
                        // if we've got more data, transcode and loop
                        loopCallback.transcodeFile(loopIndex);
                    }
                    else {
                        // if we've reached the end of our data, check to see how
                        // many times we've played the entire file back. We want to play
                        // it back a couple of times because sometimes packets get
                        // lost or corrupted.
                        if (this.ui.playCount < 2) { // set this higher for more loops!
                            this.ui.playCount++;
                            loopCallback.transcodeFile(0); // start it over!
                        }
                        else {
                            loopCallback.audioEndCB(); // clean up the UI when done
                        }
                    }
                }
            }.bind(this);

            if (index == 1)
                bufferNode.start(0); // this one goes immediately
            else if (index == 2)
                bufferNode.start(window.audioContext.currentTime + 0.1); // redundant send of control packet
            else if (index == 3)
                bufferNode.start(window.audioContext.currentTime + 0.5); // 0.5s for bulk flash erase to complete
            else
                bufferNode.start(window.audioContext.currentTime + 0.08); // slight pause between packets to allow burning
        },

        drawWaveformToCanvas: function(buffer, start, canvas) {
            if (!canvas || !canvas.getContext)
                return;

            var strip = canvas.getContext('2d');

            // Resize the canvas to be the window size.
            canvas.width  = window.innerWidth;
            canvas.height = window.innerHeight;

            var h = strip.canvas.height;
            var w = strip.canvas.width;
            strip.clearRect(0, 0, w, h);

            var y;
            // Draw scale lines at 10% interval
            strip.lineWidth = 1.0;
            strip.strokeStyle = "#55a";
            strip.beginPath();
            y = 1 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 2 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 3 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 4 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 5 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 6 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 7 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 8 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
            y = 9 * (h/10); strip.moveTo(0, y); strip.lineTo(w, y);
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

    /* Set up the constructor, so we can do "new ModulationController()" */
    window.Modulator = function(params) {
        return new Modulator(params);
    };
}(this));
