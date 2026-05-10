#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <fstream>

using namespace std;
using namespace sf;

// Forward declaration
class Enemy;

typedef void (*SpawnCallback)(Enemy* e, void* ctx);

// ============================================================================
// CONSTANTS
// ============================================================================
const int TILE_SIZE = 50;
const int GRID_WIDTH = 16;
const int GRID_HEIGHT = 12;
const int WINDOW_WIDTH = GRID_WIDTH * TILE_SIZE;
const int WINDOW_HEIGHT = GRID_HEIGHT * TILE_SIZE + 130;

const int STARTING_GOLD = 300;
const int STARTING_LIVES = 20;

const int CANNON_COST = 50;
const int SNIPER_COST = 80;
const int MACHINEGUN_COST = 60;
const int SLOW_COST = 100;
const int FLAME_COST = 120;

const float CANNON_DAMAGE = 50.f;
const float SNIPER_DAMAGE = 100.f;
const float MACHINEGUN_DAMAGE = 10.f;
const float SLOW_DAMAGE = 0.f;
const float FLAME_DAMAGE = 15.f;

const float CANNON_RANGE = 150.f;
const float SNIPER_RANGE = 300.f;
const float MACHINEGUN_RANGE = 120.f;
const float SLOW_RANGE = 100.f;
const float FLAME_RANGE = 100.f;

const float CANNON_FIRERATE = 1.5f;
const float SNIPER_FIRERATE = 2.5f;
const float MACHINEGUN_FIRERATE = 0.15f;
const float SLOW_FIRERATE = 1.0f;
const float FLAME_FIRERATE = 0.5f;

const float PROJECTILE_SPEED = 400.f;
const float UPGRADE_MULTIPLIER = 1.15f;

const float UI_Y = GRID_HEIGHT * TILE_SIZE;
const float BTN_W = 110.f;
const float BTN_H = 90.f;
const float BTN_X0 = 15.f;
const float BTN_GAP = 15.f;

static const Color TOWER_COLORS[5] = {
    Color(70,  110, 210),
    Color(50,  170,  60),
    Color(210, 110,  30),
    Color(110,  60, 210),
    Color(220,  70,  15)
};

// ============================================================================
// ENTITY
// ============================================================================
class Entity {
protected:
    Vector2f position;
    float maxHealth, health;
    bool alive;
public:
    Entity(const Vector2f& pos, float maxHp)
        : position(pos), maxHealth(maxHp), health(maxHp), alive(true) {
    }
    virtual ~Entity() = default;
    virtual void update(float dt) = 0;
    virtual void render(RenderWindow& window) = 0;
    virtual void takeDamage(float dmg) {
        health -= dmg;
        if (health <= 0) { health = 0; alive = false; }
    }
    bool     isAlive()      const { return alive; }
    Vector2f getPosition()  const { return position; }
    float    getHealth()    const { return health; }
    float    getMaxHealth() const { return maxHealth; }
};

// ============================================================================
// ENEMY base
// ============================================================================
class Enemy : public Entity {
protected:
    float speed, baseSpeed;
    int currentWaypoint, rewardGold;
    SpawnCallback spawnCallback;
    void* spawnContext;

public:
    Enemy(const Vector2f& startPos, float hp, float spd, int gold,
        SpawnCallback cb = nullptr, void* ctx = nullptr)
        : Entity(startPos, hp), speed(spd), baseSpeed(spd),
        currentWaypoint(0), rewardGold(gold),
        spawnCallback(cb), spawnContext(ctx) {
    }

    virtual void move(float dt, const vector<Vector2f>& path) = 0;

    void applySlow(float factor, float) { speed = baseSpeed * factor; }

    bool reachedEnd(const Vector2f& exit) const {
        return hypot(position.x - exit.x, position.y - exit.y) < 5.f;
    }

    int getReward() const { return rewardGold; }

    void setSpawnCallback(SpawnCallback cb, void* ctx) {
        spawnCallback = cb;
        spawnContext = ctx;
    }

    void trySpawn(Enemy* e) {
        if (spawnCallback) spawnCallback(e, spawnContext);
    }
};

// ============================================================================
// BASIC ENEMY types
// ============================================================================
class BasicEnemy : public Enemy {
public:
    BasicEnemy(const Vector2f& startPos, SpawnCallback cb = nullptr, void* ctx = nullptr)
        : Enemy(startPos, 100, 80, 10, cb, ctx) {
    }

    void move(float dt, const vector<Vector2f>& path) override {
        if (currentWaypoint >= (int)path.size()) return;
        Vector2f tgt = path[currentWaypoint];
        Vector2f dir = tgt - position;
        float dist = hypot(dir.x, dir.y);
        if (dist < 2.f) { currentWaypoint++; return; }
        dir /= dist;
        position += dir * speed * dt;
    }

    void update(float) override {}

    void render(RenderWindow& window) override {
        CircleShape body(12);
        body.setFillColor(Color(220, 60, 60));
        body.setOutlineColor(Color(255, 120, 120));
        body.setOutlineThickness(1.5f);
        body.setPosition({ position.x - 12.f, position.y - 12.f });
        window.draw(body);
        drawHealthBar(window);
    }

protected:
    void drawHealthBar(RenderWindow& window) {
        RectangleShape hpBg({ 24.f, 4.f });
        hpBg.setFillColor(Color(80, 20, 20));
        hpBg.setPosition({ position.x - 12.f, position.y - 18.f });
        window.draw(hpBg);

        float ratio = health / maxHealth;
        Color c = (ratio > 0.5f) ? Color(60, 200, 60) :
            (ratio > 0.25f) ? Color(220, 180, 40) : Color(220, 60, 60);

        RectangleShape hpBar({ 24.f * ratio, 4.f });
        hpBar.setFillColor(c);
        hpBar.setPosition({ position.x - 12.f, position.y - 18.f });
        window.draw(hpBar);
    }
};

class FastEnemy : public BasicEnemy {
public:
    FastEnemy(const Vector2f& startPos, SpawnCallback cb = nullptr, void* ctx = nullptr)
        : BasicEnemy(startPos, cb, ctx) {
        maxHealth = health = 40;
        speed = baseSpeed = 160;
        rewardGold = 12;
    }

    void render(RenderWindow& window) override {
        CircleShape body(9);
        body.setFillColor(Color(220, 210, 40));
        body.setOutlineColor(Color(255, 240, 120));
        body.setOutlineThickness(1.5f);
        body.setPosition({ position.x - 9.f, position.y - 9.f });
        window.draw(body);
        drawHealthBar(window);
    }
};

