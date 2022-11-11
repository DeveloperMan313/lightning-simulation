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
        void setCellColor(int x, int y, const sf::Color& color);

        sf::RenderWindow* window;
        int fieldWidth;
        int fieldHeight;
        int cellSide;

        enum class DrawMode
        {
            Pressure, Ground, Lightning
        };

        struct Mouse
        {
            struct Btn
            {
                bool down = false;
                bool click = false;
            };
            Btn lmb, mmb, rmb;
        };

        int brushRadius;
        int interpolationWindow;
        ld flowMultiplier;
        ld interpolationStrength;
        ld flowDampMultiplier;
        Simulation::DrawMode drawMode;
        Mouse mouse;

        struct PressureCell
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
            bool isGround = false;
        };

        Simulation::PressureCell** pressureField;
        sf::VertexArray* rectArray;
};

#endif // SIMULATION_H
