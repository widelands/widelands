dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "artifact01",
   descname = _ "Artifact",
   editor_category = "artifacts",
   size = "small",
   attributes = { "artifact" },
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 14, 20 },
      },
   }
}
