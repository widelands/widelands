function ware_helptext(tribe)
   local helptext = {
		-- TRANSLATORS: Helptext for a ware: Ax
		barbarians = pgettext("barbarians_ware", "The ax is the basic weapon of the Barbarians. All young soldiers are equipped with it.")
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
