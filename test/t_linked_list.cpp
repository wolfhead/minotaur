#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <queue/linked_list.h>
#include <common/logger.h>
#include "unittest_logger.h"

using namespace minotaur;
using namespace minotaur::unittest;

static minotaur::unittest::UnittestLogger logger_config;
LOGGER_STATIC_DECL_IMPL(logger, "root");

BOOST_AUTO_TEST_SUITE(TestDoubleLinkedList);

struct Node {
  Node(int _value) : value(_value) {}
  int value;
  Node* prev_;
  Node* next_;
};

BOOST_AUTO_TEST_CASE(testPushFront) {
  LinkedList<Node> list;
  Node* node = NULL;

  list.push_front(new Node(1))
      .push_front(new Node(2))
      .push_front(new Node(3))
      .push_front(new Node(4))
      .push_front(new Node(5));

  BOOST_CHECK_EQUAL(list.size(), 5);
  BOOST_CHECK_EQUAL(list.front()->value, 5);
  BOOST_CHECK_EQUAL(list.tail()->value, 1);

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 5);
  BOOST_CHECK_EQUAL(list.size(), 4);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 4);
  BOOST_CHECK_EQUAL(list.size(), 3);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 3);
  BOOST_CHECK_EQUAL(list.size(), 2);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 2);
  BOOST_CHECK_EQUAL(list.size(), 1);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 1);
  BOOST_CHECK_EQUAL(list.size(), 0);
  delete node;
  
  BOOST_CHECK(list.front() == NULL);
  BOOST_CHECK(list.tail() == NULL);
}

BOOST_AUTO_TEST_CASE(testPushTail) {
  LinkedList<Node> list;
  Node* node = NULL;

  list.push_back(new Node(5))
      .push_back(new Node(4))
      .push_back(new Node(3))
      .push_back(new Node(2))
      .push_back(new Node(1));

  BOOST_CHECK_EQUAL(list.size(), 5);
  BOOST_CHECK_EQUAL(list.front()->value, 5);
  BOOST_CHECK_EQUAL(list.tail()->value, 1);

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 5);
  BOOST_CHECK_EQUAL(list.size(), 4);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 4);
  BOOST_CHECK_EQUAL(list.size(), 3);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 3);
  BOOST_CHECK_EQUAL(list.size(), 2);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 2);
  BOOST_CHECK_EQUAL(list.size(), 1);
  delete node;

  node = list.front();
  list.pop_front();
  BOOST_CHECK_EQUAL(node->value, 1);
  BOOST_CHECK_EQUAL(list.size(), 0);
  delete node;
  
  BOOST_CHECK(list.front() == NULL);
  BOOST_CHECK(list.tail() == NULL);
}

BOOST_AUTO_TEST_SUITE_END()
