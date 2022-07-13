#include "Simulation.h"

int main()
{
    Simulation sim(50, 50, 800);

    while (sim.window->isOpen())
    {
        sf::sleep(sf::seconds(1.0 / 500.0));
        sim.eventStep();
        sim.simStep();
        sim.render();
    }

    return 0;
}
