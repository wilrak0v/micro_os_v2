#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "gfx/gfx.hpp"

class LGFX_Local : public lgfx::LGFX_Device {
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Panel_ILI9341 _panel_instance;

public:
    LGFX_Local(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 60000000;
            cfg.freq_read  = 16000000;
            cfg.pin_sclk = 14; 
            cfg.pin_mosi = 13; 
            cfg.pin_miso = 12;
            cfg.pin_dc   = 22;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 15; 
            cfg.pin_rst          = 4;
            cfg.pin_busy         = -1;
            cfg.memory_width     = 240;
            cfg.memory_height    = 320;
            cfg.panel_width      = 240;
            cfg.panel_height     = 320;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }
};

LGFX_Local lcd;
Renderer renderer{&lcd};

// --- 4. VARIABLES DE JEU (PHYSIQUE) ---
int box_x = 50;
int box_y = 50;
int box_size = 20;

int vel_x = 3; // Vitesse X (pixels par frame)
int vel_y = 2; // Vitesse Y

int old_box_x = 50;
int old_box_y = 50;

void setup() {
    lcd.init();
    lcd.setRotation(1); // Mode Paysage (320x240)
    lcd.clear(TFT_BLACK);
}

void loop() {
    // A. Sauvegarde de l'ancienne position pour pouvoir l'effacer
    old_box_x = box_x;
    old_box_y = box_y;

    // B. Mise à jour de la physique (Déplacement)
    box_x += vel_x;
    box_y += vel_y;

    // C. Gestion des rebonds sur les bordures de l'écran (320x240)
    if (box_x <= 0 || box_x + box_size >= 320) {
        vel_x = -vel_x; // Inverse la vitesse X
    }
    if (box_y <= 0 || box_y + box_size >= 240) {
        vel_y = -vel_y; // Inverse la vitesse Y
    }

    // D. Enregistrement des intentions de dessin
    // 1. On demande à effacer l'ancien carré (génère le dirty rect N)
    renderer.fillRect(old_box_x, old_box_y, box_size, box_size, TFT_BLACK);

    // 2. On demande à dessiner le nouveau carré (génère le dirty rect N+1)
    renderer.fillRect(box_x, box_y, box_size, box_size, TFT_GREEN);

    // E. Rendu final optimisé
    renderer.render();

    delay(16); // ~60 FPS
}