function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Mud
      frisians = pgettext("frisians_ware", "Mud is dug out of the hard ground in a mud mine. It is used to produce clay, which is turned into bricks.")
   }
   local result = ""
   if tribe then
      result = helptext[tribe]
   else
      result = helptext["default"]
   end
   if (result == nil) then result = "" end
   return result
end
