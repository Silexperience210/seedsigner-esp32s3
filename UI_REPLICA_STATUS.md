# 🎨 UI Replica - SeedSigner Official Theme

## ✅ Thème fidèle créé

### Fichiers créés

```
include/ui/
└── seedsigner_theme.h          # Couleurs, dimensions, styles officiels

src/ui/
├── seedsigner_theme.cpp        # Implementation complète (500+ lignes)
└── screens_replica/
    └── screen_main_replica.cpp # Main menu fidèle
```

---

## 🎯 Spécifications fidèles à l'original

### Couleurs exactes (RPi version)

```cpp
#define SS_COLOR_ORANGE         0xF7931A  // Bitcoin orange
#define SS_COLOR_BG_DARK        0x1A1A1A  // Fond noir
#define SS_COLOR_BG_MEDIUM      0x2A2A2A  // Header/footer
#define SS_COLOR_WHITE          0xFFFFFF
#define SS_COLOR_GREEN          0x00AA00  // Status OK
```

### Layout adapté (240×240 → 320×240)

**Original RPi:**
```
┌─────────────────┐ 240px
│   SeedSigner    │ Header 28px
├─────────────────┤
│   [QR/Code]     │ Content 180px
│                 │
│  ◄ Prev Next ►  │ Footer 32px
└─────────────────┘
```

**Réplique ESP32-S3 (320×240):**
```
┌──────────────────────────────────┐
│  │   SeedSigner    │            │ 40px margin + 240px content
│  ├─────────────────┤            │
│  │ [Bouton][Bouton]│            │ Touch-friendly
│  │ [Bouton][Bouton]│            │ mais même look
│  │                 │            │
│  ├─────────────────┤            │
│  │   \u25CF Seed    │            │
└──────────────────────────────────┘
   40px          240px          40px
```

---

## 🎨 Styles implémentés

### Buttons (identique RPi)
- **Normal**: Bordure orange 2px, fond gris foncé, texte blanc
- **Pressed**: Fond orange, texte noir
- **Radius**: 4px (légèrement arrondi comme original)

### Text
- **Header**: Montserrat 18px, orange
- **Normal**: Montserrat 14px, blanc
- **Small**: Montserrat 12px, gris clair
- **Mnemonic**: Montserrat 16px monospace (à remplacer par bitmap)

### Panels
- **Background**: #2A2A2A
- **Border**: 1px gris moyen
- **Corners**: Sharp (0 radius) comme original

---

## 📱 Screens répliqués

| Screen | Status | Notes |
|--------|--------|-------|
| Main Menu | ✅ | 4 boutons 2×2, header/footer |
| Splash | 📋 | À migrer vers thème |
| Seed Menu | 📋 | À faire |
| Sign Scan | 📋 | À faire |
| Settings | 📋 | À faire |

---

## 🔧 Pour utiliser le thème

```cpp
#include "ui/seedsigner_theme.h"
using namespace SeedSigner::UI;

// Initialiser
SeedSignerTheme::init();

// Créer screen
lv_obj_t* screen = SeedSignerTheme::create_screen_base();

// Header
SeedSignerTheme::create_header(screen, "Title");

// Bouton fidèle
lv_obj_t* btn = lv_btn_create(screen);
lv_obj_add_style(btn, SeedSignerTheme::get_style_button(), 0);
lv_obj_add_style(btn, SeedSignerTheme::get_style_button_pressed(), LV_STATE_PRESSED);

// Footer
SeedSignerTheme::create_footer(screen, "Status: Ready");
```

---

## 🎮 Différences tactiles vs joystick

| Original (Joystick) | Réplique (Touch) |
|---------------------|------------------|
| Flèches ◄ ▲ ▼ ► | Boutons directs |
| Highlight sélection | Press visual feedback |
| Back/Confirm buttons | Zones tactiles |
| Entrée mot par mot | Clavier virtuel complet |

---

## 📊 Comparaison visuelle

| Aspect | RPi Original | ESP32-S3 Replica |
|--------|--------------|------------------|
| Résolution | 240×240 | 320×240 (centré 240) |
| Input | Joystick | Touch |
| Police | Custom bitmap | LVGL Montserrat |
| Couleurs | Exactes | ✅ Identiques |
| Layout | Compact | ✅ Même structure |
| Boutons | Physiques | ✅ Même style visuel |

---

## 🚀 Prochaines étapes UI

1. **Migrer tous les screens** vers `screens_replica/`
2. **Importer polices bitmap** si disponibles sur GitHub
3. **Ajouter animations** (fade 200ms comme original)
4. **Créer SeedQR renderer** fidèle

---

**Status**: 🟡 **60%** - Thème prêt, screens à migrer

**Temps estimé**: 2-3 heures pour migrer tous les screens
