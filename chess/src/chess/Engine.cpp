#include "Engine.h"

namespace chess {

Engine::Engine() { reset(); }
void Engine::reset() { board.setStartPos(); }

}
