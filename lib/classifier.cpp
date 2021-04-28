#include <fstream>
#include <iostream>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <emscripten.h>
#include <sstream>
#include "zinnia.h"

using namespace emscripten;

enum ModelIndex {
  ALPHABET = 0,
  NUMERIC,
  KATAKANA,
  JAPANESE,
  CHINESE,
  FIRST_GRADE,
  SECOND_GRADE,
  THIRD_GRADE,
  FOURTH_GRADE,
  FIFTH_GRADE,
  SIXTH_GRADE,
  NUMBER_OF_MODELS
};

typedef struct FetchUserData {
  ModelIndex model_index;
  val callback = val::null();
} FetchUserData;

typedef emscripten_fetch_t* PreloadedModel;

static PreloadedModel models[NUMBER_OF_MODELS];

// val preload_call_back = val::null();

void preload_success(emscripten_fetch_t *fetch) {
  FetchUserData* userdata = (FetchUserData*)fetch->userData;
  if(fetch->numBytes > 0) {
    models[userdata->model_index] = fetch;
    userdata->callback(true);
  } else {
    userdata->callback(false);
  }
}

void preload_failure(emscripten_fetch_t *fetch) {
  FetchUserData* userdata = (FetchUserData*)fetch->userData;
  userdata->callback(false);
  emscripten_fetch_close(fetch);
}

void downloadFileToIndexedDB(emscripten_fetch_t *fetch) {
  if(fetch) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_REPLACE;
    attr.onsuccess = preload_success;
    attr.onerror = preload_failure;
    attr.userData = fetch->userData;
    emscripten_fetch(&attr, fetch->url);
  } else {
    preload_failure(nullptr);
  }
}

void idxdb_load_failure(emscripten_fetch_t *fetch) {
  downloadFileToIndexedDB(fetch);
  emscripten_fetch_close(fetch);
}

std::string getURL(ModelIndex model_idx) {
  std::string url;
  switch (model_idx) {
    case ALPHABET:
      url =  "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/alphabet.model";
      break;
    case NUMERIC:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/arabic_numeric.model";
      break;
    case KATAKANA:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/katakana.model";
      break;
    case JAPANESE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-ja.model";
      break;
    case CHINESE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/handwriting-zh_CN.model";
      break;
    case FIRST_GRADE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/first_grade.model";
      break;
    case SECOND_GRADE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/second_grade.model";
      break;
    case THIRD_GRADE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/third_grade.model";
      break;
    case FOURTH_GRADE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/fourth_grade.model";
      break;
    case FIFTH_GRADE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/fifth_grade.model";
      break;
    case SIXTH_GRADE:
      url = "https://zinnia-demo.s3-ap-northeast-1.amazonaws.com/sixth_grade.model";
      break;
    default:
      url =  "";
  }
  return url;
}

void loadModel(ModelIndex model_idx, val callback) {
  std::string url = getURL(model_idx);
  if(url.length() <= 0) {
    std::cerr << "Invalid model index" << std::endl;
    callback(false);
    return;
  }

  if(!models[model_idx]->data || models[model_idx]->numBytes <= 0) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    FetchUserData* userdata = new FetchUserData();
    userdata->model_index = model_idx;
    userdata->callback = callback;
    attr.onsuccess = preload_success;
    attr.userData = (void*)userdata;
    attr.onerror = idxdb_load_failure;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_NO_DOWNLOAD;
    emscripten_fetch(&attr, url.c_str());
  }
}

void classify(std::string input, ModelIndex model_idx, int item_count, val callback){
  std::stringstream result_json;
  PreloadedModel model = models[model_idx];
  // std::cout << model->url << "\t" << model->numBytes << "\t" << model->data << std::endl;
  zinnia::Recognizer *recognizer = zinnia::Recognizer::create();
  if (!recognizer->open(model->data ,static_cast<size_t>(model->numBytes))) {
    std::cerr << "recognizer: " << recognizer->what() << std::endl;
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
    // std::cerr << "freeing character" << std::endl;
    delete character;
  }
  if (recognizer != nullptr) {
    // std::cerr << "freeing recognizer" << std::endl;
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
  function("loadModel", &loadModel);
  enum_<ModelIndex>("Models")
        .value("ALPHABET", ALPHABET)
        .value("NUMERIC", NUMERIC)
        .value("KATAKANA", KATAKANA)
        .value("JAPANESE", JAPANESE)
        .value("CHINESE", CHINESE)
        .value("FIRST_GRADE", FIRST_GRADE)
        .value("SECOND_GRADE", SECOND_GRADE)
        .value("THIRD_GRADE", THIRD_GRADE)
        .value("FOURTH_GRADE", FOURTH_GRADE)
        .value("FIFTH_GRADE", FIFTH_GRADE)
        .value("SIXTH_GRADE", SIXTH_GRADE);
}
