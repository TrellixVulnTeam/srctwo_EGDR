// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/tools/compositor_model_bench/render_tree.h"

#include <memory>
#include <sstream>
#include <vector>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/values.h"
#include "gpu/tools/compositor_model_bench/shaders.h"

using base::JSONReader;
using base::JSONWriter;
using base::ReadFileToString;
using base::Value;

GLenum TextureFormatFromString(const std::string& format) {
  if (format == "RGBA")
    return GL_RGBA;
  if (format == "RGB")
    return GL_RGB;
  if (format == "LUMINANCE")
    return GL_LUMINANCE;
  return GL_INVALID_ENUM;
}

const char* TextureFormatName(GLenum format) {
  switch (format) {
    case GL_RGBA:
      return "RGBA";
    case GL_RGB:
      return "RGB";
    case GL_LUMINANCE:
      return "LUMINANCE";
    default:
      return "(unknown format)";
  }
}

int FormatBytesPerPixel(GLenum format) {
  switch (format) {
    case GL_RGBA:
      return 4;
    case GL_RGB:
      return 3;
    case GL_LUMINANCE:
      return 1;
    default:
      return 0;
  }
}

RenderNode::RenderNode() {
}

RenderNode::~RenderNode() {
}

void RenderNode::Accept(RenderNodeVisitor* v) {
  v->BeginVisitRenderNode(this);
  v->EndVisitRenderNode(this);
}

ContentLayerNode::ContentLayerNode() {
}

ContentLayerNode::~ContentLayerNode() {
}

void ContentLayerNode::Accept(RenderNodeVisitor* v) {
  v->BeginVisitContentLayerNode(this);
  for (auto& child : children_) {
    child->Accept(v);
  }
  v->EndVisitContentLayerNode(this);
}

CCNode::CCNode() {
}

CCNode::~CCNode() {
}

void CCNode::Accept(RenderNodeVisitor* v) {
  v->BeginVisitCCNode(this);
  v->EndVisitCCNode(this);
}

RenderNodeVisitor::~RenderNodeVisitor() {
}

void RenderNodeVisitor::BeginVisitContentLayerNode(ContentLayerNode* v) {
  this->BeginVisitRenderNode(v);
}

void RenderNodeVisitor::BeginVisitCCNode(CCNode* v) {
  this->BeginVisitRenderNode(v);
}

void RenderNodeVisitor::EndVisitRenderNode(RenderNode* v) {
}

void RenderNodeVisitor::EndVisitContentLayerNode(ContentLayerNode* v) {
  this->EndVisitRenderNode(v);
}

void RenderNodeVisitor::EndVisitCCNode(CCNode* v) {
  this->EndVisitRenderNode(v);
}

std::unique_ptr<RenderNode> InterpretNode(const base::DictionaryValue& node);

std::string ValueTypeAsString(Value::Type type) {
  switch (type) {
    case Value::Type::NONE:
      return "NULL";
    case Value::Type::BOOLEAN:
      return "BOOLEAN";
    case Value::Type::INTEGER:
      return "INTEGER";
    case Value::Type::DOUBLE:
      return "DOUBLE";
    case Value::Type::STRING:
      return "STRING";
    case Value::Type::BINARY:
      return "BINARY";
    case Value::Type::DICTIONARY:
      return "DICTIONARY";
    case Value::Type::LIST:
      return "LIST";
    default:
      return "(UNKNOWN TYPE)";
  }
}

// Makes sure that the key exists and has the type we expect.
bool VerifyDictionaryEntry(const base::DictionaryValue& node,
                           const std::string& key,
                           Value::Type type) {
  if (!node.HasKey(key)) {
    LOG(ERROR) << "Missing value for key: " << key;
    return false;
  }

  const Value* child;
  node.Get(key, &child);
  if (!child->IsType(type)) {
    LOG(ERROR) << key << " did not have the expected type "
      "(expected " << ValueTypeAsString(type) << ")";
    return false;
  }

  return true;
}

// Makes sure that the list entry has the type we expect.
bool VerifyListEntry(const base::ListValue& l,
                     int idx,
                     Value::Type type,
                     const char* listName = nullptr) {
  // Assume the idx is valid (since we'll be able to generate a better
  // error message for this elsewhere.)
  const Value* el;
  l.Get(idx, &el);
  if (!el->IsType(type)) {
    LOG(ERROR) << (listName ? listName : "List") << "element " << idx
               << " did not have the expected type (expected "
               << ValueTypeAsString(type) << ")\n";
    return false;
  }

  return true;
}

