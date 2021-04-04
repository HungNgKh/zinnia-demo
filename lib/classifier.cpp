#include <iostream>
#include <fstream>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <emscripten.h>
#include "zinnia.h"

using namespace emscripten;

static std::string INPUT = NULL;

void success(emscripten_fetch_t *fetch) {
  std::cerr << "IDB store successed" << std::endl;
  emscripten_fetch_close(fetch);
}

void failure(emscripten_fetch_t *fetch) {
  std::cerr << "IDB store failed." << std::endl;
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

inline bool fileIsExist(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

emscripten_fetch_t* persistFilesFromIDB() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  attr.attributes = EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-ja.model");
  if (fetch->status == 200) {
    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  } else {
    printf("Downloading %llu failed, HTTP failure status code: %d.\n", fetch->numBytes, fetch->status);
  }
  emscripten_fetch_close(fetch);
  return fetch;
}

void fetch_success(emscripten_fetch_t *fetch) {
  zinnia::Recognizer *recognizer = zinnia::Recognizer::create();
  if (!recognizer->open(fetch->data, (size_t)fetch->numBytes)) {
    std::cerr << recognizer->what() << std::endl;
  }

  zinnia::Character *character = zinnia::Character::create();
  if (!character->parse(INPUT.c_str())) {
    std::cerr <<character->what() << std::endl;
  }

  zinnia::Result *result = recognizer->classify(*character, 10);
  if (!result) {
      std::cerr << recognizer->what() << std::endl;
  }
  for (size_t i = 0; i < result->size(); ++i) {
    std::cout << result->value(i) << "\t" << result->score(i) << std::endl;
  }
  delete result;
  delete character;
  delete recognizer; 
}

void *fetch_model(void *fetch) {
  emscripten_fetch_t **result;
  result=(emscripten_fetch_t**)fetch;

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  // strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_NO_DOWNLOAD | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  *result = emscripten_fetch(&attr, "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-ja.model");
  if ((*result)->status == 200) {
    printf("Finished downloading %llu bytes from URL %s.\n", (*result)->numBytes, (*result)->url);
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  } else {
    printf("Downloading %llu failed, HTTP failure status code: %d.\n", (*result)->numBytes, (*result)->status);
  }
  emscripten_fetch_close(*result);
  pthread_exit(NULL);
}

void classify(std::string input) {
  INPUT = input;
  // pthread_t fetch_thread;
  // emscripten_fetch_t *fetch_result = NULL;
  // pthread_create(&fetch_thread, NULL, fetch_model,(void *)&fetch_result);
  // pthread_join(fetch_thread,NULL);
  // fetch_success(fetch_result);
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  // strcpy(attr.requestMethod, "GET");
  attr.onsuccess = fetch_success;
  attr.onerror = failure;
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_NO_DOWNLOAD;
  emscripten_fetch(&attr, "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-ja.model");
//   if (fetch->status == 200) {
//     printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
//     // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
//   } else {
//     printf("Downloading %llu failed, HTTP failure status code: %d.\n", fetch->numBytes, fetch->status);
//   }
}

void clear() {
  //TODO
}

EMSCRIPTEN_BINDINGS(classifier) {
    function("classify", &classify);
    function("preload", &preload);
}
