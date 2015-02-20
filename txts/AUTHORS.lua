include "scripting/formatting.lua"

set_textdomain("texts")

-- Data for people categories

local coders = {
	"Holger Rapp (SirVer)",
	"Nicolai Hähnle (ixprefect)",
	"Florian Bluemel",
	"Florian Falkner (foldrian)",
	"Florian Weber (Bedouin)",
	"Philipp Engelhard",
	"Stefan Boettner",
	"Tron",
	"Martin Quinson",
	"Raul Ferriz",
	"Willem Jan Palenstijn",
	"Josef Spillner",
	"Christof Petig",
	"Erik Sigra (sigra)",
	"Nanne Wams",
	"Surgery",
	"Andrius R. (knutux)",
	"Jari Hautio (jarih)",
	"Peter Schwanemann (Nasenbaer)",
	"Victor Pelt (Dwarik)",
	"Axel Gehlert (dunkelbrauer)",
	"András Eisenberger (Kiscsirke)",
	"Andi",
	"Timo",
	"Hannes",
	"TimoW",
	"Jens Beyer (Qcumber-some)",
	"Andreas Breitschopp (ab-tools)",
	"Joachim Breitner (nomeata)",
	"Nizamov Shawkat",
	"Carl-Philip Hänsch (carli)",
	"Martin Prussak (martin)",
	"David Allwicher (aber)",
	"Nathan Peters (nathanpeters)",
	"Leif Sandstede (lcsand)",
	"Matthias Horne (shevonar)",
	"Borim (borim)",
	"Angelo Locritani (alocritani)",
	"Gabriel Margiani (gamag)",
	"Anthony J. Bentley (anthonyjbentley)",
	"Peter Waller (iri)",
	"Johannes Ebke (sirius-in4matiker)",
	"Andreas Eriksson",
	"Mark Scott",
	"Teppo Mäenpää",
	"Steven De Herdt",
	"Charly Ghislain (cghislai)",
	"Tino Miegel (TinoM)",
	"Tibor Bamhor (tiborb95)",
	"Ferdinand Thiessen (f-thiessen)",
	"Willy Scheibel (willyscheibel)",
	"Martin Schmidt (mars)",
	"Simon Eilting",
}

local graphicians = {
	"Albert Jasiowka",
	"Holger Rapp (SirVer)",
	"Marz",
	"Philipp Engelhard",
	"Yannick Warnier",
	"Delia",
	"Florian Neuerburg",
	"Jerome Rosinski",
	"Mats Olsson",
	"Odin Omdal",
	"Wolfgang Weidner",
	"Andreas Baier",
	"Juri Chomé",
	"Toralf Bethke (bithunter32)",
	"Peter Schwanemann (Nasenbaer)",
	"Alexia Death",
	"Repsa Jih",
	"Geoffroy Schmitlin (Trimard)",
	"Benedikt Freisen (Objpaswriter)",
	"Stefano Guidoni (Ilguido)",
	"Samith Sandanayake (samithdisal)",
	"Chuck Wilder (chuckw)",
	"Astuur",
	"Gerrit Familiegrosskopf (kingcreole)",
	"Florian Angermeier (fraang)"
}

local musicians = {
	"Kristian",
	"MiddleFinger",
	"Valerio Orlandini (Symbiosis)",
	"Barry van Oudtshoorn (barryvan)",
	"Jan Bruns (Solatis)",
	"Nikola Whallon (Saturn)",
	"Joshua O'Leary (Joshun)"
}

local sound_people = {
	"Stefan de Konik",
	"Peter Schwanemann (Nasenbaer)",
	"Stephan",
	"Adam Piggott (_aD)",
	"Stanisław Gackowski (Soeb)"
}

local map_creators = {
	"Michal Szopa (Winterwind)",
	"Jan-Henrik Kluth (Isch)",
	"Peter Schwanemann (Nasenbaer)",
	"Sven (deviant)",
	"Tuxlands",
	"Kamil Wilczek (Another Barbarian)",
	"Tarvo Reim (Tarrei)",
	"Manuel Holzmeier (Quappo)",
	"ivh",
	"Hanna Podewski (kristin)",
	"Teppo Mäenpää",
	"fk",
	"Einstein13",
	"Jenia",
	"Robnick"
}

local campaign_authors = {
	{
		_"Tutorials",
		{
			"wl-zocker"
		}
	},
	{
		_"Barbarian",
		{
			"Bastian Rapp",
			"Alexander Kahl (wolfpac)",
			"Peter Schwanemann (Nasenbaer)"
		}
	},
	{
		_"Empire",
		{
			"Peter Schwanemann (Nasenbaer)"
		}
	},
	{
		_"Atlantean",
		{
			"Holger Rapp (SirVer)",
			"Peter Schwanemann (Nasenbaer)"
		}
	}
}

local homepage_authors = {
	{
		_"Homepage Coders",
		{
			"Holger Rapp (SirVer)",
			"Stanislaw Gackowski (Soeb)",
			"Markus Pfitzner (janus)",
			"Tobi"
		}
	},
	{
		_"Documentation, Help and Wiki",
		{
			"Erik Sigra (sigra)",
			"Florian Falkner (foldrian)",
			"Florian Weber (bedouin)",
			"Nicolai Haehnle",
			"Holger Rapp (SirVer)",
			"Johannes (nuefke)",
			"Alexander Kahl (wolfpac)",
			"Stanislaw Gackowski (Soeb)",
			"Hanna Podewski (kristin)"
		}
	}
}

local packagers_linux = {
	{
		"Debian",
		{
			"Martin Quinson"
		}
	},
	{
		"Fedora",
		{
			"Karol Trzcionka",
			"Jochen Wiedmann"
		}
	},
	{
		"Mandriva",
		{
			"Emmanuel Andry (eandry)"
		}
	}
}

