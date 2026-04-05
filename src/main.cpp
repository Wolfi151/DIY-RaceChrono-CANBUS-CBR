/**
 * ========================================================================
 * PROJET: DIY RaceChrono CAN Bus pour KTM 790
 * ========================================================================
 * 
 * DESCRIPTION:
 * Ce projet permet de connecter une moto KTM 790 au circuit de chronométrage
 * RaceChrono via Bluetooth Low Energy (BLE). Il capture les données du bus
 * CAN de la moto et les transmet à l'application RaceChrono sur un smartphone.
 * 
 * MATÉRIEL REQUIS:
 * - ESP32 (module avec support CAN Bus intégré)
 * - Module MCP2515 ou compatible CAN Bus (optionnel selon ESP32 utilisé)
 * 
 * CONNEXIONS:
 * - CAN_TX: GPIO 27
 * - CAN_RX: GPIO 26
 * 
 * PROTOCOLE:
 * L'ESP32 écoute le bus CAN de la moto et filtre les messages par ID.
 * Les données sont ensuite envoyées via BLE à l'app RaceChrono qui les
 * affiche en temps réel sur le smartphone.
 * 
 * ========================================================================
 */

// Inclusion des bibliothèques nécessaires
#include <Arduino.h>              // Bibliothèque de base Arduino
#include <HardwareSerial.h>
#include "FastLED.h"
#include <RaceChrono.h>           // Bibliothèque RaceChrono BLE
#include "driver/twai.h"          // Driver TWAI (CAN) pour ESP32
#include <map>                     // Pour std::map (gestion des PIDs autorisés)

// ========================================================================
// CONFIGURATION DES PINS CAN BUS
// ========================================================================
// Ces pins permettent la communication avec le transceiver CAN
// Attention: Vérifiez la compatibilité avec votre matériel
#define CAN_TX_PIN 27   // Pin de transmission CAN (GPIO 27)
#define CAN_RX_PIN 26   // Pin de réception CAN (GPIO 26)


// ========================================================================
// CONFIGURATION DES PINS LED
// ========================================================================


#define NUM_LEDS 1
#define DATA_PIN 4
CRGB leds[NUM_LEDS];

// CONFIGURATION DES MEMO
uint8_t MemodataGrear = 255;
uint8_t MemoParaMap = 255;
uint8_t MemoParaTC = 255;
uint8_t MemoParaW= 255;
uint8_t MemoParaEB= 255;
uint8_t Inc=0;
uint8_t ActuSlow=10;



// ========================================================================
// GESTIONNAIRE RACECHRONO BLE
// ========================================================================
// Classe qui implémente l'interface RaceChronoBleCanHandler
// Cette classe gère les requêtes PIDs envoyées par l'app RaceChrono
class RaceChronoCanHandler : public RaceChronoBleCanHandler {
public:
    RaceChronoCanHandler() : _allowAllPids(false), _updateIntervalMs(50) {}

    // Méthode appelée quand RaceChrono demande d'autoriser tous les PIDs
    void allowAllPids(uint16_t updateIntervalMs) override {
        _allowAllPids = true;
        _updateIntervalMs = updateIntervalMs;
        Serial.println("RaceChrono: Tous les PIDs autorises");
    }

    // Méthode appelée quand RaceChrono demande de refuser tous les PIDs
    void denyAllPids() override {
        _allowAllPids = false;
        Serial.println("RaceChrono: Tous les PIDs refuses");
    }

    // Méthode appelée quand RaceChrono demande d'autoriser un PID spécifique
    void allowPid(uint32_t pid, uint16_t updateIntervalMs) override {
        _allowedPids[pid] = updateIntervalMs;
        Serial.print("RaceChrono: PID autorise: 0x");
        Serial.println(pid, HEX);
    }

    // Vérifie si un PID est autorisé (soit tous, soit individuellement)
    bool isPidAllowed(uint32_t pid) const {
        if (_allowAllPids) return true;
        return _allowedPids.find(pid) != _allowedPids.end();
    }

