#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>


class PhysicsObject
{
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;
    float mass;
    float restitution = 0.75f;
    sf::CircleShape shape;

    PhysicsObject(sf::Vector2f startingPosition, sf::Vector2f startingVelocity, float objectRadius)
    {
        this->radius = objectRadius;
        this->position = startingPosition;
        this->velocity = startingVelocity;
        this->mass = this->radius * this->radius;

        this->shape.setRadius(this->radius);
        this->shape.setOrigin(this->radius, this->radius);
    }

    virtual ~PhysicsObject() = default;

    void draw(sf::RenderWindow& window)
    {
        shape.setPosition(position);
        window.draw(shape);
    }

    void boundaryChecks(float windowWidth, float windowHeight)
    {
        if (this->position.y + this->radius > windowHeight)
        {
            this->position.y = windowHeight - this->radius;
            this->velocity.y *= -restitution;
        }

        if (this->position.y - this->radius < 0)
        {
            this->position.y = this->radius;
            this->velocity.y *= -restitution;
        }

        if (this->position.x + this->radius > windowWidth)
        {
            this->position.x = windowWidth - this->radius;
            this->velocity.x *= -restitution;
        }

        if (this->position.x - this->radius < 0)
        {
            this->position.x = this->radius;
            this->velocity.x *= -restitution;
        }
    }

    void update(float gravity, float dt)
    {
        this->velocity.y += gravity * dt;
        this->position += this->velocity * dt;
    }
};


void collisionDetection(std::vector<std::unique_ptr<PhysicsObject>>& objectsList)
{
    for (int i=0; i<objectsList.size(); i++)
    {
        for (int j=i+1; j<objectsList.size(); j++)
        {

            PhysicsObject* obj1 = objectsList[i].get();
            PhysicsObject* obj2 = objectsList[j].get();

            float dx = obj1->position.x - obj2->position.x;
            float dy = obj1->position.y - obj2->position.y;
            float distanceSquared = (dx * dx) + (dy * dy);
            float sumOfRadii = obj1->radius + obj2->radius;
            float radisSquared = sumOfRadii * sumOfRadii;

            if (distanceSquared < radisSquared)
            {
                float distance = sqrt(distanceSquared);
                float overLapDistance = sumOfRadii - distance;
                float normalX = dx / distance;
                float normalY = dy / distance;
                float speedOfObj1Normal = (obj1->velocity.x * normalX) + (obj1->velocity.y * normalY);
                float speedOfObj2Normal = (obj2->velocity.x * normalX) + (obj2->velocity.y * normalY);
                float newVelocityObj1 = (speedOfObj1Normal * (obj1->mass - obj2->mass) + 2 * obj2->mass * speedOfObj2Normal) / (obj1->mass + obj2->mass);
                float newVelocityObj2 = (speedOfObj2Normal * (obj2->mass - obj1->mass) + 2 * obj1->mass * speedOfObj1Normal) / (obj2->mass + obj1->mass);
                float changeInVelocityObj1 = newVelocityObj1 - speedOfObj1Normal;
                float changeInVelocityObj2 = newVelocityObj2 - speedOfObj2Normal;

                if (speedOfObj1Normal - speedOfObj2Normal < 0)
                {
                    obj1->position.x -= normalX * (overLapDistance / 2.0f);
                    obj1->position.y -= normalY * (overLapDistance / 2.0f);
                    obj2->position.x += normalX * (overLapDistance / 2.0f);
                    obj2->position.y += normalY * (overLapDistance / 2.0f);

                    obj1->velocity.x += changeInVelocityObj1 * normalX;
                    obj1->velocity.y += changeInVelocityObj1 * normalY;
                    obj2->velocity.x += changeInVelocityObj2 * normalX;
                    obj2->velocity.y += changeInVelocityObj2 * normalY;
                }
            }
        }
    }
}



class Stone : public PhysicsObject
{
public:
    Stone(sf::Vector2f startingPosition, sf::Vector2f startingVelocity, float objectRadius) : PhysicsObject(startingPosition, startingVelocity, objectRadius)
    {
        this->restitution = 0.5f;
        this->mass *= 4.f;
        this->shape.setFillColor(sf::Color(173, 165, 135));
    }
};

class Wood : public PhysicsObject
{
public:
    Wood(sf::Vector2f startingPosition, sf::Vector2f startingVelocity, float objectRadius) : PhysicsObject(startingPosition, startingVelocity, objectRadius)
    {
        this->restitution = 0.3f;
        this->mass *= 2.f;
        this->shape.setFillColor(sf::Color(150, 111, 51));
    }
};

class Metal : public PhysicsObject
{
public:
    Metal(sf::Vector2f startingPosition, sf::Vector2f startingVelocity, float objectRadius) : PhysicsObject(startingPosition, startingVelocity, objectRadius)
    {
        this->restitution = 0.1f;
        this->mass *= 4.f;
        this->shape.setFillColor((sf::Color(192, 192, 192)));
    }
};





int main()
{
    const float windowWidth = 800.f;
    const float windowHeight = 600.f;
    const float gravity = 981.f;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Demolition Lab");
    window.setFramerateLimit(60);

    std::random_device rd;
    std::mt19937 generator(rd());

    std::vector<std::unique_ptr<PhysicsObject>> objectsList;


    for (int i=0; i<2; i++)
    {
        std::uniform_real_distribution<float> stonePositionGenerator(100.f, 700.f);
        std::uniform_real_distribution<float> stoneVelocityGenerator(-150.f, 150.f);

        sf::Vector2f stoneStartingPosition = {stonePositionGenerator(generator), 150.f};
        sf::Vector2f stoneStartingVelocity = {stoneVelocityGenerator(generator), 0.f};

        objectsList.push_back(std::make_unique<Stone>(stoneStartingPosition, stoneStartingVelocity, 25.f));
    }

    for (int i=0; i<2; i++)
    {
        std::uniform_real_distribution<float> woodPositionGenerator(100.f, 700.f);
        std::uniform_real_distribution<float> woodVelocityGenerator(-150.f, 150.f);

        sf::Vector2f woodStartingPosition = {woodPositionGenerator(generator), 150.f};
        sf::Vector2f woodStartingVelocity = {woodVelocityGenerator(generator), 0.f};

        objectsList.push_back(std::make_unique<Wood>(woodStartingPosition, woodStartingVelocity, 25.f));
    }

    for (int i=0; i<2; i++)
    {
        std::uniform_real_distribution<float> metalPositionGenerator(100.f, 700.f);
        std::uniform_real_distribution<float> metalVelocityGenerator(-150.f, 0.f);

        sf::Vector2f metalStartingPosition = {metalPositionGenerator(generator), 150.f};
        sf::Vector2f metalStartingVelocity = {metalVelocityGenerator(generator), 0.f};

        objectsList.push_back(std::make_unique<Metal>(metalStartingPosition, metalStartingVelocity, 25.f));
    }


    sf::Clock clock;
    float dt = 1.f / 60.f;

    while(window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear(sf::Color::Black);

        for (auto& currObj : objectsList)
        {
            currObj->update(gravity, dt);
            currObj->boundaryChecks(windowWidth, windowHeight);
            collisionDetection(objectsList);
            currObj->draw(window);
        }


        window.display();

    }

    return 0;
}
