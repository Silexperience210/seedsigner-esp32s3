# 🎨 Enclosures / Boîtiers

## Designs disponibles

### 1. `seedsigner_cores3_compact/` (Recommandé)

Boîtier compact et minimaliste pour M5Stack CoreS3 avec NFC intégré.

**Caractéristiques:**
- Dimensions: 75×60×25mm
- Accès NFC latéral
- Support pour tamper switch
- Montage par vis M2.5

**Fichiers:**
- `case_bottom.stl` - Partie inférieure
- `case_top.stl` - Partie supérieure
- `nfc_spacer.stl` - Entretoise module NFC
- `button_cap.stl` - Capuchons boutons

**Impression:**
- Matériau: PETG recommandé (résistance)
- Remplissage: 30%
- Supports: Non (sauf pour overhangs)
- Épaisseur parois: 1.2mm

### 2. `seedsigner_cores3_secure/`

Version haute sécurité avec:
- Tamper switch intégré
- Blindage visuel (pas de vue sur écran depuis côtés)
- Fermeture sécurisée

**Fichiers:**
- `secure_case.stl`
- `tamper_switch_mount.stl`
- `security_seal_template.pdf`

### 3. `nfc_tag_holders/`

Porte-tags NFC pour backup physique:
- `keychain_tag.stl` - Porte-clés
- `wallet_card.stl` - Format carte de crédit

---

## Instructions d'assemblage

### Version Compact

```
1. Imprimer case_bottom.stl (orientation: plat)
2. Imprimer case_top.stl
3. Imprimer nfc_spacer.stl
4. Positionner M5Stack CoreS3 dans case_bottom
5. Connecter module NFC avec Grove cable
6. Positionner module NFC sur spacer
7. Refermer avec case_top
8. Fixer avec 4 vis M2.5×8mm
```

---

## Hardware nécessaire

| Item | Qté | Notes |
|------|-----|-------|
| Vis M2.5×8mm | 4 | Fixation boîtier |
| Vis M2.5×5mm | 2 | Fixation NFC module |
| Rondelles M2.5 | 4 | Distribution pression |
| Tamper switch | 1 | Optionnel |
| Security seals | 2 | Stickers inviolables |

---

## Customization

### Couleurs recommandées

| Pièce | Couleur | Raison |
|-------|---------|--------|
| Case | Noir/Gris foncé | Discrétion |
| Boutons | Orange | Thème Bitcoin |
| NFC spacer | Transparent | Visibilité LED |

### Labels

Utiliser le fichier `seedsigner_label.svg` pour découpe vinyl ou impression.
