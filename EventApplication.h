#ifndef EVENT_APPLICATION_H
#define EVENT_APPLICATION_H

#include <memory>

struct event_base;
class SignalHandlerThread;

class EventApplication {

public:
  EventApplication();
  ~EventApplication();

  EventApplication(const EventApplication &) = delete;
  EventApplication &operator=(const EventApplication &) = delete;

  struct event_base *getBase();
  int run();

private:
  struct event_base *base_;
  std::unique_ptr<SignalHandlerThread> signalHandler;
};

#endif