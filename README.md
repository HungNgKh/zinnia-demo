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
- https://github.com/aHungNguyenKhanh/zinnia-demo/blob/main/lib/classifier.cpp

### 
```
cp zinnia-demo/libs/
em++ --bind -lidbfs.js -o classifier.html -s WASM=1 -s FETCH=1 -s tests/classifier.cpp recognizer.o character.o libzinnia.o param.o feature.o sexp.o svm.o trainer.o -s EXPORT_NAME="'Classifier'" -s ALLOW_MEMORY_GROWTH=1
```

### Output

- classifier.js
- classifier.wasm

## WebAssembly module

### Functions
- preload(callback(success){}) //Load model file from remote server to indexedDB 
  - Parameter: 
    - callback function:
      - parameters:
        - success: Boolean
  - Return: 
    
- classify(input, callback(success, result){}) // get saved model file from indexedDB then classify the input and print result.
  - Parameters:
    - input: formated handwriting strokes. Example: "(character (width 1000)(height 1000)(strokes ((243 273)(393 450))((700 253)(343 486)(280 716)(393 866)(710 880))))"
    - callback function:
      - parameters:
        - success: Boolean
        - result: String. Example: "('あ': 0.8923, 'い': 0.342, 'う': 0.5462, 'え':0.234, 'お': 0.456)"
  - Return:

### Import module

```
<script async type="text/javascript" src="classifier.js"></script>
```

### Use

```
Classifier.preload(function callback(success) {
  if(success == true) {
    // TODO
  } else {
    // TODO
  }
});
```

```
Classifier.classify("input strokes", function callback(success, result) {
  if(success == true) {
    // Do whatever we want with result
  } else {
    // TODO
  }
});
```
## Sequence

### preload()
```mermaid
sequenceDiagram;
  participant Javascript
  participant ClassifierModule
  participant IndexedDB
  participant RemoteServer
  
  Javascript->>ClassifierModule: async load()
  activate ClassifierModule
  ClassifierModule->>RemoteServer: FETCH request
  RemoteServer->>IndexedDB: download to
  RemoteServer-->>ClassifierModule: FETCH response
  ClassifierModule-->>Javascript: invoke result callback
  deactivate ClassifierModule
```

### classify()

```mermaid
sequenceDiagram;
  participant Javascript
  participant ClassifierModule
  participant IndexedDB

  Javascript->>ClassifierModule: async classify(input)
  activate ClassifierModule
  ClassifierModule->>IndexedDB: FETCH model binary
  IndexedDB-->>ClassifierModule: load binary to memory
  ClassifierModule-->>Javascript: invoke result callback
  deactivate ClassifierModule
```