class TankEnemy : public BasicEnemy {
public:
    TankEnemy(const Vector2f& startPos, SpawnCallback cb = nullptr, void* ctx = nullptr)
        : BasicEnemy(startPos, cb, ctx) {
        maxHealth = health = 400;
        speed = baseSpeed = 40;
        rewardGold = 25;
    }

    void render(RenderWindow& window) override {
        RectangleShape body({ 26.f, 26.f });
        body.setFillColor(Color(100, 100, 110));
        body.setOutlineColor(Color(180, 180, 200));
        body.setOutlineThickness(2.f);
        body.setPosition({ position.x - 13.f, position.y - 13.f });
        window.draw(body);
        drawHealthBar(window);
    }
};

class FlyingEnemy : public Enemy {
    Vector2f targetExit;
public:
    FlyingEnemy(const Vector2f& startPos, const Vector2f& exit,
        SpawnCallback cb = nullptr, void* ctx = nullptr)
        : Enemy(startPos, 80, 100, 15, cb, ctx), targetExit(exit) {
    }

    void move(float dt, const vector<Vector2f>&) override {
        Vector2f dir = targetExit - position;
        float dist = hypot(dir.x, dir.y);
        if (dist < 5.f) { position = targetExit; return; }
        dir /= dist;
        position += dir * speed * dt;
    }

    void update(float) override {}

    void render(RenderWindow& window) override {
        RectangleShape body({ 14.f, 14.f });
        body.setFillColor(Color(40, 200, 220));
        body.setOrigin({ 7.f, 7.f });
        body.setRotation(sf::degrees(45.f));
        body.setPosition({ position.x, position.y });
        window.draw(body);
    }
};

class RegeneratingEnemy : public BasicEnemy {
    float regenRate = 5.f;
public:
    RegeneratingEnemy(const Vector2f& startPos, SpawnCallback cb = nullptr, void* ctx = nullptr)
        : BasicEnemy(startPos, cb, ctx) {
        maxHealth = health = 150;
        speed = baseSpeed = 60;
        rewardGold = 20;
    }

    void update(float dt) override {
        if (alive && health < maxHealth) {
            health += regenRate * dt;
            if (health > maxHealth) health = maxHealth;
        }
    }

    void render(RenderWindow& window) override {
        CircleShape body(12);
        body.setFillColor(Color(180, 40, 180));
        body.setPosition({ position.x - 12.f, position.y - 12.f });
        window.draw(body);
        drawHealthBar(window);
    }
};

// ============================================================================
// SPECIAL ENEMIES
// ============================================================================
class ShieldedEnemy : public BasicEnemy {
    float shieldHP = 80.f, maxShield = 80.f;
public:
    ShieldedEnemy(const Vector2f& startPos, SpawnCallback cb = nullptr, void* ctx = nullptr)
        : BasicEnemy(startPos, cb, ctx) {
        maxHealth = health = 120;
        speed = baseSpeed = 70;
        rewardGold = 18;
    }

    void takeDamage(float dmg) override {
        if (shieldHP > 0) {
            float half = dmg * 0.5f;
            float blocked = (shieldHP < half) ? shieldHP : half;
            shieldHP -= blocked;
            dmg -= blocked;
        }
        health -= dmg;
        if (health <= 0) { health = 0; alive = false; }
    }

    void update(float dt) override {
        if (alive && shieldHP < maxShield) {
            shieldHP += 2.f * dt;
            if (shieldHP > maxShield) shieldHP = maxShield;
        }
    }

    void render(RenderWindow& window) override {
        CircleShape body(14);
        body.setFillColor(Color(100, 180, 255));
        body.setOutlineColor(Color(200, 230, 255));
        body.setOutlineThickness(2.f);
        body.setPosition({ position.x - 14.f, position.y - 14.f });
        window.draw(body);

        RectangleShape shieldBg({ 28.f, 3.f });
        shieldBg.setFillColor(Color(0, 0, 80));
        shieldBg.setPosition({ position.x - 14.f, position.y - 22.f });
        window.draw(shieldBg);

        RectangleShape shieldBar({ 28.f * (shieldHP / maxShield), 3.f });
        shieldBar.setFillColor(Color(100, 200, 255));
        shieldBar.setPosition({ position.x - 14.f, position.y - 22.f });
        window.draw(shieldBar);

        drawHealthBar(window);
    }
};

class SplittingEnemy : public BasicEnemy {
public:
    SplittingEnemy(const Vector2f& startPos, SpawnCallback cb = nullptr, void* ctx = nullptr)
        : BasicEnemy(startPos, cb, ctx) {
        maxHealth = health = 80;
        speed = baseSpeed = 100;
        rewardGold = 15;
    }

    void takeDamage(float dmg) override {
        health -= dmg;
        if (health <= 0 && alive) {
            alive = false;
            trySpawn(new FastEnemy(Vector2f{ position.x - 10, position.y }, spawnCallback, spawnContext));
            trySpawn(new FastEnemy(Vector2f{ position.x + 10, position.y }, spawnCallback, spawnContext));
        }
    }

    void render(RenderWindow& window) override {
        CircleShape body(12);
        body.setFillColor(Color(220, 140, 40));
        body.setPosition({ position.x - 12.f, position.y - 12.f });
        window.draw(body);
        drawHealthBar(window);
    }
};

class HealingEnemy : public BasicEnemy {
    float auraRadius = 120.f;
    float healRate = 8.f;
    vector<Enemy*>* allEnemies = nullptr;
public:
    HealingEnemy(const Vector2f& startPos, SpawnCallback cb = nullptr, void* ctx = nullptr)
        : BasicEnemy(startPos, cb, ctx) {
        maxHealth = health = 100;
        speed = baseSpeed = 50;
        rewardGold = 22;
    }

    void setEnemyList(vector<Enemy*>* list) { allEnemies = list; }

    void update(float dt) override {
        if (!alive || !allEnemies) return;
        for (Enemy* e : *allEnemies) {
            if (e == this || !e->isAlive()) continue;
            float dist = hypot(position.x - e->getPosition().x,
                position.y - e->getPosition().y);
            if (dist <= auraRadius && e->getHealth() < e->getMaxHealth())
                e->takeDamage(-healRate * dt);
        }
    }

