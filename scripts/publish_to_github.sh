#!/usr/bin/env bash
# Script auxiliar para vincular y subir ARCEUS GRID a GitHub

set -e

REPO_NAME="arceus_grid"
GITHUB_USER="LNieto-V"

echo "=========================================================="
echo "  ARCEUS GRID — Vinculación y Publicación en GitHub       "
echo "=========================================================="

echo "Verificando estado del repositorio local..."
cd "$(dirname "$0")/.."

git status

echo ""
echo "Opciones de vinculación:"
echo " 1) SSH:   git@github.com:${GITHUB_USER}/${REPO_NAME}.git"
echo " 2) HTTPS: https://github.com/${GITHUB_USER}/${REPO_NAME}.git"
echo ""

read -p "Selecciona protocolo [1 para SSH, 2 para HTTPS] (default: 1): " PROTO
PROTO=${PROTO:-1}

if [ "$PROTO" = "1" ]; then
    REMOTE_URL="git@github.com:${GITHUB_USER}/${REPO_NAME}.git"
else
    REMOTE_URL="https://github.com/${GITHUB_USER}/${REPO_NAME}.git"
fi

if git remote | grep -q "^origin$"; then
    echo "Actualizando remote 'origin' a: $REMOTE_URL"
    git remote set-url origin "$REMOTE_URL"
else
    echo "Añadiendo remote 'origin': $REMOTE_URL"
    git remote add origin "$REMOTE_URL"
fi

git branch -M main

echo ""
echo "Empujando rama 'main' a GitHub ($REMOTE_URL)..."
git push -u origin main

echo ""
echo "=========================================================="
echo " ¡Repositorio publicado exitosamente en GitHub!          "
echo " URL: https://github.com/${GITHUB_USER}/${REPO_NAME}     "
echo "=========================================================="
