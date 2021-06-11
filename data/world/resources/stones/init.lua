push_textdomain("world")

local pics_dir = path.dirname(__file__) .. "../pics/"

wl.Descriptions():new_resource_type{
   name = "resource_stones",
   descname = _ "Stones",
   max_amount = 20,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "stones4.png",
   editor_pictures = {
      [5] = pics_dir .. "stones1.png",
      [10] = pics_dir .. "stones2.png",
      [15] = pics_dir .. "stones3.png",
      [1000] = pics_dir .. "stones4.png",
   }
}

pop_textdomain()
