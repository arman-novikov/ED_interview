#include "Ball.h"
#include "MiddleAverageFilter.h"

#include <vector>
#include <unordered_map>
#include <utility>
#include <future>
#include <mutex>

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 299;

Math::MiddleAverageFilter<float,100> fpscounter;

static void draw_ball(sf::RenderWindow& window, const Ball& ball)
{
    sf::CircleShape gball;
    gball.setRadius(ball.GetRadius());
    gball.setPosition(ball.GetX(), ball.GetY());
    window.draw(gball);
}

static void draw_fps(sf::RenderWindow& window, float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    std::string string(c);
    sf::String str(c);
    window.setTitle(str);
}

using colliding_ball_container_t = std::unordered_map<Ball*, Ball*>;

static void validateCollisions(std::vector<Ball*>& balls,
    colliding_ball_container_t& colliding_balls,
    std::mutex& colliding_mtx)
{
    for (auto& ball : balls)
    {        
        for (auto& next : balls) {
            if (next == ball)
            {
                continue;
            }
            if (ball->Displace(*next))
            {
                Ball *k, *v;
                if (ball < next)
                {
                    k = ball;
                    v = next;
                }
                else
                {
                    k = next;
                    v = ball;
                }
                std::scoped_lock lock{ colliding_mtx };
                auto iter = colliding_balls.find(k);
                if (iter == colliding_balls.end() || iter->second != v)
                {
                    colliding_balls[k] = v;
                }

            }
        }
    }
}

static std::vector<Ball> random_balls()
{
    srand(static_cast<unsigned int>(time(NULL)));
    const auto ball_rand = rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS;
    std::vector<Ball> balls{};
    balls.reserve(ball_rand);

    // randomly initialize balls
    for (int i = 0; i < ball_rand; i++)
    {
        balls.emplace_back(
            static_cast<float>(rand() % (WINDOW_X - 50)),
            static_cast<float>(rand() % (WINDOW_Y - 50)),
            static_cast<float>((-5 + (rand() % 10)) / 3.f),
            static_cast<float>((-5 + (rand() % 10)) / 3.f),
            static_cast<float>(5 + rand() % 5),
            static_cast<float>(i == 0 ? 1000.f : 30.0f + rand() % 30)
        );
    }
    return balls;
}

auto get_segmentation_points(int xMax, int yMax, size_t segm_count = 8u)
{
    const size_t n = segm_count / 4u;
    const size_t xStep = static_cast<size_t>(xMax) / n;
    const size_t yStep = static_cast<size_t>(yMax) / n;
    std::vector<std::pair<sf::Vector2f, sf::Vector2f>> res;

    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            const float x0 = static_cast<float>(i * xStep);
            const float x1 = x0 + static_cast<float>(xStep);
            const float y0 = static_cast<float>(j * yStep);
            const float y1 = y0 + static_cast<float>(yStep);
            res.emplace_back(
                sf::Vector2f{ x0, y0 },
                sf::Vector2f{ x1, y1 }
            );
        }
    }
    return res;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    const auto segm_points = get_segmentation_points(WINDOW_X, WINDOW_Y);
    // window.setFramerateLimit(60);
    sf::Clock clock;
    auto balls = random_balls();
    float lastime = clock.restart().asSeconds();
    std::mutex colliding_mutex{};

    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;

        colliding_ball_container_t colliding_balls{};
        std::vector<std::vector<Ball*>> ball_groups{};
        ball_groups.reserve(segm_points.size());

        for (auto& ball : balls)
        {
            ball.ProcessBorderCollision(WINDOW_X, WINDOW_Y);
            ball.Move(deltaTime);
        }

        for (size_t i = 0; i < segm_points.size(); ++i)
        {
            const auto& [start, end] = segm_points[i];
            std::vector<Ball*> ball_group{};
            for (auto& ball : balls)
            {
                if (ball.ContainedIn(start, end))
                {
                    ball_group.push_back(&ball);
                }
            }
            ball_groups.push_back(ball_group);
        }

        // it may be significantly better to use TBB worker pool 
        std::vector<std::future<void>> done;
        for (size_t i = 0; i < ball_groups.size(); ++i)
        {
            done.push_back(
                std::async(
                    std::launch::async,
                    validateCollisions,
                    std::ref(ball_groups[i]),
                    std::ref(colliding_balls),
                    std::ref(colliding_mutex))
            );
        }

        for (size_t i = 0; i < ball_groups.size(); ++i)
        {
            if (done[i].valid())
            {
                done[i].get();
            }
        }

        for (auto& [b1, b2] : colliding_balls)
        {
           b1->Collide(*b2);
        }

        window.clear();
        for (auto& ball : balls)
        {
            draw_ball(window, ball);
        }

		draw_fps(window, fpscounter.getAverage());
		window.display();
    }
    return 0;
}
