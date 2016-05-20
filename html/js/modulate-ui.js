var ui = {
    mode: null,
    saveState: false,
    sendState: false,
    modData: null,
    byteArray: null,
    fileSelect: 1,
    fileName: "microtest1.bin",
    playCount: 0,

    init: function() {
        this.sendButton = document.getElementById("send");  // plays a single packet of data for MAC testing
        this.ctrlButton = document.getElementById("ctrl");  // plays a single packet of control data for MAC testing
        this.fileButton = document.getElementById("file");  // transcodes and plays the selected file, in a loop 3 times

        this.fileShortButton = document.getElementById("short");  // which file we're going to send
        this.fileTest1Button = document.getElementById("test1");
        this.fileTest2Button = document.getElementById("test2");

        this.doFileSelect();  // implements a shitty radio button

        this.saveButton     = document.getElementById("save");  // when selected, the generated audio is also spit out to .wav file for analysis/debug

        // add handler hooks
        var self = this;
        this.sendButton.addEventListener("click",
                                         function(e) { self.onModeButton("send"); e.preventDefault(); });
        this.ctrlButton.addEventListener("click",
                                         function(e) { self.onModeButton("ctrl"); e.preventDefault(); });
        this.fileButton.addEventListener("click",
                                         function(e) { self.onModeButton("file"); e.preventDefault(); });

        this.fileShortButton.addEventListener("click",
                                              function(e) { self.onModeButton("short"); e.preventDefault(); });
        this.fileTest1Button.addEventListener("click",
                                              function(e) { self.onModeButton("test1"); e.preventDefault(); });
        this.fileTest2Button.addEventListener("click",
                                              function(e) { self.onModeButton("test2"); e.preventDefault(); });


        this.saveButton.addEventListener("click",
                                         function(e) { self.onSaveButton(); e.preventDefault(); });

        this.txLed    = document.getElementById("txLed");

        this.modData = new modulator(); // the modulator object contains our window's audio context
    },

    onModeButton: function(mode) {
        this.sendState = !this.sendState;
        this.mode = mode;

        // some code to light up the buttons when pressed
        if( mode == "send") {
            this.sendButton.setAttribute("selected", "");
        } else if( mode == "ctrl") {
            this.ctrlButton.setAttribute("selected", "");
        } else if( mode == "file") {
            this.fileButton.setAttribute("selected", "");
        }
        if( mode != "test1" && mode != "short" && mode != "test2" )
            this.txLed.setAttribute("lit", "");

        // code that assembles packets, and then hands them off to the modulator to transcode & play
        if(mode == "send") {
            // assemble a single test data packet
            var preamble = [00,00,00,00,0xaa,0x55,0x42];
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

            hash = murmurhash3_32_gc(buffer.subarray(preamble.length, 256 + preamble.length + sector.length), 0xdeadbeef);
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
        } else if( mode == "ctrl" ) {
            // assemble a single control packet for testing
            var preamble = [00,00,00,00,0xaa,0x55,0x42];
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

            hash = murmurhash3_32_gc(buffer.subarray(preamble.length, 24 + preamble.length + sector.length), 0xdeadbeef);
            console.log("buffer hash: " + hash);
            buffer[i++] = hash & 0xFF;
            buffer[i++] = (hash >> 8) & 0xFF;
            buffer[i++] = (hash >> 16) & 0xFF;
            buffer[i++] = (hash >> 24) & 0xFF
            buffer[i] = 0xFF;  // terminate with 0xFF to let last bit demodulate correctly

            // control packets don't need striping because (a) they are short and
            // (b) most of it is a hash which is pretty much guaranteed to have plenty of bit transitions
        } else if( mode == "file" ) {
            // fetch a file and transcode it
            var fileReq = new XMLHttpRequest();
            fileReq.open("GET", this.fileName, true);
            fileReq.responseType = "arraybuffer";

            fileReq.onload = function(oEvent) {
                var arrayBuffer = fileReq.response;
                if(arrayBuffer) {
                    self.ui.byteArray = new Uint8Array(arrayBuffer);

                    self.ui.playCount = 0;
                    self.ui.transcodeFile(0);
                }
            }
            fileReq.send(); // this request is asynchronous

        } // done with packet code, now we are back to UI code
        else if( mode == "short" ) { // there has to be a better way to do radio buttons, but meh
            this.fileSelect = 0;
            this.doFileSelect();
        } else if( mode == "test1" ) {
            this.fileSelect = 1;
            this.doFileSelect();
        } else if( mode == "test2" ) {
            this.fileSelect = 2;
            this.doFileSelect();
        } else {
            throw "unknown onModeButton: " + mode;
        }

        if( mode == "send" || mode == "ctrl" ) {
            this.modData.modulate(buffer);
            this.modData.playBuffer(this);
            this.modData.drawWaveform();

            if( this.saveState ) {
                this.modData.saveWAV();
            }
        }
    },

    // shitty radio button
    doFileSelect: function() {
        switch( this.fileSelect ) {
        case 0:
            this.fileShortButton.setAttribute("selected", "");
            this.fileTest1Button.removeAttribute("selected");
            this.fileTest2Button.removeAttribute("selected");
            this.fileName = "shortfile.bin";
            break;
        case 1:
            this.fileShortButton.removeAttribute("selected");
            this.fileTest1Button.setAttribute("selected", "");
            this.fileTest2Button.removeAttribute("selected");
            this.fileName = "microtest1.bin";
            break;
        case 2:
            this.fileShortButton.removeAttribute("selected");
            this.fileTest1Button.removeAttribute("selected");
            this.fileTest2Button.setAttribute("selected", "");
            this.fileName = "app.bin";
            break;
        default:
            throw "unknown fileSelect mode: " + thisfileSelect;
            break;
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
        var fileLen = self.ui.byteArray.length;
        var blocks = Math.ceil(fileLen / 256);

        // index 0 & 1 create identical control packets. We transmit the control packet
        // twice in the beginning because (a) it's tiny and almost free and (b) if we
        // happen to miss it, we waste an entire playback cycle before we start committing
        // data to memory
        if( index == 0  || index == 1 ) {
            var ctlPacket = self.ui.makeCtlPacket(self.ui.byteArray.subarray(0, fileLen));

            self.ui.modData.modulate(ctlPacket);
            self.ui.modData.playLoop(self.ui, index + 1);
            self.ui.modData.drawWaveform();
        } else {
            // data index starts at 2, due to two sends of the control packet up front
            var i = index - 2;
            // handle whole blocks
            if( i < blocks - 1 ) {
                var dataPacket = self.ui.makeDataPacket(self.ui.byteArray.subarray(i * 256, i * 256 + 256), i);
                self.ui.modData.modulate(dataPacket);
                self.ui.modData.playLoop(self.ui, index + 1);
                self.ui.modData.drawWaveform();
            } else {
                // handle last block of data, which may not be 256 bytes long
                var dataPacket = self.ui.makeDataPacket(self.ui.byteArray.subarray(i * 256, fileLen), i);
                self.ui.modData.modulate(dataPacket);
                self.ui.modData.playLoop(self.ui, index + 1);
                self.ui.modData.drawWaveform();
            }

        }
    },
    makeCtlPacket: function(data) {
        // parameters from microcontroller spec. Probably a better way
        // to do this in javascript, but I don't know how (seems like "const" could be used, but not universal)
        var bytePreamble = [00,00,00,00,0xaa,0x55,0x42];
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
        for( i = 0; i < guidStr.length-1; i += 2 ) {
            pktGuid.push(parseInt(guidStr.substr(i,2),16));
        }

        var packetlen = bytePreamble.length + bytePadding.length + byteVersion.length + byteType.length + byteLength.length + 4 + pktGuid.length + 4 + 1;
        var pkt = new Uint8Array(packetlen);
        var pktIndex = 0;
        for( i = 0; i < bytePreamble.length; i++ ) {
            pkt[pktIndex++] = bytePreamble[i];
        }
        pkt[pktIndex++] = byteVersion[0];
        pkt[pktIndex++] = byteType[0];
        pkt[pktIndex++] = bytePadding[0];
        pkt[pktIndex++] = bytePadding[1];
        for( i = 0; i < byteLength.length; i++ ) {
            pkt[pktIndex++] = byteLength[i];
        }
        pkt[pktIndex++] = pktFullhash & 0xFF;
        pkt[pktIndex++] = (pktFullhash >> 8) & 0xFF;
        pkt[pktIndex++] = (pktFullhash >> 16) & 0xFF;
        pkt[pktIndex++] = (pktFullhash >> 24) & 0xFF;
        for( i = 0; i < 16; i++ ) {
            pkt[pktIndex++] = pktGuid[i];
        }

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
        if( dataIn.length != 256 ) {
            // if our data array isn't a whole packet in length, pad it out with FF's
            data = new Uint8Array(256);
            for( i = 0; i < dataIn.length; i ++ ) {
                data[i] = dataIn[i];
            }
            for( ; i < 256; i++ ) {
                data[i] = 0xFF; // 1's pad out the final data packet
            }
        } else {
            data = dataIn;
        }
        // now assemble the packet
        var preamble = [00,00,00,00,0xaa,0x55,0x42];
        var sector = [0x01, 0x02, blocknum & 0xFF, (blocknum >> 8) & 0xFF];   // version 1
        // 256 byte payload, preamble, sector offset + 4 bytes hash + 1 byte stop
        var packetlen = 256 + preamble.length + sector.length + 4 + 1;

        var buffer = new Uint8Array(packetlen);
        for( i = 0; i < preamble.length; i++ ) {
            buffer[i] = preamble[i];
        }
        for( var j = 0; i < sector.length + preamble.length; i++, j++ ) {
            buffer[i] = sector[j];
        }
        for( j = 0; i < packetlen - 1 - 4; i++, j++ ) {
            buffer[i] = data[j];
        }

        hash = murmurhash3_32_gc(buffer.subarray(preamble.length, 256 + preamble.length + sector.length), 0xdeadbeef);
        buffer[i++] = hash & 0xFF;
        buffer[i++] = (hash >> 8) & 0xFF;
        buffer[i++] = (hash >> 16) & 0xFF;
        buffer[i++] = (hash >> 24) & 0xFF
        buffer[i] = 0xFF;  // terminate with 0xFF to let last bit demodulate correctly

        // now stripe the buffer to ensure transitions for baud sync
        // don't stripe the premable or the hash
        for( i = 9; i < (buffer.length - 5); i++ ) {
            if( (i % 16) == 14 )
                buffer[i] ^= 0x55;
            else if ( (i % 16) == 6 )
                buffer[i] ^= 0xaa;
        }

        return buffer;
    },

    // once all audio is done playing, call this to reset UI elements to idle state
    audioEndCB: function() {
        this.sendButton.removeAttribute("selected", "");
        this.ctrlButton.removeAttribute("selected", "");
        this.fileButton.removeAttribute("selected", "");
        this.txLed.removeAttribute("lit");
    },

    // deal with save button toggling
    onSaveButton: function() {
        this.saveState = !this.saveState;
        if (this.saveState) {
            this.saveButton.setAttribute("selected", "");
        } else {
            this.saveButton.removeAttribute("selected");
        }
    },
}
