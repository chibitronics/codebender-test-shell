

(function (window) {
	'use strict';

	var chr8 = function () {
		return Array.prototype.map.call(arguments, function(a){
			return String.fromCharCode(a&0xff)
		}).join('');
	}
	var chr16 = function () {
		return Array.prototype.map.call(arguments, function(a){
			return String.fromCharCode(a&0xff, (a>>8)&0xff)
		}).join('');
	}
	var chr32 = function () {
		return Array.prototype.map.call(arguments, function(a){
			return String.fromCharCode(a&0xff, (a>>8)&0xff,(a>>16)&0xff, (a>>24)&0xff);
		}).join('');
	}
	var toUTF8 = function (str) {
		var utf8 = [];
		for (var i = 0; i < str.length; i++) {
			var c = str.charCodeAt(i);
			if (c <= 0x7f)
				utf8.push(c);
			else if (c <= 0x7ff) {
				utf8.push(0xc0 | (c >>> 6));
				utf8.push(0x80 | (c & 0x3f));
			} else if (c <= 0xffff) {
				utf8.push(0xe0 | (c >>> 12));
				utf8.push(0x80 | ((c >>> 6) & 0x3f));
				utf8.push(0x80 | (c & 0x3f));
			} else {
				var j = 4;
				while (c >>> (6*j)) j++;
				utf8.push(((0xff00 >>> j) & 0xff) | (c >>> (6*--j)));
				while (j--) 
					utf8[idx++] = 0x80 | ((c >>> (6*j)) & 0x3f);
			}
		}
		return utf8;
	}

	var FSK = function(params) {

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

		this.freqHigh = params.freqHigh || 7350;
		this.freqLow = params.freqLow || 4900;
		this.baud = params.baud || 1225;
		this.sampleRate = params.sampleRate || 29400;
	}

	FSK.prototype = {
		generate: function (str) {

			if (str.length === 0)
				return;
			//var utf8 = toUTF8(str);
			//console.log(str);
			
			var sampleRate = this.sampleRate;
			var baud = this.baud;
			var freqHigh = this.freqHigh;
			var freqLow  = this.freqLow;
			var spb = sampleRate/baud; // 24 samples per bit
			var preCarrierBits = Math.ceil(sampleRate*40/1000/spb); // 49 bits
			var postCarrierBits = Math.ceil(sampleRate*5/1000/spb); // 6.125 bits => 7 bits
			var size = (preCarrierBits + postCarrierBits + 10*str.length) * spb;

			var data = "RIFF" + chr32(size+36) + "WAVE" +
					"fmt " + chr32(16, 0x00010001, sampleRate, sampleRate, 0x00080001) +
					"data" + chr32(size);
			
			var pushData = function (freq, samples) {
				for (var i = 0; i < samples; i++) {
					var v = 128 + 127 * Math.sin((2 * Math.PI) * (i / sampleRate) * freq);
					data += chr8(v);
				}
			}
			pushData(freqHigh, preCarrierBits*spb);
			for (var x in str) {
				var c = (str[x] << 1) | 0x200;
				for (var i = 0; i < 10; i++, c >>>= 1)
					pushData((c&1) ? freqHigh : freqLow, spb);
			}
			pushData(freqHigh, postCarrierBits*spb);
			
			if (size+44 != data.length) alert("wrong size: " + size+44 + " != " + data.length);
			
			var dataURI = escape(btoa(data));

			var arrayBuff = Base64Binary.decodeArrayBuffer(dataURI);
			var fixme = this;
			this.audioContext.decodeAudioData(arrayBuff, function (audioData) {
				fixme.buffer = audioData;
			});
		},

		play: function () {
			this.source = this.audioContext.createBufferSource();
			this.source.buffer = this.buffer;
			this.source.connect(this.audioContext.destination);
			if ('AudioContext' in window) {
				this.source.start(0);
			} else if ('webkitAudioContext' in window) {
				this.source.noteOn(0);
			} 
		}
	};

	window.FSK = function(params) {
		return new FSK(params);
	};
}(this));
