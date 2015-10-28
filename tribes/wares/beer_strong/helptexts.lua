function ware_helptext(tribe)
   local helptext = {
		-- TRANSLATORS: Helptext for a ware: Strong Beer
		barbarians = pgettext("barbarians_ware", "Only this beer is acceptable for the warriors in a battle arena. Some say that the whole power of the Barbarians lies in this ale. It helps to train the soldiers’ evade level from 0 to 1 to 2. Strong beer is also used in big inns to prepare meals.")
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
