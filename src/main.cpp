#include "Simulation.h"

int main()
{
    Simulation sim(100, 100, 400);

    while (sim.window->isOpen())
    {
        sf::sleep(sf::seconds(1 / 60));
        sim.eventStep();
        sim.simStep(0.001);
        sim.render();
    }

    return 0;
}
