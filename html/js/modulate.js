// one modulator per web page
// the modulator object contains the audio context
// we get on per window. The context can dole out buffers
// for mulitple audio streams.

function modulator(ui) {
    // this odd construct is for safari compatibility
    this.audioCtx = new (window.AudioContext || window.webkitAudioContext)();

    this.samplerate = this.audioCtx.sampleRate;

    console.log("speakerSampleRate is " + this.samplerate);

    this.encoder = new FskEncoder(this.samplerate);
    this.ui = ui;

}
modulator.prototype = {
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
        this.outputAudioBuffer = this.audioCtx.createBuffer(1, bufLen, this.samplerate);

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
        var b = this.outputAudioBuffer.getChannelData(0);
        drawWaveformToCanvas(b, 0, canvas);
    },
    // immediately play the modulated audio exactly once. Useful for debugging single packets
    playBuffer: function(callBack) {
        if( callBack )
            uiCallback = callBack;
        console.log("-- playAudioBuffer --");
        var bufferNode = this.audioCtx.createBufferSource();
        bufferNode.buffer = this.outputAudioBuffer;
        bufferNode.connect(this.audioCtx.destination); // Connect to speakers
        bufferNode.addEventListener("ended", audioEnded);
        playTimeStart = performance.now();
        bufferNode.start(0); // play immediately
    },
    // Plays through an entire file. You need to set the callback so once
    // a single audio packet is finished, the next can start. The index
    // tells where to start playing. You could, in theory, start modulating
    // part-way through an audio stream by setting index to a higher number on your
    // first call.
    playLoop: function(callBack, index) {
        if (callBack) {
            loopCallback = callBack;
            loopIndex = index;
        }
        var bufferNode = this.audioCtx.createBufferSource();
        bufferNode.buffer = this.outputAudioBuffer;
        bufferNode.connect(this.audioCtx.destination); // Connect to speakers

        // our callback to trigger the next packet
        var thismod = this;
        bufferNode.onended = function audioLoopEnded() {
            if (loopCallback) {
                if (((loopIndex - 2) * 256) < thismod.ui.byteArray.length) {
                    // if we've got more data, transcode and loop
                    loopCallback.transcodeFile(loopIndex);
                }
                else {
                    // if we've reached the end of our data, check to see how
                    // many times we've played the entire file back. We want to play
                    // it back a couple of times because sometimes packets get
                    // lost or corrupted.
                    if (thismod.ui.playCount < 2) { // set this higher for more loops!
                        thismod.ui.playCount++;
                        loopCallback.transcodeFile(0); // start it over!
                    }
                    else {
                        loopCallback.audioEndCB(); // clean up the UI when done
                    }
                }
            }
        };

        if (index == 1)
            bufferNode.start(0); // this one goes immediately
        else if (index == 2)
            bufferNode.start(this.audioCtx.currentTime + 0.1); // redundant send of control packet
        else if (index == 3)
            bufferNode.start(this.audioCtx.currentTime + 0.5); // 0.5s for bulk flash erase to complete
        else
            bufferNode.start(this.audioCtx.currentTime + 0.08); // slight pause between packets to allow burning
    },

    saveWAV: function() {
        exportMonoWAV(this.outputAudioBuffer.getChannelData(0), this.outputAudioBuffer.length);
    },
}

var playTimeStart;

function audioEnded() {
    var playTimeEnd = performance.now();
    var timeElapsed = playTimeEnd - playTimeStart;
    console.log("got audio ended event after " + timeElapsed.toFixed(2) + "ms");
    if (uiCallback)
        uiCallback.audioEndCB();
}

// some code to export wave files for debug. Can lose this in production
function exportMonoWAV(buffer, length){
    var saveData = (function () {
        var a = document.createElement("a");
        document.body.appendChild(a);
        a.style = "display: none";
        return function (data, fileName) {
            url = window.URL.createObjectURL(data);
            a.href = url;
            a.download = fileName;
            a.click();
            window.URL.revokeObjectURL(url);
        };
    }());

    var type = 'audio/wav';
    var dataview = encodeWAV(buffer, true);
    var audioBlob = new Blob([dataview], { type: type });

    saveData(audioBlob, 'modulated.wav');
}

function mergeBuffers(recBuffers, recLength){
    var result = new Float32Array(recLength);
    var offset = 0;
    for (var i = 0; i < recBuffers.length; i++){
        console.log(recBuffers[i]);
        result.set(recBuffers[i], offset);
        offset += recBuffers[i].length;
    }
    return result;
}

function floatTo16BitPCM(output, offset, input){
    for (var i = 0; i < input.length; i++, offset+=2){
        var s = Math.max(-1, Math.min(1, input[i]));
        output.setInt16(offset, s < 0 ? s * 0x8000 : s * 0x7FFF, true);
    }
}

function writeString(view, offset, string){
    for (var i = 0; i < string.length; i++){
        view.setUint8(offset + i, string.charCodeAt(i));
    }
}

function encodeWAV(samples, mono){
    var buffer = new ArrayBuffer(44 + samples.length * 2);
    var view = new DataView(buffer);

    sampleRate = 48000;

    /* RIFF identifier */
    writeString(view, 0, 'RIFF');
    /* file length */
    view.setUint32(4, 32 + samples.length * 2, true);
    /* RIFF type */
    writeString(view, 8, 'WAVE');
    /* format chunk identifier */
    writeString(view, 12, 'fmt ');
    /* format chunk length */
    view.setUint32(16, 16, true);
    /* sample format (raw) */
    view.setUint16(20, 1, true);
    /* channel count */
    view.setUint16(22, mono?1:2, true);
    /* sample rate */
    view.setUint32(24, sampleRate, true);
    /* byte rate (sample rate * block align) */
    view.setUint32(28, sampleRate * 4, true);
    /* block align (channel count * bytes per sample) */
    view.setUint16(32, 4, true);
    /* bits per sample */
    view.setUint16(34, 16, true);
    /* data chunk identifier */
    writeString(view, 36, 'data');
    /* data chunk length */
    view.setUint32(40, samples.length * 2, true);

    floatTo16BitPCM(view, 44, samples);

    return view;
}


/* ============================================================ */

function drawWaveformToCanvas(buffer, start, canvas) {
    console.log("-- drawWaveformToCanvas --");

    if (!canvas)
        return;

    var strip = canvas.getContext('2d');

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
        if (b > buffer.length) break;
        strip.beginPath();
        strip.moveTo(x - 1, h - lastSample * h);
        strip.lineTo(x, h - sample * h);
        strip.stroke();
        lastSample = sample;
    }
}
