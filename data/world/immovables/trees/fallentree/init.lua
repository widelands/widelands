dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "fallentree_idle.png"),
      hotspot = { 2, 31 },
   },
}

world:new_immovable_type{
   name = "fallentree",
   descname = _ "Fallen Tree",
   size = "none",
   programs = {
      main = {
         "animate=idle duration:30s",
         "remove="
      }
   },
   animations = animations,
}
