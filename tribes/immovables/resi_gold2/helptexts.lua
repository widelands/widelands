-- TODO(GunChleoc): This is unused

function immovable_helptext(tribe)
   local helptext = {
      -- #TRANSLATORS: Helptext for a resource: Gold
      default = "Gold veins contain gold ore that can be dug up by gold mines." .. " " .. "There is a lot of gold here."
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