    void render(RenderWindow& window) override {
        CircleShape aura(auraRadius);
        aura.setOrigin({ auraRadius, auraRadius });
        aura.setPosition(position);
        aura.setFillColor(Color(50, 255, 50, 30));
        window.draw(aura);

        CircleShape body(12);
        body.setFillColor(Color(100, 220, 100));
        body.setPosition({ position.x - 12.f, position.y - 12.f });
        window.draw(body);
        drawHealthBar(window);
    }
};

// ============================================================================
// PROJECTILE
// ============================================================================
class Projectile {
    Vector2f position, velocity;
    float damage, speed;
    Enemy* target;
    bool active;
    CircleShape shape;
    bool slowEffect;
    float slowFactor, slowDuration, lifeTimer;
public:
    Projectile(const Vector2f& startPos, float dmg, float spd, Enemy* tgt,
        bool slow = false, float slowFac = 1.0f, float slowDur = 0.f)
        : position(startPos), damage(dmg), speed(spd), target(tgt),
        active(true), slowEffect(slow), slowFactor(slowFac),
        slowDuration(slowDur), lifeTimer(3.f) {
        shape.setRadius(4.f);
        shape.setFillColor(slowEffect ? Color::Blue : Color::Yellow);
        shape.setPosition({ position.x - 4.f, position.y - 4.f });
        if (target) {
            Vector2f dir = target->getPosition() - position;
            float len = hypot(dir.x, dir.y);
            if (len > 0.f) velocity = (dir / len) * speed;
        }
        else { velocity = { 0.f, -speed }; }
    }

    void update(float dt) {
        if (!active) return;
        if (target && !target->isAlive()) target = nullptr;
        if (target) {
            Vector2f dir = target->getPosition() - position;
            float len = hypot(dir.x, dir.y);
            if (len > 0.f) velocity = (dir / len) * speed;
        }
        position += velocity * dt;
        shape.setPosition({ position.x - 4.f, position.y - 4.f });

        if (target) {
            float dist = hypot(position.x - target->getPosition().x,
                position.y - target->getPosition().y);
            if (dist < 15.f) {
                target->takeDamage(damage);
                if (slowEffect) target->applySlow(slowFactor, slowDuration);
                active = false;
                target = nullptr;
            }
        }

        if (!target) {
            lifeTimer -= dt;
            if (lifeTimer <= 0.f) active = false;
        }
    }

    void render(RenderWindow& window) { if (active) window.draw(shape); }
    bool isActive() const { return active; }
};

// ============================================================================
// TOWER base
// ============================================================================
class Tower : public Entity {
protected:
    float range, fireRate, damage, timeSinceLastShot;
    float baseRange, baseFireRate, baseDamage;
    int level, maxLevel;
    int upgradeCostBase;
    CircleShape rangeIndicator;
    Color towerColor;

public:
    Tower(const Vector2f& pos, float maxHp, float rng, float rate,
        float dmg, Color col, int upCostBase)
        : Entity(pos, maxHp), range(rng), fireRate(rate), damage(dmg),
        timeSinceLastShot(0.f), baseRange(rng), baseFireRate(rate),
        baseDamage(dmg), level(1), maxLevel(3),
        upgradeCostBase(upCostBase), towerColor(col) {
        rangeIndicator.setRadius(range);
        rangeIndicator.setOrigin({ range, range });
        rangeIndicator.setFillColor(Color(255, 255, 255, 18));
        rangeIndicator.setOutlineColor(Color(255, 255, 255, 40));
        rangeIndicator.setOutlineThickness(1.f);
        rangeIndicator.setPosition(pos);
    }

    virtual void attack(vector<Enemy*>& enemies, vector<Projectile>& projectiles) = 0;

    void update(float dt) override { timeSinceLastShot += dt; }

    void render(RenderWindow& window) override {
        RectangleShape base({ 30.f, 30.f });
        base.setOrigin({ 15.f, 15.f });
        base.setPosition(position);
        base.setFillColor(towerColor);
        base.setOutlineColor(Color(40, 40, 40));
        base.setOutlineThickness(2.f);
        window.draw(base);

        CircleShape core(8.f);
        core.setOrigin({ 8.f, 8.f });
        core.setPosition(position);
        core.setFillColor(Color(200, 200, 200));
        window.draw(core);

        for (int i = 0; i < level; i++) {
            CircleShape star(3.f);
            star.setFillColor(Color::Yellow);
            star.setPosition({ position.x - 15.f + i * 8.f, position.y - 20.f });
            window.draw(star);
        }
    }

    void renderRange(RenderWindow& window) const { window.draw(rangeIndicator); }

    float getRange()   const { return range; }
    int   getLevel()   const { return level; }
    int   getMaxLevel() const { return maxLevel; }
    int   getUpgradeCost() const {
        return (level < maxLevel) ? upgradeCostBase * level : 0;
    }

    virtual void upgrade() {
        if (level >= maxLevel) return;
        level++;
        damage = baseDamage * powf(UPGRADE_MULTIPLIER, (float)(level - 1));
        range = baseRange * powf(UPGRADE_MULTIPLIER, (float)(level - 1));
        fireRate = baseFireRate / powf(UPGRADE_MULTIPLIER, (float)(level - 1));
        rangeIndicator.setRadius(range);
        rangeIndicator.setOrigin({ range, range });
    }
};

// ============================================================================
// TOWER types
// ============================================================================
class CannonTower : public Tower {
public:
    CannonTower(const Vector2f& pos)
        : Tower(pos, 100, CANNON_RANGE, CANNON_FIRERATE, CANNON_DAMAGE,
            TOWER_COLORS[0], CANNON_COST) {
    }

    void attack(vector<Enemy*>& enemies, vector<Projectile>& projectiles) override {
        if (timeSinceLastShot < fireRate) return;
        Enemy* tgt = nullptr;
        float minDist = range;
        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            float d = hypot(e->getPosition().x - position.x, e->getPosition().y - position.y);
            if (d <= range && d < minDist) { minDist = d; tgt = e; }
        }
        if (tgt) {
            projectiles.push_back(Projectile(position, damage, PROJECTILE_SPEED, tgt));
            timeSinceLastShot = 0.f;
        }
    }
};

class SniperTower : public Tower {
public:
    SniperTower(const Vector2f& pos)
        : Tower(pos, 120, SNIPER_RANGE, SNIPER_FIRERATE, SNIPER_DAMAGE,
            TOWER_COLORS[1], SNIPER_COST) {
    }

