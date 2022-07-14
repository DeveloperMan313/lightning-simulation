#include "Simulation.h"

int main()
{
    // create a simulation with 100 by 100 pressure field and 1000 px window width
    Simulation sim(100, 100, 1000);

    // how much simulation iterations are skipped between frames
    int renderSkipIters = 20;

    // simulation main loop
    int iterRenderCount = 0;
    while (sim.window->isOpen())
    {
        sim.eventStep();
        sim.simStep();
        if (iterRenderCount == renderSkipIters)
        {
            sim.render();
            iterRenderCount = 0;
        }
        iterRenderCount++;
    }

    return 0;
}
