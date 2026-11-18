# Presentación LaTeX Beamer (Modo Claro) — ARCEUS GRID

Presentación académica/técnica en **LaTeX Beamer (16:9, Modo Claro)** sincronizada **1 a 1 con las 12 diapositivas** de la carpeta [`/diapositivas`](../diapositivas) del repositorio (`00_INICIALIZACION_Y_APERTURA.md` a `06_MODELO_MATEMATICO_EXPLICADO.md`).

Incluye:
- **Explicaciones técnicas puntuales y diagramas vectoriales TikZ** (sin cajas vacías de fotos).
- **Barra `Guía Expositor`** al pie de cada diapositiva con la idea clave y orden de explicación para guiarse durante la sustentación.
- **4 vistas reales de la interfaz Streamlit en Modo Claro** integradas en las diapositivas 8, 10 y 11.

## 📁 Archivos

- [`presentacion_arceus_grid.tex`](./presentacion_arceus_grid.tex): Código fuente LaTeX Beamer (12 diapositivas).
- [`presentacion_arceus_grid.pdf`](./presentacion_arceus_grid.pdf): Presentación compilada lista para proyectar.
- [`Makefile`](./Makefile): Compilación (`make`) y limpieza automática de archivos auxiliares.

## 🖥️ Imágenes de la Interfaz (Modo Claro, en `figuras/`)

1. [`figuras/interfaz_monitoreo.png`](./figuras/interfaz_monitoreo.png) — **Diapositiva 8:** Pestaña *Monitoreo en Tiempo Real* ($V, I, P$ y relés).
2. [`figuras/interfaz_fedavg.png`](./figuras/interfaz_fedavg.png) — **Diapositiva 10:** Pestaña *Aprendizaje Federado (FedAvg)* y curva de convergencia MSE.
3. [`figuras/interfaz_control.png`](./figuras/interfaz_control.png) — **Diapositiva 11:** Pestaña *Control Remoto* de Bombillas 1/2 y pilotos 12V.
4. [`figuras/interfaz_alarmas.png`](./figuras/interfaz_alarmas.png) — **Diapositiva 11:** Pestaña *Registro Central de Alarmas*.