    void attack(vector<Enemy*>& enemies, vector<Projectile>& projectiles) override {
        if (timeSinceLastShot < fireRate) return;
        Enemy* tgt = nullptr;
        float minDist = range;
        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            float d = hypot(e->getPosition().x - position.x, e->getPosition().y - position.y);
            if (d <= range && d < minDist) { minDist = d; tgt = e; }
        }
        if (tgt) {
            projectiles.push_back(Projectile(position, damage, PROJECTILE_SPEED * 1.5f, tgt));
            timeSinceLastShot = 0.f;
        }
    }
};

class MachineGunTower : public Tower {
public:
    MachineGunTower(const Vector2f& pos)
        : Tower(pos, 80, MACHINEGUN_RANGE, MACHINEGUN_FIRERATE, MACHINEGUN_DAMAGE,
            TOWER_COLORS[2], MACHINEGUN_COST) {
    }

    void attack(vector<Enemy*>& enemies, vector<Projectile>& projectiles) override {
        if (timeSinceLastShot < fireRate) return;
        Enemy* tgt = nullptr;
        float minDist = range;
        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            float d = hypot(e->getPosition().x - position.x, e->getPosition().y - position.y);
            if (d <= range && d < minDist) { minDist = d; tgt = e; }
        }
        if (tgt) {
            projectiles.push_back(Projectile(position, damage, PROJECTILE_SPEED * 2.f, tgt));
            timeSinceLastShot = 0.f;
        }
    }
};

class SlowTower : public Tower {
public:
    SlowTower(const Vector2f& pos)
        : Tower(pos, 150, SLOW_RANGE, SLOW_FIRERATE, SLOW_DAMAGE,
            TOWER_COLORS[3], SLOW_COST) {
    }

    void attack(vector<Enemy*>& enemies, vector<Projectile>&) override {
        if (timeSinceLastShot < fireRate) return;
        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            float d = hypot(e->getPosition().x - position.x, e->getPosition().y - position.y);
            if (d <= range) e->applySlow(0.5f, 1.0f);
        }
        timeSinceLastShot = 0.f;
    }
};

class FlameTower : public Tower {
    float laserTimer = 0.f;
    vector<Vector2f> laserTargets;
public:
    FlameTower(const Vector2f& pos)
        : Tower(pos, 100, FLAME_RANGE, FLAME_FIRERATE, FLAME_DAMAGE,
            TOWER_COLORS[4], FLAME_COST) {
    }

    void attack(vector<Enemy*>& enemies, vector<Projectile>&) override {
        if (timeSinceLastShot < fireRate) return;
        laserTargets.clear();
        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            float d = hypot(e->getPosition().x - position.x, e->getPosition().y - position.y);
            if (d <= range) {
                e->takeDamage(damage);
                laserTargets.push_back(e->getPosition());
            }
        }
        laserTimer = 0.1f;
        timeSinceLastShot = 0.f;
    }

    void update(float dt) override {
        Tower::update(dt);
        if (laserTimer > 0.f) laserTimer -= dt;
    }

    void render(RenderWindow& window) override {
        Tower::render(window);
        if (laserTimer > 0.f) {
            for (const Vector2f& tgt : laserTargets) {
                VertexArray line(sf::PrimitiveType::Lines, 2);
                line[0].position = position; line[0].color = Color(50, 255, 50, 180);
                line[1].position = tgt;      line[1].color = Color(50, 255, 50, 80);
                window.draw(line);
            }
        }
    }
};

// ============================================================================
// MAP
// ============================================================================
class Map {
    vector<vector<int>> tileGrid;
    vector<Vector2f>    waypoints;
    Vector2f entryPoint, exitPoint;
    RectangleShape tile;
public:
    Map() : tile(Vector2f((float)TILE_SIZE, (float)TILE_SIZE)) {
        tileGrid.resize(GRID_HEIGHT, vector<int>(GRID_WIDTH, 0));
    }

    void buildLayout(int id) {
        tileGrid.assign(GRID_HEIGHT, vector<int>(GRID_WIDTH, 0));
        waypoints.clear();

        switch (id) {
        case 0: // Classic
            for (int x = 0; x <= 3; x++) {
                tileGrid[5][x] = 1;
                waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                      5 * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int y = 6; y <= 8; y++) {
                tileGrid[y][3] = 1;
                waypoints.push_back({ 3 * TILE_SIZE + TILE_SIZE / 2.f,
                                      y * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int x = 4; x <= 10; x++) {
                tileGrid[8][x] = 1;
                waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                      8 * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int y = 7; y >= 3; y--) {
                tileGrid[y][10] = 1;
                waypoints.push_back({ 10 * TILE_SIZE + TILE_SIZE / 2.f,
                                       y * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int x = 11; x <= 15; x++) {
                tileGrid[3][x] = 1;
                waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                      3 * TILE_SIZE + TILE_SIZE / 2.f });
            }
            break;

        case 1: // Spiral
            for (int x = 1; x < 14; x++) {
                tileGrid[1][x] = 1;
                waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                      1 * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int y = 2; y < 10; y++) {
                tileGrid[y][13] = 1;
                waypoints.push_back({ 13 * TILE_SIZE + TILE_SIZE / 2.f,
                                       y * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int x = 12; x >= 2; x--) {
                tileGrid[9][x] = 1;
                waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                      9 * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int y = 8; y >= 3; y--) {
                tileGrid[y][2] = 1;
                waypoints.push_back({ 2 * TILE_SIZE + TILE_SIZE / 2.f,
                                       y * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int x = 3; x <= 7; x++) {
                tileGrid[3][x] = 1;
                waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                      3 * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int y = 4; y <= 6; y++) {
                tileGrid[y][7] = 1;
                waypoints.push_back({ 7 * TILE_SIZE + TILE_SIZE / 2.f,
                                       y * TILE_SIZE + TILE_SIZE / 2.f });
            }
            for (int x = 6; x >= 4; x--) {
                tileGrid[6][x] = 1;
                waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                      6 * TILE_SIZE + TILE_SIZE / 2.f });
            }
            break;

        case 2: { // Zigzag
            for (int r = 1; r <= 11; r += 2) {
                bool goRight = ((r / 2) % 2 == 0);
                if (goRight) {
                    for (int x = 0; x <= 14; ++x) {
                        tileGrid[r][x] = 1;
                        waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                              r * TILE_SIZE + TILE_SIZE / 2.f });
                    }
                }
                else {
                    for (int x = 14; x >= 0; --x) {
                        tileGrid[r][x] = 1;
                        waypoints.push_back({ x * TILE_SIZE + TILE_SIZE / 2.f,
                                              r * TILE_SIZE + TILE_SIZE / 2.f });
                    }
                }
                if (r < 11) {
                    int col = goRight ? 14 : 0;
                    tileGrid[r + 1][col] = 1;
                    waypoints.push_back({ col * TILE_SIZE + TILE_SIZE / 2.f,
                                          (r + 1) * TILE_SIZE + TILE_SIZE / 2.f });
                }
            }
            break;
        }
        }

        entryPoint = waypoints.front();
        exitPoint = waypoints.back();
    }

    void render(RenderWindow& window) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                tile.setPosition({ (float)(x * TILE_SIZE), (float)(y * TILE_SIZE) });
                tile.setFillColor(tileGrid[y][x] == 1
                    ? Color(139, 69, 19) : Color(34, 139, 34));
                window.draw(tile);
            }
        }
        CircleShape c(8.f);
        c.setFillColor(Color::White);
        c.setPosition({ entryPoint.x - 8.f, entryPoint.y - 8.f });
        window.draw(c);
        c.setFillColor(Color::Black);
        c.setPosition({ exitPoint.x - 8.f, exitPoint.y - 8.f });
        window.draw(c);
    }

    bool isPath(int x, int y) const {
        return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT
            && tileGrid[y][x] == 1;
    }

    const vector<Vector2f>& getPath()  const { return waypoints; }
    Vector2f getEntry() const { return entryPoint; }
    Vector2f getExit()  const { return exitPoint; }
};

