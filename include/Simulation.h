#ifndef SIMULATION_H
#define SIMULATION_H

#include <SFML/Graphics.hpp>
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
                return (ld)1 / std::max(std::abs(this->pressure - 1), (ld)0.001);
            }
            ld outResistance() {
                return (ld)1 / std::max(std::abs(this->pressure + 1), (ld)0.001);
            }
        };

        Simulation::pressureCell** pressureField;
        sf::VertexArray* rectArray;
};

#endif // SIMULATION_H
