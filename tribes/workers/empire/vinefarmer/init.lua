dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 4, 23 }
   },
   dig = {
      template = "dig_??",
      directory = dirname,
      hotspot = { 8, 24 },
      fps = 10
   },
   plant = {
      template = "plant_??",
      directory = dirname,
      hotspot = { 15, 23 },
      fps = 10
   },
   gather = {
      template = "gather_??",
      directory = dirname,
      hotspot = { 9, 22 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_vinefarmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Vine Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1,
		shovel = 1,
		basket = 1
	},

	programs = {
		plantvine = {
			"findspace size:any radius:1",
			"walk coords",
			"animation dig 2000",
			"plant tribe:grapevine_tiny",
			"animation plant 3000",
			"return"
		},
		harvestvine = {
			"findobject attrib:ripe_grapes radius:1",
			"walk object",
			"animation gather 10000",
			"object harvest",
			"animation gather 2000",
			"createware grape",
			"return"
		}
	},

   animations = animations,
}