    uint16_t getUpdateInterval() const { return _updateIntervalMs; }

private:
    bool _allowAllPids;
    uint16_t _updateIntervalMs;
    std::map<uint32_t, uint16_t> _allowedPids;
};

// Instance globale du gestionnaire RaceChrono
RaceChronoCanHandler rcHandler;

/**
 * ========================================================================
 * FONCTION SETUP - Initialisation du système
 * ========================================================================
 * Cette fonction est exécutée une seule fois au démarrage de l'ESP32
 * Elle configure:
 * 1. La communication série pour le débogage
 * 2. Le driver CAN Bus (TWAI) à 500 kbps
 * 3. Le Bluetooth BLE pour RaceChrono
 */
void setup() {
    // Initialisation de la communication série (115200 bauds)
    // Utilisée pour le débogage et la vérification des messages
    Serial.begin(115200);

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); // GRB ordering is typical
    FastLED.setBrightness(50);


    // ========================================================================
    // 1. CONFIGURATION DU DRIVER CAN (TWAI) - 500 kbps
    // ========================================================================
    // TWAI = Two-Wire Automotive Interface (CAN bus pour ESP32)
    // Configuration à 500 kbps (standard pour la plupart des véhicules)
    
    // Configuration générale des pins et du mode
    // Mode LISTEN_ONLY: on écoute seulement, on n'envoie pas sur le bus
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)CAN_TX_PIN, 
        (gpio_num_t)CAN_RX_PIN, 
        TWAI_MODE_LISTEN_ONLY
    );

    // Configuration du timing (500 kbit/s)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    
    // Configuration du filtre (accepter tous les messages)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Installation et démarrage du driver CAN
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK && 
        twai_start() == ESP_OK) {
        Serial.println("CAN Bus: Systeme demarre correctement");
        leds[0] = CRGB::Orange;
        FastLED.show();
    } else {
        Serial.println("CAN Bus: Erreur de demarrage");
        leds[0] = CRGB::Red;
        FastLED.show();
    }

    // ========================================================================
    // 2. DÉMARRAGE DU BLUETOOTH BLE
    // ========================================================================
    // Configuration de RaceChronoBle avec le nom de l'appareil et le handler
    RaceChronoBle.setUp("CBR CAN BUS Gateway", &rcHandler);
    RaceChronoBle.startAdvertising();
    Serial.println("Bluetooth: En attente de connexion RaceChrono...");
    leds[0] = CRGB::Blue;
    FastLED.show();
}

/**
 * ========================================================================
 * FONCTION LOOP - Boucle principale
 * ========================================================================
 * Cette fonction s'exécute en permanence après setup()
 * Elle:
 * 1. Écoute les messages CAN entrants
 * 2. Filtre les messages par ID
 * 3. Envoie les données via BLE à RaceChrono
 * 4. Maintient la connexion BLE active
 */