// ============================================================================
// WAVE MANAGER
// ============================================================================
class Wave {
public:
    int numBasic, numFast, numTank, numFlying, numRegen,
        numShielded, numSplitting, numHealing;
};

class WaveManager {
    vector<Wave> waves;
    int   currentWaveIndex;
    float spawnTimer;
    int   enemiesSpawned;
    bool  waveActive;
    Vector2f entryPoint, exitPoint;
    vector<Enemy*>& enemiesRef;
    const vector<Vector2f>& pathRef;
    SpawnCallback enemySpawnCB;
    void* spawnCtx;

    // Helper: register callback and push to list
    void doSpawn(Enemy* enemy) {
        enemy->setSpawnCallback(enemySpawnCB, spawnCtx);
        enemiesRef.push_back(enemy);
        enemiesSpawned++;
    }

public:
    WaveManager(const Vector2f& entry, const Vector2f& exit,
        vector<Enemy*>& enemies, const vector<Vector2f>& path,
        SpawnCallback spawnFunc, void* ctx)
        : currentWaveIndex(0), spawnTimer(0.f), enemiesSpawned(0),
        waveActive(false), entryPoint(entry), exitPoint(exit),
        enemiesRef(enemies), pathRef(path),
        enemySpawnCB(spawnFunc), spawnCtx(ctx) {
        waves.push_back({ 3, 0, 0, 0, 0, 0, 0, 0 });
        waves.push_back({ 2, 3, 0, 0, 0, 0, 0, 0 });
        waves.push_back({ 3, 2, 1, 0, 0, 0, 0, 0 });
        waves.push_back({ 2, 2, 1, 1, 0, 1, 1, 0 });
        waves.push_back({ 3, 1, 1, 1, 1, 1, 1, 1 });
    }

    void startNextWave() {
        if (currentWaveIndex >= (int)waves.size()) return;
        waveActive = true;
        enemiesSpawned = 0;
        spawnTimer = 0.f;
    }

    void update(float dt) {
        if (!waveActive || currentWaveIndex >= (int)waves.size()) return;
        Wave& w = waves[currentWaveIndex];
        spawnTimer += dt;

        int total = w.numBasic + w.numFast + w.numTank + w.numFlying + w.numRegen
            + w.numShielded + w.numSplitting + w.numHealing;

        if (spawnTimer >= 0.5f && enemiesSpawned < total) {
            spawnTimer = 0.f;
            int idx = enemiesSpawned;

            int basicEnd = w.numBasic;
            int fastEnd = basicEnd + w.numFast;
            int tankEnd = fastEnd + w.numTank;
            int flyingEnd = tankEnd + w.numFlying;
            int regenEnd = flyingEnd + w.numRegen;
            int shieldEnd = regenEnd + w.numShielded;
            int splitEnd = shieldEnd + w.numSplitting;

            if (idx < basicEnd)  doSpawn(new BasicEnemy(entryPoint));
            else if (idx < fastEnd)   doSpawn(new FastEnemy(entryPoint));
            else if (idx < tankEnd)   doSpawn(new TankEnemy(entryPoint));
            else if (idx < flyingEnd) doSpawn(new FlyingEnemy(entryPoint, exitPoint));
            else if (idx < regenEnd)  doSpawn(new RegeneratingEnemy(entryPoint));
            else if (idx < shieldEnd) doSpawn(new ShieldedEnemy(entryPoint));
            else if (idx < splitEnd)  doSpawn(new SplittingEnemy(entryPoint));
            else                      doSpawn(new HealingEnemy(entryPoint));
        }

        bool allDead = true;
        for (Enemy* e : enemiesRef)
            if (e->isAlive()) { allDead = false; break; }

        if (enemiesSpawned == total && allDead) {
            waveActive = false;
            currentWaveIndex++;
        }
    }

    bool isWaveComplete() const { return !waveActive; }
    bool allWavesDone()   const {
        return currentWaveIndex >= (int)waves.size() && !waveActive;
    }
    int getCurrentWaveNumber() const {
        int w = currentWaveIndex + 1;
        int sz = (int)waves.size();
        return (w < sz) ? w : sz;
    }
    int getTotalWaves() const { return (int)waves.size(); }
};

// ============================================================================
// UI helper
// ============================================================================
class UI {
public:
    static void drawText(RenderWindow& w, const string& str, const Font& font,
        unsigned int size, float x, float y,
        Color c = Color::White) {
        Text text(font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(c);
        text.setPosition({ x, y });
        w.draw(text);
    }
};

// ============================================================================
// GAME
// ============================================================================
class Game {
    enum State { MAIN_MENU, PLAYING, PAUSED, GAMEOVER, WIN };

    RenderWindow window;
    Map          map;
    Font         font;
    State        state;

    vector<Enemy*>     enemies;
    vector<Tower*>     towers;
    vector<int>        towerTypes;
    vector<Projectile> projectiles;
    WaveManager* waveManager;

    int gold, lives;
    int selectedTowerType;
    int selectedPlacedTowerIndex;
    int highScore;
    const string scoreFile;

    static const int NUM_TOWERS = 5;