bool InterpretCommonContents(const base::DictionaryValue& node, RenderNode* c) {
  if (!VerifyDictionaryEntry(node, "layerID", Value::Type::INTEGER) ||
      !VerifyDictionaryEntry(node, "width", Value::Type::INTEGER) ||
      !VerifyDictionaryEntry(node, "height", Value::Type::INTEGER) ||
      !VerifyDictionaryEntry(node, "drawsContent", Value::Type::BOOLEAN) ||
      !VerifyDictionaryEntry(node, "targetSurfaceID", Value::Type::INTEGER) ||
      !VerifyDictionaryEntry(node, "transform", Value::Type::LIST)) {
    return false;
  }

  int layerID;
  node.GetInteger("layerID", &layerID);
  c->set_layerID(layerID);
  int width;
  node.GetInteger("width", &width);
  c->set_width(width);
  int height;
  node.GetInteger("height", &height);
  c->set_height(height);
  bool drawsContent;
  node.GetBoolean("drawsContent", &drawsContent);
  c->set_drawsContent(drawsContent);
  int targetSurface;
  node.GetInteger("targetSurfaceID", &targetSurface);
  c->set_targetSurface(targetSurface);

  const base::ListValue* transform;
  node.GetList("transform", &transform);
  if (transform->GetSize() != 16) {
    LOG(ERROR) << "4x4 transform matrix did not have 16 elements";
    return false;
  }
  float transform_mat[16];
  for (int i = 0; i < 16; ++i) {
    if (!VerifyListEntry(*transform, i, Value::Type::DOUBLE, "Transform"))
      return false;
    double el;
    transform->GetDouble(i, &el);
    transform_mat[i] = el;
  }
  c->set_transform(transform_mat);

  if (!node.HasKey("tiles"))
    return true;

  if (!VerifyDictionaryEntry(node, "tiles", Value::Type::DICTIONARY))
    return false;
  const base::DictionaryValue* tiles_dict;
  node.GetDictionary("tiles", &tiles_dict);
  if (!VerifyDictionaryEntry(*tiles_dict, "dim", Value::Type::LIST))
    return false;
  const base::ListValue* dim;
  tiles_dict->GetList("dim", &dim);
  if (!VerifyListEntry(*dim, 0, Value::Type::INTEGER, "Tile dimension") ||
      !VerifyListEntry(*dim, 1, Value::Type::INTEGER, "Tile dimension")) {
    return false;
  }
  int tile_width;
  dim->GetInteger(0, &tile_width);
  c->set_tile_width(tile_width);
  int tile_height;
  dim->GetInteger(1, &tile_height);
  c->set_tile_height(tile_height);

  if (!VerifyDictionaryEntry(*tiles_dict, "info", Value::Type::LIST))
    return false;
  const base::ListValue* tiles;
  tiles_dict->GetList("info", &tiles);
  for (unsigned int i = 0; i < tiles->GetSize(); ++i) {
    if (!VerifyListEntry(*tiles, i, Value::Type::DICTIONARY, "Tile info"))
      return false;
    const base::DictionaryValue* tdict;
    tiles->GetDictionary(i, &tdict);

    if (!VerifyDictionaryEntry(*tdict, "x", Value::Type::INTEGER) ||
        !VerifyDictionaryEntry(*tdict, "y", Value::Type::INTEGER)) {
      return false;
    }
    Tile t;
    tdict->GetInteger("x", &t.x);
    tdict->GetInteger("y", &t.y);
    if (tdict->HasKey("texID")) {
      if (!VerifyDictionaryEntry(*tdict, "texID", Value::Type::INTEGER))
        return false;
      tdict->GetInteger("texID", &t.texID);
    } else {
      t.texID = -1;
    }
    c->add_tile(t);
  }
  return true;
}

bool InterpretCCData(const base::DictionaryValue& node, CCNode* c) {
  if (!VerifyDictionaryEntry(node, "vertex_shader", Value::Type::STRING) ||
      !VerifyDictionaryEntry(node, "fragment_shader", Value::Type::STRING) ||
      !VerifyDictionaryEntry(node, "textures", Value::Type::LIST)) {
    return false;
  }
  std::string vertex_shader_name, fragment_shader_name;
  node.GetString("vertex_shader", &vertex_shader_name);
  node.GetString("fragment_shader", &fragment_shader_name);

  c->set_vertex_shader(ShaderIDFromString(vertex_shader_name));
  c->set_fragment_shader(ShaderIDFromString(fragment_shader_name));
  const base::ListValue* textures;
  node.GetList("textures", &textures);
  for (unsigned int i = 0; i < textures->GetSize(); ++i) {
    if (!VerifyListEntry(*textures, i, Value::Type::DICTIONARY, "Tex list"))
      return false;
    const base::DictionaryValue* tex;
    textures->GetDictionary(i, &tex);

    if (!VerifyDictionaryEntry(*tex, "texID", Value::Type::INTEGER) ||
        !VerifyDictionaryEntry(*tex, "height", Value::Type::INTEGER) ||
        !VerifyDictionaryEntry(*tex, "width", Value::Type::INTEGER) ||
        !VerifyDictionaryEntry(*tex, "format", Value::Type::STRING)) {
      return false;
    }
    Texture t;
    tex->GetInteger("texID", &t.texID);
    tex->GetInteger("height", &t.height);
    tex->GetInteger("width", &t.width);

    std::string formatName;
    tex->GetString("format", &formatName);
    t.format = TextureFormatFromString(formatName);
    if (t.format == GL_INVALID_ENUM) {
      LOG(ERROR) << "Unrecognized texture format in layer " << c->layerID()
                 << " (format: " << formatName << ")\n"
                                                  "The layer had "
                 << textures->GetSize() << " children.";
      return false;
    }

    c->add_texture(t);
  }

  if (c->vertex_shader() == SHADER_UNRECOGNIZED) {
    LOG(ERROR) << "Unrecognized vertex shader name, layer " << c->layerID()
               << " (shader: " << vertex_shader_name << ")";
    return false;
  }

  if (c->fragment_shader() == SHADER_UNRECOGNIZED) {
    LOG(ERROR) << "Unrecognized fragment shader name, layer " << c->layerID()
               << " (shader: " << fragment_shader_name << ")";
    return false;
  }

  return true;
}

