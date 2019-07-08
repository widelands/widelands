dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", { 60, 78 })
add_animation(animations, "build", dirname, "build", { 60, 78 })

tribes:new_warehouse_type {
   msgctxt = "barbarians_building",
   name = "barbarians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Warehouse"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 2,
      grout = 3,
      reed = 1
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 1,
      grout = 1
   },

   animations = animations,

   aihints = {},

   heal_per_second = 170,
}
