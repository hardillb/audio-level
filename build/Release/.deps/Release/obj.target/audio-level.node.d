cmd_Release/obj.target/audio-level.node := flock ./Release/linker.lock g++ -shared -pthread -rdynamic -m64  -Wl,-soname=audio-level.node -o Release/obj.target/audio-level.node -Wl,--start-group Release/obj.target/audio-level/src/audio.o -Wl,--end-group -lasound
