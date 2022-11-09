#include "Simulation.h"

Simulation::Simulation(int _fieldWidth, int _fieldHeight, int _windowWidth)
{
    // calculate windows height based on field size ratio
    int _windowHeight = (int)((ld)_windowWidth * ((ld)_fieldHeight / (ld)_fieldWidth));
    // initialize object variables
    this->window = new sf::RenderWindow(sf::VideoMode(_windowWidth, _windowHeight), "Lightning simulation", sf::Style::Close);
    this->rectArray = new sf::VertexArray(sf::Quads, _fieldWidth * _fieldHeight * 4);
    this->fieldWidth = _fieldWidth;
    this->fieldHeight = _fieldHeight;
    this->cellSide = _windowWidth / _fieldWidth;
    this->currentDrawMode = Simulation::drawMode::Lightning;
    this->lmbPressed = false;
    this->rmbPressed = false;
    this->brushRadius = 5;
    this->interpolationWindow = 2;
    this->flowMultiplier = 0.2;
    this->interpolationStrength = 0.05;
    this->flowDampMultiplier = 0.999;
    // initialize this->pressureField
    this->pressureField = new Simulation::pressureCell*[_fieldHeight];
    for (int i = 0; i < _fieldHeight; i++)
    {
        this->pressureField[i] = new Simulation::pressureCell[_fieldWidth];
        // initialize quad vertex array this->rectArray
        for (int j = 0; j < _fieldWidth; j++)
        {
            int v0 = (i * _fieldWidth + j) * 4;
            int cs = this->cellSide;
            (*this->rectArray)[v0].position = sf::Vector2f(cs * j, cs * i);
            (*this->rectArray)[v0 + 1].position = sf::Vector2f(cs * j + cs, cs * i);
            (*this->rectArray)[v0 + 2].position = sf::Vector2f(cs * j + cs, cs * i + cs);
            (*this->rectArray)[v0 + 3].position = sf::Vector2f(cs * j, cs * i + cs);
        }
    }
}

Simulation::~Simulation()
{
    // clear memory
    delete this->window;
    for(int i = 0; i < this->fieldHeight; i++)
        delete[] this->pressureField[i];
    delete this->pressureField;
    delete this->rectArray;
}

