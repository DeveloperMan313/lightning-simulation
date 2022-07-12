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
        void simStep(float deltaT);
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
        int cursorRadius;

        float simTime = 0;

        struct pressureCell
        {
            float pressure = 0;
            float flowUp = 0;
            float flowRight = 0;
            float flowDown = 0;
            float flowLeft = 0;
        };

        Simulation::pressureCell** pressureField;
        sf::VertexArray* rectArray;

    protected:

    private:
};

#endif // SIMULATION_H
