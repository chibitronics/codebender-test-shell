(function (window) {
    'use strict';

    var ModulationController = function(params) {

        if (!params)
            params = new Object();

        if ('AudioContext' in window) {
            this.audioContext = new AudioContext();
        }
        else if ('webkitAudioContext' in window) {
            this.audioContext = new webkitAudioContext();
        }
        else
            alert('Your browser does not support yet Web Audio API');

        /*
        this.freqHigh = params.freqHigh || 7350;
        this.freqLow = params.freqLow || 4900;
        this.baud = params.baud || 1225;
        this.sampleRate = params.sampleRate || 29400;
        */
        this.canvas = params.canvas || undefined;

        /* Are these needed? */
        this.saveState = false;

        this.isSending = false;
        this.playing = false;
        this.done = false;
        this.stoppedAt = 0;
        this.modData = null;
        this.byteArray = null;
        this.playCount = 0;

        this.modData = new modulator(this); // the modulator object contains our window's audio context

        /* Preamble sent before every audio packet */
        this.preamble = [0x00, 0x00, 0x00, 0x00, 0xaa, 0x55, 0x42];
    }

    ModulationController.prototype = {
        sendTestData: function() {
            // assemble a single test data packet
            var preamble = this.preamble;
            var sector = [0x01, 0x02, 0x80, 0x04]; // version code + two bytes for sector offset
            // note to self: version codes are now checked by Rx so let's not mess with that anymore
            // 256 byte payload, preamble, sector offset + 4 bytes hash + 1 byte stop
            var packetlen = 256 + preamble.length + sector.length + 4 + 1;
            var buffer = new Uint8Array(packetlen);
            var i = 0;
            for( i = 0; i < preamble.length; i++ ) {
                buffer[i] = preamble[i];
            }
            for( var j = 0; i < sector.length + preamble.length; i++, j++ ) {
                buffer[i] = sector[j];
            }
            for( ; i < packetlen - 1 - 4; i++ ) {
                //buffer[i] = i & 0xff;
                buffer[i] = 0x00;  // this is a corner case it turns out due to baud sync issues
                //buffer[i] = Math.floor((Math.random() * 256));
            }

            var hash = murmurhash3_32_gc(buffer.subarray(preamble.length, 256 + preamble.length + sector.length), 0xdeadbeef);
            console.log("buffer hash: " + hash);
            buffer[i++] = hash & 0xFF;
            buffer[i++] = (hash >> 8) & 0xFF;
            buffer[i++] = (hash >> 16) & 0xFF;
            buffer[i++] = (hash >> 24) & 0xFF
            buffer[i] = 0xFF;  // terminate with 0xFF to let last bit demodulate correctly

            // now stripe the buffer to ensure transitions for baud sync
            // but don't stripe the premable or the hash
            // we do this instead of eg 8b10b, manchester, or NRZ because I'm trying to
            // minimize overhead. In practice we can go several hundred bits without a
            // transition and keep sync; so high-overhead schemes aren't necessary
            // there are theoretical pathological patterns that can defeat the transition scheme
            // but given that we'll be uploading primarily ARM code our biggest enemy are
            // long runs of 0's and 1's
            var ctr = 2;
            for (i = preamble.length + 2; i < (buffer.length - 5); i++, ctr++) {
                if ((ctr % 16) == 7)
                    buffer[i] ^= 0x55;
                else if ((ctr % 16) == 15)
                    buffer[i] ^= 0xaa;
            }
            this.modData.modulate(buffer);
            this.modData.playBuffer(this);
            this.modData.drawWaveform(this.canvas);

            if (this.saveState)
                this.modData.saveWAV();

            this.isSending = true;
        },

        sendTestCtrl: function() {
            // assemble a single control packet for testing
            var preamble = this.preamble;
            var sector = [0x01, 0x01, 0x00, 0x00];   // version 1, packet type 1(ctrl), two bytes of padding
            var packetlen = 24 + preamble.length + sector.length + 4 + 1;
            var buffer = new Uint8Array(packetlen);
            var i = 0;
            for( i = 0; i < preamble.length; i++ ) {
                buffer[i] = preamble[i];
            }
            for( var j = 0; i < sector.length + preamble.length; i++, j++ ) {
                buffer[i] = sector[j];
            }
            for( ; i < packetlen - 1 - 4; i++ ) {
                //buffer[i] = i & 0xff;
                //buffer[i] = 0x00;
                buffer[i] = Math.floor((Math.random() * 256));
            }

            var hash = murmurhash3_32_gc(buffer.subarray(preamble.length, 24 + preamble.length + sector.length), 0xdeadbeef);
            console.log("buffer hash: " + hash);
            buffer[i++] = hash & 0xFF;
            buffer[i++] = (hash >> 8) & 0xFF;
            buffer[i++] = (hash >> 16) & 0xFF;
            buffer[i++] = (hash >> 24) & 0xFF
            buffer[i] = 0xFF;  // terminate with 0xFF to let last bit demodulate correctly

            // control packets don't need striping because (a) they are short and
            // (b) most of it is a hash which is pretty much guaranteed to have plenty of bit transitions

            this.modData.modulate(buffer);
            this.modData.playBuffer(this);
            this.modData.drawWaveform(this.canvas);

            if (this.saveState)
                this.modData.saveWAV();

            this.isSending = true;
        },

        sendData: function(data) {
            if (data) {
                var dataLength = data.length;
                var array = new Uint8Array(new ArrayBuffer(dataLength));
                for (i = 0; i < dataLength; i++)
                    array[i] = data.charCodeAt(i);

                this.byteArray = array;
                this.playCount = 0;
                this.transcodeFile(0);
                this.isSending = true;
            }
        },

        // this is the core function for transcoding
        // two object variables must be set:
        // byteArray, and playCount.
        // byteArray is the binary file to transmit
        // playCount keeps track of how many times the entire file has been replayed

        // the parameter to this, "index", is a packet counter. We have to recursively call
        // transcodeFile using callbacks triggered by the completion of audio playback. I couldn't
        // think of any other way to do it.
        transcodeFile: function(index) {
            var fileLen = this.byteArray.length;
            var blocks = Math.ceil(fileLen / 256);

            // index 0 & 1 create identical control packets. We transmit the control packet
            // twice in the beginning because (a) it's tiny and almost free and (b) if we
            // happen to miss it, we waste an entire playback cycle before we start committing
            // data to memory
            if (index == 0  || index == 1) {
                var ctlPacket = this.makeCtlPacket(this.byteArray.subarray(0, fileLen));

                this.modData.modulate(ctlPacket);
                this.modData.playLoop(this, index + 1);
                this.modData.drawWaveform(this.canvas);
            }
            else {
                // data index starts at 2, due to two sends of the control packet up front
                var i = index - 2;
                // handle whole blocks
                if (i < blocks - 1) {
                    var dataPacket = this.makeDataPacket(this.byteArray.subarray(i * 256, i * 256 + 256), i);
                    this.modData.modulate(dataPacket);
                    this.modData.playLoop(this, index + 1);
                    this.modData.drawWaveform(this.canvas);
                }
                else {
                    // handle last block of data, which may not be 256 bytes long
                    var dataPacket = this.makeDataPacket(this.byteArray.subarray(i * 256, fileLen), i);
                    this.modData.modulate(dataPacket);
                    this.modData.playLoop(this, index + 1);
                    this.modData.drawWaveform(this.canvas);
                }
            }
        },

        makeCtlPacket: function(data) {
            // parameters from microcontroller spec. Probably a better way
            // to do this in javascript, but I don't know how (seems like "const" could be used, but not universal)
            var bytePreamble = this.preamble;
            var byteVersion = [0x01];
            var byteType = [0x01];
            var bytePadding = [0x00, 0x00];
            var pktLength = data.length;
            var byteLength = [pktLength & 0xFF, (pktLength >> 8) & 0xFF,
                              (pktLength >> 16) & 0xFF, (pktLength >> 24) & 0xFF];
            var pktFullhash = murmurhash3_32_gc(data, 0x32d0babe);  // 0x32d0babe by convention
            var guidStr = SparkMD5.hash(String.fromCharCode.apply(null,data), false);
            var pktGuid = [];
            var i;
            for (i = 0; i < guidStr.length-1; i += 2)
                pktGuid.push(parseInt(guidStr.substr(i,2),16));

            var packetlen = bytePreamble.length + bytePadding.length + byteVersion.length + byteType.length + byteLength.length + 4 + pktGuid.length + 4 + 1;
            var pkt = new Uint8Array(packetlen);
            var pktIndex = 0;
            for (i = 0; i < bytePreamble.length; i++)
                pkt[pktIndex++] = bytePreamble[i];

            pkt[pktIndex++] = byteVersion[0];
            pkt[pktIndex++] = byteType[0];
            pkt[pktIndex++] = bytePadding[0];
            pkt[pktIndex++] = bytePadding[1];
            for (i = 0; i < byteLength.length; i++)
                pkt[pktIndex++] = byteLength[i];

            pkt[pktIndex++] = pktFullhash & 0xFF;
            pkt[pktIndex++] = (pktFullhash >> 8) & 0xFF;
            pkt[pktIndex++] = (pktFullhash >> 16) & 0xFF;
            pkt[pktIndex++] = (pktFullhash >> 24) & 0xFF;
            for (i = 0; i < 16; i++)
                pkt[pktIndex++] = pktGuid[i];

            var hash = murmurhash3_32_gc(pkt.subarray(bytePreamble.length, 24 + bytePreamble.length + byteVersion.length + byteType.length + bytePadding.length), 0xdeadbeef); // deadbeef is just by convention
            pkt[pktIndex++] = hash & 0xFF;
            pkt[pktIndex++] = (hash >> 8) & 0xFF;
            pkt[pktIndex++] = (hash >> 16) & 0xFF;
            pkt[pktIndex++] = (hash >> 24) & 0xFF
            pkt[pktIndex] = 0xFF;  // terminate with 0xFF to let last bit demodulate correctly

            return pkt;
        },

        makeDataPacket: function(dataIn, blocknum) {
            var data;
            var i;
            if (dataIn.length != 256) {
                // if our data array isn't a whole packet in length, pad it out with FF's
                data = new Uint8Array(256);
                for( i = 0; i < dataIn.length; i ++ ) {
                    data[i] = dataIn[i];
                }
                for( ; i < 256; i++ ) {
                    data[i] = 0xFF; // 1's pad out the final data packet
                }
            }
            else {
                data = dataIn;
            }

            // now assemble the packet
            var preamble = this.preamble;
            var sector = [0x01, 0x02, blocknum & 0xFF, (blocknum >> 8) & 0xFF];   // version 1
            // 256 byte payload, preamble, sector offset + 4 bytes hash + 1 byte stop
            var packetlen = 256 + preamble.length + sector.length + 4 + 1;

            var buffer = new Uint8Array(packetlen);
            for (i = 0; i < preamble.length; i++)
                buffer[i] = preamble[i];
            for (var j = 0; i < sector.length + preamble.length; i++, j++)
                buffer[i] = sector[j];
            for (j = 0; i < packetlen - 1 - 4; i++, j++)
                buffer[i] = data[j];

            var hash = murmurhash3_32_gc(buffer.subarray(preamble.length, 256 + preamble.length + sector.length), 0xdeadbeef);
            buffer[i++] = hash & 0xFF;
            buffer[i++] = (hash >> 8) & 0xFF;
            buffer[i++] = (hash >> 16) & 0xFF;
            buffer[i++] = (hash >> 24) & 0xFF
            buffer[i] = 0xFF;  // terminate with 0xFF to let last bit demodulate correctly

            // now stripe the buffer to ensure transitions for baud sync
            // don't stripe the premable or the hash
            for (i = 9; i < (buffer.length - 5); i++) {
                if ((i % 16) == 14)
                    buffer[i] ^= 0x55;
                else if ((i % 16) == 6)
                    buffer[i] ^= 0xaa;
            }

            return buffer;
        },

        // once all audio is done playing, call this to reset UI elements to idle state
        audioEndCB: function() {
            this.isSending = false;
        },

        stop: function() {
            ;
        },

        isRunning: function() {
            return this.isSending;
        }
    }

    /* Set up the constructor, so we can do "new ModulationController()" */
    window.ModulationController = function(params) {
        return new ModulationController(params);
    };
}(this));

