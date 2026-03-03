# 📝 Changelog

Toutes les modifications notables de ce projet seront documentées dans ce fichier.

Le format est basé sur [Keep a Changelog](https://keepachangelog.com/fr/1.0.0/),
et ce projet adhère au [Semantic Versioning](https://semver.org/).

## [1.0.0] - 2024-11-07

### ✨ Ajouté
- Interface utilisateur futuriste noir/rouge style gothique médiéval
- Support complet pour écran ESP32-4827S043 (800x480)
- Contrôleur tactile GT911 avec détection de swipe
- Monitoring en temps réel de plusieurs mineurs Bitaxe
- Affichage du hashrate total, température moyenne, nombre de mineurs actifs
- Intégration API mempool.space pour:
  - Prix Bitcoin en direct
  - Hauteur de bloc actuelle
  - Frais moyens recommandés
- Panneau central rotatif (swipe gauche/droite):
  - Affichage de l'heure
  - Prix Bitcoin
  - Hashrate total
- Animations:
  - Flash rouge lors de la découverte d'un nouveau bloc
  - Effets lumineux sur les bordures
  - Transitions fluides
- Alertes visuelles:
  - Clignotement rouge si mineur hors ligne
  - Changement de couleur selon la température
  - Alerte critique si température > 85°C
- Configuration WiFi via portail captif au premier démarrage
- Synchronisation NTP pour l'heure exacte
- Bibliothèque GT911 pour gestion tactile I2C
- Configuration centralisée via fichier config.h
- Support LVGL 8.3 pour l'interface graphique
- Documentation complète:
  - README.md - Guide principal
  - QUICKSTART.md - Démarrage rapide
  - ADVANCED.md - Configurations avancées
  - HARDWARE.md - Spécifications et câblage

### 🔧 Technique
- ESP32-S3 avec PSRAM
- TFT_eSPI pour l'affichage
- LVGL pour l'interface graphique
- ArduinoJson pour le parsing API
- WiFiManager pour la configuration réseau
- HTTPClient pour les requêtes API
- Architecture modulaire et maintenable

### 📋 Fichiers de configuration
- platformio.ini - Configuration du projet
- lv_conf.h - Configuration LVGL
- config.h - Paramètres utilisateur
- User_Setup.h - Configuration TFT_eSPI
- partitions.csv - Schéma de partition mémoire

## [À venir] - Version future

### 🚀 Prévu
- [ ] Mise à jour OTA (Over-The-Air)
- [ ] Mode nuit automatique
- [ ] Logs sur carte microSD
- [ ] Graphiques historiques du hashrate
- [ ] Notifications push (Telegram/Discord)
- [ ] Interface web complémentaire
- [ ] Support multi-pools
- [ ] Calcul automatique de rentabilité
- [ ] Statistiques détaillées par mineur
- [ ] Export des données en CSV
- [ ] Thèmes personnalisables
- [ ] Support d'autres langues (français, espagnol, allemand)
- [ ] Gestion multi-utilisateurs
- [ ] API REST pour intégrations tierces

### 💡 Idées en considération
- [ ] Support pour d'autres mineurs (S9, Whatsminer)
- [ ] Alertes sonores via buzzer
- [ ] Capteurs de température/humidité ambiante
- [ ] Contrôle du ventilateur des Bitaxe
- [ ] Redémarrage automatique des mineurs en panne
- [ ] Dashboard mobile (app iOS/Android)
- [ ] Intégration Home Assistant
- [ ] Support Lightning Network
- [ ] Affichage de la difficulté du réseau
- [ ] Prévisions de rentabilité

### 🐛 Bugs connus
- Aucun bug majeur connu pour le moment

### ⚠️ Limitations actuelles
- Pas de chiffrement pour les credentials WiFi
- Pas de système de backup/restore de configuration
- Interface uniquement en anglais
- Limité à 10 mineurs maximum (pour performance)

## [0.9.0] - 2024-11-05 (Bêta)

### ✨ Version bêta initiale
- Prototype fonctionnel
- Tests internes
- Validation du concept

## Notes de version

### Version 1.0.0
Cette première version stable offre toutes les fonctionnalités de base pour monitorer efficacement vos mineurs Bitaxe avec un design ultra-futuriste. L'interface a été optimisée pour l'ESP32-S3 et offre une expérience fluide et réactive.

### Contributions
Nous accueillons les contributions de la communauté! Consultez le README.md pour plus d'informations sur comment contribuer.

### Support
Pour toute question ou problème, ouvrez une issue sur GitHub ou rejoignez notre Discord.

---

**🔥 Keep mining! ⚡**
