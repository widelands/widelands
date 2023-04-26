-- =======================================================================
--                             Along The River
-- =======================================================================

function remove_swamps(player)
   local map = wl.Game().map

   local swamp = array_combine(
      map:get_field(57,  83):region(1),
      map:get_field(58,  84):region(1),
      map:get_field(60,  84):region(1),
      map:get_field(61,  85):region(1),
      map:get_field(63,  86):region(1),
      map:get_field(64,  88):region(1),
      map:get_field(65,  90):region(1),
      map:get_field(66,  91):region(1),
      map:get_field(67,  92):region(1),
      map:get_field(68,  93):region(1),
      map:get_field(69,  94):region(1),
      map:get_field(70,  96):region(1),
      map:get_field(71,  98):region(1),
      map:get_field(72, 100):region(1),
      map:get_field(72, 102):region(1),
      map:get_field(73, 104):region(1),
      map:get_field(72, 105):region(1),
      map:get_field(73, 106):region(1),
      map:get_field(72, 107):region(1)
   )

   local swamp_down = {
      map:get_field(57,  83),
      map:get_field(58,  84),
      map:get_field(60,  84),
      map:get_field(61,  84),
      map:get_field(61,  85),
      map:get_field(62,  85),
      map:get_field(63,  86),
      map:get_field(63,  87),
      map:get_field(64,  88),
      map:get_field(64,  89),
      map:get_field(65,  90),
      map:get_field(66,  90),
      map:get_field(66,  91),
      map:get_field(67,  92),
      map:get_field(68,  92),
      map:get_field(68,  93),
      map:get_field(69,  94),
      map:get_field(69,  95),
      map:get_field(70,  96),
      map:get_field(70,  97),
      map:get_field(71,  98),
      map:get_field(71,  99),
      map:get_field(72, 100),
      map:get_field(72, 101),
      map:get_field(72, 102),
      map:get_field(72, 103),
      map:get_field(73, 104),
      map:get_field(72, 105),
      map:get_field(73, 106),
      map:get_field(72, 107),
      map:get_field(65,  98),
      map:get_field(64,  93),
      map:get_field(70, 102),
      map:get_field(66,  98)
   }

   local swamp_right = {
      map:get_field(57,  84),
      map:get_field(57,  85),
      map:get_field(58,  84),
      map:get_field(59,  84),
      map:get_field(60,  84),
      map:get_field(60,  85),
      map:get_field(61,  85),
      map:get_field(62,  86),
      map:get_field(62,  87),
      map:get_field(63,  88),
      map:get_field(63,  89),
      map:get_field(64,  90),
      map:get_field(64,  91),
      map:get_field(65,  90),
      map:get_field(65,  91),
      map:get_field(66,  92),
      map:get_field(67,  92),
      map:get_field(67,  93),
      map:get_field(68,  94),
      map:get_field(68,  95),
      map:get_field(69,  96),
      map:get_field(69,  97),
      map:get_field(70,  98),
      map:get_field(70,  99),
      map:get_field(71, 100),
      map:get_field(71, 101),
      map:get_field(72, 102),
      map:get_field(71, 103),
      map:get_field(72, 104),
      map:get_field(72, 105),
      map:get_field(72, 106),
      map:get_field(72, 107),
      map:get_field(65,  99)
   }

   player:reveal_fields(swamp)
   run(function()
      for i = 1, #swamp_down do
         swamp_down[i].terd="summer_meadow2"
         sleep(5827)
      end
   end)
   run(function()
      for j = 1, #swamp_right do
         swamp_right[j].terr="summer_meadow3"
         sleep(6007)
      end
      player:hide_fields(swamp)
   end)
end

-- =======================================================================
