#!/bin/bash

# Пути к файлам
SO_SOURCE="/home/whoisheremoron/wimor/build/libgamesneeze.so"
GAME_BIN_DIR="/home/whoisheremoron/.local/share/Steam/steamapps/common/csgo legacy/bin"

# 1. Копируем свежую либу в папку игры
echo "[*] Копирование libgamesneeze.so в директорию игры..."
cp "$SO_SOURCE" "$GAME_BIN_DIR/"

# 2. Ищем PID запущенной игры
PID=$(pidof csgo_linux64)

if [ -z "$PID" ]; then
    echo "[!] Ошибка: CS:GO Legacy не запущена! Сначала запусти игру."
    exit 1
fi

echo "[*] Найдена запущенная игра (PID: $PID)"
echo "[*] Инжектим чит..."

# 3. Наш победный инжект через GDB
sudo gdb -p $PID --batch \
  -ex 'set sysroot target:/' \
  -ex 'set solib-search-path target:/bin/linux64' \
  -ex 'p (void*)dlopen("libgamesneeze.so", 1)'

echo "[+] Готово! Меню чита открывается на кнопку Insert."
