#include "Simulation.h"

Simulation::Simulation(int _fieldWidth, int _fieldHeight, int _windowWidth)
{
    // calculate windows height based on field size ratio
    int _windowHeight = (int)((double)_windowWidth * ((double)_fieldHeight / (double)_fieldWidth));

    // initialize object variables
    this->window = new sf::RenderWindow(sf::VideoMode(_windowWidth, _windowHeight), "Lightning simulation", sf::Style::Close);
    this->rectArray = new sf::VertexArray(sf::Quads, _fieldWidth * _fieldHeight * 4);
    this->fieldWidth = _fieldWidth;
    this->fieldHeight = _fieldHeight;
    this->cellSide = _windowWidth / _fieldWidth;
    this->currentDrawMode = Simulation::drawMode::Lightning;
    this->lmbPressed = false;
    this->rmbPressed = false;
    this->brushRadius = 3;
    this->flowMultiplier = 0.001;
    this->interpWindowSize = 2;
    this->cellClearThreshold = 0.005;
    this->interpolationStrength = 0.005;
    this->flowIgnoreThreshold = 0.00001;

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
        delete this->pressureField[i];
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
        // get mouse position, subtract window header height 30 px
        sf::Vector2i mousePos = sf::Mouse::getPosition() - this->window->getPosition();
        int centerX = mousePos.x / this->cellSide;
        int centerY = (mousePos.y - 30) / this->cellSide;
        // add or reduce pressure in this->brushRadius radius of the cursor (currently the brush is square, radius = half a side)
        for (int y = std::max(centerY - this->brushRadius + 1, 1); y < std::min(centerY + this->brushRadius, this->fieldHeight - 1); y++)
        {
            for (int x = std::max(centerX - this->brushRadius + 1, 1); x < std::min(centerX + this->brushRadius, this->fieldWidth - 1); x++)
            {
                if (this->lmbPressed)
                    this->pressureField[y][x].pressure += 0.1;
                if (this->rmbPressed)
                    this->pressureField[y][x].pressure -= 0.1;
                this->pressureField[y][x].pressure = std::clamp(this->pressureField[y][x].pressure, -1.0, 1.0);
            }
        }
    }

    // loop through this->pressureField
    for(int y = 0; y < this->fieldHeight; y++)
    {
        for(int x = 0; x < this->fieldWidth; x++)
        {
            // create links to current cell and its neighbors
            Simulation::pressureCell& cell = this->pressureField[y][x];
            Simulation::pressureCell& cellUp = this->pressureField[y - 1][x];
            Simulation::pressureCell& cellRight = this->pressureField[y][x + 1];
            Simulation::pressureCell& cellDown = this->pressureField[y + 1][x];
            Simulation::pressureCell& cellLeft = this->pressureField[y][x - 1];

            // field borders are used as a buffer, drain their pressure
            if (y == 0 || y == this->fieldHeight - 1 || x == 0 || x == this->fieldWidth - 1)
            {
                cell.pressure *= 0.95;
                continue;
            }

            // reset cells with too little pressure to increase stability
            if (std::abs(cell.pressure) < this->cellClearThreshold)
            {
                cell.pressure = 0;
                cell.flowUp = 0;
                cell.flowRight = 0;
                cell.flowDown = 0;
                cell.flowLeft = 0;
            }

            // adjust air flow based on pressure differences
            cell.flowUp += this->flowMultiplier * (cell.pressure - cellUp.pressure);
            cell.flowRight += this->flowMultiplier * (cell.pressure - cellRight.pressure);
            cell.flowDown += this->flowMultiplier * (cell.pressure - cellDown.pressure);
            cell.flowLeft += this->flowMultiplier * (cell.pressure - cellLeft.pressure);

            // ignore flow if it is below set threshold
            if (std::abs(cell.flowUp) < this->flowIgnoreThreshold) cell.flowUp = 0.0;
            if (std::abs(cell.flowRight) < this->flowIgnoreThreshold) cell.flowRight = 0.0;
            if (std::abs(cell.flowDown) < this->flowIgnoreThreshold) cell.flowDown = 0.0;
            if (std::abs(cell.flowLeft) < this->flowIgnoreThreshold) cell.flowLeft = 0.0;

            // set caps based on current cell pressure
            double minFlowCap = cell.pressure - 1.0;
            double maxFlowCap = cell.pressure + 1.0;

            // clamp flow for current cell
            cell.flowUp = std::clamp(cell.flowUp, minFlowCap, maxFlowCap);
            cell.flowRight = std::clamp(cell.flowRight, minFlowCap, maxFlowCap);
            cell.flowDown = std::clamp(cell.flowDown, minFlowCap, maxFlowCap);
            cell.flowLeft = std::clamp(cell.flowLeft, minFlowCap, maxFlowCap);

            // clamp flow for neighbor cells
            cell.flowUp = std::clamp(cell.flowUp, -1.0 - cellUp.pressure, 1.0 - cellUp.pressure);
            cell.flowRight = std::clamp(cell.flowRight, -1.0 - cellRight.pressure, 1.0 - cellRight.pressure);
            cell.flowDown = std::clamp(cell.flowDown, -1.0 - cellDown.pressure, 1.0 - cellDown.pressure);
            cell.flowLeft = std::clamp(cell.flowLeft, -1.0 - cellLeft.pressure, 1.0 - cellLeft.pressure);

            // transfer air by flow
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

    // interpolate this->pressureField for stability and better looks using window with a set size
    double interpWindowAreaReverse = 1.0 / (this->interpWindowSize * this->interpWindowSize);
    for(int y0 = 1; y0 < this->fieldHeight - this->interpWindowSize; y0++)
    {
        for(int x0 = 1; x0 < this->fieldWidth - this->interpWindowSize; x0++)
        {
            double sum = 0.0;
            // calculate average pressure
            for (int y = y0; y < y0 + this->interpWindowSize; y++)
            {
                for (int x = x0; x < x0 + this->interpWindowSize; x++)
                {
                    sum += this->pressureField[y][x].pressure;
                }
            }
            double avg = sum * interpWindowAreaReverse;
            double newSum = 0.0;

            // set window pressure to average
            for (int y = y0; y < y0 + this->interpWindowSize; y++)
            {
                for (int x = x0; x < x0 + this->interpWindowSize; x++)
                {
                    this->pressureField[y][x].pressure += this->interpolationStrength * (avg - this->pressureField[y][x].pressure);
                    newSum += this->pressureField[y][x].pressure;
                }
            }

            // correct pressure sum within window
            if (newSum == 0.0) continue;
            double correctionMult = sum / newSum;
            for (int y = y0; y < y0 + this->interpWindowSize; y++)
            {
                for (int x = x0; x < x0 + this->interpWindowSize; x++)
                {
                    this->pressureField[y][x].pressure *= correctionMult;
                }
            }
        }
    }
}

void Simulation::render()
{
    this->window->clear();
    // loop through this->pressureField
    for(int y = 0; y < this->fieldHeight; y++)
    {
        for(int x = 0; x < this->fieldWidth; x++)
        {
            double pressure = this->pressureField[y][x].pressure;
            // assign colors to pressure values:
            // 1.0 - blue
            // -1.0 - red
            sf::Color color = (pressure >= 0) ? sf::Color(0, 0, pressure * 255) : sf::Color(-pressure * 255, 0, 0);
            for (int v = (y * this->fieldWidth + x) * 4; v < (y * this->fieldWidth + x) * 4 + 4; v++)
            {
                (*this->rectArray)[v].color = color;
            }
        }
    }
    // draw this->rectArray
    this->window->draw(*this->rectArray);
    this->window->display();
}
