#include <iostream>
#include <vector>
#include <cassert>
#include <cstdlib>

#define main game_main
#include "../main.cpp"
#undef main

const int POINTS_PER_FRUIT = 10;
const int BOARD_SIZE = 20;

// StubPositionSource for Rule 2 Seam Test
class StubPositionSource : public PositionSource {
    std::vector<Pos> positions;
    size_t index;
public:
    StubPositionSource(const std::vector<Pos>& posList) : positions(posList), index(0) {}
    Pos next(const Snake& s, int size) override {
        (void)s;
        (void)size;
        if (index < positions.size()) {
            return positions[index++];
        }
        return Pos(0, 0);
    }
};

void test_rule1_self_collision() {
    // Rule 1: The snake dies after colliding with its own body.
    Snake s(10, 10);
    const int GROW_STEPS = 3;

    // Grow snake to length 6 (head + 5 body segments)
    for (int i = 0; i < GROW_STEPS; ++i) {
        s.grow();
        s.move();
    }

    // Move in a loop: UP, LEFT, DOWN into own body
    s.setDir(UP);
    s.move();
    s.setDir(LEFT);
    s.move();

    assert(!s.hitSelf() && "Snake must not report hitSelf before final collision move");

    s.setDir(DOWN);
    s.move(); // Head moves into body

    assert(s.hitSelf() && "Snake must report hitSelf when head collides with own body");
    std::cout << "Rule 1 Test Passed: Self collision correctly detected." << std::endl;
}

void test_rule2_score_increase() {
    // Rule 2: Eating one fruit increases the score by exactly 10 points.
    // Initial snake is at (10,10) moving RIGHT.
    // Initial fruit (consumed at Game constructor): Pos(11, 10) directly in front of head.
    // Next fruit: Pos(0, 0) far away.
    std::vector<Pos> predeterminedPositions = { Pos(11, 10), Pos(0, 0) };
    StubPositionSource stub(predeterminedPositions);

    Game g(BOARD_SIZE, &stub);

    assert(g.score() == 0 && "Initial game score must be 0");

    // Move 1: Head moves from (10,10) to (11,10) and eats fruit
    g.update();

    assert(g.score() == POINTS_PER_FRUIT && "Eating one fruit must increase score by exactly 10 points");

    // Move 2: Head moves from (11,10) to (12,10), no fruit eaten
    g.update();

    assert(g.score() == POINTS_PER_FRUIT && "Moving without eating fruit must not change score");
    std::cout << "Rule 2 Test Passed: Score increases by exactly 10 points per fruit." << std::endl;
}

void test_rule3_fruit_placement() {
    // Rule 3: Fruit never appears on a cell occupied by the snake.
    srand(42);
    Snake s(10, 10);
    Food f;
    const int SPAWN_TRIALS = 100;

    for (int i = 0; i < SPAWN_TRIALS; ++i) {
        f.spawn(s, BOARD_SIZE);
        assert(!s.onPos(f.pos()) && "Fruit must never spawn on a cell occupied by the snake");
    }
    std::cout << "Rule 3 Test Passed: Fruit never lands on snake." << std::endl;
}

void test_rule4_direction_reversal() {
    // Rule 4: The snake cannot immediately reverse into the opposite direction.
    Snake s(10, 10);
    assert(s.getDirection() == RIGHT && "Initial snake direction must be RIGHT");

    // Attempt direct reversal to LEFT
    s.setDir(LEFT);
    assert(s.getDirection() == RIGHT && "Snake setDir must ignore immediate opposite direction reversal");

    // Change to orthogonal direction UP
    s.setDir(UP);
    s.move();
    assert(s.getDirection() == UP && "Snake setDir must accept orthogonal direction change");
    std::cout << "Rule 4 Test Passed: Immediate direction reversal is prevented." << std::endl;
}

void test_rule5_wall_collision() {
    // Rule 5: The snake dies after colliding with the wall boundary.
    Game g(BOARD_SIZE);

    assert(!g.over() && "New game must not start in game over state");

    const int MAX_STEPS = BOARD_SIZE * 2;
    int steps = 0;
    while (!g.over() && steps < MAX_STEPS) {
        g.update();
        steps++;
    }

    assert(g.over() && "Snake moving straight must collide with wall boundary and trigger game over");
    std::cout << "Rule 5 Test Passed: Wall collision triggers game over." << std::endl;
}

int main() {
    test_rule1_self_collision();
    test_rule2_score_increase();
    test_rule3_fruit_placement();
    test_rule4_direction_reversal();
    test_rule5_wall_collision();
    std::cout << "\nAll 5 rule tests executed and passed successfully!" << std::endl;
    return 0;
}
