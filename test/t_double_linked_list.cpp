#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <queue/double_linked_list.h>
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
  DoubleLinkedList<Node> list;
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
  DoubleLinkedList<Node> list;
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

BOOST_AUTO_TEST_CASE(testErase) {
  DoubleLinkedList<Node> list;

  Node* node_1 = NULL;
  Node* node_2 = NULL;
  Node* node_3 = NULL;
  Node* node_4 = NULL;
  Node* node_5 = NULL;

  list.push_front(node_3 = new Node(3))
      .push_back(node_4 = new Node(4))
      .push_front(node_2 = new Node(2))
      .push_back(node_5 = new Node(5))
      .push_front(node_1 = new Node(1));

  BOOST_CHECK_EQUAL(list.size(), 5);
  BOOST_CHECK_EQUAL(list.front()->value, 1);
  BOOST_CHECK_EQUAL(list.tail()->value, 5);
  
  list.erase(node_3);
  delete node_3;
  BOOST_CHECK_EQUAL(list.size(), 4);

  list.erase(node_1);
  delete node_1;
  BOOST_CHECK_EQUAL(list.size(), 3);
  BOOST_CHECK_EQUAL(list.front()->value, 2);

  list.erase(node_5);
  delete node_5;
  BOOST_CHECK_EQUAL(list.size(), 2);
  BOOST_CHECK_EQUAL(list.tail()->value, 4);

  list.erase(node_2);
  delete node_2;
  BOOST_CHECK_EQUAL(list.size(), 1);
  BOOST_CHECK_EQUAL(list.front()->value, 4);
  BOOST_CHECK_EQUAL(list.tail()->value, 4);

  list.erase(node_4);
  delete node_4;

  BOOST_CHECK_EQUAL(list.size(), 0);
  BOOST_CHECK(list.front() == NULL);
  BOOST_CHECK(list.tail() == NULL);
}




BOOST_AUTO_TEST_SUITE_END()
