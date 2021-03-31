# zinnia-demo

## Requirement
- ruby 2.2.0
- rails 4.1.7
- zinnia-0.0.6
- zinnia-tomoe-0.6.0

## Zinnia

### Download
https://sourceforge.net/projects/zinnia/files/

### Installation

```
cd zinnia-0.0.6/ruby
ruby extconf.rb
make
make install
```

```
cd zinnia-tomoe-0.6.0-20080911
./configure
make
make install
```

## APIs
- home/top (GET)
- home/classify (GET)

## Demo

![demo](https://github.com/aHungNguyenKhanh/zinnia-demo/blob/main/demo.gif)

# WASM build

## Requirement

- emscripten
https://emscripten.org/docs/getting_started/downloads.html

## Build

### Input
- https://github.com/aHungNguyenKhanh/zinnia-demo/blob/main/lib/example.cpp

### 
```
cd zinnia-0.0.6
./emconfigure ./configure
./emmake make
cp /usr/local/lib/zinnia/model/tomoe/handwriting-ja.model ./
cp ~/zinnia-demo/libs/example.cpp ./
em++ -o example.html -WASM=1 -s ALLOW_MEMORY_GROWTH=1 example.cpp recognizer.o character.o libzinnia.o param.o feature.o sexp.o svm.o trainer.o --embed-file=handwriting-ja.model
```

## Output

- example.html
- example.js
- example.wasm
