dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 13, 26 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {6, 26}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {6, 26}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Hunter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hunting_spear = 1
   },

   programs = {
      hunt = {
         "findobject type:bob radius:13 attrib:eatable",
         "walk object",
         "animation idle 1000",
         "object remove",
         "createware meat",
         "return"
      }
   },

   animations = animations,
}