    struct TowerInfo { string name; int cost; };
    vector<TowerInfo>       towerInfo;
    vector<RectangleShape>  btnShapes;
    RectangleShape          btnUpgrade;

    vector<Map>    maps;
    int            currentMapIndex;
    vector<string> mapNames;

    RectangleShape btnPlay, btnQuit, btnChangeMapMain;
    bool           showMapListMain;
    vector<RectangleShape> mapBtnsMain;

    RectangleShape btnResume, btnChangeMap, btnMainMenuPause;
    bool           showMapList;
    vector<RectangleShape> mapBtns;

    RectangleShape btnMainMenuGW;

public:
    Game()
        : window(VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Tower Defense"),
        state(MAIN_MENU), waveManager(nullptr),
        gold(STARTING_GOLD), lives(STARTING_LIVES),
        selectedTowerType(-1), selectedPlacedTowerIndex(-1),
        highScore(0), scoreFile("towerdefense_score.dat"),
        currentMapIndex(0), showMapListMain(false), showMapList(false) {
        window.setFramerateLimit(60);

        if (!font.openFromFile("arial.ttf")) {
            if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
                bool loaded = font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
                if (!loaded) cout << "Warning: could not load any font." << endl;
            }
        }

        loadHighScore();

        towerInfo = {
            { "CANNON",  CANNON_COST },
            { "SNIPER",  SNIPER_COST },
            { "M-GUN",   MACHINEGUN_COST },
            { "SLOW",    SLOW_COST },
            { "FLAME",   FLAME_COST }
        };

        for (int i = 0; i < NUM_TOWERS; i++) {
            RectangleShape btn({ BTN_W, BTN_H });
            btn.setPosition({ BTN_X0 + i * (BTN_W + BTN_GAP), UI_Y + 20.f });
            btnShapes.push_back(btn);
        }

        btnUpgrade = RectangleShape({ BTN_W, 30.f });
        btnUpgrade.setPosition({ BTN_X0 + NUM_TOWERS * (BTN_W + BTN_GAP), UI_Y + 30.f });

        mapNames = { "Classic", "Spiral", "Zigzag" };
        Map m;
        m.buildLayout(0); maps.push_back(m);
        m.buildLayout(1); maps.push_back(m);
        m.buildLayout(2); maps.push_back(m);
        map = maps[0];

        float mw = 200.f, mh = 50.f;
        float mx = WINDOW_WIDTH / 2.f - mw / 2.f, my0 = 200.f;
        btnPlay = RectangleShape({ mw, mh }); btnPlay.setPosition({ mx, my0 });
        btnQuit = RectangleShape({ mw, mh }); btnQuit.setPosition({ mx, my0 + 70.f });
        btnChangeMapMain = RectangleShape({ mw, mh }); btnChangeMapMain.setPosition({ mx, my0 + 140.f });
        for (int i = 0; i < (int)maps.size(); i++) {
            RectangleShape mb({ mw, 30.f });
            mb.setPosition({ mx, my0 + 210.f + i * 35.f });
            mapBtnsMain.push_back(mb);
        }

        mx = WINDOW_WIDTH / 2.f - 100.f;
        my0 = WINDOW_HEIGHT / 2.f - 100.f;
        btnResume = RectangleShape({ 200.f, 40.f }); btnResume.setPosition({ mx, my0 });
        btnChangeMap = RectangleShape({ 200.f, 40.f }); btnChangeMap.setPosition({ mx, my0 + 50.f });
        btnMainMenuPause = RectangleShape({ 200.f, 40.f }); btnMainMenuPause.setPosition({ mx, my0 + 100.f });
        for (int i = 0; i < (int)maps.size(); i++) {
            RectangleShape mb({ 200.f, 30.f });
            mb.setPosition({ mx, my0 + 160.f + i * 35.f });
            mapBtns.push_back(mb);
        }

        mx = WINDOW_WIDTH / 2.f - 100.f;
        my0 = WINDOW_HEIGHT / 2.f + 40.f;
        btnMainMenuGW = RectangleShape({ 200.f, 40.f });
        btnMainMenuGW.setPosition({ mx, my0 });
    }

    ~Game() {
        for (Enemy* e : enemies) delete e;
        for (Tower* t : towers) delete t;
        delete waveManager;
        saveHighScore();
    }

    void run() {
        Clock clock;
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();
            processEvents();
            if (state == PLAYING) update(dt);
            render();
        }
    }

private:
    // ---- static spawn callback ----
    static void enemyAdderFunc(Enemy* e, void* ctx) {
        Game* self = static_cast<Game*>(ctx);
        self->enemies.push_back(e);
    }

    void loadHighScore() {
        ifstream file(scoreFile);
        if (file.is_open()) { file >> highScore; file.close(); }
    }

    void saveHighScore() {
        if (gold > highScore) highScore = gold;
        ofstream file(scoreFile);
        if (file.is_open()) { file << highScore; file.close(); }
    }

    void startGame() {
        for (Enemy* e : enemies) delete e; enemies.clear();
        for (Tower* t : towers) delete t; towers.clear();
        towerTypes.clear();
        projectiles.clear();
        gold = STARTING_GOLD;
        lives = STARTING_LIVES;
        selectedTowerType = -1;
        selectedPlacedTowerIndex = -1;
        map = maps[currentMapIndex];

        delete waveManager;
        waveManager = new WaveManager(map.getEntry(), map.getExit(),
            enemies, map.getPath(),
            enemyAdderFunc, this);
        waveManager->startNextWave();
        state = PLAYING;
    }

    // ---- unified button renderer (replaces all lambdas) ----
    void drawMenuButton(RectangleShape& btn, const string& text,
        bool hover, bool selected = false,
        unsigned int fontSize = 20, float offY = 6.f) {
        btn.setFillColor(selected ? Color(100, 140, 100)
            : (hover ? Color(100, 100, 140)
                : Color(60, 60, 80)));
        btn.setOutlineColor(Color(150, 150, 180));
        btn.setOutlineThickness(1.f);
        window.draw(btn);
        FloatRect b = btn.getGlobalBounds();
        UI::drawText(window, text, font, fontSize,
            b.position.x + 10.f, b.position.y + offY, Color::White);
    }

    void processEvents() {
        while (const auto event = window.pollEvent()) {
            if (event->getIf<Event::Closed>()) {
                saveHighScore();
                window.close();
                return;
            }

            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->scancode == Keyboard::Scancode::P) {
                    if (state == PLAYING) state = PAUSED;
                    else if (state == PAUSED)  state = PLAYING;
                }
                else if (key->scancode == Keyboard::Scancode::Escape) {
                    selectedTowerType = -1;
                    selectedPlacedTowerIndex = -1;
                }
            }

