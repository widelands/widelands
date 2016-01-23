-- TODO(GunChleoc): This is unused

function immovable_helptext(tribe)
   local helptext = {
      -- TRANSLATORS#: Helptext for a resource: Iron
      default = "Iron veins contain iron ore that can be dug up by iron mines." .. " " .. "There is a lot of iron here."
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
