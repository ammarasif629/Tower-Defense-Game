# Tower-Defense-Game-
Using OOP and SFML 3.0.2
# **SFML Tower Defense**

A feature-rich, 2D Tower Defense game built using C++ and the **Simple and Fast Multimedia Library (SFML)**. Defend your base against waves of increasingly difficult enemies by strategically placing and upgrading various types of towers.

## **🌟 Features**

* **Grid-Based Strategy:** 16x12 tile map layout dynamically built based on the selected level.  
* **3 Unique Maps:** Play on "Classic", "Spiral", and "Zigzag" layouts.  
* **Wave System:** Survive 5 challenging waves with varying enemy compositions.  
* **Tower Upgrades:** Towers can be upgraded up to Level 3, increasing their damage, range, and fire rate.  
* **Dynamic UI:** Build towers using an in-game UI system with starting resources (300 Gold, 20 Lives).

## **🗼 Towers**

You can build 5 distinct types of towers, each serving a unique tactical purpose:

| Tower | Cost | Color | Description |
| :---- | :---- | :---- | :---- |
| **Cannon** | 50g | Blue | Standard tower with balanced range, damage, and fire rate. |
| **Sniper** | 80g | Green | Long-range, high-damage tower with a slow fire rate. |
| **Machine Gun** | 60g | Orange | Short-range tower that fires very rapidly but deals low damage. |
| **Slow** | 100g | Purple | Deals no damage but significantly slows down enemies within its radius. |
| **Flame** | 120g | Red | Emits a laser/AoE attack that damages all enemies within range simultaneously. |

## **👾 Enemies**

The game features 8 unique enemy types that require different strategies to defeat. Each enemy can be identified by its color on the map:

| Enemy | Color | Description |
| :---- | :---- | :---- |
| **Basic Enemy** | Red | Standard speed and health. |
| **Fast Enemy** | Yellow | Moves quickly but has lower health. |
| **Tank Enemy** | Gray | Extremely high health but moves very slowly. |
| **Flying Enemy** | Cyan | Ignores the path and flies in a straight line directly to the exit. |
| **Regenerating Enemy** | Purple | Slowly heals itself over time. |
| **Shielded Enemy** | Light Blue | Possesses a rechargeable dark blue shield that must be depleted before health is damaged. |
| **Splitting Enemy** | Orange | When killed, it splits into two smaller Fast enemies. |
| **Healing Enemy** | Light Green | Projects a glowing aura that heals other nearby enemies. |

## **🛠️ Prerequisites**

To compile and run this game, you will need:

* A C++ compiler (supports C++17 or higher)  
* [SFML (Simple and Fast Multimedia Library)](https://www.SFML-dev.org/download.php) (SFML 3.0.2 is used; required files are already provided)

## **🚀 How to Build and Run**

### **On Windows**

If you are using **Visual Studio**, you will need to link the SFML libraries (sfml-graphics.lib, sfml-window.lib, sfml-system.lib) in your project settings. Please refer to the [official SFML Visual Studio tutorial](https://www.sfml-dev.org/tutorials/2.5/start-vc.php) for detailed instructions.

## **🎮 How to Play**

1. **Start:** You begin with **300 Gold** and **20 Lives**.  
2. **Build:** Click on the UI buttons at the bottom of the screen to select a tower, then click on a green (grass) tile to place it.  
3. **Defend:** Enemies will spawn at the white circle and travel along the brown path to the black circle.  
4. **Upgrade:** (Implementation dependent) Select an existing tower and spend gold to upgrade its level and stats.  
5. **Survive:** Prevent enemies from reaching the end. If 20 enemies reach the exit, it's Game Over\!
