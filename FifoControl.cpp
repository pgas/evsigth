#include "FifoControl.h"

#include <iostream>
#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

FifoControl::FifoControl(struct event_base *base, const char *name)
    : base_{base}, name_{name}, evFifo_{nullptr}, fifo_{-1} {
  unlink(name_);
  if (mkfifo(name_, 0600) < 0) {
    std::cerr << "Failed to get fifo `" << name_ << "'." << std::endl;
  } else {
    registerFifo_();
  }
}

FifoControl::~FifoControl() {
  if (fifo_ > 0) {
    close(fifo_);
  }
  if (evFifo_) {
    event_del(evFifo_);
    event_free(evFifo_);
  }
  unlink(name_);
}

void readFifo_(evutil_socket_t fd, short event, void *arg) {
  char buf[255];
  int len;

  auto this_ = static_cast<FifoControl *>(arg);

  len = read(fd, buf, sizeof(buf) - 1);

  if (len > 0) {
    buf[len] = '\0';
    std::cerr << " read  " << buf << std::endl;

    if (std::string(buf) == "quit\n") {
      event_base_loopbreak(this_->base_);
    }
  } else {
    event_del(this_->evFifo_);
    event_free(this_->evFifo_);
    this_->evFifo_ = nullptr;
    if (len == 0) {
      // fifo was closed re-opening it
      this_->registerFifo_();
    }
  }
}

void FifoControl::registerFifo_() {
  fifo_ = open(name_, O_RDONLY | O_NONBLOCK, 0);
  if (fifo_ >= 0) {
    evFifo_ = event_new(base_, fifo_, EV_READ | EV_PERSIST, readFifo_, this);
    event_add(evFifo_, NULL);
  }
}