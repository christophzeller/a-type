project (a-type)

add_executable(a-type 
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

target_include_directories(a-type
    PUBLIC
    src
    )
