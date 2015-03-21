#ifndef _MINOTAUR_HANDLER_SKELETON_H_
#define _MINOTAUR_HANDLER_SKELETON_H_
/**
 * @file handler_skeleton.h
 * @author Wolfhead
 */
#include "stage.h"

namespace minotaur {

class IOService;

class Handler {
 public:
  enum {
    kUnspecifiedId = 0,
  };

  Handler() : io_service_(NULL) {}
  Handler(const Handler& handler);
  Handler& operator= (const Handler& handler);

  virtual ~Handler() {}

  void SetIOService(IOService* service) {io_service_ = service;}
  IOService* GetIOService() {return io_service_;}

  void Run(StageData* data);

 private:
  IOService* io_service_;
};

class HandlerSkeleton : public Handler {
 public:
  void Run(StageData* data);

 protected:
  void OnStart();
  void OnLoop();
  void OnIdle();
  void Handle(const EventMessage& message);
  void OnStop();
};

} //namespace minotaur

#endif // _MINOTAUR_HANDLER_SKELETON_H_
