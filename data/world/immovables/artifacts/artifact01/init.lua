dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "artifact01",
   descname = _ "Artifact",
   size = "small",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 14, 20 },
      },
   }
}
