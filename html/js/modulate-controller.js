(function (window) {
    'use strict';

    var ModulationController = function(params) {

        if (!params)
            params = new Object();

        this.canvas = params.canvas || undefined;
        this.endCallback = params.endCallback || undefined;

        /* Are these needed? */
        this.saveState = false;

        this.isSending = false;
        this.playing = false;
        this.done = false;
        this.stoppedAt = 0;
        this.modData = null;
        this.byteArray = null;
        this.playCount = 0;

        this.PROT_VERSION = 0x01;   // Protocol v1.0

        this.CONTROL_PACKET = 0x01;
        this.DATA_PACKET = 0x02;

        this.modData = new modulator(this); // the modulator object contains our window's audio context

        /* Preamble sent before every audio packet */
        this.preamble = [0x00, 0x00, 0x00, 0x00, 0xaa, 0x55, 0x42];
    }

    ModulationController.prototype = {

        makeControlHeader: function() {
            return [this.PROT_VERSION, this.CONTROL_PACKET, 0x00, 0x00];
        },

        makeDataHeader: function(blockNum) {
            return [this.PROT_VERSION, this.DATA_PACKET, blockNum & 0xff, (blockNum >> 8) & 0xff];
        },

        sendTestData: function() {
            // assemble a single test data packet
            var preamble = this.preamble;
            var sector = this.makeDataHeader(0x03); // version code + two bytes for sector offset
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
            var sector = this.makeControlHeader();
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

        makeUint32: function(num) {
            return [num & 0xff,
                   (num >> 8) & 0xff,
                   (num >> 16) & 0xff,
                   (num >> 24) & 0xff];
        },

        makeUint16: function(num) {
            return [num & 0xff,
                   (num >> 8) & 0xff];
        },

        makePacket: function() {
            var len = 0;
            var i;
            for (i = 0; i < arguments.length; i++)
                len += arguments[i].length;

            var pkt = new Uint8Array(len);
            var offset = 0;

            for (i = 0; i < arguments.length; i++)
                offset += this.appendData(pkt, arguments[i], offset);

            return pkt;
        },

        makeCtlPacket: function(data) {
            // parameters from microcontroller spec. Probably a better way
            // to do this in javascript, but I don't know how (seems like "const" could be used, but not universal)
            var preamble = this.preamble;
            var header = this.makeControlHeader();
            var program_length = this.makeUint32(data.length);
            var program_hash = this.makeHash(data, 0x32d0babe);  // 0x32d0babe by convention
            var program_guid_str = SparkMD5.hash(String.fromCharCode.apply(null,data), false);
            var program_guid = [];
            var i;
            for (i = 0; i < program_guid_str.length-1; i += 2)
                program_guid.push(parseInt(program_guid_str.substr(i,2),16));

            var footer = this.makeFooter(header, program_length, program_hash, program_guid);
            var stop = [0xff];

            return this.makePacket(preamble, header, program_length, program_hash, program_guid, footer, stop);
        },

        /* Appends "src" to "dst", beginning at offset "offset".
         * Handy for populating data buffers.
         */
        appendData: function(dst, src, offset) {
            var i;
            for (i = 0; i < src.length; i++)
                dst[offset + i] = src[i];
            return i;
        },

        makeHash: function(data, hash) {
            return this.makeUint32(murmurhash3_32_gc(data, hash));
        },

        makeFooter: function(packet) {
            var hash = 0xdeadbeef;
            var data = new Array();
            var i;
            var j;

            // Join all argument arrays together into "data"
            for (i = 0; i < arguments.length; i++)
                for (j = 0; j < arguments[i].length; j++)
                    data.push(arguments[i][j]);

            return this.makeHash(data, hash);
        },

        makeDataPacket: function(dataIn, blocknum) {
            var data;
            var i;

            // now assemble the packet
            var preamble = this.preamble;
            var header = this.makeDataHeader(blocknum);

            // Ensure the "data" payload is 256 bytes long.
            var data = new Uint8Array(256);
            data.fill(0xff);    // Pad extra data with 0xff
            this.appendData(data, dataIn, 0);

            var footer = this.makeFooter(header, data);
            var stop = [0xff];

            // 256 byte payload, preamble, sector offset + 4 bytes hash + 1 byte stop
            var packetlen = preamble.length + header.length + data.length + footer.length + stop.length;

            // now stripe the buffer to ensure transitions for baud sync
            // don't stripe the premable or the hash
            for (i = 0; i < data.length; i++) {
                if ((i % 16) == 3)
                    data[i] ^= 0x55;
                else if ((i % 16) == 11)
                    data[i] ^= 0xaa;
            }

            return this.makePacket(preamble, header, data, footer, stop);
        },

        // once all audio is done playing, call this to reset UI elements to idle state
        audioEndCB: function() {
            this.isSending = false;
            if (this.endCallback)
                this.endCallback();
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

