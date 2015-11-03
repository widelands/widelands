dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 6, 24 },
      fps = 10
   },
   hacking = {
      template = "hacking_??",
      directory = dirname,
      hotspot = { 23, 23 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {9, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_lumberjack",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Lumberjack"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1,
		felling_ax = 1
	},

	programs = {
		chop = {
			"findobject attrib:tree radius:10",
			"walk object",
			"playFX sound/woodcutting fast_woodcutting 250",
			"animation hacking 10000",
			"playFX sound/spoken timber 156",
			"object fall",
			"animation idle 2000",
			"createware log",
			"return"
		}
	},

   animations = animations,
}