std::unique_ptr<RenderNode> InterpretContentLayer(
    const base::DictionaryValue& node) {
  auto n = base::MakeUnique<ContentLayerNode>();
  if (!InterpretCommonContents(node, n.get()))
    return nullptr;

  if (!VerifyDictionaryEntry(node, "type", Value::Type::STRING) ||
      !VerifyDictionaryEntry(node, "skipsDraw", Value::Type::BOOLEAN) ||
      !VerifyDictionaryEntry(node, "children", Value::Type::LIST)) {
    return nullptr;
  }

  std::string type;
  node.GetString("type", &type);
  DCHECK_EQ(type, "ContentLayer");
  bool skipsDraw;
  node.GetBoolean("skipsDraw", &skipsDraw);
  n->set_skipsDraw(skipsDraw);

  const base::ListValue* children;
  node.GetList("children", &children);
  for (unsigned int i = 0; i < children->GetSize(); ++i) {
    const base::DictionaryValue* childNode;
    if (!children->GetDictionary(i, &childNode))
      continue;
    std::unique_ptr<RenderNode> child = InterpretNode(*childNode);
    if (child)
      n->add_child(child.release());
  }

  return std::move(n);
}

std::unique_ptr<RenderNode> InterpretCanvasLayer(
    const base::DictionaryValue& node) {
  auto n = base::MakeUnique<CCNode>();
  if (!InterpretCommonContents(node, n.get()))
    return nullptr;

  if (!VerifyDictionaryEntry(node, "type", Value::Type::STRING))
    return nullptr;

  std::string type;
  node.GetString("type", &type);
  assert(type == "CanvasLayer");

  if (!InterpretCCData(node, n.get()))
    return nullptr;

  return std::move(n);
}

std::unique_ptr<RenderNode> InterpretVideoLayer(
    const base::DictionaryValue& node) {
  auto n = base::MakeUnique<CCNode>();
  if (!InterpretCommonContents(node, n.get()))
    return nullptr;

  if (!VerifyDictionaryEntry(node, "type", Value::Type::STRING))
    return nullptr;

  std::string type;
  node.GetString("type", &type);
  assert(type == "VideoLayer");

  if (!InterpretCCData(node, n.get()))
    return nullptr;

  return std::move(n);
}

std::unique_ptr<RenderNode> InterpretImageLayer(
    const base::DictionaryValue& node) {
  auto n = base::MakeUnique<CCNode>();
  if (!InterpretCommonContents(node, n.get()))
    return nullptr;

  if (!VerifyDictionaryEntry(node, "type", Value::Type::STRING))
    return nullptr;

  std::string type;
  node.GetString("type", &type);
  assert(type == "ImageLayer");

  if (!InterpretCCData(node, n.get()))
    return nullptr;

  return std::move(n);
}

std::unique_ptr<RenderNode> InterpretNode(const base::DictionaryValue& node) {
  if (!VerifyDictionaryEntry(node, "type", Value::Type::STRING))
    return nullptr;

  std::string type;
  node.GetString("type", &type);
  if (type == "ContentLayer")
    return InterpretContentLayer(node);
  if (type == "CanvasLayer")
    return InterpretCanvasLayer(node);
  if (type == "VideoLayer")
    return InterpretVideoLayer(node);
  if (type == "ImageLayer")
    return InterpretImageLayer(node);

  std::string outjson;
  JSONWriter::WriteWithOptions(node, base::JSONWriter::OPTIONS_PRETTY_PRINT,
                               &outjson);
  LOG(ERROR) << "Unrecognized node type! JSON:\n\n"
                "-----------------------\n"
             << outjson << "-----------------------";

  return nullptr;
}

std::unique_ptr<RenderNode> BuildRenderTreeFromFile(
    const base::FilePath& path) {
  LOG(INFO) << "Reading " << path.LossyDisplayName();
  std::string contents;
  if (!ReadFileToString(path, &contents))
    return nullptr;

  int error_code = 0;
  std::string error_message;
  std::unique_ptr<base::DictionaryValue> root = base::DictionaryValue::From(
      JSONReader::ReadAndReturnError(contents, base::JSON_ALLOW_TRAILING_COMMAS,
                                     &error_code, &error_message));
  if (!root) {
    if (error_code) {
      LOG(ERROR) << "Failed to parse JSON file " << path.LossyDisplayName()
                 << "\n(" << error_message << ")";
    } else {
      LOG(ERROR) << path.LossyDisplayName()
                 << " doesn not encode a JSON dictionary.";
    }
    return nullptr;
  }

  return InterpretContentLayer(*root);
}
