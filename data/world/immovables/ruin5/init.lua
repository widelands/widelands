dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "ruin5",
   descname = _ "Tombstone",
   editor_category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 29, 36 },
      },
   }
}
