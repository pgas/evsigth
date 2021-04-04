#include "EventApplication.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "event2/event.h"
#include "event2/thread.h"

class SignalHandler;
// we need a global variable in the signal handler
static SignalHandler *event_application_signal_handler;
// list of signal handled by the application
static const std::vector<int> event_application_signals{SIGINT, SIGTERM};

class SignalHandler {

public:
  SignalHandler(struct event_base *base, sigset_t *oset) : ending_{false} {
    event_application_signal_handler = this;
    handlerThread_ =
        std::thread([this](struct event_base *base,
                           sigset_t *oset) { this->run_(base, oset); },
                    base, oset);
  }

  void stop(int signal = 0) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      ending_ = true;
      signal_ = signal;
    }
    cv_.notify_one();
  }

  ~SignalHandler() {
    stop();
    handlerThread_.join();
  }

private:
  void run_(struct event_base *base, sigset_t *oset) {
    // restore the original mask in the handler thread
    if (pthread_sigmask(SIG_SETMASK, oset, nullptr) != 0) {
      std::cerr << "Failed to set sigmask" << std::endl;
    }

    struct sigaction sa {};
    sa.sa_handler = [](int i) { event_application_signal_handler->stop(i); };
    sa.sa_mask = *oset;
    for (const int si : event_application_signals) {
      sigaction(si, &sa, nullptr);
    }
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return ending_ || signal_ > 0; });
    if (signal_ > 0) {
      event_base_loopbreak(base);
    }
  }

  std::thread handlerThread_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool ending_;
  int signal_;
};

EventApplication::EventApplication() {
  if (evthread_use_pthreads() == 0) {

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

    // start the thread that will handle the signals
    signalHandler = std::make_unique<SignalHandler>(base_, &oset);

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