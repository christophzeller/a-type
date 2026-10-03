cmake_minimum_required(VERSION 3.28)
project (a-type)

find_package(GTest)

set(a-type-sources 
		src/main.cxx

		src/engine/ai/ai.hxx
		src/engine/input/input.hxx
		src/engine/terminal/terminal.hxx	
		src/engine/objects.hxx
		src/engine/utilities.hxx
		src/engine/engine.hxx
		src/engine/engine_state.hxx

		src/game/game_world.hxx
		src/game/player.hxx
		src/game/projectile.hxx
		src/game/starfield.hxx

		src/game/enemies/enemies.hxx
		src/game/enemies/rhombus.hxx
		src/game/enemies/torus.hxx
		src/game/enemies/diamond.hxx
    )

# vanilla

add_executable(a-type 
        ${a-type-sources}
	)

target_include_directories(a-type
    PUBLIC
    src
    )

# address & undefined behavior sanitizers

add_executable(a-type_ASanUBSan
        ${a-type-sources}
    )

target_include_directories(a-type_ASanUBSan
    PUBLIC
    src
    )

target_compile_options(a-type_ASanUBSan PRIVATE -fsanitize=address,undefined)
target_link_options(a-type_ASanUBSan PRIVATE -fsanitize=address,undefined)

# thread sanitizer

add_executable(a-type_TSan
     ${a-type-sources}
    )

target_include_directories(a-type_TSan
    PUBLIC
    src
    )

target_compile_options(a-type_TSan PRIVATE -fsanitize=thread)
target_link_options(a-type_TSan PRIVATE -fsanitize=thread)

# gtest

enable_testing()

add_executable(a-type-test
    test/a-type-test.cxx
    )

target_include_directories(a-type-test
    PUBLIC
    src
    )

target_link_libraries(a-type-test
    GTest::gtest_main
    )

include(GoogleTest)
gtest_discover_tests(a-type-test)
