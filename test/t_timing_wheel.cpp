#define BOOST_AUTO_TEST_MAIN

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#define private public
#include <event/timer/timing_wheel.h>
#undef private

using namespace minotaur;

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
  node->ticks = 0;
  node->data = 1;

  tw.AddNode(0, node);
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
  node->ticks = 0;
  node->data = 1;
  tw.AddNode(1, node);
  node = new TW::TimingNode;
  node->ticks = 0;
  node->data = 2;
  tw.AddNode(1, node);
  node = new TW::TimingNode;
  node->ticks = 0;
  node->data = 3;
  tw.AddNode(255, node);
  node = new TW::TimingNode;
  node->ticks = 0;
  node->data = 4;
  tw.AddNode(255, node);

  head = tw.Advance(456);
  BOOST_CHECK(head != NULL);
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK(head->data == 2);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK(head->data == 1);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK(head->data == 4);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next == NULL);
  BOOST_CHECK(head->data == 3);
  delete head;
}

BOOST_AUTO_TEST_CASE(TestThreeLayer) {
  typedef TimingWheel<int> TW;
  TW tw(64);
  TW tw1(64);
  TW tw2(64);

  tw.BindNext(&tw1);
  tw1.BindNext(&tw2);

  TW::TimingNode* head = NULL;
  TW::TimingNode* node = NULL;

  node = new TW::TimingNode;
  node->ticks = 0;
  node->data = 1;

  tw.AddNode(0, node);
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
  node->ticks = 0;
  node->data = 1;
  tw.AddNode(1, node);
  node = new TW::TimingNode;
  node->ticks = 0;
  node->data = 2;
  tw.AddNode(1, node);
  node = new TW::TimingNode;
  node->ticks = 255;
  node->data = 3;
  tw.AddNode(255, node);
  node = new TW::TimingNode;
  node->ticks = 255;
  node->data = 4;
  tw.AddNode(255, node);

  head = tw.Advance(256);
  BOOST_CHECK(head != NULL);
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK(head->data == 2);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next != NULL);
  BOOST_CHECK(head->data == 1);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next != NULL);
  node = head;
  head = head->next;
  delete node;
  BOOST_CHECK(head->next == NULL);
  delete head;

  tw.Reset();
  node = new TW::TimingNode;
  node->ticks = 64;
  node->data = 1;
  tw.AddNode(64, node);

  head = tw.Advance(64 + 1);
  BOOST_CHECK(head != NULL);
  BOOST_CHECK(head->next == NULL);
  BOOST_CHECK(head->data == 1);
  delete head;


  node = new TW::TimingNode;
  node->ticks = 64 * 64;
  node->data = 1;
  tw.Reset();
  tw.AddNode(64 * 64, node);

  head = tw.Advance(64 * 64 + 1);
  BOOST_CHECK(head != NULL);
  BOOST_CHECK(head->next == NULL);
  BOOST_CHECK(head->data == 1);
  delete head;
}

BOOST_AUTO_TEST_SUITE_END()
