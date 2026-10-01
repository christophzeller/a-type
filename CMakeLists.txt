project (a-type)

add_executable(a-type 
		src/main.cxx

		src/ai/ai.hxx

		src/input/input.hxx

		src/terminal/terminal.hxx

		src/world/enemies.hxx
		src/world/gameworld.hxx
		src/world/objects.hxx
		src/world/player.hxx
		src/world/projectile.hxx
		src/world/rhombus.hxx
		src/world/starfield.hxx
		src/world/world.hxx
	)
