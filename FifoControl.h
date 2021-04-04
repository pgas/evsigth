#ifndef FIFO_CONTROL_H
#define FIFO_CONTROL_H

#include "event2/event.h"

class FifoControl {

public:
  FifoControl(struct event_base *base, const char *name);
  ~FifoControl();

  FifoControl(const FifoControl &) = delete;
  FifoControl &operator=(const FifoControl &) = delete;

private:
  friend void readFifo_(evutil_socket_t fd, short event, void *arg);

  void registerFifo_();

  struct event_base *base_;
  const char *name_;
  struct event *evFifo_;
  int fifo_;
};

#endif