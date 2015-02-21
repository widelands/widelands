dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 7, 20 }
   },
   hacking = {
      pictures = path.list_directory(dirname, "hacking_\\d+.png"),
      hotspot = { 10, 19 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 21}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 23}, 10)


tribes:new_worker_type {
   name = "atlanteans_stonecutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Stonecutter",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		pick = 1
	},

	programs = {
		cut_granite = {
			"findobject attrib:granite radius:6",
			"walk object",
			"playFX sound/stonecutting/stonecutter 192",
			"animation hacking 10000",
			"object shrink",
			"createware granite",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Stonecutter
   helptext = _"Cuts blocks of granite out of rocks in the vicinity.",
   animations = animations,
}
