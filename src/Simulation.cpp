#include "Simulation.h"

Simulation::Simulation(int _fieldWidth, int _fieldHeight, int _windowWidth)
{
    int _windowHeight = (int)((double)_windowWidth * ((double)_fieldHeight / (double)_fieldWidth));
    this->window = new sf::RenderWindow(sf::VideoMode(_windowWidth, _windowHeight), "Lightning simulation", sf::Style::Close);
    this->rectArray = new sf::VertexArray(sf::Quads, _fieldWidth * _fieldHeight * 4);
    this->fieldWidth = _fieldWidth;
    this->fieldHeight = _fieldHeight;
    this->cellSide = _windowWidth / _fieldWidth;
    this->pressureField = new Simulation::pressureCell*[_fieldHeight];
    for (int i = 0; i < _fieldHeight; i++)
    {
        this->pressureField[i] = new Simulation::pressureCell[_fieldWidth];
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
    this->currentDrawMode = Simulation::drawMode::Lightning;
    this->lmbPressed = false;
    this->shiftPressed = false;
    this->brushRadius = 3;
    this->minPressure = -1.0;
    this->maxPressure = 1.0;
    this->flowMultiplier = 0.001;
}

Simulation::~Simulation()
{
    delete this->window;
    for(int i = 0; i < this->fieldHeight; i++)
        delete this->pressureField[i];
    delete this->pressureField;
    delete this->rectArray;
}

void Simulation::eventStep()
{
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
            default:
                break;
            }
            break;
        case sf::Event::KeyPressed:
            switch (event.key.code)
            {
            case sf::Keyboard::Key::LShift:
            case sf::Keyboard::Key::RShift:
                this->shiftPressed = true;
                break;
            default:
                break;
            }
            break;
        case sf::Event::KeyReleased:
            switch (event.key.code)
            {
            case sf::Keyboard::Key::LShift:
            case sf::Keyboard::Key::RShift:
                this->shiftPressed = false;
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
    if (this->lmbPressed)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition() - this->window->getPosition();
        int centerX = mousePos.x / this->cellSide;
        int centerY = (mousePos.y - 30) / this->cellSide;
        for (int y = std::max(centerY - this->brushRadius + 1, 1); y < std::min(centerY + this->brushRadius, this->fieldHeight - 1); y++)
        {
            for (int x = std::max(centerX - this->brushRadius + 1, 1); x < std::min(centerX + this->brushRadius, this->fieldWidth - 1); x++)
            {
                if (this->shiftPressed)
                    this->pressureField[y][x].pressure -= 0.05;
                else this->pressureField[y][x].pressure += 0.05;
                this->pressureField[y][x].pressure = std::clamp(this->pressureField[y][x].pressure, this->minPressure, this->maxPressure);
            }
        }
    }

    for(int y = 0; y < this->fieldHeight; y++)
    {
        for(int x = 0; x < this->fieldWidth; x++)
        {
            Simulation::pressureCell& cell = this->pressureField[y][x];
            Simulation::pressureCell& cellUp = this->pressureField[y - 1][x];
            Simulation::pressureCell& cellRight = this->pressureField[y][x + 1];
            Simulation::pressureCell& cellDown = this->pressureField[y + 1][x];
            Simulation::pressureCell& cellLeft = this->pressureField[y][x - 1];
            if (y == 0 || y == this->fieldHeight - 1 || x == 0 || x == this->fieldWidth - 1)
            {
                cell.pressure = 0;
                cell.flowUp = 0;
                cell.flowRight = 0;
                cell.flowDown = 0;
                cell.flowLeft = 0;
                continue;
            }
            if (std::abs(cell.pressure) < 0.001)
            {
                cell.pressure = 0;
                cell.flowUp = 0;
                cell.flowRight = 0;
                cell.flowDown = 0;
                cell.flowLeft = 0;
            }
            cell.flowUp += this->flowMultiplier * (cell.pressure - cellUp.pressure);
            cell.flowRight += this->flowMultiplier * (cell.pressure - cellRight.pressure);
            cell.flowDown += this->flowMultiplier * (cell.pressure - cellDown.pressure);
            cell.flowLeft += this->flowMultiplier * (cell.pressure - cellLeft.pressure);

            double minFlowCap = cell.pressure - this->maxPressure;
            double maxFlowCap = cell.pressure - this->minPressure;

            cell.flowUp = std::clamp(cell.flowUp, minFlowCap, maxFlowCap);
            cell.flowRight = std::clamp(cell.flowRight, minFlowCap, maxFlowCap);
            cell.flowDown = std::clamp(cell.flowDown, minFlowCap, maxFlowCap);
            cell.flowLeft = std::clamp(cell.flowLeft, minFlowCap, maxFlowCap);

            cell.flowUp = std::clamp(cell.flowUp, this->minPressure - cellUp.pressure, this->maxPressure - cellUp.pressure);
            cell.flowRight = std::clamp(cell.flowRight, this->minPressure - cellRight.pressure, this->maxPressure - cellRight.pressure);
            cell.flowDown = std::clamp(cell.flowDown, this->minPressure - cellDown.pressure, this->maxPressure - cellDown.pressure);
            cell.flowLeft = std::clamp(cell.flowLeft, this->minPressure - cellLeft.pressure, this->maxPressure - cellLeft.pressure);

            cell.pressure -= cell.flowUp;
            cellUp.pressure += cell.flowUp;
            cell.pressure -= cell.flowRight;
            cellRight.pressure += cell.flowRight;
            cell.pressure -= cell.flowDown;
            cellDown.pressure += cell.flowDown;
            cell.pressure -= cell.flowLeft;
            cellLeft.pressure += cell.flowLeft;
        }
    }
}

void Simulation::render()
{
    this->window->clear();
    for(int y = 0; y < this->fieldHeight; y++)
    {
        for(int x = 0; x < this->fieldWidth; x++)
        {
            double pressure = this->pressureField[y][x].pressure;
            sf::Color color = (pressure >= 0) ? sf::Color(0, pressure * 255, 0) : sf::Color(-pressure * 255, 0, 0);
            for (int v = (y * this->fieldWidth + x) * 4; v < (y * this->fieldWidth + x) * 4 + 4; v++)
            {
                (*this->rectArray)[v].color = color;
            }
        }
    }
    this->window->draw(*this->rectArray);
    this->window->display();
}
