# 🎨 Design visuel de l'interface

## Vue d'ensemble (800x480 pixels)

```
╔════════════════════════════════════════════════════════════════════════════╗
║                    ⛏ BITAXE COMMAND CENTER ⛏                              ║ 60px
║                         [Titre rouge sur fond gris]                        ║
╠════════════════════════════════════════════════════════════════════════════╣
║                                                                            ║
║  ┌─────────────┐     ┌────────────────────────┐     ┌─────────────┐      ║
║  │⚙ MINERS    │     │                        │     │₿ BITCOIN    │      ║
║  │STATUS      │     │    CENTRAL DISPLAY     │     │NETWORK      │      ║
║  │            │     │                        │     │             │      ║
║  │Active: 3/3 │     │     12:34:56          │     │Price: $42K  │      ║
║  │Total:      │     │                        │     │             │      ║ 300px
║  │  1500 GH/s │     │   (swipe ◄ ►)         │     │Block: 87000 │      ║
║  │Temp: 65°C  │     │                        │     │             │      ║
║  │            │     │                        │     │Fees: 5 sat/vB│     ║
║  └─────────────┘     └────────────────────────┘     └─────────────┘      ║
║   220x150px                 360x300px                  220x150px          ║
║                                                                            ║
╠════════════════════════════════════════════════════════════════════════════╣
║                       [Barre d'alertes / Status]                          ║ 40px
╚════════════════════════════════════════════════════════════════════════════╝
```

## Panneau gauche - Miners Status (220x150px)

```
┌─────────────────────┐
│ ⚙ MINERS STATUS     │ ← Titre rouge 16px
├─────────────────────┤
│                     │
│ Active: 3/3         │ ← Blanc 14px
│                     │
│ Total: 1500 GH/s    │ ← Blanc 14px
│                     │
│ Temp: 65.2°C        │ ← Vert/Orange/Rouge selon temp
│                     │
└─────────────────────┘
  Fond: #1A1A1A
  Bordure: Rouge brillant avec glow
```

## Panneau central - Display principal (360x300px)

```
┌────────────────────────┐
│                        │
│                        │
│     12:34:56          │ ← Police 48px rouge
│                        │
│                        │
│                        │
│      ◄ Swipe ►        │ ← Indication 14px gris
└────────────────────────┘
  Fond: #1A1A1A
  Bordure: Crimson avec glow intense
  
  3 vues alternées:
  1. Heure (HH:MM:SS)
  2. Prix Bitcoin ($42,000)
  3. Hashrate (1500 GH/s)
```

## Panneau droit - Bitcoin Network (220x150px)

```
┌─────────────────────┐
│ ₿ BITCOIN NETWORK   │ ← Titre rouge 16px
├─────────────────────┤
│                     │
│ Price: $42,000      │ ← Blanc 14px
│                     │
│ Block: 870,000      │ ← Blanc 14px
│                     │
│ Fees: 5 sat/vB      │ ← Blanc 14px
│                     │
└─────────────────────┘
  Fond: #1A1A1A
  Bordure: Rouge brillant avec glow
```

## Barre supérieure (800x60px)

```
╔════════════════════════════════════════════════════════╗
║        ⛏ BITAXE COMMAND CENTER ⛏                      ║
╚════════════════════════════════════════════════════════╝
  Fond: #333333
  Texte: Rouge #FF0000, 24px
  Bordure: Rouge avec shadow glow
```

## Barre inférieure - Alertes (800x40px)

```
╔════════════════════════════════════════════════════════╗
║  [Zone d'alertes et notifications]                     ║
╚════════════════════════════════════════════════════════╝
  
  États:
  - Normal: Fond #333333
  - Alerte: Fond rouge clignotant #FF0000
  - Warning: Fond orange #FFA500
```

## Palette de couleurs

### Couleurs principales
```
Fond principal:     #0A0A0A  (Noir profond)
Fond panneaux:      #1A1A1A  (Gris très foncé)
Fond barres:        #333333  (Gris foncé)

Rouge principal:    #FF0000  (Rouge vif)
Rouge foncé:        #8B0000  (Bordeaux)
Crimson:            #DC143C  (Cramoisi)

Texte normal:       #FFFFFF  (Blanc)
Texte secondaire:   #CCCCCC  (Gris clair)
```

