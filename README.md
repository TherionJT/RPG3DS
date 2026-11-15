# RPG para Nintendo 3DS

## Requisitos

El proyecto se compila con el toolchain de devkitPro:

- devkitARM
- libctru
- citro2d y citro3d
- `make`
- `tex3ds`, `smdhtool` y `3dsxtool`

En Linux o WSL, las variables principales deben apuntar a la instalación de
devkitPro:

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export CTRULIB=$DEVKITPRO/libctru
```

Si devkitPro está instalado en otra ruta, ajusta esos valores.

## Compilar la aplicación

Abre una terminal Linux/WSL y entra en la carpeta que contiene este
`Makefile`:

```bash
cd /home/morap/OneDrive/Desktop/Rpg3ds/RPG
make
```

En Windows, la misma carpeta puede verse como:

```text
C:\Users\morap\OneDrive\Desktop\Rpg3ds\RPG
```

El `Makefile` detecta automáticamente los archivos fuente de `source/` y las
escenas gráficas `.t3s` de `gfx/`. Durante la compilación:

1. Se compilan los archivos C en `build/`.
2. Se genera `RPG.elf`.
3. Cada archivo `.t3s` se convierte en `.t3x` dentro de `romfs/`.
4. Se genera `RPG.smdh` con los metadatos de la aplicación.
5. Se crea `RPG.3dsx`, incluyendo la carpeta `romfs/`.

Los archivos que se pueden ejecutar o distribuir son:

```text
RPG.3dsx
RPG.smdh
romfs/
```

## Compilación limpia

Para borrar los objetos, archivos generados y gráficos convertidos antes de
compilar de nuevo:

```bash
make clean
make
```

No borres manualmente `source/`, `include/`, `gfx/` ni `romfs/`: contienen
archivos del proyecto.

## Añadir recursos

- Los fuentes C nuevos deben colocarse en `source/`.
- Los encabezados deben colocarse en `include/`.
- Las escenas de gráficos deben colocarse en `gfx/` como archivos `.t3s`.
- Los sonidos y otros recursos deben colocarse en `romfs/`.

Al ejecutar `make`, los `.t3s` se convierten automáticamente a `.t3x` y se
incluyen en el RomFS de `RPG.3dsx`.

La música debe estar en:

```text
romfs/audio/exploracion.wav
romfs/audio/combate.wav
```

Para evitar pausas al cambiar de exploración a combate, las dos pistas se
precargan al iniciar. Se recomienda usar WAV PCM16 mono a 22050 Hz.

## Warnings y errores comunes

Los avisos `warning:` del compilador no detienen la compilación. El comando
solo falla si aparece `Error 1`, `Error 2` u otro mensaje de error.

- `Please set DEVKITARM`: define `DEVKITARM` antes de ejecutar `make`.
- `No rule to make target ... .t3x`: comprueba que exista el `.t3s`
  correspondiente dentro de `gfx/`.
- `Cannot open SMDH file`: ejecuta `make clean` y después `make`; el
  `Makefile` genera `RPG.smdh` automáticamente.
- `command not found`: comprueba que devkitPro esté instalado y que sus
  herramientas estén disponibles en `PATH`.