void loop() {
    // Structure pour stocker le message CAN reçu
    twai_message_t msg;
    
    // ========================================================================
    // TRAITEMENT DES MESSAGES CAN
    // ========================================================================
    // twai_receive() retourne ESP_OK si un message est disponible
    // Le paramètre 0 = pas de timeout (non-bloquant)

    /*uint8_t DataTest[1] = {0x18};
    uint8_t ValdataTest[1] = {DataTest[0]/8};
    //if (DataTest[0] >= 0x20) {
    //    ValdataTest[0] = {100};
    //}
    //else {
    //    ValdataTest[0] = {0};
    //}
    RaceChronoBle.sendCanData(0x050, ValdataTest, 1);
    Serial.print("CAN Bus: ID 0x050 envoyer ");
    Serial.print(ValdataTest[0]);
    Serial.println();*/
    
    while (twai_receive(&msg, 0) == ESP_OK) {
        
        // ========================================================================
        // INTERPRÉTATION DES MESSAGES SELON LEUR ID CAN
        // ========================================================================
        // Chaque ID correspond à un type de données spécifique sur la CBR 600 RR
        // Les données sont extraites des bytes du message
    
        switch (msg.identifier) {              

            // ID 0x157: Rapport + Voyant TC + Vitesses roues avant/arrière
            case 0x157: {
                uint8_t dataGrear[1] = {msg.data[5]};
                if (dataGrear[0] <= 0x30) {// Filtre pour éviter les valeurs aberrantes (ex: 0x38)
                    if (dataGrear[0] != MemodataGrear) {//Actu que si changement de rapport
                        MemodataGrear = {dataGrear[0]};
                        uint8_t ValdataGrear[1] = {dataGrear[0]/8};                      
                        RaceChronoBle.sendCanData(0x050, ValdataGrear, 1);
                    }
                }

                uint8_t dataV_Av[2] = {msg.data[3],msg.data[4]};
                uint8_t ValdataV_Av[2] = { dataV_Av[0], dataV_Av[1] };
                RaceChronoBle.sendCanData(0x051, ValdataV_Av, 2);

                uint8_t dataV_Ar[2] = {msg.data[1],msg.data[2]};
                uint8_t ValdataV_Ar[2] = { dataV_Ar[0], dataV_Ar[1] };
                RaceChronoBle.sendCanData(0x052, ValdataV_Ar, 2);

                uint8_t ActionTC[1] = {msg.data[0]};
                uint8_t ValAtionTC[1] = {0};
                if (ActionTC[0] >= 0x20) {
                     ValAtionTC[0] = {100};
                }
                else {
                    ValAtionTC[0] = {0};
                }
                RaceChronoBle.sendCanData(0x053, ValAtionTC, 1);

                break;
            }
          
            // ID 0x1DC: Régime moteur (RPM)
            case 0x1DC: {
                uint8_t data_Rpm[2] = { msg.data[2], msg.data[3] };
                uint8_t ValData_Rpm[2] = { data_Rpm[0], data_Rpm[1] };
                RaceChronoBle.sendCanData(0x01DC, ValData_Rpm, 2);
                break;
           }

           // ID 0x193: Vitesse Général Compteur
            case 0x193: {
                uint8_t data_V_G[2] = { msg.data[0], msg.data[1] };
                uint8_t ValData_V_G[2] = { data_V_G[0], data_V_G[1] };
                RaceChronoBle.sendCanData(0x0193, ValData_V_G, 2);
                break;
           }

           // ID 0x15B: Position poignée des gaz
            case 0x15B: {
                uint8_t dataGaz[2] = { msg.data[4], msg.data[5] };
                uint8_t ValDataGaz[2] = { dataGaz[0], dataGaz[1] };
                RaceChronoBle.sendCanData(0x015B, ValDataGaz, 2);
                break;
           }
                 
            // ID 0x154: Paramètre moto
            case 0x154: {
                // Map (bytes 0)
                uint8_t ParaMap[1] = { msg.data[0] };
                if (ParaMap[0] != MemoParaMap) {
                    MemoParaMap = {ParaMap[0]};
                    uint8_t ValParaMap[1] = { ParaMap[0]/16 };
                    RaceChronoBle.sendCanData(0x100, ValParaMap, 1);
                }
                             
                // TC (bytes 1)
                uint8_t ParaTC[1] = { msg.data[1] };
                if (ParaTC[0] != MemoParaTC) { // Actu que si changement de paramètre
                    MemoParaTC = {ParaTC[0]};
                    uint8_t ValParaTC[1] = { ParaTC[0]/16};
                    RaceChronoBle.sendCanData(0x101, ValParaTC, 1);
                }
                
                // Anti Wheelling et Frein moteur (byte 2)
                uint8_t ParaW[1] = { msg.data[2] }; 
                if (ParaW[0] != MemoParaW) { // Actu que si changement de paramètre
                    MemoParaW = {ParaW[0]};           
                    uint8_t  CaseParaW= (ParaW[0] & 0x0F);  // chiffre octal de poids faible
                    uint8_t ValParaW[1]= {CaseParaW};
                    switch (CaseParaW) {
                        case 0x4:
                            ValParaW[0]= 0x1;
                        break;
                        case 0x8:
                            ValParaW[0]= 0x2;
                        break;
                        case 0xC:
                            ValParaW[0]= 0x3;
                        break;
                        default:
                            ValParaW[0]= 0x0;
                        break;
                    }
                    RaceChronoBle.sendCanData(0x102, ValParaW, 1);
                }
                               
                // Frein moteur (byte 2)
                uint8_t ParaEB[1] = { msg.data[2] };
                if (ParaEB[0] != MemoParaEB) { // Actu que si changement de paramètre
                    MemoParaEB = {ParaEB[0]};
                    uint8_t  CaseParaEB= ((ParaEB[0] >> 4) & 0x0F);  // chiffre octal de poids fort               
                    uint8_t ValParaEB[1]= {CaseParaEB};
                    switch (CaseParaEB) {
                        case 0x1:
                            ValParaEB[0]= 0x1;
                        break;
                        case 0x2:
                            ValParaEB[0]= 0x2;
                        break;
                        case 0x3:
                            ValParaEB[0]= 0x3;
                        break;
                        default:
                            ValParaEB[0]= 0x0;
                        break;
                    }
                    RaceChronoBle.sendCanData(0x103, ValParaEB, 1);
                }
                
                Inc=Inc+1;
                Serial.print("Val Inc ");
                Serial.print(Inc);
                Serial.println();
                if (Inc >= ActuSlow) {// Forcer une actu tous les x=(Val de ActuSlow) si perte de trame BLE
                        Inc = 0;
                        MemodataGrear = 255;
                        MemoParaMap = 255;
                        MemoParaTC = 255;
                        MemoParaW = 255;
                        MemoParaEB = 255;
                }
                
               
            
            break;

            }


//////////////////////////////////////////////////////////////////////////////////////////////
              /* Exemple KTM 790:
                // ID 0x12A: Position du papillon (TPS)
            case 0x12A: {
                uint8_t data[1] = { msg.data[0] };
                RaceChronoBle.sendCanData(0x0104, data, 1);
                break;
            }*/

            // ID 0x12B: Vitesses des roues et données IMU
            /*case 0x12B: {
                // Vitesse roue arriere (bytes 0-1)
                uint8_t dataRear[2] = { msg.data[0], msg.data[1] };
                RaceChronoBle.sendCanData(0x012B, dataRear, 2);
                
                // Vitesse roue avant (bytes 2-3)
                uint8_t dataFront[2] = { msg.data[2], msg.data[3] };
                RaceChronoBle.sendCanData(0x012A, dataFront, 2);
                
                // Angle d'inclinaison (byte 4)
                uint8_t dataLean[1] = { msg.data[4] };
                RaceChronoBle.sendCanData(0x012C, dataLean, 1);
                
                // Angle de wheelie (byte 5)
                uint8_t dataPitch[1] = { msg.data[5] };
                RaceChronoBle.sendCanData(0x012D, dataPitch, 1);
                break;
            }*/

            // ID 0x290: Pressions de frein et état ABS
            /*case 0x290: {
                // Pression frein avant (bytes 0-1)
                uint8_t dataBrkF[2] = { msg.data[0], msg.data[1] };
                RaceChronoBle.sendCanData(0x0290, dataBrkF, 2);
                
                // Pression frein arriere (bytes 2-3)
                uint8_t dataBrkR[2] = { msg.data[2], msg.data[3] };
                RaceChronoBle.sendCanData(0x0291, dataBrkR, 2);
                break;
            }*/
 ////////////////////////////////////////////////////////////////////////////////////////////////////////          
            
        }
    }

    // ========================================================================
    // MAINTIEN DE LA CONNEXION BLUETOOTH
    // ========================================================================
    // On vérifie si la connexion est toujours active
    // Si déconnecté, on redémarre la publicité pour permettre une nouvelle connexion
    if (!RaceChronoBle.isConnected()) {
        Serial.println("RaceChrono deconnecte, redemarrage de la publicite...");
        leds[0] = CRGB::Blue;
        FastLED.show();
        RaceChronoBle.startAdvertising();
    }   else{
        //Serial.println("RaceChrono connecte");
        leds[0] = CRGB::Green;
        FastLED.show();
    }
    
    
    // Petit délai pour éviter de saturer le BLE
    delay(40);
}
