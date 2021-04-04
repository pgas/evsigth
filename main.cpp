#include "EventApplication.h"
#include "FifoControl.h"

#include <iostream>

int main(int, char **) {

  EventApplication app;
  FifoControl fifo(app.getBase(), "myfifo");

  std::cout << "Starting loop !\n";
  int ret = app.run();
  std::cout << "Loop  Ending!\n";

  return ret;
}
