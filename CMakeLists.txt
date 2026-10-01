project (a-type)

add_executable(a-type 
		src/main.cxx

		src/ai/ai.hxx

		src/input/input.hxx

		src/terminal/terminal.hxx

		src/game/player.hxx
		src/game/projectile.hxx
		src/game/starfield.hxx

		src/game/enemies/enemies.hxx
		src/game/enemies/rhombus.hxx
		src/game/enemies/torus.hxx
		src/game/enemies/diamond.hxx

		src/world/objects.hxx
		src/world/utilities.hxx
		src/world/engine.hxx
		src/world/game_world.hxx


	)
