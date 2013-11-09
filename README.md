audio-level
===========

A node module to report average audio level at a configurable interval (defaulting to once per second)

Example
    var audio-level = require('audio-level');
    
    audio-level.start(function(level) {
      console.log(level + "db");
    });
    
    setTimeout(audio-level.stop,5000);