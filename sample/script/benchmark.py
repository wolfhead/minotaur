from gevent import monkey; monkey.patch_all()
import gevent
import random
import importlib
import datetime

from log import logger

def GetClass(module_name, class_name):
  module_obj = __import__(module_name)
  if None == module_obj:
    logger.error("no module found: %s" % module_name)
    return None

  class_obj = getattr(module_obj, class_name)
  if None == class_obj:
    logger.error("no class found: %s" % class_name)
    return None

  return class_obj()

class SimpleStringGenerator(object):
  def gen(self):
    return random.choice('abcdefghijklmnopqrstuvwxyz!@#$%^&*()')

  def match(self, request, response):
    return request == response


class BenchMark(object):
  def __init__(self, module_name, class_name, concurreny_level, request_count):
    self._module_name = module_name
    self._class_name = class_name
    self._concurreny_level = concurreny_level
    self._request_count = request_count
    self._coro_pool = []
    self._finish_count = 0
    self._success_count = 0
    self._fail_count = 0
    self._latency = datetime.timedelta()
    self._run_time = datetime.timedelta()

  def Run(self, host, port, request_gen):
    start = datetime.datetime.now()
    for i in xrange(self._concurreny_level):
      self._coro_pool.append(gevent.spawn(BenchMark.Process, self, \
        host, port, request_gen))

    gevent.joinall(self._coro_pool)
    end = datetime.datetime.now()
    self._run_time = (end - start)

  def Process(self, host, port, gen):
    client = GetClass(self._module_name, self._class_name)
    if client is None:
      raise RuntimeError("BenchMark Connect fail: ({0}:{1})".format(host, port))

    client.connect(host, port)

    while self._request_count >= self._finish_count :
      start = datetime.datetime.now()
      request = gen.gen()
      response = client.send_recv(request)
      if gen.match(request, response):
        self._success_count += 1
      else:
        self._fail_count += 1
      self._finish_count += 1;
      end = datetime.datetime.now()
      self._latency += (end - start)

  def Summary(self):
    print "run time: {0}".format(self._run_time)
    print "success count {0}".format(self._success_count)
    print "fail count {0}".format(self._fail_count)
    print "average latency: {0}".format(self._latency / self._finish_count)

if __name__ == "__main__" :
  benchmark = BenchMark("rapid_client", "RapidClient", 128, 100000)
  benchmark.Run("localhost", "6601", SimpleStringGenerator())

  benchmark.Summary()