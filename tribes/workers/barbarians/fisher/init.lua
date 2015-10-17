dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 9, 39 },
   },
   fishing = {
      pictures = path.list_directory(dirname, "fishing_\\d+.png"),
      hotspot = { 9, 39 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 38}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {10, 38}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Fisher"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		fishing_rod = 1
	},

	programs = {
		fish = {
			"findspace size:any radius:7 resource:fish",
			"walk coords",
			"playFX sound/fisher fisher_throw_net 192",
			"mine fish 1",
			"animation fishing 3000",
			"playFX sound/fisher fisher_pull_net 192",
			"createware fish",
			"return"
		}
	},

   animations = animations,
}
