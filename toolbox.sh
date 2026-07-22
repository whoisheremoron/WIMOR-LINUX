#!/bin/bash

gdb="gdb"
game_bin_dir="/home/whoisheremoron/.local/share/Steam/steamapps/common/csgo legacy/bin"
csgo_pid=$(pidof csgo_linux64)

# Set to true to compile with clang. (if changing to true, make sure to delete the build directory from gcc)
export USE_CLANG="false"

if [[ $EUID -eq 0 ]]; then
    echo "You cannot run this as root." 
    exit 1
fi

rm -rf /tmp/dumps
mkdir -p --mode=000 /tmp/dumps

function unload {
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Unloading cheat...\e[0m"
    echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope > /dev/null
    if grep -q "libwimor.so" "/proc/$csgo_pid/maps" 2>/dev/null; then
        sudo $gdb -p $csgo_pid --batch \
            -ex 'set sysroot target:/' \
            -ex 'set solib-search-path target:/bin/linux64' \
            -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
            -ex "set \$dlclose = (int(*)(void*)) dlclose" \
            -ex "set \$library = \$dlopen(\"libwimor.so\", 6)" \
            -ex "call \$dlclose(\$library)" \
            -ex "call \$dlclose(\$library)" \
            -ex "detach" \
            -ex "quit"
    fi
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Unloaded!\e[0m"
}

function load {
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Loading cheat...\e[0m"
    echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope > /dev/null
    cp build/libwimor.so "$game_bin_dir/"
    gdbOut=$(
      sudo $gdb -p $csgo_pid --batch \
      -ex 'set sysroot target:/' \
      -ex 'set solib-search-path target:/bin/linux64' \
      -ex 'p (void*)dlopen("libwimor.so", 1)' 2> /dev/null
    )
    lastLine="${gdbOut##*$'\n'}"
    if [[ "$lastLine" != "\$1 = (void *) 0x0" ]]; then
      echo -e "\e[97m[wm]\e[38;2;25;1;145m Successfully injected!\e[0m"
    else
      echo "Injection failed, make sure you have compiled."
    fi
}

function load_debug {
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Loading cheat...\e[0m"
    echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope > /dev/null
    cp build/libwimor.so "$game_bin_dir/"
    sudo $gdb -p $csgo_pid --batch \
        -ex 'set sysroot target:/' \
        -ex 'set solib-search-path target:/bin/linux64' \
        -ex 'p (void*)dlopen("libwimor.so", 1)'
    sudo $gdb -p "$csgo_pid"
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Successfully loaded!\e[0m"
}

function build {
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Building cheat...\e[0m"
    mkdir -p build
    cd build
    cmake -D CMAKE_BUILD_TYPE=Release ..
    make -j $(nproc --all)
    cd ..
}

function build_debug {
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Building cheat...\e[0m"
    mkdir -p build
    cd build
    cmake -D CMAKE_BUILD_TYPE=Debug ..
    make -j $(nproc --all)
    cd ..
}

function pull {
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Stashing local changes...\e[0m"
    git stash
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Pulling latest updates...\e[0m"
    git pull
    echo -e "\e[97m[wm]\e[38;2;25;1;145m Reapplying local changes...\e[0m"
    git stash pop
}

while [[ $# -gt 0 ]]
do
keys="$1"
case $keys in
    -u|--unload)
        unload
        shift
        ;;
    -l|--load)
        load
        shift
        ;;
    -ld|--load_debug)
        load_debug
        shift
        ;;
    -b|--build)
        build
        shift
        ;;
    -bd|--build_debug)
        build_debug
        shift
        ;;
    -p|--pull)
        pull
        shift
        ;;
    -h|--help)
        echo "
 help
Toolbox script for wimor the beste lincuck cheat 2026
=======================================================================
| Argument             | Description                                  |
| -------------------- | -------------------------------------------- |
| -u (--unload)        | Unload the cheat from CS:GO if loaded.       |
| -l (--load)          | Load/inject the cheat via gdb.               |
| -ld (--load_debug)   | Load/inject the cheat and debug via gdb.     |
| -b (--build)         | Build to the build/ dir.                     |
| -bd (--build_debug)  | Build to the build/ dir as debug.            |
| -p (--pull)          | Update the cheat.                            |
| -h (--help)          | Show help.                                   |
=======================================================================

All args are executed in the order they are written in, for
example, \"-p -u -b -l\" would update the cheat, then unload, then build it, and
then load it back into csgo.
"
        exit
        ;;
    *)
        echo "Unknown arg $1, use -h for help"
        exit
        ;;
esac
done
