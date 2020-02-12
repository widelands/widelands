function immovable_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a reed field
      default = _("This reed field has just been planted.")
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
