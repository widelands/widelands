dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "artifact02",
   descname = _ "Artifact",
   editor_category = "artifacts",
   size = "small",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 14, 20 },
      },
   }
}
