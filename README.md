# StarTrek Ultra: 3D Volume Simulation

Benvenuto in **StarTrek Ultra**, un simulatore tattico spaziale all'avanguardia che fonde la profondità strategica dei classici giochi testuali anni '70 con una moderna visualizzazione volumetrica 3D in tempo reale. 

Comanda la **USS Enterprise** attraverso una galassia ostile, gestisci sistemi complessi e affronta le più iconiche civiltà della Federazione in una sfida di sopravvivenza e abilità tattica.

---

## 🚀 Guida Rapida

### Obiettivo della Missione
La tua missione è eliminare tutte le minacce nemiche presenti nella galassia (un cubo 10x10x10 composto da 1000 quadranti). Dovrai esplorare lo spazio, estrarre risorse dai pianeti, riparare la nave e difendere i confini della Federazione da 11 diverse specie aliene.

### Requisiti e Compilazione
- **Standard**: ISO C23 (Richiede GCC 15.2+).
- **Librerie**: OpenGL, FreeGLUT, GLU, Math (lm).
- **Compilazione**: Esegui `make` nella directory principale.
- **Esecuzione**: Avvia con `./startrekultra`. Il terminale si sbloccherà solo quando il modulo grafico sarà `READY`.

---

## 🕹️ Comandi del Terminale

### Navigazione e Calcolo
*   `nav`: **Navigazione Warp**. Imposta Rotta (0-359), Inclinazione (Mark -90/+90) e Fattore Warp. Velocità sincronizzata a **3 secondi per quadrante**.
*   `cal`: **Nav-Calculator**. Strumento astrometrico per calcolare vettori, energia e tempi verso qualsiasi punto della galassia.
*   `apr`: **Avvicinamento Automatico**. Manovra la nave fino a una distanza specifica da stelle, basi, pianeti o nemici.
*   `lrs`: **Long Range Scan**. Scansione 3D dei 26 quadranti adiacenti su 3 livelli (Deck).

### Combattimento Tattico
*   `lock`: **Lock-on**. Aggancia un nemico specifico per automatizzare il puntamento di Phaser e Siluri.
*   `pha`: **Phaser**. Fuoco a energia istantaneo. Il danno dipende dalla distanza 3D.
*   `tor`: **Siluri Fotonici**. Proiettili balistici ad alto potenziale. Include analisi vettoriale 3D.
*   `bor`: **Abbordaggio (Boarding)**. Invia squadre di sicurezza su navi nemiche indebolite (Energia < 150) per catturarle e smantellarle.
*   `she`: **Scudi Direzionali**. Gestisce l'energia nei settori FRONT, REAR, TOP e BOTTOM.

### Gestione Risorse e Logistica
*   `min`: **Mining Planetario**. Estrae minerali e gas atmosferici dai pianeti (richiede distanza < 2.0).
*   `inv`: **Inventario**. Visualizza le riserve di Dilithium, Tritanium, Verterium, Monotanium, Isolinear e Gases.
*   `rep`: **Riparazione**. Utilizza Monotanium (scafo/motori) o Isolinear (elettronica) per riparazioni autonome.
*   `con`: **Conversione**. Raffina materiali in Energia, Siluri o Aria respirabile.
*   `doc`: **Attracco**. Rifornimento completo e riparazioni gratuite presso una Base Stellare.

### Sistemi e Report
*   `sta`: **Stato**. Report missione, intelligence sulle specie rimanenti e dati tattici del quadrante.
*   `dam`: **Danni**. Integrità percentuale degli 8 sistemi critici (incluso il Supporto Vitale).
*   `psy`: **Guerra Psicologica**. Resa, Corbomite Bluff o "Play Dead" (mascheramento emissioni).
*   `aux`: **Sistemi Ausiliari**. Lancio sonde o espulsione d'emergenza della sezione ingegneria.
*   `axs` / `grd`: **Visual**. Attiva/disattiva gli assi cartesiani o la griglia dei settori nel visore 3D.

---

## 🛠️ Meccaniche Avanzate

### 1. Supporto Vitale & Aria
La vita dell'equipaggio è legata all'integrità del sistema **Life Support**.
- Se il sistema è danneggiato (< 100%), le riserve d'aria calano costantemente (max 1% a turno).
- Sotto il 30% scatta l'allarme rosso. Allo 0% la missione fallisce.
- Ripristina l'aria convertendo i **Atmospheric Gases** con il comando `con`.

### 2. Danni Localizzati e Scudi
I nemici non colpiscono a caso. Il computer calcola la loro posizione relativa e l'impatto avverrà sullo scudo più vicino. Se quello scudo è scarico, il danno penetra nei sistemi interni.

### 3. Intelligence Galattica
Sono presenti 11 specie nemiche con modelli 3D e caratteristiche uniche:
- **Klingon, Romulani, Borg, Cardassiani, Jem'Hadar, Tholiani, Gorn, Ferengi, Specie 8472, Breen, Hirogen**.
Il comando `sta` fornisce il conteggio in tempo reale delle navi rimanenti per ogni fazione.

---

## ⚙️ Specifiche Tecniche (IPC Architecture)

- **Doppio Processo**: Separazione netta tra Motore Logico e Visore Grafico via `fork/exec`.
- **Handshake Sincrono**: Sincronizzazione perfetta tramite segnali `SIGUSR1` (Data Ready) e `SIGUSR2` (Frame ACK).
- **Stabilità**: Caricamento **Double-Buffered** dei dati, protezione da coordinate `NaN` e **Boundary Protection** per evitare crash ai confini galattici.
- **Visual**: Rendering a 30 FPS costanti con interpolazione lineare dei movimenti e scia di propulsione a scomparsa dinamica (100 frame).

---
*Versione 3.7* 
*Sviluppato da Nicola Taibi.* 
*Supportato da Google Gemini.* 
**Copyright**: © 2026 Nicola Taibi  
**Licenza**: GNU GPL v3.0
