function ware_helptext(tribe)
   local helptext = {
		-- TRANSLATORS: Helptext for a ware: Basket
		empire = pgettext("empire_ware", "This basket is needed by the vinefarmer for harvesting the grapes. It is produced by the toolsmith.")
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
