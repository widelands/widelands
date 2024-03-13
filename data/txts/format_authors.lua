include "scripting/richtext.lua"

-- Formatting functions for developers and translators

function h1_authors(text)
   return styles.as_paragraph("authors_heading_1", text)
end

function h2_authors(text)
   return styles.as_p_with_attr("fs_heading_2", "align=center", text)
end

function columns(list, bullet)
   -- Format list with bullet points in 3 cloumns in case of more than 5 members,
   -- otherwise centered in a single column.

	local cols = {"", "", ""}
	local col_num = 1
	local col_end = math.ceil(#list / 3)
	local align = ""
	if #list < 6 then
		cols = {""}
		col_end = 6
		align = "align=center "
	end

	for k, member in ipairs(list)  do
		cols[col_num] = cols[col_num] .. p(align .. "valign=center", bullet .. " " .. member)
		if k % col_end == 0 then
			col_num = col_num + 1
		end
	end

   local result = ""
	if #cols > 1 then
		for i, col in ipairs(cols) do
			result = result .. div("width=33% valign=top", col)
		end
	else
		result = div("width=100%", cols[1])
	end

   return result
end