### États de couleur pour la température
```
< 75°C:   #FFFFFF  (Blanc - Normal)
75-85°C:  #FFA500  (Orange - Attention)
> 85°C:   #FF0000  (Rouge - Critique, clignotant)
```

## Animations

### 1. Animation nouveau bloc
```
┌────────────────────┐
│                    │
│   ●●●●●●●●●●●●●   │  Expansion circulaire
│  ●           ●   │  du centre vers l'extérieur
│  ●     ⚡    ●   │  Couleur: Rouge → Transparent
│  ●           ●   │  Durée: 1000ms
│   ●●●●●●●●●●●●●   │
│                    │
└────────────────────┘
```

### 2. Effet de swipe
```
Vue 1          →         Vue 2          →         Vue 3
┌────┐               ┌────┐               ┌────┐
│12:34│  slide right │$42K│  slide right │1500│
└────┘               └────┘               └────┘

Transition: 300ms ease-out
```

### 3. Glow effect sur bordures
```
Normal state:
┌─────────┐  Bordure: 2px solid #FF0000
│         │  Shadow: 5px #FF0000
└─────────┘

Glow state:
┏━━━━━━━━━┓  Bordure: 2px solid #FF0000
┃    ✨   ┃  Shadow: 20px #FF0000 (spread)
┗━━━━━━━━━┛  Pulse animation
```

### 4. Alerte clignotante
```
État 1 (500ms):        État 2 (500ms):
┌─────────┐            ┌─────────┐
│  ROUGE  │    ↔      │  GRIS   │
└─────────┘            └─────────┘
#FF0000                #333333

Répète jusqu'à résolution
```

## Typographie

### Polices utilisées
```
Montserrat 48px - Affichage central (heure/prix/hashrate)
Montserrat 32px - Messages d'alerte importants
Montserrat 24px - Titre principal
Montserrat 20px - Sous-titres
Montserrat 16px - Titres de panneaux
Montserrat 14px - Texte normal
Montserrat 12px - Texte secondaire
```

### Style de texte
- **Titles**: Bold, Rouge #FF0000
- **Data**: Regular, Blanc #FFFFFF
- **Labels**: Regular, Gris #CCCCCC
- **Alerts**: Bold, Rouge clignotant

## Zones tactiles

### Swipe zones
```
←──────────────────────────────────────→
     Zone de détection du swipe
     Toute la surface du panneau central
     Seuil: 100px de mouvement horizontal
     
     Swipe droit: Vue suivante
     Swipe gauche: Vue précédente
```

### Zones cliquables futures (v2.0)
```
[Panneau gauche]   → Détails des mineurs
[Panneau droit]    → Détails blockchain
[Barre supérieure] → Menu de configuration
[Barre inférieure] → Historique des alertes
```

## Responsive behavior

### Orientation portrait (non supporté actuellement)
```
L'écran est conçu pour l'orientation landscape (800x480)
Pour support portrait, rotation à 90° recommandée
```

### Scaling
```
Base: 800x480
Les éléments sont positionnés en valeurs absolues
Pour autres résolutions, ajuster les constantes dans le code
```

## État des éléments

### États normaux
```
✅ Tous les mineurs actifs
✅ Température normale (< 75°C)
✅ WiFi connecté
✅ Données à jour

Interface: Bordures rouges fixes
          Fond noir/gris foncé
          Texte blanc
```

### États d'alerte
```
⚠️  Température élevée (75-85°C)
    → Texte température en orange
    
❌ Température critique (> 85°C)
    → Texte en rouge
    → Bordure supérieure clignote
    
❌ Mineur hors ligne
    → Barre inférieure clignote rouge
    → Compte "Active" en rouge
    
❌ Perte WiFi
    → Tous les panneaux grisés
    → Message "Reconnexion..." 
```

## Performance

### Frame rate cible
```
Interface: 30 FPS minimum
Animations: 60 FPS
LVGL refresh: 16ms (62.5 FPS)
```

### Optimisations
```
- Double buffering activé
- Anti-aliasing sur les textes
- Shadow caching pour les glows
- Dirty rectangle pour repaints partiels
```

---

**Design inspiré par le style Bitaxe et l'esthétique cyberpunk/gothique! 🔥**
