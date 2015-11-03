dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "shooting_??.png"),
      hotspot = { 6, 29 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {14, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {13, 23}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Hunter"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hunting_bow = 1
   },

   programs = {
      hunt = {
         "findobject type:bob radius:13 attrib:eatable",
         "walk object",
         "animation idle 1500",
         "object remove",
         "createware meat",
         "return"
      }
   },

   animations = animations,
}
