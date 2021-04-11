# triangle

Сборка под `Manjaro Linux x86_64`:
требуемые пакеты: 
```bash
> pacman -Q | grep x11
lib32-libx11 1.7.0-1
libx11 1.7.0-4
```
- https://wiki.archlinux.org/index.php/OpenGL
- https://wiki.archlinux.org/index.php/Xorg_(%D0%A0%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9)

Сборка под `Ubuntu`(сам не собирал):
`sudo apt-get install libxss-dev libxxf86vm-dev libxkbfile-dev libxv-dev`
`sudo apt-get install libglu1-mesa-dev mesa-common-dev`

Более подробно смотреть в `CMakeLists.txt`
