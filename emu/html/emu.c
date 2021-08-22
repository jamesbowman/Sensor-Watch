#include <stdio.h>
#include <emscripten.h>

#include "watch.h"

enum { RUN, SLEEP, BACKUP } state;

void loop() {
  if (state == SLEEP)
    app_wake_from_sleep();
  state = RUN;
  while (!app_loop())
    ;
  app_prepare_for_sleep();
  state = SLEEP;
}

void incoming_event(int e) {
  watch_dispatch_callback(e);
  loop();
}

int main() {
  state = RUN;
  app_setup();
  loop();
  return 0;
}
