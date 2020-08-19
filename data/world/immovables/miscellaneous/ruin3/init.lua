dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "ruin3",
   descname = _ "Tombstone",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 29, 36 },
      },
   }
}
