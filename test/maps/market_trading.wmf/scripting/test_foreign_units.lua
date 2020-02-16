run(function()
   sleep(100)

   -- Test foreign immovables and their transformations
   map:place_immovable("berry_bush_strawberry_tiny", map:get_field(22, 27 ), "tribes")
   map:place_immovable("barleyfield_tiny", map:get_field(23, 28 ), "tribes")

   -- 11 minutes until we have a ripe barleyfield
   sleep(1000 * 60 * 11)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
