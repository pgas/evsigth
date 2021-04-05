#include "EventApplication.h"

#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "event2/event.h"
#include "event2/thread.h"

class SignalHandlerThread {

public:
  SignalHandlerThread(sigset_t *oset) {
    std::thread([=]() {
      // restore the original mask in the handler thread
      if (pthread_sigmask(SIG_SETMASK, oset, nullptr) != 0) {
        std::cerr << "Failed to set sigmask" << std::endl;
      }
      start_.set_value();
      // just wait until it is destroyed
      stop_.get_future().wait();
    }).detach();
    // wait until the thread is started
    start_.get_future().wait();
  }

  SignalHandlerThread(const SignalHandlerThread &) = delete;
  SignalHandlerThread &operator=(const SignalHandlerThread &) = delete;

  ~SignalHandlerThread() { stop_.set_value(); }

private:
  std::promise<void> stop_;
  std::promise<void> start_;
};

EventApplication::EventApplication() {
  if (evthread_use_pthreads() == 0) {
    // list of signal handled by the application
    const std::vector<int> event_application_signals{SIGINT, SIGTERM};

    // Block signals for present and future threads
    sigset_t set;
    sigset_t oset;
    sigemptyset(&set);
    for (const int si : event_application_signals) {
      sigaddset(&set, si);
    }

    if (pthread_sigmask(SIG_BLOCK, &set, &oset) != 0) {
      std::cerr << "Failed to set sigmask" << std::endl;
    }

    // Initialize the event library
    base_ = event_base_new();

    // exit the loop if a signal is received
    for (const int si : event_application_signals) {
      struct event *ev = evsignal_new(
          base_, si,
          [](evutil_socket_t, short, void *arg) {
            event_base_loopbreak(static_cast<struct event_base *>(arg));
          },
          base_);
      if (ev) {
        event_add(ev, NULL);
      }
    }

    // start the thread that will handle the signals
    signalHandler = std::make_unique<SignalHandlerThread>(&oset);

  } else {
    std::cerr << "Failed to init libevent." << std::endl;
  }
}

EventApplication::~EventApplication() {
  if (base_) {
    event_base_free(base_);
    base_ = nullptr;
  }
  libevent_global_shutdown();
}

struct event_base *EventApplication::getBase() {
  return base_;
}

int EventApplication::run() {
  event_base_dispatch(base_);
  return 0;
}