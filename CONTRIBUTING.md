# Guía de Contribución a ARCEUS GRID

¡Gracias por tu interés en contribuir a **ARCEUS GRID**!

## 1. Convención de Commits (Conventional Commits)
Todo commit debe seguir el formato estándar:
`<tipo>(<alcance opcional>): <descripción imperativa>`

Ejemplos:
- `feat(coordinator): anadir exportacion de metricas a CSV`
- `fix(firmware): corregir reconexion de PubSubClient tras perdida de enlace`
- `docs(hardware): actualizar calculo de caida de tension en shunt`

## 2. Higiene de Git y Seguridad Eléctrica
- **Commits atómicos:** No acumular cambios no relacionados en un solo commit.
- **Sin artefactos de compilación:** Excluir siempre `.aux`, `.log`, `.pio`, `__pycache__` o binarios pesados.
- **Rigor de Hardware:** Nunca proponer modificaciones en la etapa de potencia sin justificación matemática de corriente, disipación térmica y verificación de aislamiento galvánico.

## 3. Flujo de Trabajo
1. Crea un Fork del repositorio.
2. Crea una rama para tu característica: `git checkout -b feature/nueva-mejora`.
3. Haz tus commits atómicos.
4. Envía un Pull Request describiendo detalladamente tus cambios.
