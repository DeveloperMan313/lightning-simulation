#ifndef SIMULATION_H
#define SIMULATION_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>


class Simulation
{
    public:
        Simulation(int _fieldWidth, int _fieldHeight, int _windowWidth);
        virtual ~Simulation();

        void eventStep();
        void simStep();
        void render();

        sf::RenderWindow* window;
        int fieldWidth;
        int fieldHeight;
        int cellSide;

        enum class drawMode
        {
            Pressure, Lightning
        };

        Simulation::drawMode currentDrawMode;
        bool lmbPressed;
        bool shiftPressed;
        int brushRadius;
        double minPressure;
        double maxPressure;
        double flowMultiplier;

        struct pressureCell
        {
            double pressure = 0;
            double flowUp = 0;
            double flowRight = 0;
            double flowDown = 0;
            double flowLeft = 0;
        };

        Simulation::pressureCell** pressureField;
        sf::VertexArray* rectArray;

    protected:

    private:
};

#endif // SIMULATION_H
