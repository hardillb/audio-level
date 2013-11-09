{
  "targets": [
    {
      "target_name": "audio-level",
      "conditions": [
        ['OS=="linux"', {
          "sources": [ "src/audio.cpp" ],
          "link_settings": {
            "libraries": ['-lasound']
          }
        }
        ]
      ]
    }
  ]
}