#!/bin/bash

# Пути к файлам
SO_SOURCE="/home/whoisheremoron/wimor/build/libwimor.so"
GAME_BIN_DIR="/home/whoisheremoron/.local/share/Steam/steamapps/common/csgo legacy/bin"

function print_logo {
    echo -e " \n\
\e[97m _    _ \e[38;2;25;1;145m____  ______________ \e[0m\n\
\e[97m| |  | |\e[38;2;25;1;145m(_)  \\\\/  |  _  | ___ \\\\\e[0m\n\
\e[97m| |  | |\e[38;2;25;1;145m|_| .  . | | | | |_/ /\e[0m\n\
\e[97m| |/\\| |\e[38;2;25;1;145m| | | |\\\\/| | | |    / \e[0m\n\
\e[97m\\\\  /\\\\  /\e[38;2;25;1;145m| |  | \\\\ \\\\_/ / |\\\\ \\\\ \e[0m\n\
\e[97m \\\\/  \\\\/\e[38;2;25;1;145m|_\\\\_|  |_/\\\\___/\\\\_| \\\\_|\e[0m\n\\
==========================================================="
}

print_logo

# 1. Копируем свежую либу в папку игры
echo -e "\e[97m[wm]\e[38;2;25;1;145m Копирование libwimor.so в директорию игры...\e[0m"
cp "$SO_SOURCE" "$GAME_BIN_DIR/"

# 2. Ищем PID запущенной игры
PID=$(pidof csgo_linux64)

if [ -z "$PID" ]; then
    echo "[!] Ошибка: CS:GO Legacy не запущена! Сначала запусти игру."
    exit 1
fi

echo -e "\e[97m[wm]\e[38;2;25;1;145m Найдена запущенная игра (PID: $PID)\e[0m"
echo -e "\e[97m[wm]\e[38;2;25;1;145m Инжектим чит...\e[0m"

# 3. Наш победный инжект через GDB
sudo gdb -p $PID --batch \
  -ex 'set sysroot target:/' \
  -ex 'set solib-search-path target:/bin/linux64' \
  -ex 'p (void*)dlopen("libwimor.so", 1)'

echo -e "\e[97m[wm]\e[38;2;25;1;145m Готово! Меню чита открывается на кнопку Insert.\e[0m"
