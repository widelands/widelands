function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Wooden Spear
      empire = pgettext("empire_ware", "This wooden spear is the basic weapon in the Empire military system. It is produced in the weapon smithy. In combination with a helmet, it is the equipment to fit out young soldiers.")
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
