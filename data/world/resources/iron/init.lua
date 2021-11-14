push_textdomain("world")

local pics_dir = path.dirname(__file__) .. "../pics/"

wl.Descriptions():new_resource_type{
   name = "resource_iron",
   descname = _ "Iron",
   max_amount = 20,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "iron4.png",
   editor_pictures = {
      [5] = pics_dir .. "iron1.png",
      [10] = pics_dir .. "iron2.png",
      [15] = pics_dir .. "iron3.png",
      [1000] = pics_dir .. "iron4.png",
   }
}

pop_textdomain()
