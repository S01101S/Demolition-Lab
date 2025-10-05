    #include <iostream>
    #include <SFML/Graphics.hpp>
    #include <vector>
    #include <random>



    enum class ShapeType {Circle, Rectangle};


    class Shape
    {
    public:
        virtual ~Shape() = default;
        virtual ShapeType getType() const = 0;
        virtual void draw(sf::RenderWindow& window) = 0;
    };


    class Circle : public Shape
    {
    public:
        sf::CircleShape circleShape;
        float radius;

        Circle(float circleRadius) : radius(circleRadius)
        {
            circleShape.setRadius(radius);
            circleShape.setOrigin(radius, radius);
        }

        ShapeType getType() const override
        {
            return ShapeType::Circle;
        }

        void draw(sf::RenderWindow& window) override
        {
            window.draw(circleShape);
        }

    };


    class Rectangle : public Shape
    {
    public:
        sf::RectangleShape rectangleShape;
        float width, height;

        Rectangle(float rectangleWidth, float rectangleHeight) : width(rectangleWidth), height(rectangleHeight)
        {
            rectangleShape.setSize({width, height});
            rectangleShape.setOrigin(width / 2.0f, height / 2.0f);
        }

        ShapeType getType() const override
        {
            return ShapeType::Rectangle;
        }

        void draw(sf::RenderWindow& window) override
        {
            window.draw(rectangleShape);
        }
    };


    class PhysicsObject
    {
    public:
        sf::Vector2f position;
        sf::Vector2f velocity;
        float radius;
        float mass;
        float restitution = 0.75f;


        std::unique_ptr<Shape> shape;


        PhysicsObject(std::unique_ptr<Shape> s, sf::Vector2f startingPosition, sf::Vector2f startingVelocity) : shape(std::move(s)), position(startingPosition), velocity(startingVelocity)
        {
        }


        virtual ~PhysicsObject() = default;

        void draw(sf::RenderWindow& window)
        {
            if (auto* circle = dynamic_cast<Circle*>(shape.get()))
            {
                circle->circleShape.setPosition(position);
            }
            else if (auto* rect = dynamic_cast<Rectangle*>(shape.get()))
            {
                rect->rectangleShape.setPosition(position);
            }

            shape->draw(window);
        }

        void boundaryChecks(float windowWidth, float windowHeight)
        {
            float halfWidth = 0;
            float halfHeight = 0;

            if (auto* circle=dynamic_cast<Circle*>(this->shape.get()))
            {
                halfWidth = circle->radius;
                halfHeight = circle->radius;
            }
            else if (auto* rectangle=dynamic_cast<Rectangle*>(this->shape.get()))
            {
                halfWidth = rectangle->width / 2.0f;
                halfHeight = rectangle->height / 2.0f;
            }



            if (this->position.y + halfHeight > windowHeight)
            {
                this->position.y = windowHeight - halfHeight;
                this->velocity.y *= -restitution;
            }

            if (this->position.y - halfHeight < 0)
            {
                this->position.y = halfHeight;
                this->velocity.y *= -restitution;
            }

            if (this->position.x + halfWidth > windowWidth)
            {
                this->position.x = windowWidth - halfWidth;
                this->velocity.x *= -restitution;
            }

            if (this->position.x - halfWidth < 0)
            {
                this->position.x = halfWidth;
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

        Stone(sf::Vector2f startingPosition, sf::Vector2f startingVelocity) : PhysicsObject(std::make_unique<Circle>(25.f), startingPosition, startingVelocity)
        {
            this->restitution = 0.5f;
            this->mass *= 4.f;

            if (auto* circle=dynamic_cast<Circle*>(shape.get()))
            {
                circle->circleShape.setFillColor(sf::Color(173, 165, 135));
            }
        }
    };

    class Wood : public PhysicsObject
    {
    public:

        Wood(sf::Vector2f startingPosition, sf::Vector2f startingVelocity) : PhysicsObject(std::make_unique<Rectangle>(80.f, 60.f), startingPosition, startingVelocity)
        {
            this->restitution = 0.3f;
            this->mass *= 2.f;

            if (auto* rectangle=dynamic_cast<Rectangle*>(shape.get()))
            {
                rectangle->rectangleShape.setFillColor(sf::Color(150, 111, 51));
            }
        }

    };



    class Metal : public PhysicsObject
    {
    public:

        Metal(sf::Vector2f startingPosition, sf::Vector2f startingVelocity) : PhysicsObject(std::make_unique<Rectangle>(80.f, 60.f), startingPosition, startingVelocity)
        {
            this->restitution = 0.1f;
            this->mass *= 4.f;

            if (auto* rectangle=dynamic_cast<Rectangle*>(shape.get()))
            {
                rectangle->rectangleShape.setFillColor(sf::Color(192, 192, 192));
            }
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

            objectsList.push_back(std::make_unique<Stone>(stoneStartingPosition, stoneStartingVelocity));
        }

        for (int i=0; i<2; i++)
        {
            std::uniform_real_distribution<float> woodPositionGenerator(100.f, 700.f);
            std::uniform_real_distribution<float> woodVelocityGenerator(-150.f, 150.f);

            sf::Vector2f woodStartingPosition = {woodPositionGenerator(generator), 150.f};
            sf::Vector2f woodStartingVelocity = {woodVelocityGenerator(generator), 0.f};

            objectsList.push_back(std::make_unique<Wood>(woodStartingPosition, woodStartingVelocity));
        }

        for (int i=0; i<2; i++)
        {
            std::uniform_real_distribution<float> metalPositionGenerator(100.f, 700.f);
            std::uniform_real_distribution<float> metalVelocityGenerator(-150.f, 0.f);

            sf::Vector2f metalStartingPosition = {metalPositionGenerator(generator), 150.f};
            sf::Vector2f metalStartingVelocity = {metalVelocityGenerator(generator), 0.f};

            objectsList.push_back(std::make_unique<Metal>(metalStartingPosition, metalStartingVelocity));
        }


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
