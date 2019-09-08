dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 21 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 32}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {9, 32}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Hunter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hunting_spear = 1
   },
   programs = {
      hunt = {
         "findobject=type:bob radius:13 attrib:eatable",
         "walk=object",
         "animate=idle 1000",
         "removeobject",
         "createware=meat",
         "return"
      }
   },

   animations = animations,
}