            if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                if (mouse->button != Mouse::Button::Left) continue;
                Vector2f mpos((float)mouse->position.x, (float)mouse->position.y);

                if (state == MAIN_MENU) {
                    if (btnPlay.getGlobalBounds().contains(mpos)) { startGame(); continue; }
                    if (btnQuit.getGlobalBounds().contains(mpos)) { saveHighScore(); window.close(); continue; }
                    if (btnChangeMapMain.getGlobalBounds().contains(mpos)) { showMapListMain = !showMapListMain; continue; }
                    if (showMapListMain) {
                        for (int i = 0; i < (int)mapBtnsMain.size(); i++) {
                            if (mapBtnsMain[i].getGlobalBounds().contains(mpos)) {
                                currentMapIndex = i;
                                showMapListMain = false;
                                break;
                            }
                        }
                    }
                    continue;
                }

                if (state == PAUSED) {
                    if (btnResume.getGlobalBounds().contains(mpos)) { state = PLAYING; showMapList = false; continue; }
                    if (btnChangeMap.getGlobalBounds().contains(mpos)) { showMapList = !showMapList; continue; }
                    if (btnMainMenuPause.getGlobalBounds().contains(mpos)) { state = MAIN_MENU; showMapList = false; continue; }
                    if (showMapList) {
                        for (int i = 0; i < (int)mapBtns.size(); i++) {
                            if (mapBtns[i].getGlobalBounds().contains(mpos)) {
                                currentMapIndex = i;
                                startGame();
                                showMapList = false;
                                break;
                            }
                        }
                    }
                    continue;
                }

                if (state == GAMEOVER || state == WIN) {
                    if (btnMainMenuGW.getGlobalBounds().contains(mpos)) state = MAIN_MENU;
                    continue;
                }

