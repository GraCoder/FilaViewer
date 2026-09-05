#include "TOperator.h"
#include <SDL3/SDL_events.h>

namespace fv {


bool TOperator::handle(TView *view, const SDL_Event *event)
{
  if (!view || !event)
    return false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    return mousePress(view, event->button);
  case SDL_EVENT_MOUSE_BUTTON_UP:
    return mouseRelease(view, event->button);
  case SDL_EVENT_MOUSE_WHEEL:
    return mouseWheel(view, event->wheel);
  case SDL_EVENT_MOUSE_MOTION:
    return mouseMove(view, event->motion);
  case SDL_EVENT_KEY_DOWN:
    return keyPress(view, event->key);
  case SDL_EVENT_KEY_UP:
    return keyRelease(view, event->key);
  default:
    return false;
  }
}

} // namespace fv
