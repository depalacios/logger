# Building

## Modes

This project can be built in multiple modes depending on which backends you want:

- **C only**: Console/File
- **C + Tracy**: TracyClient compiled in
- **C++ + Quill**: Quill backend compiled in
- **C++ + Quill + Tracy**: both compiled; runtime composite can fan-out

## Compile flags

- `-DUSE_QUILL` enables the Quill backend compilation units.
- `-DTRACY_ENABLE` enables Tracy compilation/instrumentation.

## Notes

### Quill (header-only)
Quill is included as headers and compiled into your binary via the Quill backend TU (`quill_backend.cpp`).

Important:
- Define `QUILL_DISABLE_NON_PREFIXED_MACROS` before including Quill macros headers,
  to avoid collisions with `LOG_INFO`, `LOG_DEBUG`, etc.

### Tracy
Tracy requires compiling `TracyClient.cpp` into your binary.
Tracy does not print to stdout by default; use the Tracy UI to see events and messages.

## Example .sh + vscode task.json + settings.json

### build.sh

```bash
#!/bin/bash
set -e

source /opt/fsl-imx-xwayland/6.12-walnascar/environment-setup-armv8a-poky-linux

SYSROOT=/opt/fsl-imx-xwayland/6.12-walnascar/sysroots/armv8a-poky-linux

if [ "$TRACY_BUILD" = "true" ] && [ "$QUILL_BUILD" = "false" ]; then
  echo ">>> Building WITH Tracy"
  CXX=aarch64-poky-linux-g++

  $CXX \
    --sysroot="$SYSROOT" \
    -std=c++17 \
    -DTRACY_ENABLE \
    -I../../libraries/third_party/tracy/public \
    -Isrc \
    -g "$1" \
    src/*.c \
    ../../libraries/third_party/tracy/public/TracyClient.cpp \
    -lpthread -ldl \
    -o "$2"

elif [ "$QUILL_BUILD" = "true" ] && [ "$TRACY_BUILD" = "false" ]; then
  echo ">>> Building WITH Quill"
  CXX=aarch64-poky-linux-g++
  QUILL_ROOT=../../libraries/quill/include/

  $CXX \
    --sysroot="$SYSROOT" \
    -std=c++17 \
    -DUSE_QUILL \
    -I"$QUILL_ROOT" \
    -Isrc \
    -g "$1" \
    src/*.c \
    src/quill_backend.cpp \
    -lpthread -ldl \
    -o "$2"
elif [ "$TRACY_BUILD" = "true" ] && [ "$QUILL_BUILD" = "true" ]; then
  echo ">>> Building WITH Tracy and Quill"
  CXX=aarch64-poky-linux-g++
  QUILL_ROOT=../../libraries/quill

  $CXX \
    --sysroot="$SYSROOT" \
    -std=c++17 \
    -DTRACY_ENABLE \
    -DUSE_QUILL \
    -I"$QUILL_ROOT/include" \
    -I../../libraries/third_party/tracy/public \
    -Isrc \
    -g "$1" \
    src/*.c \
    src/quill_backend.cpp \
    ../../libraries/third_party/tracy/public/TracyClient.cpp \
    -lpthread -ldl \
    -o "$2"
else
  echo ">>> Building WITHOUT Tracy and Quill"
  CC=aarch64-poky-linux-gcc

  $CC \
    --sysroot="$SYSROOT" \
    -std=c11 \
    -Isrc \
    -g "$1" \
    src/*.c \
    -o "$2"
fi
```

### tasks.json

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Compile",
            "type": "shell",
            "command": "./.vscode/build.sh",
            "args": [
                "examples/${fileBasename}",
                "build/${fileBasenameNoExtension}"
            ],
            "group": {
                "kind": "build",
            },
            "presentation": {
                "reveal": "always",
                "panel": "new"
            },
            "options": {
                "env": {
                    "TRACY_BUILD": "${config:tracy_build}",
                    "QUILL_BUILD": "${config:quill_build}"
                }
            },
            "problemMatcher": "$gcc"
        },
        {
            "label": "Send executable to traget",
            "dependsOn": [
                "Compile"
            ],
            "type": "shell",
            "command": "scp build/${fileBasenameNoExtension} root@${config:myRemoteIpAddr}:/root/${config:myRemoteFolder}/",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "detail": "Compiles and send file to FRDM board."
        }
    ]
}
```

### settings.json

```json
{
    "myRemoteIpAddr": "192.168.1.40", // Board IP addr
    "myRemoteFolder": "", // Board target execution directory
    "tracy_build": true,
    "quill_build": true,
    "C_Cpp.default.compilerPath": "/opt/fsl-imx-xwayland/6.12-walnascar/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc",
    "files.associations": {
        "operation.h": "c",
        "stdio.h": "c",
        "libwebsockets.h": "c",
        "signal.h": "c",
        "stream.h": "c",
        "ss_stream.h": "c",
        "ss_server.h": "c",
        "cstdlib": "c"
    }
}
```