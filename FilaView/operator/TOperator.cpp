#include "TOperator.h"
#include <SDL2/SDL_events.h>

namespace fv {


bool TOperator::handle(TView *view, const SDL_Event *event)
{
  if (!view || !event)
    return false;
  switch (event->type) {
  case SDL_MOUSEBUTTONDOWN:
    return mousePress(view, event->button);
  case SDL_MOUSEBUTTONUP:
    return mouseRelease(view, event->button);
  case SDL_MOUSEWHEEL:
    return mouseWheel(view, event->wheel);
  case SDL_MOUSEMOTION:
    return mouseMove(view, event->motion);
  case SDL_KEYDOWN:
    return keyPress(view, event->key);
  case SDL_KEYUP:
    return keyRelease(view, event->key);
  default:
    return false;
  }
}

} // namespace fv
