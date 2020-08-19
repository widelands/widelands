dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "skeleton1",
   descname = _ "Skeleton",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 29, 36 },
      },
   }
}
