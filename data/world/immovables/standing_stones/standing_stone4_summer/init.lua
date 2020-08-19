dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone4_summer",
   descname = _ "Standing Stone",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 11, 74 },
      },
   }
}
