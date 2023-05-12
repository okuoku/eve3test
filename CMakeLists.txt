cmake_minimum_required(VERSION 3.12)
project(eve3test)

set(simavr_root /home/oku/repos/simavr)
set(simavr_arch obj-x86_64-pc-cygwin)
set(simavr_lib ${simavr_root}/simavr/${simavr_arch}/libsimavr.a)

set(emu_lib ${CMAKE_CURRENT_LIST_DIR}/EveApps/common/eve_hal/Bin/Simulation/x64/bt8xxemu.dll)

include_directories(
    # EveApps
    ${CMAKE_CURRENT_LIST_DIR}/EveApps/common/eve_hal/Hdr
    # simavr
    ${simavr_root}/examples/parts
    ${simavr_root}/include
    ${simavr_root}/simavr/sim)

add_executable(sim
    bt8xxemubridge.c

    simduino.c
    ${simavr_root}/examples/parts/uart_pty.c
    )

add_custom_command(TARGET sim POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy ${emu_lib} ${CMAKE_CURRENT_BINARY_DIR} .
    COMMAND
    chmod +x bt8xxemu.dll)

target_link_libraries(sim
    ${emu_lib}
    ${simavr_lib} elf)
