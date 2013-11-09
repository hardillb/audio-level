config:
	node-gyp configure

build: config
	node-gyp build

all: build

clean:
	rm -rf build