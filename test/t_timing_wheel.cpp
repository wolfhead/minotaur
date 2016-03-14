#define BOOST_AUTO_TEST_MAIN

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#define private public
#include <event/timer/timing_wheel.h>
#undef private

using namespace ade;
using namespace ade::event;

BOOST_AUTO_TEST_SUITE(TestTimingWheel);

BOOST_AUTO_TEST_CASE(TestFastLog2) {
  BOOST_CHECK_EQUAL(TimingWheel<void*>::FastLog2(0), 0);
  BOOST_CHECK_EQUAL(TimingWheel<void*>::FastLog2(256), 8);
  BOOST_CHECK_EQUAL(TimingWheel<void*>::FastLog2(1048), 10);
}

BOOST_AUTO_TEST_CASE(TestGetTail) {
  typedef TimingWheel<void*> TW;
  TW::TimingNode node_1;
  TW::TimingNode node_2;
  TW::TimingNode node_3;

  TW::LinkNode(NULL, &node_3);
  TW::LinkNode(&node_3, &node_2);
  TW::LinkNode(&node_2, &node_1);

  BOOST_CHECK(TW::GetTail(NULL) == NULL);
  BOOST_CHECK_EQUAL(TW::GetTail(&node_3), &node_3);
  BOOST_CHECK_EQUAL(TW::GetTail(&node_2), &node_3);
  BOOST_CHECK_EQUAL(TW::GetTail(&node_1), &node_3);
}

BOOST_AUTO_TEST_CASE(TestOneLayer) {
  typedef TimingWheel<int> TW;
  TW tw(256);

  TW::TimingNode* head = NULL;
  TW::TimingNode* node = new TW::TimingNode;
  node->ticks = 1;
  node->data = 1;

  tw.AddNode(node);
  head = tw.Advance(0);
  BOOST_CHECK(head == NULL);

  head = tw.Advance(1);
  BOOST_CHECK(head != NULL);
  BOOST_CHECK(head->next == NULL);
  BOOST_CHECK(head->data == 1);
  delete head;

  head = tw.Advance(456);
  BOOST_CHECK(head == NULL);

  node = new TW::TimingNode;
  node->ticks = 1;
  node->data = 1;
  tw.AddNode(node);
  node = new TW::TimingNode;
  node->ticks = 1;
  node->data = 2;
  tw.AddNode(node);
  node = new TW::TimingNode;
  node->ticks = 255;
  node->data = 3;
  tw.AddNode(node);
  node = new TW::TimingNode;
  node->ticks = 255;
  node->data = 4;
  tw.AddNode(node);

  head = tw.Advance(456);
  BOOST_CHECK(head != NULL);
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK_EQUAL(head->data, 2);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK_EQUAL(head->data, 1);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK_EQUAL(head->data, 4);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next == NULL);
  BOOST_CHECK_EQUAL(head->data, 3);
  delete head;
}

BOOST_AUTO_TEST_CASE(TestTwoLayer) {
  typedef TimingWheel<int> TW;
  TW tw(4);
  TW tw1(4);

  TW::TimingNode* head = NULL;
  TW::TimingNode* node = NULL;

  tw.BindNext(&tw1);

  int j = 100;
  int i = 0;
  while (j--) {
    node = new TW::TimingNode;
    node->ticks = 5;
    node->data = 1;
    node->next = NULL;

    tw.AddNode(node);

    i = 5;
    while (!head && i--) {
      head = tw.Advance(1);
    }

    BOOST_CHECK_EQUAL(i, 0);
    BOOST_CHECK(head != NULL);
    BOOST_CHECK(head->next == NULL);
    delete head;
    head = NULL;

    node = new TW::TimingNode;
    node->ticks = 8;
    node->data = 1;
    node->next = NULL;

    tw.AddNode(node);

    i = 8;
    while (!head && i--) {
      head = tw.Advance(1);
    }

    BOOST_CHECK_EQUAL(i, 0);
    BOOST_CHECK(head != NULL);
    BOOST_CHECK(head->next == NULL);
    BOOST_CHECK(head->data == 1);
    delete head;
    head = NULL;
  }
}

BOOST_AUTO_TEST_CASE(TestThreeLayer) {
  typedef TimingWheel<int> TW;
  TW tw(4);
  TW tw1(4);
  TW tw2(4);


  TW::TimingNode* head = NULL;
  TW::TimingNode* node = NULL;

  tw.BindNext(&tw1);
  tw1.BindNext(&tw2);


  int j = 100;
  int i = 0;
  while (j--) {
    node = new TW::TimingNode;
    node->ticks = 5;
    node->data = 1;
    node->next = NULL;

    tw.AddNode(node);

    i = 5;
    while (!head && i--) {
      head = tw.Advance(1);
    }

    BOOST_CHECK_EQUAL(i, 0);
    BOOST_CHECK(head != NULL);
    BOOST_CHECK(head->next == NULL);
    delete head;
    head = NULL;

    node = new TW::TimingNode;
    node->ticks = 8;
    node->data = 1;
    node->next = NULL;

    tw.AddNode(node);

    i = 8;
    while (!head && i--) {
      head = tw.Advance(1);
    }

    BOOST_CHECK_EQUAL(i, 0);
    BOOST_CHECK(head != NULL);
    BOOST_CHECK(head->next == NULL);
    BOOST_CHECK(head->data == 1);
    delete head;
    head = NULL;

    node = new TW::TimingNode;
    node->ticks = 17;
    node->data = 1;
    node->next = NULL;
    tw.AddNode(node);

    i = 17;
    while (!head && i--) {
      head = tw.Advance(1);
    }

    BOOST_CHECK_EQUAL(i, 0);
    BOOST_CHECK(head != NULL);
    BOOST_CHECK(head->next == NULL);
    delete head;
    head = NULL;

    node = new TW::TimingNode;
    node->ticks = 16;
    node->data = 1;
    node->next = NULL;

    tw.AddNode(node);

    i = 16;
    while (!head && i--) {
      head = tw.Advance(1);
    }

    BOOST_CHECK_EQUAL(i, 0);
    BOOST_CHECK(head != NULL);
    BOOST_CHECK(head->next == NULL);
    BOOST_CHECK(head->data == 1);
    delete head;
    head = NULL;

    node = new TW::TimingNode;
    node->ticks = 63;
    node->data = 1;
    node->next = NULL;

    tw.AddNode(node);

    i = 63;
    while (!head && i--) {
      head = tw.Advance(1);
    }

    BOOST_CHECK_EQUAL(i, 0);
    BOOST_CHECK(head != NULL);
    BOOST_CHECK(head->next == NULL);
    BOOST_CHECK(head->data == 1);
    delete head;
    head = NULL;
  }
}


BOOST_AUTO_TEST_SUITE_END()
