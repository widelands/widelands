dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { -4, 11 }
   },
   planting = {
      pictures = path.list_files(dirname .. "plantreed_??.png"),
      hotspot = { 10, 21 },
      fps = 10
   },
   harvesting = {
      pictures = path.list_files(dirname .. "harvest_??.png"),
      hotspot = { 10, 22 },
      fps = 5
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {8, 23}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {7, 23}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_gardener",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Gardener"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=planting 6500",
         "plant=attrib:seed_reed",
         "animate=planting 6500",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_reed radius:1",
         "walk=object",
         "animate=harvesting 14000",
         "callobject=harvest",
         "createware=reed",
         "return"
      },
   },

   animations = animations,
}
