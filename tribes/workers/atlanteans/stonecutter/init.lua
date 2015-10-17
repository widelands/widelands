dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 7, 20 }
   },
   hacking = {
      template = "hacking_??",
      directory = dirname,
      hotspot = { 10, 19 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 21}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 23}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_stonecutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Stonecutter"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		pick = 1
	},

	programs = {
		cut_granite = {
			"findobject attrib:rocks radius:6",
			"walk object",
			"playFX sound/stonecutting stonecutter 192",
			"animation hacking 10000",
			"object shrink",
			"createware granite",
			"return"
		}
	},

   animations = animations,
}
