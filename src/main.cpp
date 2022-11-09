#include "Simulation.h"

int main()
{
    // create a simulation with 100 by 100 pressure field and 800 px window width
    Simulation sim(100, 100, 800);

    int targetFPS = 60;
    ld frameMs = (ld)1 / targetFPS * 1000;
    // simulation main loop
    while (sim.window->isOpen())
    {
        sf::Clock frame;
        sim.eventStep();
        sim.simStep();
        sim.render();
        ld elapsed = frame.getElapsedTime().asMilliseconds();
        // wait for next frame if needed
        if (elapsed < frameMs) {
            sf::sleep(sf::milliseconds(frameMs - elapsed));
        }
    }
    return 0;
}
