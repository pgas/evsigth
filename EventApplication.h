#ifndef EVENT_APPLICATION_H
#define EVENT_APPLICATION_H

#include <memory>
#include <vector>

struct event_base;
struct event;
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
  std::vector<struct event *> signalEvents;
  std::unique_ptr<SignalHandlerThread> signalHandler_;
};

#endif