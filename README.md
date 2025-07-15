# 🛸 G1 Space Invaders

Remake del clásico **Space Invaders**, desarrollado en C, con compatibilidad para PC y Raspberry Pi. Usa gráficos y sonidos retro para recrear una experiencia arcade fiel.

---

## 🎮 Características

- Movimiento y disparo del jugador.
- Oleadas de invasores alienígenas con patrones de ataque clásicos.
- Efectos de sonido originales y música retro.
- Interfaz gráfica con imágenes y tipografía estilo arcade.
- Compatible tanto con **PC** como con **Raspberry Pi**.
- Puntuación, vidas, niveles y efectos visuales originales.

---

## 🗂️ Estructura del Proyecto

```
G1_Space-Invaders-main/
├── assets/              # Recursos multimedia: imágenes, sonidos, fuentes
├── src/                 # Código fuente del juego
│   ├── main.c           # Entrada principal
│   ├── game.c           # Lógica del juego
│   ├── playSound.c      # Manejo de audio
│   └── frontend/        # Interfaces específicas para PC o Raspberry Pi
├── Makefile             # Compilación automática
├── .vscode/             # Configuraciones del entorno VSCode
└── README.md            # Este archivo
```

---

## ⚙️ Requisitos

### En PC (Linux):
- `gcc`
- `make`
- [Allegro 5](https://liballeg.org/)

Instalación de dependencias en Ubuntu/Debian:

```bash
sudo apt update
sudo apt install liballegro5-dev make gcc
```

---

## ▶️ Compilación y Ejecución

```bash
make
./space_invaders
```

---

## 🖼️ Créditos Visuales y Sonoros

- Fuentes de estilo arcade: `Invaders-From-Space.ttf`, `pixel-invaders.ttf`, etc.
- Sonidos inspirados en el Space Invaders original.
- Imágenes propias y adaptadas con estética retrofuturista.

---

## 📦 Autoría

Grupo 1  
Proyecto de programación en C — 2025  
Universidad o institución correspondiente.

---

## 🕹️ Licencia

Uso académico y recreativo. Si vas a reutilizar recursos gráficos o sonidos, respetá sus licencias originales.
