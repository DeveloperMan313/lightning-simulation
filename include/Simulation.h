#ifndef SIMULATION_H
#define SIMULATION_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

using ld = long double;

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
            Pressure, Ground, Lightning
        };

        Simulation::drawMode currentDrawMode;
        bool lmbPressed;
        bool rmbPressed;
        int brushRadius;
        int interpolationWindow;
        ld flowMultiplier;
        ld interpolationStrength;
        ld flowDampMultiplier;

        struct pressureCell
        {
            ld pressure = 0;
            ld flowUp = 0;
            ld flowRight = 0;
            ld flowDown = 0;
            ld flowLeft = 0;
            ld inResistance() {
                return (ld)1 / std::abs(this->pressure - 1);
            }
            ld outResistance() {
                return (ld)1 / std::abs(this->pressure + 1);
            }
        };

        Simulation::pressureCell** pressureField;
        sf::VertexArray* rectArray;
};

#endif // SIMULATION_H
