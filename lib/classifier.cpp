#include <fstream>
#include <iostream>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <emscripten.h>
#include <sstream>
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
}

void preload_failure(emscripten_fetch_t *fetch) {
  std::cerr << "Failed to load model" << std::endl;
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

void classify(std::string input, int item_count, val callback){
  std::stringstream result_json;
  zinnia::Recognizer *recognizer = zinnia::Recognizer::create();
  if (!recognizer->open(fetched_model->data, (size_t)fetched_model->numBytes)) {
    // std::cerr << "recognizer: " << recognizer->what() << std::endl;
    callback(false, result_json.str());
    delete recognizer;
    return;
  }
  
  zinnia::Character *character = zinnia::Character::create();
  if (!character->parse(input.c_str())) {
    callback(false, result_json.str());
  } else {
    zinnia::Result *result = recognizer->classify(*character, item_count);
    if (!result) {
      std::cerr << "result: " << recognizer->what() << std::endl;
      callback(false, result_json.str());
    } else {
      result_json << "{";
      for (size_t i = 0; i < result->size(); ++i) {
        if(i > 0)
          result_json << ",";
        // std::cout << result->value(i) << "\t" << result->score(i) << std::endl;
        result_json << "\"" << result->value(i) << "\":";
        result_json << "\"" << std::to_string(result->score(i)) << "\"";
      }
      result_json << "}";
      callback(true, result_json.str());
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
  // return output_map;
}

std::map<std::wstring, double> returnMapData() {
  std::map<std::wstring, double> m;
  const std::wstring key = L"Key";
  const std::wstring key2 = L"Key2";
  m.insert(std::pair<std::wstring, double>(key, 2.4123));
  m.insert(std::pair<std::wstring, double>(key2, 4.2333));
  return m;
}

std::vector<std::vector<std::string>> returnVectorData () {
  std::vector<std::vector<std::string>> v;
  std::vector<std::string> vi = {"ghsa", "2.423"};
  v.push_back(vi);
  return v;
}

EMSCRIPTEN_BINDINGS(classifier) {
  function("classify", &classify);
  function("preload", &preload);
  function("returnMapData", &returnMapData);
  function("returnVectorData",&returnVectorData, allow_raw_pointers());
  // register_map<std::string, double>("map<string, double>");
  register_vector<std::vector<std::string>>("vector<std::vector<std::string>");
  register_map<std::wstring, double>("map<wstring, double>");
}
