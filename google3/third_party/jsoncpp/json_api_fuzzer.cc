// JsonCpp fuzzing wrapper to help with automated fuzz testing of the API.
// Note: The tricky bit here is that (given it's C++) the API has to be used
// in a fully correct way, otherwise faults might show up due to invalid API
// usage and not actuall API internal implementation errors.

#include <stdint.h>
#include <climits>
#include <cstdio>
#include <stack>
#include <string>
#include "testing/base/public/benchmark.h"
#include "jsoncpp/json.h"

namespace {

// The number of value types in Json::ValueType enum.
const size_t kValueTypeNumber = 8;

// Initial JSON object to be the base for transformations.
const char kInitialJson[] =
    "{"
    "  \"A\": [ 1, 2, 3, null, true ],"
    "  \"B\": { \"A\": 1.23 }"
    "}";

// A simple data buffer used to feed various data types to the reader from a
// uint8_t data stream.
class DataBuffer {
 public:
  DataBuffer(const uint8_t *data, size_t size)
      : buffer_(data), size_(size), index_(0), error_(false) { }

  // Returns true if end of data was reached.
  bool Error() const {
    return error_;
  }

  // Returns a value of given type, taken from the buffer.
  template<typename T>
  T Get() {
    const size_t byte_count = sizeof(T);
    if (size_ - index_ < byte_count) {
      error_ = true;
      return T();
    }

    T value;
    memcpy(&value, buffer_ + index_, byte_count);
    index_ += byte_count;

    return value;
  }

 private:
  const uint8_t *buffer_;
  size_t size_;
  size_t index_;
  bool error_;
};

// The API stresser, which calls various API functions based on input. It's 100%
// deterministic for the given input within the given version.
class APIStresser {
 public:
  APIStresser(const uint8_t *data, size_t size, Json::Value *root)
      : bits_(data, size), cursor_(root) { }

  void Start();

 private:
  enum Command : uint8_t {
    kCmdTravelUp = 0,          // Move the cursor to parent node.
    kCmdTravelDown = 1,        // Move to child node within the container.
    kCmdReferenceValue = 2,    // Read the value of node.
    kCmdModifyValue = 3,       // Change the value of node.
    kCmdDeleteValue = 4,       // Remove the node and move up if possible.
                               // Note: Removing root respawns it.
    kCmdAddValue = 5,          // If possible add a child node.
    kCmdClear = 6,             // Clears currently selected container.
    kCmdSetComment = 7,        // Add or modify node's comment.
    kCmdReferenceComment = 8,  // Reads all of the node's comments.
    kCmdReparse = 9,           // Encodes the tree to JSON and decodes it.

    kCmdSize                   // Enum size marker.
  };

  struct StackElement {
    enum Type {
      kChildByKey,
      kChildByIndex
    };

    // Parent node.
    Json::Value *node;

    // How child was reached (object key or array index).
    Type type;
    std::string key;
    Json::ArrayIndex index;
  };

  void CmdTravelUp();
  void CmdTravelDown();
  void CmdReferenceValue();
  void CmdModifyValue();
  void CmdDeleteValue();
  void CmdAddValue();
  void CmdClear();
  void CmdSetComment();
  void CmdReferenceComment();
  void CmdReparse();

  // Generates a pseudo-random JSON value seeded on input.
  Json::Value GenerateValue();

