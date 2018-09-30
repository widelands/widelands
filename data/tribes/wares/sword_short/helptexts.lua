function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Short sword
      frisians = pgettext ("frisians_ware", "This is the basic weapon of the Frisian soldiers. Together with a fur garment, it makes up the equipment of young soldiers. Short swords are produced by the small armor smithy.")
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
