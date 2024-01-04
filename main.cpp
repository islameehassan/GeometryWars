#include "src/Vec2.h"
#include <memory>
#include "src/Game.h"

int main(){
    Game g("config.txt", "scores.txt");
    g.run();
}


// Vec2 unit testing
// Vec2 v1(1, 3), v2(4, 5), v3(6, 9);

// std::cout << v1 + v2 << '\n';
// std::cout << v1 - v2 << '\n';
// std::cout << v1 * 6.6 << '\n';
// std::cout << v2 / 3.2 << '\n';

// v1 += v2;
// v3 *= 5.0;

// std::cout << v1 << '\n';
// std::cout << v3 << '\n';

// std::cout << v2.length() << '\n';
// std::cout << v2.distance(v3) << '\n';

// v1.normalize();

// std::cout << v1 << '\n';