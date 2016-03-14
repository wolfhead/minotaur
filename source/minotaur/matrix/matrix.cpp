/**
 * @file matrix.cpp
 * @author Wolfhead
 */
#include "matrix.h"
#include <boost/lexical_cast.hpp>
#include "matrix_item_map.h"
#include "matrix_collector.h"

namespace ade { namespace matrix {

LOGGER_CLASS_IMPL(logger, GlobalMatrix);
LOGGER_CLASS_IMPL(logger, Matrix);

Matrix* GlobalMatrix::global_matrix_ = NULL;

int GlobalMatrix::Init(
    uint32_t item_map_bucket,
    uint32_t queue_bucket,
    uint32_t queue_size) {

  if (item_map_bucket & (item_map_bucket - 1)) {
    LOG_ERROR(logger, "GlobalMatrix::Init item_map_bucket error:" << item_map_bucket);
    return -1;
  }

  if (queue_bucket & (queue_bucket - 1)) {
    LOG_ERROR(logger, "GlobalMatrix::Init queue_bucket error:" << queue_bucket);
    return -1;
  }

  if (queue_size & (queue_size - 1)) {
    LOG_ERROR(logger, "GlobalMatrix::Init queue_size error:" << queue_size);
    return -1;
  }

  MatrixItemMap* item_map = new MatrixItemMap(item_map_bucket);
  MatrixCollector* collector = new MatrixCollector(queue_bucket, queue_size);
  Matrix* global_matrix = new Matrix(item_map, collector);

  if (0 != collector->Start()) {
    LOG_ERROR(logger, "GlobalMatrix::Init Collector start fail");
    delete global_matrix;
    return -1;
  }

  global_matrix_ = global_matrix;
  return 0;
}

int GlobalMatrix::InitFromXml(tinyxml2::XMLElement* xml) {
  uint32_t item_map_bucket;
  uint32_t queue_bucket;
  uint32_t queue_size;

  int tmp;
  XML_LOAD_INT(xml, "item_map_bucket", &tmp, -1);
  item_map_bucket = tmp;
  XML_LOAD_INT(xml, "queue_bucket", &tmp, -1);
  queue_bucket = tmp;
  XML_LOAD_INT(xml, "queue_size", &tmp, -1);
  queue_size = tmp;
  return Init(item_map_bucket, queue_bucket, queue_size);
}

int GlobalMatrix::Destroy() {
  if (global_matrix_) {
    delete global_matrix_;
    global_matrix_ = NULL;
  }
  return 0;
}

Matrix::Matrix(MatrixItemMap* item_map, MatrixCollector* collector) 
    : item_map_(item_map)
    , collector_(collector) {
}

Matrix::~Matrix() {
  delete collector_;
  delete item_map_;
}

uint64_t Matrix::MarkBegin(const std::string& name) {
  return item_map_->GenerateToken(name);
}

void Matrix::Set(const std::string& name, uint32_t value) {
  SendToCollector(new MatrixItem(kSet, name, value));
}

void Matrix::Add(const std::string& name, uint32_t value) {
  SendToCollector(new MatrixItem(kAdd, name, value));
}

void Matrix::Sub(const std::string& name, uint32_t value) {
  SendToCollector(new MatrixItem(kSub, name, value));
}

void Matrix::PersistentSet(const std::string& name, uint32_t value) {
  SendToCollector(new MatrixItem(kSet, name, value, true));
}

void Matrix::PersistentAdd(const std::string& name, uint32_t value) {
  SendToCollector(new MatrixItem(kAdd, name, value, true));
}

void Matrix::PersistentSub(const std::string& name, uint32_t value) {
  SendToCollector(new MatrixItem(kSub, name, value, true));
}

void Matrix::Reset(const std::string& name) {
  SendToCollector(new MatrixItem(kReset, name, 0));
}

int Matrix::MarkEnd(uint64_t token) {
  MatrixItem* item;
  if (0 != item_map_->FetchToken(token, &item)) {
    return -1;
  }
  if (0 != SendToCollector(item)) {
    return -1;
  }
  return 0;
}

int Matrix::MarkEnd(uint64_t token, const std::string& result) {
  MatrixItem* item;
  if (0 != item_map_->FetchToken(token, &item)) {
    return -1;
  }

  item->result = result;

  if (0 != SendToCollector(item)) {
    return -1;
  }
  return 0;
}

int Matrix::MarkEnd(uint64_t token, int64_t result) {
  MatrixItem* item;
  if (0 != item_map_->FetchToken(token, &item)) {
    return -1;
  }

  item->result = boost::lexical_cast<std::string>(result);

  if (0 != SendToCollector(item)) {
    return -1;
  }
  return 0;
}

int Matrix::SendToCollector(const MatrixItem* item) {
  if (0 != collector_->Send(item)) {
    LOG_WARN(logger, "Matrix::SendToCollector fail");
    delete item;
    return -1;
  }
  return 0;
}

Matrix::MatrixStatMapPtr Matrix::GetMatrixStatMap() {
  return collector_->GetMatrixStatMap();
}

} //namespace matrix
} //namespace ade
