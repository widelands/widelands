function immovable_helptext(tribe)
   local helptext = {
      default = pgettext("sentence_separator", "%s %s"):bformat(
         -- TRANSLATORS: Helptext for a resource: Iron
         _("Iron veins contain iron ore that can be dug up by iron mines."),
         -- TRANSLATORS: Helptext for a resource: Iron
         _("There is a lot of iron here."))
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
