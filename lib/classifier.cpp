#include <fstream>
#include <iostream>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <emscripten.h>
#include "zinnia.h"

using namespace emscripten;

static std::string URL;
static emscripten_fetch_t *fetched_model;
val preload_call_back = val::null();
// val classify_call_back = val::null();

void preload_success(emscripten_fetch_t *fetch) {
  if(fetch->numBytes > 0) {
    fetched_model = fetch;
    preload_call_back(true);
  } else {
    preload_call_back(false);
  } 
  // emscripten_fetch_close(fetch);
}

void preload_failure(emscripten_fetch_t *fetch) {
  std::cerr << "IDB store failed." << std::endl;
  emscripten_fetch_close(fetch);
}

void downloadFileToIndexedDB() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_REPLACE;
  // attr.requestData = data;
  // attr.requestDataSize = numBytes;
  attr.onsuccess = preload_success;
  attr.onerror = preload_failure;
  emscripten_fetch(&attr, URL.c_str());
}

void idxdb_load_failure(emscripten_fetch_t *fetch) {
  emscripten_fetch_close(fetch);
  downloadFileToIndexedDB();
}

void preload(std::string url, val callback) {
  preload_call_back = callback;
  if(!fetched_model) {
    URL = url;
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    attr.onsuccess = preload_success;
    attr.onerror = idxdb_load_failure;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_NO_DOWNLOAD;
    emscripten_fetch(&attr, URL.c_str());
  }
}

std::map<std::string, double> classify(std::string input, int item_count, val callback){
  // binary = (char*)malloc(30000000);
  // call_back = callback;
  // emscripten_fetch_attr_t attr;
  // emscripten_fetch_attr_init(&attr);
  // attr.onsuccess = fetch_success;
  // attr.onerror = failure;
  // attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_NO_DOWNLOAD;
  // emscripten_fetch(&attr, "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-ja.model");
  std::map<std::string, double> output_map;
  zinnia::Recognizer *recognizer = zinnia::Recognizer::create();
  if (!recognizer->open(fetched_model->data, (size_t)fetched_model->numBytes)) {
    std::cerr << "recognizer: " << recognizer->what() << std::endl;
  }
  
  zinnia::Character *character = zinnia::Character::create();
  if (!character->parse(input.c_str())) {
    std::cerr << "character: " << character->what() << std::endl;
  } else {
    zinnia::Result *result = recognizer->classify(*character, item_count);
    if (!result) {
      std::cerr << "result: " << recognizer->what() << std::endl;
    } else {
      for (size_t i = 0; i < result->size(); ++i) {
        // std::cout << result->value(i) << "\t" << result->score(i) << std::endl;
        std::string result_value(result->value(i));
        output_map.insert(std::pair<std::string, double>(result_value, result->score(i)));
        std::cout << output_map[result->value(i)];
      }
      // callback(output_map);
      delete result;
    }
  }
  
  if (character != nullptr) {
    std::cerr << "freeing character" << std::endl;
    delete character;
  }
  if (recognizer != nullptr) {
    std::cerr << "freeing recognizer" << std::endl;
    delete recognizer;
  }
  return output_map;
}

EMSCRIPTEN_BINDINGS(classifier) {
    function("classify", &classify);
    function("preload", &preload);
    register_map<std::string, double>("map<string, double>");
}
