#include "chess/Engine.h"
#include "gui/Gui.h"

int main() {
    chess::Engine engine;
    Gui gui(engine);
    gui.run();
    return 0;
}
