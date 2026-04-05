# DIY RaceChrono CAN Bus for Honda CBR 600 RR

[![PlatformIO](https://img.shields.io/badge/PlatformIO-PlatformIO-orange.svg)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![LilyGO T-CAN485](https://img.shields.io/badge/LilyGO-T--CAN485-purple.svg)](https://github.com/Xinyuan-LilyGO/T-CAN485)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## 🇫🇷 Français

### Description du Projet

Ce projet permet de connecter une moto **Honda CBR 600 RR** à l'application de chronométrage **RaceChrono** via Bluetooth Low Energy (BLE). Il capture les données du bus CAN de la moto et les transmet en temps réel à l'application RaceChrono sur un smartphone pour l'analyse des performances sur circuit.

### Fonctionnalités

- ✅ **Capture des données CAN Bus** en temps réel
- ✅ **Transmission BLE** vers l'application RaceChrono
- ✅ **LED de statut** indiquant l'état de la connexion
- ✅ **Mode écoute uniquement** (ne perturbe pas le fonctionnement de la moto)
- ✅ **Données capturées** :
  - Régime moteur (RPM)
  - Vitesse du véhicule
  - Rapport de boîte de vitesses
  - Vitesses des roues avant et arrière
  - Indicateur de contrôle de traction (TC)
  - Position du papillon des gaz (TPS)
  - Paramètres de la moto (Map, TC, Anti-wheeling, Frein moteur)

### Matériel Requis

| Composant | Description | Quantité |
|-----------|-------------|----------|
| **LilyGO T-CAN485** | Carte ESP32 avec CAN Bus intégré ([Documentation](https://github.com/Xinyuan-LilyGO/T-CAN485)) | 1 |
| LED WS2812B | LED RGB pour le statut (optionnel) | 1 |
| Connecteur OBD-II | Pour se connecter au port diagnostic de la moto | 1 |
| Câbles | Pour les connexions | - |

**ℹ️ Note :** Ce projet utilise la carte **LilyGO T-CAN485** qui intègre un ESP32 avec un transceiver CAN Bus (SN65HVD230). Aucun module CAN externe n'est nécessaire. Pour plus d'informations sur cette carte, consultez la [documentation officielle](https://github.com/Xinyuan-LilyGO/T-CAN485).

### Schéma de Câblage

La carte **LilyGO T-CAN485** intègre déjà le transceiver CAN Bus (SN65HVD230), le câblage est donc simplifié :

```
LilyGO T-CAN485              Moto (Port OBD-II)
───────────────              ─────────────────
CAN_TX (GPIO 27)  ─────►     CAN High (Pin 6)
CAN_RX (GPIO 26)  ─────►     CAN Low (Pin 14)
GPIO 4 (LED)      ─────►     Données LED WS2812B
GND               ─────►     Masse (Pin 4/5)
```

**ℹ️ Note :** La carte LilyGO T-CAN485 possède un connecteur CAN intégré. Vous pouvez soit utiliser le connecteur CAN de la carte, soit souder directement les fils sur les pins GPIO 26 et 27.

### Installation

#### Prérequis

- [PlatformIO](https://platformio.org/) installé (VSCode extension ou CLI)
- [Visual Studio Code](https://code.visualstudio.com/) (recommandé)

#### Étapes d'Installation

1. **Cloner le projet**
   ```bash
   git clone https://github.com/votre-username/DIY-RaceChrono-CANBUS-CBR.git
   cd DIY-RaceChrono-CANBUS-CBR
   ```

2. **Ouvrir avec PlatformIO**
   - Ouvrez le dossier dans VSCode
   - PlatformIO détectera automatiquement le projet

3. **Compiler et téléverser**
   ```bash
   pio run -t upload
   ```

4. **Monitorer le port série** (optionnel)
   ```bash
   pio device monitor -b 115200
   ```

### Utilisation

1. **Connexion matérielle**
   - Connectez l'ESP32 directement sur le Bus CAN Interne de la  CBR 600 RR
   - Alimentez l'ESP32 sur le 12V de la moto

2. **Configuration RaceChrono**
   - Ouvrez l'application RaceChrono sur votre smartphone
   - Allez dans **Paramètres** → **Appareils**
   - Recherchez et connectez l'appareil **"CBR CAN BUS Gateway"**
   - Configurez les PIDs CAN selon vos besoins

3. **Indicateurs LED**
   - 🔵 **Bleu** : En attente de connexion BLE
   - 🟢 **Vert** : Connecté à RaceChrono
   - 🟠 **Orange** : CAN Bus initialisé
   - 🔴 **Rouge** : Erreur d'initialisation

### Cartographie des IDs CAN

| ID CAN Source | ID CAN RaceChrono | Données | Format | Description |
|---------------|-------------------|---------|--------|-------------|
| 0x157 | 0x050 | Byte 5 | 1 byte | Rapport de boîte de vitesses |
| 0x157 | 0x051 | Bytes 3-4 | 2 bytes | Vitesse roue avant |
| 0x157 | 0x052 | Bytes 1-2 | 2 bytes | Vitesse roue arrière |
| 0x157 | 0x053 | Byte 0 | 1 byte | Indicateur TC (0/100) |
| 0x1DC | 0x01DC | Bytes 2-3 | 2 bytes | Régime moteur (RPM) |
| 0x193 | 0x0193 | Bytes 0-1 | 2 bytes | Vitesse compteur |
| 0x15B | 0x015B | Bytes 4-5 | 2 bytes | Position papillon des gaz (TPS) |
| 0x154 | 0x100 | Byte 0 | 1 byte | Mode moteur (Map) |
| 0x154 | 0x101 | Byte 1 | 1 byte | Niveau TC |
| 0x154 | 0x102 | Byte 2 | 1 byte | Anti-wheeling |
| 0x154 | 0x103 | Byte 2 | 1 byte | Frein moteur |

### Structure du Projet

```
DIY-RaceChrono-CANBUS-CBR/
├── src/
│   └── main.cpp              # Code principal
├── lib/
│   └── arduino-RaceChrono/   # Bibliothèque RaceChrono
├── include/                   # Fichiers d'en-tête et données de test
│   ├── Test TC.csv            # Données de test CAN
│   └── Test TC - digital hex.csv
├── test/                      # Tests
├── platformio.ini             # Configuration PlatformIO
└── README.md                  # Ce fichier
```

### Dépannage

| Problème | Solution |
|----------|----------|
| Pas de connexion BLE | Vérifiez que l'ESP32 est alimenté et que la LED est bleue |
| Pas de données CAN | Vérifiez les connexions CAN_H et CAN_L |
| Erreur de compilation | Vérifiez que toutes les dépendances sont installées |
| Données incorrectes | Vérifiez la cartographie des IDs CAN pour votre modèle Honda CBR 600 RR |

### Contribuer

Les contributions sont les bienvenues ! N'hésitez pas à :
- Signaler des bugs
- Proposer de nouvelles fonctionnalités
- Soumettre des pull requests

### Licence

Ce projet est sous licence MIT. Voir le fichier [LICENSE](LICENSE) pour plus de détails.

---

## 🇬🇧 English

### Project Description

This project connects a **Honda CBR 600 RR** motorcycle to the **RaceChrono** timing application via Bluetooth Low Energy (BLE). It captures CAN bus data from the motorcycle and transmits it in real-time to the RaceChrono app on a smartphone for track performance analysis.

### Features

- ✅ **Real-time CAN Bus data capture**
- ✅ **BLE transmission** to RaceChrono app
- ✅ **Status LED** indicating connection state
- ✅ **Listen-only mode** (doesn't interfere with motorcycle operation)
- ✅ **Captured data**:
  - Engine RPM
  - Vehicle speed
  - Gear position
  - Front and rear wheel speeds
  - Traction control (TC) indicator
  - Throttle position (TPS)
  - Motorcycle parameters (Map, TC, Anti-wheelie, Engine brake)

### Hardware Requirements

| Component | Description | Quantity |
|-----------|-------------|----------|
| **LilyGO T-CAN485** | ESP32 board with integrated CAN Bus ([Documentation](https://github.com/Xinyuan-LilyGO/T-CAN485)) | 1 |
| WS2812B LED | RGB LED for status (optional) | 1 |
| OBD-II Connector | To connect to motorcycle diagnostic port | 1 |
| Wires | For connections | - |

**ℹ️ Note:** This project uses the **LilyGO T-CAN485** board which integrates an ESP32 with a CAN Bus transceiver (SN65HVD230). No external CAN module is required. For more information about this board, see the [official documentation](https://github.com/Xinyuan-LilyGO/T-CAN485).

### Wiring Diagram

The **LilyGO T-CAN485** board already integrates the CAN Bus transceiver (SN65HVD230), so wiring is simplified:

```
LilyGO T-CAN485              Motorcycle (OBD-II Port)
───────────────              ───────────────────────
CAN_TX (GPIO 27)  ─────►     CAN High (Pin 6)
CAN_RX (GPIO 26)  ─────►     CAN Low (Pin 14)
GPIO 4 (LED)      ─────►     WS2812B LED Data
GND               ─────►     Ground (Pin 4/5)
```

**ℹ️ Note:** The LilyGO T-CAN485 board has an integrated CAN connector. You can either use the board's CAN connector or solder wires directly to GPIO pins 26 and 27.

### Installation

#### Prerequisites

- [PlatformIO](https://platformio.org/) installed (VSCode extension or CLI)
- [Visual Studio Code](https://code.visualstudio.com/) (recommended)

#### Installation Steps

1. **Clone the project**
   ```bash
   git clone https://github.com/your-username/DIY-RaceChrono-CANBUS-CBR.git
   cd DIY-RaceChrono-CANBUS-CBR
   ```

2. **Open with PlatformIO**
   - Open the folder in VSCode
   - PlatformIO will automatically detect the project

3. **Compile and upload**
   ```bash
   pio run -t upload
   ```

4. **Monitor serial port** (optional)
   ```bash
   pio device monitor -b 115200
   ```

### Usage

1. **Hardware Connection**
   - Connect the ESP32 directly to the internal CAN bus of the CBR 600 RR
   - Power the ESP32 from the motorcycle’s 12 V supply

2. **RaceChrono Configuration**
   - Open the RaceChrono app on your smartphone
   - Go to **Settings** → **Devices**
   - Search for and connect to the device **"CBR CAN BUS Gateway"**
   - Configure CAN PIDs according to your needs

3. **LED Indicators**
   - 🔵 **Blue**: Waiting for BLE connection
   - 🟢 **Green**: Connected to RaceChrono
   - 🟠 **Orange**: CAN Bus initialized
   - 🔴 **Red**: Initialization error

### CAN ID Mapping

| Source CAN ID | RaceChrono CAN ID | Data | Format | Description |
|---------------|-------------------|------|--------|-------------|
| 0x157 | 0x050 | Byte 5 | 1 byte | Gear position |
| 0x157 | 0x051 | Bytes 3-4 | 2 bytes | Front wheel speed |
| 0x157 | 0x052 | Bytes 1-2 | 2 bytes | Rear wheel speed |
| 0x157 | 0x053 | Byte 0 | 1 byte | TC indicator (0/100) |
| 0x1DC | 0x01DC | Bytes 2-3 | 2 bytes | Engine RPM |
| 0x193 | 0x0193 | Bytes 0-1 | 2 bytes | Speedometer speed |
| 0x15B | 0x015B | Bytes 4-5 | 2 bytes | Throttle position (TPS) |
| 0x154 | 0x100 | Byte 0 | 1 byte | Engine mode (Map) |
| 0x154 | 0x101 | Byte 1 | 1 byte | TC level |
| 0x154 | 0x102 | Byte 2 | 1 byte | Anti-wheelie |
| 0x154 | 0x103 | Byte 2 | 1 byte | Engine brake |

### Project Structure

```
DIY-RaceChrono-CANBUS-CBR/
├── src/
│   └── main.cpp              # Main code
├── lib/
│   └── arduino-RaceChrono/   # RaceChrono library
├── include/                   # Header files and test data
│   ├── Test TC.csv            # CAN test data
│   └── Test TC - digital hex.csv
├── test/                      # Tests
├── platformio.ini             # PlatformIO configuration
└── README.md                  # This file
```

### Troubleshooting

| Problem | Solution |
|---------|----------|
| No BLE connection | Check ESP32 is powered and LED is blue |
| No CAN data | Verify CAN_H and CAN_L connections |
| Compilation error | Ensure all dependencies are installed |
| Incorrect data | Check CAN ID mapping for your Honda CBR 600 RR model |

### Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest new features
- Submit pull requests

### License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## 📚 Ressources / Resources

- [LilyGO T-CAN485 Documentation](https://github.com/Xinyuan-LilyGO/T-CAN485)
- [RaceChrono App](https://racechrono.com/)
- [RaceChrono API Documentation](https://racechrono.com/api)
- [ESP32 TWAI (CAN) Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html)
- [PlatformIO Documentation](https://docs.platformio.org/)

---

## 📝 Notes

- Ce projet est développé pour la Honda CBR 600 RR. La cartographie des IDs CAN peut varier selon les modèles et années.
- Ce projet utilise la carte **LilyGO T-CAN485** ([Documentation](https://github.com/Xinyuan-LilyGO/T-CAN485)) qui intègre un ESP32 avec un transceiver CAN Bus.
- This project is developed for the Honda CBR 600 RR. CAN ID mapping may vary by model and year.
- This project uses the **LilyGO T-CAN485** board ([Documentation](https://github.com/Xinyuan-LilyGO/T-CAN485)) which integrates an ESP32 with a CAN Bus transceiver.

---

**Développé avec ❤️ pour la communauté moto / Developed with ❤️ for the motorcycle community**
