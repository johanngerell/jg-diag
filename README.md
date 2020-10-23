# jg-diag

## Configure

    ~/source/jg-diag/build/macos/debug> cmake ../../.. -DCMAKE_BUILD_TYPE=Debug -GNinja

## Build

    ~/source/jg-diag/build/macos/debug> cmake --build . && ./jg_diag > jg_diag.svg && open jg_diag.svg
