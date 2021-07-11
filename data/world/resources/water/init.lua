push_textdomain("world")

local pics_dir = path.dirname(__file__) .. "../pics/"

wl.Descriptions():new_resource_type{
   name = "resource_water",
   descname = _ "Water",
   max_amount = 50,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "water4.png",
   editor_pictures = {
      [10] = pics_dir .."water1.png",
      [20] = pics_dir .."water2.png",
      [30] = pics_dir .."water3.png",
      [1000] = pics_dir .. "water4.png",
   }
}

pop_textdomain()
