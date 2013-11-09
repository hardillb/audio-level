var audio = require('../build/Release/audio-level');

function callback(level) {
	console.log("dB = " + level);
}

setTimeout(audio.stop, 5000);
audio.start(callback);

