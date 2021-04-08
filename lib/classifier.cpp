#include <fstream>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <emscripten.h>
#include <sanitizer/lsan_interface.h>
#include "zinnia.h"

using namespace emscripten;

static std::string INPUT;
static char *binary;

val call_back = val::global("");

void success(emscripten_fetch_t *fetch) {
  // std::cerr << "IDB store successed." << std::endl;
  emscripten_fetch_close(fetch);
}

void failure(emscripten_fetch_t *fetch) {
  // std::cerr << "IDB store failed." << std::endl;
  emscripten_fetch_close(fetch);
}

void downloadFileToIndexedDB(const char *url) {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_REPLACE;
  // attr.requestData = data;
  // attr.requestDataSize = numBytes;
  attr.onsuccess = success;
  attr.onerror = failure;
  emscripten_fetch(&attr, url);
}

void preload() {
  downloadFileToIndexedDB("https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-ja.model");
}

void fetch_success(emscripten_fetch_t *fetch) {
  binary = (char*)malloc(15);
  zinnia::Recognizer *recognizer = zinnia::Recognizer::create();
  if (!recognizer->open(fetch->data, (size_t)fetch->numBytes)) {
    // std::cerr << recognizer->what() << std::endl;
  }
  
  zinnia::Character *character = zinnia::Character::create();
  if (!character->parse(INPUT.c_str())) {
    // std::cerr <<character->what() << std::endl;
  }

  zinnia::Result *result = recognizer->classify(*character, 10);
  if (!result) {
      // std::cerr << recognizer->what() << std::endl;
  }
  for (size_t i = 0; i < result->size(); ++i) {
    // std::cout << result->value(i) << "\t" << result->score(i) << std::endl;
  }
  delete result;
  delete character;
  delete recognizer; 
  // delete fetch;
  call_back();
}

void classify(std::string input, val callback) {
  INPUT = input;
  call_back = callback;
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  // strcpy(attr.requestMethod, "GET");
  attr.onsuccess = fetch_success;
  attr.onerror = failure;
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_NO_DOWNLOAD;
  emscripten_fetch(&attr, "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-ja.model");
}

EMSCRIPTEN_BINDINGS(classifier) {
    function("classify", &classify);
    function("preload", &preload);
    function("doLeakCheck", &__lsan_do_recoverable_leak_check);
}
