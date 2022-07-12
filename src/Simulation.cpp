#include "Simulation.h"

Simulation::Simulation(int _fieldWidth, int _fieldHeight, int _windowWidth)
{
    int _windowHeight = (int)((float)_windowWidth * ((float)_fieldHeight / (float)_fieldWidth));
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
    this->cursorRadius = 5;
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

void Simulation::simStep(float deltaT)
{
    if (this->lmbPressed)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition() - this->window->getPosition();
        int centerX = mousePos.x / this->cellSide;
        int centerY = (mousePos.y - 30) / this->cellSide;
        for (int y = std::max(centerY - this->cursorRadius + 1, 1); y < std::min(centerY + this->cursorRadius, this->fieldHeight - 1); y++)
        {
            for (int x = std::max(centerX - this->cursorRadius + 1, 1); x < std::min(centerX + this->cursorRadius, this->fieldWidth - 1); x++)
            {
                if (this->shiftPressed)
                    this->pressureField[y][x].pressure -= 0.005;
                else this->pressureField[y][x].pressure += 0.005;
            }
        }
    }


    //simulation here


    for(int y = 0; y < this->fieldHeight; y++)
    {
        for(int x = 0; x < this->fieldWidth; x++)
        {
            this->pressureField[y][x].pressure = std::max(-1.f, std::min(this->pressureField[y][x].pressure, 1.f));
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
            float pressure = this->pressureField[y][x].pressure;
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
