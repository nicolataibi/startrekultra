# StarTrek Ultra: 3D Volume Simulation

Welcome to **StarTrek Ultra**, a cutting-edge space tactical simulator that merges the strategic depth of 1970s text-based classics with a modern real-time 3D volumetric visualization.

Command the **USS Enterprise** through a hostile galaxy, manage complex ship systems, and face the Federation's most iconic enemies in a ultimate challenge of survival and tactical skill.

---

## 🚀 QUICK START

### Mission Objective
Your mission is to eliminate all enemy threats in the galaxy (a 10x10x10 cube containing 1000 quadrants). You must explore space, mine planetary resources, repair your ship, and defend Federation borders against 11 different alien species.

### Requirements & Compilation
- **Standard**: ISO C23 (Requires GCC 15.2+).
- **Libraries**: OpenGL, FreeGLUT, GLU, Math (lm).
- **Build**: Run `make` in the main directory.
- **Launch**: Execute `./startrekultra`. The terminal will unlock only when the graphics module is `READY`.

---

## 🕹️ TERMINAL COMMANDS

### Navigation & Calculation
*   `nav`: **Warp Navigation**. Set Heading (0-359), Mark (-90/+90), and Warp factor. Speed is synchronized at **3 seconds per quadrant**.
*   `cal`: **Nav-Calculator**. Astrometric tool to calculate vectors, energy, and travel time to any galactic coordinates.
*   `apr`: **Automatic Approach**. Maneuvers the ship to a specific distance from stars, bases, planets, or enemies.
*   `lrs`: **Long Range Scan**. 3D scan of the 26 surrounding quadrants across 3 Levels (Decks).

### Tactical Combat
*   `lock`: **Tactical Lock-on**. Links sensors to weapons for automated Phaser and Torpedo targeting.
*   `pha`: **Phaser Banks**. Instant energy fire. Damage is based on 3D distance.
*   `tor`: **Photon Torpedoes**. High-yield ballistic projectiles with full 3D vector analysis.
*   `bor`: **Boarding Operation**. Send security teams to capture and dismantle weakened enemy ships (Energy < 150).
*   `she`: **Directional Shields**. Manage energy levels for FRONT, REAR, TOP, and BOTTOM sectors.

### Resource Management & Logistics
*   `min`: **Planetary Mining**. Extract minerals and atmospheric gases from planets (requires distance < 2.0).
*   `inv`: **Inventory**. View reserves of Dilithium, Tritanium, Verterium, Monotanium, Isolinear, and Gases.
*   `rep`: **Repair System**. Use Monotanium (hull/engines) or Isolinear (electronics) for autonomous field repairs.
*   `con`: **Refining**. Convert raw materials into Energy, Torpedoes, or Breathable Air.
*   `doc`: **Docking**. Full resupply and free repairs at a Starbase.

### Systems & Reports
*   `sta`: **Status**. Mission progress, species intelligence, and local tactical data.
*   `dam`: **Damage Control**. Health percentage of all 8 critical systems (including Life Support).
*   `psy`: **Psychological Warfare**. Surrender requests, Corbomite Bluff, or "Play Dead" (emissions masking).
*   `aux`: **Auxiliary Systems**. Launch probes or execute emergency Engineering Jettison.
*   `axs` / `grd`: **Visuals**. Toggle Cartesian axes or sector grid in the 3D viewer.

---

## 🛠️ ADVANCED MECHANICS

### 1. Life Support & Air Reserves
The crew's life depends on the integrity of the **Life Support** system.
- If the system is damaged (< 100%), air reserves will drain over time (max 1% per turn).
- Red Alert triggers below 30% air. At 0%, the mission fails due to crew suffocation.
- Replenish air by converting **Atmospheric Gases** using the `con` command.

### 2. Localized Damage & Shields
Enemies hit specific targets. The computer calculates their relative position, and impacts occur on the closest shield sector. If that sector is empty, damage penetrates to internal systems.

### 3. Galactic Intelligence
The galaxy is populated by 11 species with unique 3D models and traits:
- **Klingon, Romulan, Borg, Cardassian, Jem'Hadar, Tholian, Gorn, Ferengi, Species 8472, Breen, Hirogen**.
The `sta` command provides a real-time count of remaining ships for each faction.

---

## ⚙️ TECHNICAL SPECS (IPC Architecture)

- **Dual-Process**: Clean separation between Logic Engine and Visualizer via `fork/exec`.
- **Synchronous Handshake**: Perfect sync via **`SIGUSR1`** (Data Ready) and **`SIGUSR2`** (Frame ACK) signals.
- **Stability**: **Double-Buffered** data loading, **NaN protection**, and **Boundary Protection** logic.
- **Visuals**: Constant 30 FPS rendering with linear movement interpolation and dynamic propulsion trails (100 frames).

---
*Version 3.7 - Developed by Nicola Taibi and Google Gemini.*  
**Copyright**: © 2026 Nicola Taibi  
**License**: GNU GPL v3.0