  DataBuffer bits_;
  std::stack<StackElement> path_;
  Json::Value *cursor_;
};

void APIStresser::CmdTravelUp() {
  if (path_.empty()) {
    return;
  }

  cursor_ = path_.top().node;
  path_.pop();
}

void APIStresser::CmdTravelDown() {
  if (cursor_->type() != Json::arrayValue &&
      cursor_->type() != Json::objectValue) {
    return;
  }

  if (cursor_->empty()) {
    return;
  }

  Json::ArrayIndex index = bits_.Get<Json::ArrayIndex>() % cursor_->size();

  if (cursor_->type() == Json::arrayValue) {
    path_.push(StackElement{cursor_, StackElement::kChildByIndex, "", index});
    cursor_ = &(*cursor_)[index];
    return;
  }

  if (cursor_->type() == Json::objectValue) {
    auto members = cursor_->getMemberNames();
    const auto& key = members.at(index);
    path_.push(StackElement{cursor_, StackElement::kChildByKey, key, 0});
    cursor_ = &(*cursor_)[key];
    return;
  }
}

void APIStresser::CmdReferenceValue() {
  Json::ValueType type =
      static_cast<Json::ValueType>(bits_.Get<uint8_t>() % kValueTypeNumber);

  if (!cursor_->isConvertibleTo(type)) {
    return;
  }

  switch (type) {
    case Json::nullValue:
      break;

    case Json::intValue:
      testing::DoNotOptimize(cursor_->asInt());
      break;

    case Json::uintValue:
      testing::DoNotOptimize(cursor_->asUInt());
      break;

    case Json::realValue:
      testing::DoNotOptimize(cursor_->asFloat());
      testing::DoNotOptimize(cursor_->asDouble());
      break;

    case Json::stringValue:
      testing::DoNotOptimize(cursor_->asString());
      break;

    case Json::booleanValue:
      testing::DoNotOptimize(cursor_->asBool());
      break;

    case Json::arrayValue:
    case Json::objectValue:
      for (Json::Value& v : *cursor_) {
         testing::DoNotOptimize(v.toStyledString());
      }
      break;
  }
}

void APIStresser::CmdModifyValue() {
  *cursor_ = GenerateValue();
}

void APIStresser::CmdDeleteValue() {
  if (path_.empty()) {
    // Note: cursor_ is at root now.
    *cursor_ = Json::Value(Json::objectValue);
    return;
  }

  cursor_ = path_.top().node;

  Json::Value removed;
  switch (path_.top().type) {
    case StackElement::kChildByIndex:
      cursor_->removeIndex(path_.top().index, &removed);
      break;

    case StackElement::kChildByKey:
      removed = cursor_->removeMember(path_.top().key);
      break;
  }

  path_.pop();
}

void APIStresser::CmdAddValue() {
  if (cursor_->type() != Json::arrayValue &&
    cursor_->type() != Json::objectValue) {
    return;
  }

  Json::Value new_value = GenerateValue();

  if (cursor_->type() == Json::arrayValue) {
    cursor_->append(new_value);
    return;
  }

  if (cursor_->type() == Json::objectValue) {
    uint16_t pseudo_uid = bits_.Get<uint16_t>();
    char key[8] = {0};
    snprintf(key, sizeof(key), "%.4x", pseudo_uid);
    (*cursor_)[key] = new_value;
    return;
  }
}

void APIStresser::CmdClear() {
  if (cursor_->type() != Json::arrayValue &&
    cursor_->type() != Json::objectValue) {
    return;
  }

  cursor_->clear();
}

void APIStresser::CmdSetComment() {
  auto placement = static_cast<Json::CommentPlacement>(
      bits_.Get<uint8_t>() % Json::numberOfCommentPlacement);

  cursor_->setComment("// comment", placement);
}

void APIStresser::CmdReferenceComment() {
  for (int i = 0; i < Json::numberOfCommentPlacement; ++i) {
    auto placement = static_cast<Json::CommentPlacement>(i);
    if (cursor_->hasComment(placement)) {
      testing::DoNotOptimize(cursor_->getComment(placement));
    }
  }
}

void APIStresser::CmdReparse() {
  std::string json = cursor_->toStyledString();

  // Note: The cursor_ might be at a non-container node, therefor the reader
  // must be set in permissive mode in order to not fail.
  Json::Features features;
  features.allowComments_ = true;
  features.strictRoot_ = false;

  Json::Reader reader(features);
  Json::Value new_root;
  if (!reader.parse(kInitialJson, new_root, /*collectComments=*/true)) {
    return;
  }

  cursor_->swap(new_root);
}

Json::Value APIStresser::GenerateValue() {
  Json::ValueType type =
      static_cast<Json::ValueType>(bits_.Get<uint8_t>() % kValueTypeNumber);
  return Json::Value(type);
}

void APIStresser::Start() {
  // Transform the JSON object based on the input data.
  for (;;) {
    Command cmd = static_cast<Command>(bits_.Get<uint8_t>() % kCmdSize);
    if (bits_.Error()) {
      break;
    }

    switch (cmd) {
      case kCmdTravelUp: CmdTravelUp(); break;
      case kCmdTravelDown: CmdTravelDown(); break;
      case kCmdReferenceValue: CmdReferenceValue(); break;
      case kCmdModifyValue: CmdModifyValue(); break;
      case kCmdDeleteValue: CmdDeleteValue(); break;
      case kCmdAddValue: CmdAddValue(); break;
      case kCmdClear: CmdClear(); break;
      case kCmdSetComment: CmdSetComment(); break;
      case kCmdReferenceComment: CmdReferenceComment(); break;
      case kCmdReparse: CmdReparse(); break;
      case kCmdSize: /* Ignore this case. */ break;
    }
  }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Setup initial JSON object.
  Json::Value root;
  Json::Reader reader;
  bool res = reader.parse(kInitialJson, root, /*collectComments=*/true);
  if (!res) {
    fprintf(stderr, "Failed to parse initial JSON, which should not happen.\n");
    abort();
  }

  APIStresser stresser(data, size, &root);
  stresser.Start();

  // Useful for debugging:
  // printf("root-> %s\n", root.toStyledString().c_str());

  return 0;
}
