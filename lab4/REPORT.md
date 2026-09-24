# Lab 4 Report

## Section 1 – Five Behaviour Rules

1. The snake dies after colliding with its own body.
2. Eating one fruit increases the score by exactly 10 points.
3. Fruit never appears on a cell occupied by the snake.
4. The snake cannot immediately reverse into the opposite direction.
5. The snake dies after colliding with the wall boundary.

---

## Section 2 – Testability Analysis

| Rule | Test written? | Blocking dependency |
| :--- | :---: | :--- |
| **Rule 1**: The snake dies after colliding with its own body. | **Yes** | None (`Snake::hitSelf()` at main.cpp:171 is a public method). |
| **Rule 2**: Eating one fruit increases the score by exactly 10 points. | **No** | Blocked by private `Game::sc` at main.cpp:200, private `Game::fd` at main.cpp:202, `rand()` call inside `Food::spawn()` at main.cpp:191, and score update in `Game::update()` at main.cpp:274. No public score getter exists in `lab4-base`. |
| **Rule 3**: Fruit never appears on a cell occupied by the snake. | **Yes** | None (`Food::spawn()` at main.cpp:189 and `Snake::onPos()` at main.cpp:178 are public). |
| **Rule 4**: The snake cannot immediately reverse into the opposite direction. | **Yes** | None (`Snake::setDir()` at main.cpp:150 and `Snake::getDirection()` at main.cpp:148 are public). |
| **Rule 5**: The snake dies after colliding with the wall boundary. | **Yes** | None (`Game::update()` at main.cpp:265 and `Game::over()` at main.cpp:338 are public). |

### Summary
**4 out of 5** behaviour rules are testable without changing the source code (`main.cpp`).

---

## Section 3 – Coverage

| Item | Value |
| :--- | :--- |
| **Line coverage** | **35.27%** |
| **Branch coverage** | **41.32%** |
| **Missing verification** | Snake body length growth at main.cpp:273 (`sn->grow()`) executes when eating fruit during `test_rule2_score_increase()`, but no assertion checks that the snake body size increased. |

---

## Section 4 – The Seam

| Seam Property | Implementation Details |
| :--- | :--- |
| **Seam kind** | **Object Seam** (`PositionSource` polymorphic interface) |
| **Enabling point** | `Game` constructor dependency injection (`Game(int size, PositionSource* src = nullptr)`) |
| **What production code gave up** | Real costs: (1) an owning raw pointer `PositionSource* posSource` and `ownPosSource` flag, (2) heap allocation of default source when null, (3) virtual method call overhead on each spawn, (4) `score()` accessor weakening `Game`'s encapsulation of `sc`. |

### Trade-off Evaluation
These production costs are fully acceptable because fruit spawning occurs infrequently (only on fruit consumption), heap allocation is limited to a single lightweight instance, virtual call overhead is negligible compared to frame sleep delays, and `score()` is a read-only getter.

*Note*: The initial fruit position is spawned during `Game` construction (`fd->spawn(*sn, size, *posSource)`), consuming the first position provided by the `PositionSource`.

### Commit Structure
1. **Commit 1 (Seam only)**: Added `PositionSource` interface and `DefaultPositionSource`, injected dependency into `Game`, and added `score()` getter in `main.cpp`. Zero behavioral or gameplay changes for players.
2. **Commit 2 (Tests only)**: Added `lab4/tests.cpp` and `lab4/Makefile` containing test suite for all 5 rules. Zero modifications to production source code (`main.cpp`).

---

## Section 5 – Test Double

**Test Double Used**: **Stub** (`StubPositionSource`).

**Justification**: The `Game` object asks a question by calling `posSource->next(s, size)` to obtain fruit coordinates (a return value). The collaborator is not told to perform an action nor verified for interaction side-effects, so we control the return value and do not assert on call execution.

---

## Section 6 – Test Smells

| Smell | Location | Description & Fix |
| :--- | :--- | :--- |
| **Magic Number** | lab4/tests.cpp:60 | Hardcoded raw numeric coordinate `Pos(11, 10)` used directly in initial fruit position list.<br>**Fix**: Replace with `const Pos INITIAL_FRUIT_POS = Pos(11, 10);`. |
| **Assertion Roulette** | lab4/tests.cpp:88 | Plain `assert(!s.onPos(f.pos()));` inside loop without a custom failure string.<br>**Fix**: Pass descriptive error string `assert(!s.onPos(f.pos()) && "Fruit position must not overlap snake body");`. |
