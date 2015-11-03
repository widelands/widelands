dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 5, 23 },
      fps = 10
   },
   hacking = {
      template = "hacking_??",
      directory = dirname,
      hotspot = { 19, 17 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 21}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_lumberjack",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Lumberjack"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		felling_ax = 1
	},

	programs = {
		chop = {
			"findobject attrib:tree radius:10",
			"walk object",
			"playFX sound/woodcutting woodcutting 255",
			"animation hacking 10000",
			"playFX sound/spoken timber 192",
			"object fall",
			"animation idle 2000",
			"createware log",
			"return"
		}
	},

   animations = animations,
}
