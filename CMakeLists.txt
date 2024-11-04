cmake_minimum_required( VERSION 3.27 )

project( remove-last-window-from-group
    DESCRIPTION "A hyprland plugin to remove last window from group"
    VERSION 0.1
)

add_library( remove-last-window-from-group SHARED main.cpp )

find_package( PkgConfig REQUIRED )
pkg_check_modules( deps REQUIRED IMPORTED_TARGET
    hyprland
    libdrm
    libinput
    libudev
    pangocairo
    pixman-1
    wayland-server
    xkbcommon
)
target_link_libraries( remove-last-window-from-group PRIVATE rt PkgConfig::deps )

set_target_properties( remove-last-window-from-group PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

install( TARGETS remove-last-window-from-group )
