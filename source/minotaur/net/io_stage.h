#ifndef _MINOTAUR_IO_STAGE_H_
#define _MINOTAUR_IO_STAGE_H_
/**
 * @file io_stage.h
 * @author Wolfhead
 */
#include "../stage.h"

namespace ade {

class IOStage : public Stage {
 protected:
  virtual uint32_t HashMessage(EventMessage) {
    return IODescriptorFactory::GetVersion(message.descriptor_id) % worker_count
  } 
};

} //namespace ade

#endif // _MINOTAUR_IO_STAGE_H_