local packagers_unix = {
	{
		"FreeBSD",
		{
			"Bartosz Fabianowski"
		}
	},
	{
		"Mac OS X",
		{
			"Philipp Engelhard",
			"Pierre Salagnac (Tarou)",
			"Wolf St. Kappesser",
			"David Allwicher (aber)"
		}
	}
}

local packagers_other = {
	{
		"Windows",
		{
			"Tino Miegel (TinoM)",
			"Alexander Kahl (Wolfpac)",
			"Geodomus",
			"Jari Hautio"
		}
	},
	{
		"ZetaOS",
		{
			"BeSman"
		}
	}
}

local chieftains = {
   "Holger Rapp (SirVer)"
}


local elders = {
	{
		_"Graphics",
		{
			_"vacant"
		}
	},
	{
		_"Homepage",
		{
			"Markus Pfitzner (janus)"
		}
	},
	{
		_"Sound",
		{
			"Jan Bruns (solatis)"
		}
	},
	{
		_"Tongues",
		{
			"GunChleoc"
		}
	}
}


local former_elders = {
	{
		_"Homepage",
		{
			"holymoly",
			"Stuart Eglington (DaaL1973)",
			"Jon Harris (jonsjava)"
		}
	},
	{
		_"Graphics",
		{
			"Salamander",
			"Alexia Death (death)",
			"Chuck Wilder (chuckw)"
		}
	},
	{
		_"Sound",
		{
			"Yannick Warnier",
		}
	},
	{
		_"Translation",
		{
			"Peter Schwanemann (Nasenbaer)",
			"Philipp Niemann (Azagtoth)"
		}
	}
}


local other_authors = {
	"Matt Howe (mdhowe)",
	"Samuel Tilly (eldamar)",
	"and many, many more (thank you for everything you've done)"
}

-- Formatting functions

function h1_authors(text)
	return "<rt text-align=left><p font-size=24 font-decoration=bold font-face=serif font-color=f4a131>" .. text .. "</p></rt>"
end

function chieftain(text)
	return "<rt image=pics/genstats_enable_plr_01.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function elder(text)
	return "<rt image=pics/genstats_enable_plr_04.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function coder(text)
	return "<rt image=pics/genstats_nrwares.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function graphician(text)
	return "<rt image=pics/genstats_nrbuildings.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function musician(text)
	return "<rt image=pics/workarea123.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function sound(text)
	return "<rt image=pics/workarea123.png image-align=left text-align=left><p font-size=12>".. text .. "</p></rt>"
end

function maps(text)
	return "<rt image=pics/genstats_landsize.png image-align=left text-align=left><p font-size=12>".. text .. "</p></rt>"
end

function campaigns(text)
	return "<rt image=pics/ls_wlmap.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function packager(text)
	return "<rt><p font-size=18 font-decoration=underline font-face=serif font-color=2F9131>" .. text .. "</p></rt>"
end

function homepage(text)
	return "<rt image=pics/WL-Editor-16.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function former_elder(text)
	return "<rt image=pics/genstats_enable_plr_04.png text-align=left image-align=left><p font-size=12>" .. text .. "</p></rt>"
end

function others(text)
	return "<rt image=pics/genstats_landsize.png image-align=left text-align=left><p font-size=12>" .. text .. "</p></rt>"
end

-- Lists people in the people_list array
-- and formats each with format_function
function list_people(people_list, format_function)
		local result = ""
		for i, person in ipairs(people_list) do
		   result = result .. format_function(person)
		end
		return result
end

function list_people_with_header(header_and_people_list, header_format_function, format_function)
		local result = ""
		for i, entry in ipairs(header_and_people_list) do
			result = result .. "<rt>" .. header_format_function(entry[1]) .. "</rt>"
			for i, person in ipairs(entry[2])  do
			   result = result .. format_function(person)
		   end
		end
		return result
end


-- Main script
return {
   title = _"Authors",
   text = rt(
		"<rt text-align=center><p font-size=28 font-decoration=bold font-face=serif font-color=2F9131>" .. _"Widelands Development Team" .. "</p></rt>" ..

		h1_authors(_"Chieftain") ..
		list_people(chieftains, chieftain) ..

		h1_authors(_"Elders") ..
		list_people_with_header(elders, h2, elder) ..

		h1_authors(_"Coders") ..
		-- NOCOM(GunChleoc): Update
		list_people(coders, coder) ..

		h1_authors(_"Graphicians") ..
		list_people(graphicians, graphician) ..

		h1_authors(_"Musicians") ..
		list_people(musicians, musician) ..

		h1_authors(_"Sound Effects") ..
		list_people(sound_people, sound) ..

		h1_authors(_"Maps and Missions") ..
		list_people(map_creators, maps) ..

		h1_authors(_"Campaign Story") ..
		list_people_with_header(campaign_authors, h2, campaigns) ..

		h1_authors(_"Translators") ..
		"%s" .. -- Place holder to insert translators list, which is parsed in C++.

		h1_authors(_"Packagers") ..
		h2(_"Linux Distributions") ..
		list_people_with_header(packagers_linux, h3, packager) ..
		h2(_"Other Unix Distributions") ..
		list_people_with_header(packagers_unix, h3, packager) ..
		h2(_"Non Unix Distributions") ..
		list_people_with_header(packagers_other, h3, packager) ..

		h1_authors(_"Homepage") ..
		list_people_with_header(homepage_authors, h2, homepage) ..

		h1_authors(_"Former Elders") ..
		list_people_with_header(former_elders, h2, former_elder) ..

		h1_authors(_"Other") ..
		list_people(other_authors, others)
	)
}