void Simulation::eventStep()
{
    // handle window event
    sf::Event event;
    while (window->pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            window->close();
            break;
        case sf::Event::MouseButtonPressed:
            switch (event.mouseButton.button)
            {
            case sf::Mouse::Button::Left:
                this->lmbPressed = true;
                break;
            case sf::Mouse::Button::Right:
                this->rmbPressed = true;
                break;
            default:
                break;
            }
            break;
        case sf::Event::MouseButtonReleased:
            switch (event.mouseButton.button)
            {
            case sf::Mouse::Button::Left:
                this->lmbPressed = false;
                break;
            case sf::Mouse::Button::Right:
                this->rmbPressed = false;
                break;
            default:
                break;
            }
            break;
        case sf::Event::KeyPressed:
            switch (event.key.code)
            {
            case sf::Keyboard::Key::Num1:
                this->currentDrawMode = Simulation::drawMode::Pressure;
                break;
            case sf::Keyboard::Key::Num2:
                this->currentDrawMode = Simulation::drawMode::Ground;
                break;
            case sf::Keyboard::Key::Num3:
                this->currentDrawMode = Simulation::drawMode::Lightning;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

void Simulation::simStep()
{
    // if lmb is pressed, handle pressure brush
    if (this->lmbPressed || this->rmbPressed)
    {
        // get mouse position, subtract window header height 30 px (tested on windows)
        sf::Vector2i mousePos = sf::Mouse::getPosition() - this->window->getPosition();
        int centerX = mousePos.x / this->cellSide;
        int centerY = (mousePos.y - 30) / this->cellSide;
        // add or reduce pressure in this->brushRadius radius of the cursor (currently the brush is square, radius = half a side)
        for (int y = std::max(centerY - this->brushRadius + 1, 1); y < std::min(centerY + this->brushRadius, this->fieldHeight - 1); y++)
        {
            for (int x = std::max(centerX - this->brushRadius + 1, 1); x < std::min(centerX + this->brushRadius, this->fieldWidth - 1); x++)
            {
                Simulation::pressureCell& cell = this->pressureField[y][x];
                if (this->lmbPressed)
                    cell.pressure += 0.3;
                if (this->rmbPressed)
                    cell.pressure -= 0.3;
                cell.pressure = std::clamp(cell.pressure, (ld)-1, (ld)1);
            }
        }
    }
    // calculate flow
    for(int y = 1; y < this->fieldHeight - 1; y++)
    {
        for(int x = 1; x < this->fieldWidth - 1; x++)
        {
            // create links to current cell and its neighbors
            Simulation::pressureCell& cell = this->pressureField[y][x];
            Simulation::pressureCell& cellUp = this->pressureField[y - 1][x];
            Simulation::pressureCell& cellRight = this->pressureField[y][x + 1];
            Simulation::pressureCell& cellDown = this->pressureField[y + 1][x];
            Simulation::pressureCell& cellLeft = this->pressureField[y][x - 1];
            // precalculate current cell's resistance
            ld outRes = cell.outResistance();
            // update flows where applicable
            if (y != 1) cell.flowUp += (cell.pressure - cellUp.pressure) * this->flowMultiplier / cellUp.inResistance() / outRes;
            if (x != this->fieldHeight - 2) cell.flowRight += (cell.pressure - cellRight.pressure) * this->flowMultiplier / cellRight.inResistance() / outRes;
            if (y != this->fieldHeight - 2) cell.flowDown += (cell.pressure - cellDown.pressure) * this->flowMultiplier / cellDown.inResistance() / outRes;
            if (x != 1) cell.flowLeft += (cell.pressure - cellLeft.pressure) * this->flowMultiplier / cellLeft.inResistance() / outRes;
            // damp flow to make oscillations fade
            cell.flowUp *= this->flowDampMultiplier;
            cell.flowRight *= this->flowDampMultiplier;
            cell.flowDown *= this->flowDampMultiplier;
            cell.flowLeft *= this->flowDampMultiplier;
        }
    }
    // calculate pressure
    for(int y = 1; y < this->fieldHeight - 1; y++)
    {
        for(int x = 1; x < this->fieldWidth - 1; x++)
        {
            // create links to cell and its neighbors
            Simulation::pressureCell& cell = this->pressureField[y][x];
            Simulation::pressureCell& cellUp = this->pressureField[y - 1][x];
            Simulation::pressureCell& cellRight = this->pressureField[y][x + 1];
            Simulation::pressureCell& cellDown = this->pressureField[y + 1][x];
            Simulation::pressureCell& cellLeft = this->pressureField[y][x - 1];
            // clamp flow so pressure stays in bounds
            cell.flowUp = std::clamp(cell.flowUp, std::max(cell.pressure - 1, -cellUp.pressure - 1) * 0.249, std::min(cell.pressure + 1, 1 - cellUp.pressure) * 0.249);
            cell.flowRight = std::clamp(cell.flowRight, std::max(cell.pressure - 1, -cellRight.pressure - 1) * 0.249, std::min(cell.pressure + 1, 1 - cellRight.pressure) * 0.249);
            cell.flowDown = std::clamp(cell.flowDown, std::max(cell.pressure - 1, -cellDown.pressure - 1) * 0.249, std::min(cell.pressure + 1, 1 - cellDown.pressure) * 0.249);
            cell.flowLeft = std::clamp(cell.flowLeft, std::max(cell.pressure - 1, -cellLeft.pressure - 1) * 0.249, std::min(cell.pressure + 1, 1 - cellLeft.pressure) * 0.249);
            // update pressure
            cell.pressure -= (cell.flowUp + cell.flowRight + cell.flowDown + cell.flowLeft);
            cellUp.pressure += cell.flowUp;
            cellRight.pressure += cell.flowRight;
            cellDown.pressure += cell.flowDown;
            cellLeft.pressure += cell.flowLeft;
            // clamp pressure so it definitely stays in bounds
            cell.pressure = std::clamp(cell.pressure, (ld)-1, (ld)1);
            cellUp.pressure = std::clamp(cellUp.pressure, (ld)-1, (ld)1);
            cellRight.pressure = std::clamp(cellRight.pressure, (ld)-1, (ld)1);
            cellDown.pressure = std::clamp(cellDown.pressure, (ld)-1, (ld)1);
            cellLeft.pressure = std::clamp(cellLeft.pressure, (ld)-1, (ld)1);
        }
    }
    // interpolate pressure for better look
    ld interpWindowAreaReverse = 1.0 / (this->interpolationWindow * this->interpolationWindow);
    for (int y0 = 1; y0 < this->fieldHeight - this->interpolationWindow; y0++)
    {
        for(int x0 = 1; x0 < this->fieldWidth - this->interpolationWindow; x0++)
        {
            ld sum = 0.0;
            // calculate average pressure
            for (int y = y0; y < y0 + this->interpolationWindow; y++)
            {
                for (int x = x0; x < x0 + this->interpolationWindow; x++)
                {
                    sum += this->pressureField[y][x].pressure;
                }
            }
            ld avg = sum * interpWindowAreaReverse;
            ld newSum = 0.0;
            // set pressure closer to average
            for (int y = y0; y < y0 + this->interpolationWindow; y++)
            {
                for (int x = x0; x < x0 + this->interpolationWindow; x++)
                {
                    this->pressureField[y][x].pressure += this->interpolationStrength * (avg - this->pressureField[y][x].pressure);
                    newSum += this->pressureField[y][x].pressure;
                }
            }
            // correct to preserve sum
            ld dPressure = (sum - newSum) * interpWindowAreaReverse;
            for (int y = y0; y < y0 + this->interpolationWindow; y++)
            {
                for (int x = x0; x < x0 + this->interpolationWindow; x++)
                {
                    this->pressureField[y][x].pressure += dPressure;
                }
            }
        }
    }
}

void Simulation::render()
{
    for(int y = 0; y < this->fieldHeight; y++)
    {
        for(int x = 0; x < this->fieldWidth; x++)
        {
            ld pressure = this->pressureField[y][x].pressure;
            // assign colors to pressure values:
            // 1.0 - red
            // -1.0 - blue
            sf::Color color = (pressure >= 0) ? sf::Color(pressure * 254, 0, 0) : sf::Color(0, 0, -pressure * 254);
            for (int v = (y * this->fieldWidth + x) * 4; v < (y * this->fieldWidth + x) * 4 + 4; v++)
            {
                (*this->rectArray)[v].color = color;
            }
        }
    }
    // display frame
    this->window->draw(*this->rectArray);
    this->window->display();
}
