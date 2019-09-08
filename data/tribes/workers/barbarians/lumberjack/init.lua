dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 5, 23 },
      fps = 10
   },
   hacking = {
      pictures = path.list_files(dirname .. "hacking_??.png"),
      hotspot = { 19, 17 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {10, 22}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 21}, 10)


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
      harvest = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 15000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle 2000",
         "createware=log",
         "return"
      }
   },

   animations = animations,
}