                // PLAYING
                float mx2 = mpos.x, my2 = mpos.y;
                if (my2 >= UI_Y) {
                    for (int i = 0; i < NUM_TOWERS; i++) {
                        if (btnShapes[i].getGlobalBounds().contains({ mx2, my2 })) {
                            selectedTowerType = (selectedTowerType == i) ? -1 : i;
                            selectedPlacedTowerIndex = -1;
                            return;
                        }
                    }
                    if (selectedPlacedTowerIndex != -1
                        && btnUpgrade.getGlobalBounds().contains({ mx2, my2 })) {
                        int cost = towers[selectedPlacedTowerIndex]->getUpgradeCost();
                        if (cost > 0 && gold >= cost) {
                            gold -= cost;
                            towers[selectedPlacedTowerIndex]->upgrade();
                        }
                    }
                }
                else {
                    int gridX = (int)mx2 / TILE_SIZE;
                    int gridY = (int)my2 / TILE_SIZE;
                    bool found = false;
                    for (int i = 0; i < (int)towers.size(); i++) {
                        int tx = (int)(towers[i]->getPosition().x / TILE_SIZE);
                        int ty = (int)(towers[i]->getPosition().y / TILE_SIZE);
                        if (tx == gridX && ty == gridY) {
                            selectedPlacedTowerIndex = i;
                            selectedTowerType = -1;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        selectedPlacedTowerIndex = -1;
                        if (selectedTowerType != -1) placeTower(gridX, gridY);
                    }
                }
            }
        }
    }

    void placeTower(int gridX, int gridY) {
        if (gridX < 0 || gridX >= GRID_WIDTH || gridY < 0 || gridY >= GRID_HEIGHT) return;
        if (map.isPath(gridX, gridY)) return;
        for (Tower* t : towers) {
            if ((int)(t->getPosition().x / TILE_SIZE) == gridX
                && (int)(t->getPosition().y / TILE_SIZE) == gridY) return;
        }
        int cost = towerInfo[selectedTowerType].cost;
        if (gold < cost) return;

        gold -= cost;
        Vector2f pos(gridX * TILE_SIZE + TILE_SIZE / 2.f,
            gridY * TILE_SIZE + TILE_SIZE / 2.f);
        Tower* t = nullptr;
        switch (selectedTowerType) {
        case 0: t = new CannonTower(pos);    break;
        case 1: t = new SniperTower(pos);    break;
        case 2: t = new MachineGunTower(pos); break;
        case 3: t = new SlowTower(pos);      break;
        case 4: t = new FlameTower(pos);     break;
        }
        if (t) {
            towers.push_back(t);
            towerTypes.push_back(selectedTowerType);
        }
        selectedTowerType = -1;
    }

    void update(float dt) {
        if (!waveManager) return;
        waveManager->update(dt);
        if (waveManager->isWaveComplete() && !waveManager->allWavesDone())
            waveManager->startNextWave();

        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            e->update(dt);
            e->move(dt, map.getPath());
        }
        for (Tower* t : towers) {
            t->update(dt);
            t->attack(enemies, projectiles);
        }
        for (Projectile& p : projectiles) p.update(dt);

        for (int i = (int)projectiles.size() - 1; i >= 0; i--)
            if (!projectiles[i].isActive())
                projectiles.erase(projectiles.begin() + i);

        for (Enemy* e : enemies) {
            if (e->isAlive() && e->reachedEnd(map.getExit())) {
                lives--;
                e->takeDamage(9999.f);
                if (lives <= 0) state = GAMEOVER;
            }
        }
        for (int i = (int)enemies.size() - 1; i >= 0; i--) {
            if (!enemies[i]->isAlive()) {
                gold += enemies[i]->getReward();
                delete enemies[i];
                enemies.erase(enemies.begin() + i);
            }
        }

        if (waveManager->allWavesDone() && enemies.empty() && state == PLAYING) {
            state = WIN;
            saveHighScore();
        }
    }

    void drawRect(float x, float y, float w, float h,
        Color fill, Color outline = Color::Transparent, float thick = 0.f) {
        RectangleShape r({ w, h });
        r.setPosition({ x, y });
        r.setFillColor(fill);
        if (thick > 0.f) { r.setOutlineColor(outline); r.setOutlineThickness(thick); }
        window.draw(r);
    }

    void drawUI() {
        drawRect(0, UI_Y, WINDOW_WIDTH, WINDOW_HEIGHT - UI_Y, Color(40, 40, 50));
        drawRect(0, UI_Y, WINDOW_WIDTH, 4.f, Color(80, 80, 100));

        for (int i = 0; i < NUM_TOWERS; i++) {
            btnShapes[i].setFillColor(selectedTowerType == i
                ? Color(100, 100, 120) : Color(60, 60, 70));
            btnShapes[i].setOutlineColor(TOWER_COLORS[i]);
            btnShapes[i].setOutlineThickness(2.f);
            window.draw(btnShapes[i]);

            float bx = btnShapes[i].getPosition().x;
            float by = btnShapes[i].getPosition().y;
            UI::drawText(window, towerInfo[i].name, font, 16, bx + 10, by + 10, Color::White);
            UI::drawText(window, "$" + to_string(towerInfo[i].cost), font, 18, bx + 10, by + 40, Color::Yellow);
        }

        if (selectedPlacedTowerIndex != -1) {
            int upCost = towers[selectedPlacedTowerIndex]->getUpgradeCost();
            string upText = (upCost > 0) ? "Upgrade $" + to_string(upCost) : "MAX";
            btnUpgrade.setFillColor(gold >= upCost ? Color(80, 100, 80) : Color(80, 60, 60));
            btnUpgrade.setOutlineColor(Color(150, 150, 180));
            btnUpgrade.setOutlineThickness(1.f);
            window.draw(btnUpgrade);
            UI::drawText(window, upText, font, 16,
                btnUpgrade.getPosition().x + 8,
                btnUpgrade.getPosition().y + 6, Color::White);
        }
    }

    void drawStatusHUD() {
        float hudX = WINDOW_WIDTH - 200.f, hudY = 10.f;
        drawRect(hudX, hudY, 190.f, 115.f, Color(0, 0, 0, 180), Color(150, 150, 150), 1.5f);
        UI::drawText(window, "GOLD: $" + to_string(gold), font, 18, hudX + 10, hudY + 10, Color::Yellow);
        UI::drawText(window, "LIVES: " + to_string(lives), font, 18, hudX + 10, hudY + 35, Color(255, 100, 100));
        if (waveManager) {
            UI::drawText(window,
                "WAVE: " + to_string(waveManager->getCurrentWaveNumber())
                + "/" + to_string(waveManager->getTotalWaves()),
                font, 18, hudX + 10, hudY + 60, Color::White);
        }
        UI::drawText(window, "BEST: $" + to_string(highScore), font, 16, hudX + 10, hudY + 85, Color(200, 200, 200));
    }

    void render() {
        window.clear(Color(20, 20, 20));
        Vector2f mpos((float)Mouse::getPosition(window).x,
            (float)Mouse::getPosition(window).y);

        if (state == MAIN_MENU) {
            UI::drawText(window, "TOWER DEFENSE", font, 48,
                WINDOW_WIDTH / 2.f - 180.f, 80.f, Color::Cyan);
            UI::drawText(window, "Best Score: $" + to_string(highScore), font, 24,
                WINDOW_WIDTH / 2.f - 70.f, 140.f, Color::Yellow);

            drawMenuButton(btnPlay, "Play", btnPlay.getGlobalBounds().contains(mpos), false, 22, 8.f);
            drawMenuButton(btnQuit, "Quit", btnQuit.getGlobalBounds().contains(mpos), false, 22, 8.f);
            drawMenuButton(btnChangeMapMain, "Change Map", btnChangeMapMain.getGlobalBounds().contains(mpos), false, 22, 8.f);

            if (showMapListMain) {
                for (int i = 0; i < (int)mapBtnsMain.size(); i++) {
                    bool hov = mapBtnsMain[i].getGlobalBounds().contains(mpos);
                    drawMenuButton(mapBtnsMain[i], mapNames[i], hov, currentMapIndex == i, 22, 8.f);
                }
            }
            window.display();
            return;
        }

        map.render(window);

        if (state == PLAYING && selectedTowerType != -1) {
            Vector2i mp = Mouse::getPosition(window);
            if (mp.x >= 0 && mp.x < WINDOW_WIDTH && mp.y >= 0 && mp.y < (int)UI_Y) {
                int tx = mp.x / TILE_SIZE, ty = mp.y / TILE_SIZE;
                RectangleShape prev({ 30.f, 30.f });
                prev.setOrigin({ 15.f, 15.f });
                prev.setPosition({ tx * TILE_SIZE + TILE_SIZE / 2.f,
                                   ty * TILE_SIZE + TILE_SIZE / 2.f });
                prev.setFillColor(Color(255, 255, 255, 100));
                window.draw(prev);
            }
        }

        for (Tower* t : towers) { t->render(window); t->renderRange(window); }
        for (Enemy* e : enemies)    e->render(window);
        for (Projectile& p : projectiles) p.render(window);

        drawUI();
        drawStatusHUD();

        if (state == PAUSED) {
            RectangleShape overlay({ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT });
            overlay.setFillColor(Color(0, 0, 0, 180));
            window.draw(overlay);

            drawMenuButton(btnResume, "Resume", btnResume.getGlobalBounds().contains(mpos));
            drawMenuButton(btnChangeMap, "Change Map", btnChangeMap.getGlobalBounds().contains(mpos));
            drawMenuButton(btnMainMenuPause, "Main Menu", btnMainMenuPause.getGlobalBounds().contains(mpos));

            if (showMapList) {
                for (int i = 0; i < (int)mapBtns.size(); i++)
                    drawMenuButton(mapBtns[i], mapNames[i],
                        mapBtns[i].getGlobalBounds().contains(mpos));
            }
        }
        else if (state == GAMEOVER) {
            RectangleShape overlay({ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT });
            overlay.setFillColor(Color(100, 0, 0, 180));
            window.draw(overlay);
            UI::drawText(window, "GAME OVER", font, 50,
                WINDOW_WIDTH / 2.f - 140.f, WINDOW_HEIGHT / 2.f - 80.f, Color::Red);
            drawMenuButton(btnMainMenuGW, "Main Menu", btnMainMenuGW.getGlobalBounds().contains(mpos));
        }
        else if (state == WIN) {
            RectangleShape overlay({ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT });
            overlay.setFillColor(Color(0, 100, 0, 180));
            window.draw(overlay);
            UI::drawText(window, "YOU WIN!", font, 50,
                WINDOW_WIDTH / 2.f - 110.f, WINDOW_HEIGHT / 2.f - 80.f, Color::Green);
            drawMenuButton(btnMainMenuGW, "Main Menu", btnMainMenuGW.getGlobalBounds().contains(mpos));
        }

        window.display();
    }
};

// ============================================================================
// MAIN
// ============================================================================
int main() {
    Game game;
    game.run();
    return 0;
}