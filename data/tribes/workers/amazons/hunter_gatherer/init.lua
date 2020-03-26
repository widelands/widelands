dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {23, 22},
      fps = 10
   },
   fishing = {
      pictures = path.list_files (dirname .. "fishing_??.png"),
      hotspot = { 9, 23 },
      fps = 10
   },
}
add_directional_animation(animations, "walk", dirname, "walk", {10, 23}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {11, 26}, 15)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_hunter_gatherer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Hunter Gatherer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      spear_wooden = 1
   },

   programs = {
      hunt = {
         "findobject=type:bob radius:14 attrib:eatable",
         "walk=object",
         "animate=idle 1000",
         "callobject=remove",
         "createware=meat",
         "return"
      },
      fish = {
         "findspace=size:any radius:14 resource:fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net 192",
         "mine=fish 1",
         "animate=fishing 9500",
         "playsound=sound/fisher/fisher_pull_net 192",
         "createware=fish",
         "return"
      }
   },

   animation_directory = dirname,
   ware_hotspot = {0, 20},
   animations = animations,
}
