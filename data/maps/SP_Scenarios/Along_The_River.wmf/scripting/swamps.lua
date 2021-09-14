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

   local swamp_down = array_combine(
      map:get_field(57,  83):region(0),
      map:get_field(58,  84):region(0),
      map:get_field(60,  84):region(0),
      map:get_field(61,  84):region(0),
      map:get_field(61,  85):region(0),
      map:get_field(62,  85):region(0),
      map:get_field(63,  86):region(0),
      map:get_field(63,  87):region(0),
      map:get_field(64,  88):region(0),
      map:get_field(64,  89):region(0),
      map:get_field(65,  90):region(0),
      map:get_field(66,  90):region(0),
      map:get_field(66,  91):region(0),
      map:get_field(67,  92):region(0),
      map:get_field(68,  92):region(0),
      map:get_field(68,  93):region(0),
      map:get_field(69,  94):region(0),
      map:get_field(69,  95):region(0),
      map:get_field(70,  96):region(0),
      map:get_field(70,  97):region(0),
      map:get_field(71,  98):region(0),
      map:get_field(71,  99):region(0),
      map:get_field(72, 100):region(0),
      map:get_field(72, 101):region(0),
      map:get_field(72, 102):region(0),
      map:get_field(72, 103):region(0),
      map:get_field(73, 104):region(0),
      map:get_field(72, 105):region(0),
      map:get_field(73, 106):region(0),
      map:get_field(72, 107):region(0),
      map:get_field(65,  98):region(0),
      map:get_field(64,  93):region(0),
      map:get_field(70, 102):region(0),
      map:get_field(66,  98):region(0)
   )

   local swamp_right = array_combine(
      map:get_field(57,  84):region(0),
      map:get_field(57,  85):region(0),
      map:get_field(58,  84):region(0),
      map:get_field(59,  84):region(0),
      map:get_field(60,  84):region(0),
      map:get_field(60,  85):region(0),
      map:get_field(61,  85):region(0),
      map:get_field(62,  86):region(0),
      map:get_field(62,  87):region(0),
      map:get_field(63,  88):region(0),
      map:get_field(63,  89):region(0),
      map:get_field(64,  90):region(0),
      map:get_field(64,  91):region(0),
      map:get_field(65,  90):region(0),
      map:get_field(65,  91):region(0),
      map:get_field(66,  92):region(0),
      map:get_field(67,  92):region(0),
      map:get_field(67,  93):region(0),
      map:get_field(68,  94):region(0),
      map:get_field(68,  95):region(0),
      map:get_field(69,  96):region(0),
      map:get_field(69,  97):region(0),
      map:get_field(70,  98):region(0),
      map:get_field(70,  99):region(0),
      map:get_field(71, 100):region(0),
      map:get_field(71, 101):region(0),
      map:get_field(72, 102):region(0),
      map:get_field(71, 103):region(0),
      map:get_field(72, 104):region(0),
      map:get_field(72, 105):region(0),
      map:get_field(72, 106):region(0),
      map:get_field(72, 107):region(0),
      map:get_field(65,  99):region(0)
   )

   player:reveal_fields(swamp)
   run(function()
      local i = 1
      while i <= #swamp_down do
         swamp_down[i].terd="summer_meadow2"
         i = i + 1
         sleep(5827)
      end
   end)
   run(function()
      local j = 1
      while j <= #swamp_right do
         swamp_right[j].terr="summer_meadow3"
         j = j + 1
         sleep(6007)
      end
      player:hide_fields(swamp)
   end)
end

-- =======================================================================
