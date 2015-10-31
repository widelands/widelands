-- TODO(GunChleoc): This is unused

function immovable_helptext(tribe)
   local helptext = {
      -- #TRANSLATORS: Helptext for a resource: Coal
      default = "Coal veins contain coal that can be dug up by coal mines." .. " " .. "There is only a little bit of coal here."
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