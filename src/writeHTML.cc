/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "writeHTML.h"

#ifdef WRITE_GAME_DATA_AS_HTML

#include "logic/item_ware_descr.h"
#include "logic/productionsite.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/worker_program.h"
#include "logic/world.h"

#include "io/filesystem/disk_filesystem.h"
#include "i18n.h"

#include "upcast.h"

void writeCrossReferences(FileWrite & fw, HTMLReferences const & references) {
	if (references[HTMLReferences::Input].size()) {
		fw.Text("<h2 id=\"requesters\">");
		fw.Text(_("Requesters"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const
			(std::set<std::string>, references[HTMLReferences::Input], i)
		{
			fw.Text("<li><a href=\"../");
			fw.Text(*i.current);
			fw.Text("\"/></a></li>\n");
		}
		fw.Text("</ul>\n");
	}
	if (references[HTMLReferences::Output].size()) {
		fw.Text("<h2 id=\"providers\">");
		fw.Text(_("Providers"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const
			(std::set<std::string>, references[HTMLReferences::Output], i)
		{
			fw.Text("<li><a href=\"../");
			fw.Text(*i.current);
			fw.Text("\"/></a></li>\n");
		}
		fw.Text("</ul>\n");
	}
	if (references[HTMLReferences::Madeof].size()) {
		fw.Text("<h2 id=\"successors\">");
		fw.Text(_("Successors"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const
			(std::set<std::string>, references[HTMLReferences::Madeof], i)
		{
			fw.Text("<li><a href=\"../");
			fw.Text(*i.current);
			fw.Text("\"/></a></li>\n");
		}
		fw.Text("</ul>\n");
	}
	if (references[HTMLReferences::Become].size()) {
		fw.Text("<h2 id=\"predecessors\">");
		fw.Text(_("Predecessors"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const
			(std::set<std::string>, references[HTMLReferences::Become], i)
		{
			fw.Text("<li><a href=\"../");
			fw.Text(*i.current);
			fw.Text("\"/></a></li>\n");
		}
		fw.Text("</ul>\n");
	}
	if (references[HTMLReferences::Employ].size()) {
		fw.Text("<h2 id=\"employers\">");
		fw.Text(_("Employers"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const
			(std::set<std::string>, references[HTMLReferences::Employ], i)
		{
			fw.Text("<li><a href=\"../");
			fw.Text(*i.current);
			fw.Text("\"/></a></li>\n");
		}
		fw.Text("</ul>\n");
	}
}

namespace Widelands {

void Tribe_Descr::referenceBuilding
	(::FileWrite        &       fw,
	 std::string  const &       backlink,
	 HTMLReferences::Role const role,
	 Building_Index       const index)
	const
{
	assert(index < get_nrbuildings());
	m_building_references[index.value()][role].insert(backlink);
	Building_Descr const & descr = *get_building_descr(index);
	std::string const & building_name     = descr.name    ();
	std::string const & building_descname = descr.descname();
	fw.Text(building_name);
	fw.Text("\" href=\"../");
	if (descr.global())
		fw.Text("../../global/militarysites");
	fw.Text(building_name);
	fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
	fw.Text(building_descname);
	fw.Text("\"><img src=\"../");
	if (descr.global())
		fw.Text("../../global/militarysites");
	fw.Text(building_name);
	fw.Text("/menu.png\" alt=\"");
	fw.Text(building_descname);
	fw.Text("\"/>");
}
void Tribe_Descr::referenceWorker
	(::FileWrite        &       fw,
	 std::string  const &       backlink,
	 HTMLReferences::Role const role,
	 Ware_Index           const index,
	 uint8_t                    multiplicity)
	const
{
	assert(index < get_nrworkers());
	m_worker_references[index.value()][role].insert(backlink);
	Worker_Descr const & descr = *get_worker_descr(index);
	std::string const & worker_name     = descr.name    ();
	std::string const & worker_descname = descr.descname();
	fw.Text(worker_name);
	fw.Text("\" href=\"../");
	fw.Text(worker_name);
	fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
	fw.Text(worker_descname);
	fw.Text("\">");
	for (; multiplicity; --multiplicity) {
		fw.Text("<img src=\"../");
		fw.Text(worker_name);
		fw.Text("/menu.png\" alt=\"");
		fw.Text(worker_descname);
		fw.Text("\"/>");
	}
}
void Tribe_Descr::referenceWare
	(::FileWrite        &       fw,
	 std::string  const &       backlink,
	 HTMLReferences::Role const role,
	 Ware_Index           const index,
	 uint8_t                    multiplicity)
	const
{
	assert(index < get_nrwares());
	m_ware_references[index.value()][role].insert(backlink);
	Item_Ware_Descr const & descr = *get_ware_descr(index);
	std::string const & ware_name     = descr.name    ();
	std::string const & ware_descname = descr.descname();
	fw.Text(ware_name);
	fw.Text("\" href=\"../");
	fw.Text(ware_name);
	fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
	fw.Text(ware_descname);
	fw.Text("\">");
	for (; multiplicity; --multiplicity) {
		fw.Text("<img src=\"../");
		fw.Text(ware_name);
		fw.Text("/menu.png\" alt=\"");
		fw.Text(ware_descname);
		fw.Text("\"/>");
	}
}

#define HTML_FILE_BEGIN                                                       \
   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"                             \
   "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "              \
   "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"                 \
   "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"                          \
   "<head>\n"                                                                 \
   "<link rel=\"icon\" type=\"image/png\" href=\"menu.png\"/>\n"              \
   "<style type=\"text/css\">\n" /*  for Firefox  */                          \
   "   a:link    {text-decoration:none;}\n"                                   \
   "   a:visited {text-decoration:none;}\n"                                   \
   "   img       {border:none;}\n"                                            \
   "</style>\n"                                                               \

#define HTML_FILE_END                                                         \
   "</body>\n"                                                                \
   "</html>\n"                                                                \

#define HTML_SCRIPT_SORTTABLE                                                 \
   "<script src=\"../../doc/sorttable.js\" type=\"text/javascript\"/>\n"      \

//  A container to keep the types ordered by descname for the table of
// contents.
struct orderer {
	bool operator () (std::string const * const a, std::string const * const b)
	{
		return *a < *b;
	}
};
#define ORDERED(index_type)                                                   \
   typedef std::multimap<std::string const *, index_type, orderer> Ordered;   \
   Ordered ordered;                                                           \

void Tribe_Descr::writeHTMLBuildings(std::string const & directory) {
	assert(directory.size());
	assert(*directory.rbegin() == '/');
	ORDERED(Building_Index);

	//  Write an index_<locale>.xhtml in each building type's directory and add
	//  the building type to ordered for the table of contents.
	for //  Must iterate backwards for cross-referencing to work.
		(Building_Index i = m_buildings.get_nitems();
		 Building_Index::First() < i;)
	{
		Building_Descr const & building_descr = *get_building_descr(--i);
		::FileWrite fw;
		building_descr.writeHTML(fw);
		writeCrossReferences(fw, m_building_references[i.value()]);
		fw.Text(HTML_FILE_END);
		RealFSImpl fs
			(building_descr.global()
			 ? "global/militarysites/" + building_descr.name()
			 : directory + building_descr.name());
		fw.Write(fs, ("index_" + i18n::get_locale() + ".xhtml").c_str());
		ordered.insert
			(std::pair<std::string const *, Building_Index>
			 	(&building_descr.descname(), i));
	}

	//  Write the table of contents to building_types.xhtml.
	::FileWrite fw;
	fw.Text
		(HTML_FILE_BEGIN
		 HTML_SCRIPT_SORTTABLE
		 "<title>");
	fw.Text(_("Building types"));
	fw.Text
		("</title>\n"
		 "</head>\n"
		 "<body>\n"
		 "<h1>");
	fw.Text(_("Building types"));
	fw.Text
		("</h1>\n"
		 "<table class=\"sortable\">\n"
		 "<thead><tr><th class=\"sorttable_nosort\">");
	fw.Text(_("Icon"));
	fw.Text("</th><th>");
	fw.Text(_("Name"));
	fw.Text("</th><th>");
	fw.Text(_("Size"));
	fw.Text("</th><th>");
	fw.Text(_("Buildable"));
	fw.Text("</th><th>");
	fw.Text(_("Enhanced"));
	fw.Text
		("</th><th><img src=\"../../pics/stop.png\" alt=\"");
	fw.Text(_("Stop"));
	fw.Text("\"/></th><th>");
	fw.Text(_("Conquer<br/>range"));
	fw.Text("</th><th>");
	fw.Text(_("Vision<br/>range"));
	fw.Text
		("</th></tr></thead>\n"
		 "<tbody>\n");
	container_iterate_const(Ordered, ordered, i) {
		Building_Descr const & building_descr =
			*get_building_descr(i.current->second);
		std::string         building_name     = building_descr    .name();
		std::string const & building_descname = building_descr.descname();
		if (building_descr.global())
			building_name = "../../global/militarysites/" + building_descr.name();
		fw.Text("<tr><td><a href=\"");
		fw.Text(building_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(building_descname);
		fw.Text("\"><img src=\"");
		fw.Text(building_name);
		fw.Text("/menu.png\" alt=\"\"/></a></td><td><a href=\"");
		fw.Text(building_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(building_descname);
		fw.Text("\">");
		fw.Text(building_descname);
		fw.Text("</a></td><td sorttable_customkey=\"");
		if        (building_descr.get_ismine())                        {
			fw.Text
				("M\"><img src=\"../../pics/mine.png\" alt=\"");
			fw.Text(_("Mine"));
		} else if (building_descr.get_size() == BaseImmovable::SMALL)  {
			fw.Text
				("1\"><img src=\"../../pics/small.png\" alt=\"");
			fw.Text(_("Small"));
		} else if (building_descr.get_size() == BaseImmovable::MEDIUM) {
			fw.Text
				("2\"><img src=\"../../pics/medium.png\" alt=\"");
			fw.Text(_("Medium"));
		} else {
			assert(building_descr.get_size() == BaseImmovable::BIG);
			fw.Text
				("3\"><img src=\"../../pics/big.png\" alt=\"");
			fw.Text(_("Big"));
		}
		fw.Text("\"/></td><td>");
		fw.Text
			(building_descr.buildable            () ? _("Yes") : _("No"));
		fw.Text("</td><td>");
		fw.Text
			(building_descr.get_enhanced_building() ? _("Yes") : _("No"));
		fw.Text("</td><td align=\"right\">");
		char buffer[32];
		sprintf(buffer, "%u", building_descr.get_conquers());
		fw.Text(buffer);
		fw.Text("</td><td align=\"right\">");
		sprintf(buffer, "%u", building_descr.vision_range());
		fw.Text(buffer);
		fw.Text("</td></tr>\n");
	}
	fw.Text
		("</tbody>\n"
		 "</table>\n"
		 HTML_FILE_END);
	RealFSImpl buildings_toc_fs(directory);
	fw.Write(buildings_toc_fs, _("building_types.xhtml"));
}


void Building_Descr::writeHTML(::FileWrite & fw) const {
	char buffer[256];
	fw.Text
		(HTML_FILE_BEGIN
		 "<title>");
	fw.Text(descname());
	fw.Text
		("</title>\n"
		 "</head>\n"
		 "<body>\n"
		 "<h1>");
	fw.Text(descname());
	fw.Text
		("</h1>\n"
		 "<p><img src=\"../../../pics/");
	if        (get_ismine()) {
		fw.Text("mine.png\" alt=\"");
		fw.Text(_("Mine"));
		fw.Text("\"/> ");
		fw.Text(_("Is mine."));
	} else if (get_size() == BaseImmovable::BIG)    {
		fw.Text("big.png\" alt=\"");
		fw.Text(_("Big"));
		fw.Text("\"/> ");
		fw.Text(_("Is big."));
	} else if (get_size() == BaseImmovable::MEDIUM) {
		fw.Text("medium.png\" alt=\"");
		fw.Text(_("Medium"));
		fw.Text("\"/> ");
		fw.Text(_("Is medium."));
	} else {
		assert (get_size() == BaseImmovable::SMALL);
		fw.Text("small.png\" alt=\"");
		fw.Text(_("Small"));
		fw.Text("\"/> ");
		fw.Text(_("Is small."));
	}

	if (not m_buildable) {
		fw.Text
			("</p>\n"
			 "<p>");
		fw.Text(_("Is not buildable."));
	}

	if (m_enhanced_building) {
		fw.Text
			("</p>\n"
			 "<p>");
		fw.Text(_("Is enhanced."));
	}

	if (uint32_t const c = get_conquers()) {
		fw.Text
			("</p>\n"
			 "<p>");
		snprintf(buffer, sizeof(buffer), _("Conquer range is %u."), c);
		fw.Text(buffer);
	}

	if (uint32_t const v = vision_range()) {
		fw.Text
			("</p>\n"
			 "<p>");
		snprintf(buffer, sizeof(buffer), _("Vision range is %u."), v);
		fw.Text(buffer);
	}

	fw.Text("</p>\n");

	if (buildcost().size()) {
		fw.Text("<h2 id=\"buildcost\">");
		fw.Text(_("Build cost"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const(Buildcost, buildcost(), j) {
			fw.Text("<li><a name=\"buildcost_");
			tribe().referenceWare
				(fw,
				 name() + "/index_" + i18n::get_locale() + ".xhtml#buildcost_"    +
				 tribe().get_ware_descr(j.current->first)->name() + "\" title=\"" +
				 descname() + _("'s constructionsite") + "\"><img src=\"../"      +
				 name() + "/menu.png\" alt=\"" + descname(),
				 HTMLReferences::Madeof,
				 j.current->first, j.current->second);
			fw.Text("</a></li>\n");
		}
		fw.Text("</ul>\n");
	}

	if (enhancements().size()) {
		fw.Text("<h2 id=\"enhancements\">");
		fw.Text(_("Enhancements"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const(Enhancements, enhancements(), i) {
			fw.Text("<li><a name=\"enhancement_");
			tribe().referenceBuilding
				(fw,
				 name() + "/index_" + i18n::get_locale() + ".xhtml#enhancement_"  +
				 tribe().get_building_descr(*i.current)->name() + "\" title=\""   +
				 descname() + _("'s enhancement") + "\"><img src=\"../" + name()  +
				 "/menu.png\" alt=\"" + descname(),
				 HTMLReferences::Become,
				 *i.current);
			fw.Text("</a></li>\n");
		}
		fw.Text("</ul>\n");
	}

	if (upcast(ProductionSite_Descr const, productionsite_descr, this))
		productionsite_descr->writeHTMLProduction(fw);
}


void Tribe_Descr::writeHTMLWorkers(std::string const & directory) {
	assert(directory.size());
	assert(*directory.rbegin() == '/');
	ORDERED(Ware_Index);

	//  Write an index_<locale>.xhtml in each worker type's directory and add
	//  the worker type to ordered for the table of contents.
	for (Ware_Index i = m_workers.get_nitems(); Ware_Index::First() < i;) {
		Worker_Descr const & worker_descr = *get_worker_descr(--i);
		::FileWrite fw;
		worker_descr.writeHTML(fw);
		writeCrossReferences(fw, m_worker_references[i.value()]);
		fw.Text(HTML_FILE_END);
		RealFSImpl fs(directory + worker_descr.name());
		fw.Write(fs, ("index_" + i18n::get_locale() + ".xhtml").c_str());
		ordered.insert
			(std::pair<std::string const *, Ware_Index>
			 	(&worker_descr.descname(), i));
	}

	//  Write the table of contents to worker_types.xhtml.
	::FileWrite fw;
	fw.Text
		(HTML_FILE_BEGIN
		 HTML_SCRIPT_SORTTABLE
		 "<title>");
	fw.Text(_("Worker types"));
	fw.Text
		("</title>\n"
		 "</head>\n"
		 "<body>\n"
		 "<h1>");
	fw.Text(_("Worker types"));
	fw.Text
		("</h1>\n"
		 "<table class=\"sortable\">\n"
		 "<thead><tr><th class=\"sorttable_nosort\">");
	fw.Text(_("Icon"));
	fw.Text("</th><th>");
	fw.Text(_("Name"));
	fw.Text("</th><th>");
	fw.Text(_("Buildable"));
	fw.Text("</th><th>");
	fw.Text(_("Vision<br/>range"));
	fw.Text("</th><th>");
	fw.Text(_("Minimum<br/>needed<br/>experience"));
	fw.Text("</th><th>");
	fw.Text(_("Maximum<br/>needed<br/>experience"));
	fw.Text("</th><th>");
	fw.Text(_("Becomes"));
	fw.Text
		("</th></tr></thead>\n"
		 "<tbody>\n");
	container_iterate_const(Ordered, ordered, i) {
		Worker_Descr const & worker_descr = *get_worker_descr(i.current->second);
		std::string const & worker_name     = worker_descr.    name();
		std::string const & worker_descname = worker_descr.descname();
		fw.Text("<tr><td><a href=\"");
		fw.Text(worker_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(worker_descname);
		fw.Text("\"><img src=\"");
		fw.Text(worker_name);
		fw.Text("/menu.png\" alt=\"\"/></a></td><td><a href=\"");
		fw.Text(worker_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(worker_descname);
		fw.Text("\">");
		fw.Text(worker_descname);
		fw.Text("</a></td><td align=\"right\">");
		char buffer[32];
		sprintf(buffer, "%u", worker_descr.vision_range());
		fw.Text(buffer);
		fw.Text("</td>");
		if (Ware_Index const becomes = worker_descr.becomes()) {
			fw.Text("<td align=\"right\">");
			sprintf(buffer, "%u", worker_descr.get_min_exp());
			fw.Text(buffer);
			fw.Text("</td><td align=\"right\">");
			sprintf(buffer, "%u", worker_descr.get_max_exp());
			fw.Text(buffer);
			Worker_Descr const & becomes_descr = *get_worker_descr(becomes);
			std::string const & becomes_name     = becomes_descr.    name();
			std::string const & becomes_descname = becomes_descr.descname();
			fw.Text("</td><td sorttable_customkey=\"");
			fw.Text(becomes_descname);
			fw.Text("\"><a href=\"");
			fw.Text(becomes_name);
			fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
			fw.Text(becomes_descname);
			fw.Text("\"><img src=\"");
			fw.Text(becomes_name);
			fw.Text("/menu.png\" alt=\"");
			fw.Text(becomes_descname);
			fw.Text("\"/></a>");
		} else
			fw.Text("<td></td><td></td><td sorttable_customkey=\"\">");
		fw.Text("</td></tr>\n");
	}
	fw.Text
		("</tbody>\n"
		 "</table>\n"
		 HTML_FILE_END);
	RealFSImpl workers_toc_fs(directory);
	fw.Write(workers_toc_fs, _("worker_types.xhtml"));
}


void Soldier_Descr::writeHTMLSoldier(::FileWrite & fw) const {
	char buffer[256];
	fw.Text("<h2 id=\"combat_properties\">");
	fw.Text(_("Combat Properties"));
	fw.Text
		("</h2>\n"
		 "<p>");
	snprintf
		(buffer, sizeof(buffer),
		 _
		 	("Hitpoints is between %u and %u, plus %u for each level above 0 "
		 	 "(maximum level is %u)."),
		 m_min_hp,      m_max_hp,      m_hp_incr,      m_max_hp_level);
	fw.Text(buffer);
	fw.Text
		("</p>\n"
		 "<p>");
	snprintf
		(buffer, sizeof(buffer),
		 _
		 	("Attack is between %u and %u, plus %u for each level above 0 "
		 	 "(maximum level is %u)."),
		 m_min_attack,  m_max_attack,  m_attack_incr,  m_max_attack_level);
	fw.Text(buffer);
	fw.Text
		("</p>\n"
		 "<p>");
	snprintf
		(buffer, sizeof(buffer),
		 _
		 	("Defense is %u, plus %u for each level above 0 "
		 	 "(maximum level is %u)."),
		 m_defense,                    m_defense_incr, m_max_defense_level);
	fw.Text(buffer);
	fw.Text
		("</p>\n"
		 "<p>");
	snprintf
		(buffer, sizeof(buffer),
		 _
		 	("Evade is %u, plus %u for each level above 0 "
		 	 "(maximum level is %u)."),
		 m_evade,                      m_evade_incr,   m_max_evade_level);
	fw.Text(buffer);
	fw.Text
		("</p>\n");
}


void Worker_Descr::writeHTML(::FileWrite & fw) const {
	char buffer[256];
	fw.Text
		(HTML_FILE_BEGIN
		 "<title>");
	fw.Text(descname());
	fw.Text
		("</title>\n"
		 "</head>\n"
		 "<body>\n"
		 "<h1>");
	fw.Text(descname());
	fw.Text
		("</h1>\n"
		 "<p>");
	fw.Text(helptext());
	fw.Text
		("</p>\n"
		 "<p>");
	snprintf(buffer, sizeof(buffer), _("Vision range is %u."), vision_range());
	fw.Text(buffer);
	fw.Text("</p>\n");

	if (buildcost().size()) {
		fw.Text("<h2 id=\"buildcost\">");
		fw.Text(_("Build cost"));
		fw.Text
			("</h2>\n"
			 "<ul>\n");
		container_iterate_const(Buildcost, buildcost(), j) {
			fw.Text("<li><a name=\"buildcost_");
			if (Ware_Index const wi = tribe().ware_index(j.current->first))
				tribe().referenceWare
					(fw,
					 name() + "/index_" + i18n::get_locale() + ".xhtml#buildcost_" +
					 j.current->first + "\" title=\"" + descname()                 +
					 _("'s creation") + "\"><img src=\"../" + name()               +
					 "/menu.png\" alt=\"" + descname(),
					 HTMLReferences::Madeof,
					 wi,
					 j.current->second);
			else
				tribe().referenceWorker
					(fw,
					 name() + "/index_" + i18n::get_locale() + ".xhtml#buildcost_" +
					 j.current->first + "\" title=\"" + descname()                 +
					 _("'s creation") + "\"><img src=\"../" + name()               +
					 "/menu.png\" alt=\"" + descname(),
					 HTMLReferences::Madeof,
					 tribe().safe_worker_index(j.current->first),
					 j.current->second);
			fw.Text("</a></li>\n");
		}
		fw.Text("</ul>\n");
	}

	if (becomes()) {
		fw.Text("<p>");
		snprintf
			(buffer, sizeof(buffer),
			 _("Needs experience from working between %u and %u times to become"),
			 get_min_exp(), get_max_exp());
		fw.Text(buffer);
		fw.Text(" <a name=\"becomes_");
		tribe().referenceWorker
			(fw,
			 name() + "/index_" + i18n::get_locale() + ".xhtml#becomes_"         +
			 tribe().get_worker_descr(becomes())->name() + "\" title=\""         +
			 descname() + _("'s promotion") + "\"><img src=\"../" + name()       +
			 "/menu.png\" alt=\"" + descname(),
			 HTMLReferences::Become,
			 becomes());
		fw.Text("</a>.</p>\n");
	}


	if (programs().size()) {
		fw.Text("<h2 id=\"programs\">");
		fw.Text(_("Programs"));
		fw.Text("</h2>\n");
		container_iterate_const(Programs, programs(), i)
			i.current->second->writeHTML(fw, *this);
	}

	if (upcast(Soldier_Descr const, soldier_descr, this))
		soldier_descr->writeHTMLSoldier(fw);
}


void WorkerProgram::writeHTML
	(::FileWrite & fw, Worker_Descr const & worker) const
{
	fw.Text("<h3 id=\"program_");
	fw.Text(get_name());
	fw.Text("\">");
	fw.Text(get_name());
	fw.Text
		("</h3>\n"
		 "<ol>\n");
	uint32_t line_number = 0;
	container_iterate_const(Actions, actions(), i) {
		char buffer[256];
		snprintf
			(buffer, sizeof(buffer),
			 "<li id=\"program_%s:%u\">", get_name().c_str(), ++line_number);
		fw.Text(buffer);
		(*i.current).writeHTML(fw, worker);
		fw.Text("</li>\n");
	}
	fw.Text("</ol>\n");
}


void Worker::Action::writeHTML(::FileWrite & fw, Worker_Descr const &) const {
	fw.Text
		(function == &Worker::run_mine              ? _("mine")              :
		 function == &Worker::run_breed             ? _("breed")             :
		 function == &Worker::run_createitem        ? _("createitem")        :
		 function == &Worker::run_setdescription    ? _("setdescription")    :
		 function == &Worker::run_setbobdescription ? _("setbobdescription") :
		 function == &Worker::run_findobject        ? _("findobject")        :
		 function == &Worker::run_findspace         ? _("findspace")         :
		 function == &Worker::run_walk              ? _("walk")              :
		 function == &Worker::run_animation         ? _("animation")         :
		 function == &Worker::run_return            ? _("return")            :
		 function == &Worker::run_object            ? _("object")            :
		 function == &Worker::run_plant             ? _("plant")             :
		 function == &Worker::run_create_bob        ? _("create_bob")        :
		 function == &Worker::run_removeobject      ? _("removeobject")      :
		 function == &Worker::run_geologist         ? _("geologist")         :
		 function == &Worker::run_geologist_find    ? _("geologist_find")    :
		 function == &Worker::run_playFX            ? _("playFX")            :
		 _("UNKNOWN"));
}


void ProductionSite_Descr::writeHTMLProduction(::FileWrite & fw) const {
	if (programs().size()) {
		fw.Text("<h2 id=\"production\">");
		fw.Text(_("Production"));
		fw.Text
			("</h2>\n"
			 "<h3 id=\"workers\">");
		fw.Text(_("Workers"));
		fw.Text
			("</h3>\n"
			 "<ul>\n");
		container_iterate_const(Ware_Types, working_positions(), i) {
			fw.Text("<li><a name=\"worker_");
			tribe().referenceWorker
				(fw,
				 name() + "/index_" + i18n::get_locale() + ".xhtml#worker_"       +
				 tribe().get_worker_descr(i.current->first)->name()               +
				 "\" title=\"" + descname() + _("'s employee")                    +
				 "\"><img src=\"../" + name() + "/menu.png\" alt=\"" + descname(),
				 HTMLReferences::Employ,
				 i.current->first, i.current->second);
			fw.Text("</a></li>\n");
		}
		fw.Text("</ul>\n");

		if (inputs().size()) {
			fw.Text("<h3 id=\"inputs\">");
			fw.Text(_("Inputs"));
			fw.Text
				("</h3>\n"
				 "<ul>\n");
			container_iterate_const(Ware_Types, inputs(), i) {
				fw.Text("<li><a name=\"input_");
				tribe().referenceWare
					(fw,
					 name() + "/index_" + i18n::get_locale() + ".xhtml#input_"     +
					 tribe().get_ware_descr(i.current->first)->name().c_str()      +
					 "\" title=\"" + descname() + _("'s input")                    +
					 "\"><img src=\"../" + name() + "/menu.png\" alt=\""           +
					 descname(),
					 HTMLReferences::Input,
					 i.current->first, i.current->second);
				fw.Text("</a></li>\n");
			}
			fw.Text("</ul>\n");
		}

		if (output().size()) {
			fw.Text("<h3 id=\"output\">");
			fw.Text(_("Output"));
			fw.Text
				("</h3>\n"
				 "<ul>\n");
			container_iterate_const(Output, output(), i) {
				fw.Text("<li><a name=\"output_");
				tribe().referenceWare
					(fw,
					 name() + "/index_" + i18n::get_locale() + ".xhtml#output_"    +
					 tribe().get_ware_descr(*i.current)->name().c_str()            +
					 "\" title=\"" + descname() + _("'s output")                   +
					 "\"><img src=\"../" + name() + "/menu.png\" alt=\""           +
					 descname(),
					 HTMLReferences::Output,
					 *i.current);
				fw.Text("</a></li>\n");
			}
			fw.Text("</ul>\n");
		}

		fw.Text("<h3 id=\"programs\">");
		fw.Text(_("Programs"));
		fw.Text("</h3>\n");
		container_iterate_const(Programs, programs(), i)
			i.current->second->writeHTML(fw, *this);
	}
}


void ProductionProgram::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	fw.Text("<h4 id=\"program_");
	fw.Text(name());
	fw.Text("\">");
	fw.Text(descname());
	fw.Text
		("</h4>\n"
		 "<ol>\n");
	uint32_t line_number = 0;
	container_iterate_const(Actions, actions(), i) {
		char buffer[256];
		snprintf
			(buffer, sizeof(buffer),
			 "<li id=\"program_%s:%u\">", name().c_str(), ++line_number);
		fw.Text(buffer);
		(*i.current)->writeHTML(fw, site);
		fw.Text("</li>\n");
	}
	fw.Text("</ol>\n");
}


void ProductionProgram::ActReturn::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#return\" title=\"");
	fw.Text(_("Documentation for program command return"));
	fw.Text("\">");
	fw.Text(_("return"));
	fw.Text("</a> ");
	assert(m_result == Failed or m_result == Completed or m_result == Skipped);
	fw.Text
		(m_result == Failed    ? _("failed")    :
		 m_result == Completed ? _("completed") :
		 _("skipped"));
	if (m_conditions.size()) {
		std::string op;
		fw.Text(" <span class=\"keyword\">");
		if (m_is_when) {
			op = _("and");
			fw.Text(_("when"));
		} else {
			op = _("or");
			fw.Text(_("unless"));
		}
		fw.Text("</span> ");
		for
			(struct {
			 	Conditions::const_iterator       current;
			 	Conditions::const_iterator const end;
			 } i = {m_conditions.begin(), m_conditions.end()};;)
		{
			(*i.current)->writeHTML(fw, site);
			if (++i.current == i.end)
				break;
			fw.Text("<span class=\"keyword\">");
			fw.Text(op);
			fw.Text("</span> ");
		}
	}
}


void ProductionProgram::ActReturn::Negation::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	fw.Text("<span class=\"keyword\">&not;</span> ");
	operand->writeHTML(fw, site);
}


void ProductionProgram::ActReturn::Economy_Needs::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	Item_Ware_Descr const & ware = *site.tribe().get_ware_descr(ware_type);
	std::string const & ware_name     = ware.    name();
	std::string const & ware_descname = ware.descname();
	fw.Text("<span class=\"keyword\">");
	fw.Text(_("economy"));
	fw.Text("</span> <span class=\"keyword\">");
	fw.Text(_("needs"));
	fw.Text("</span> <a href=\"../");
	fw.Text(ware_name);
	fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
	fw.Text(ware_descname);
	fw.Text("\"><img src=\"../");
	fw.Text(ware_name);
	fw.Text("/menu.png\" alt=\"");
	fw.Text(ware_descname);
	fw.Text("\"/></a>");
}


void ProductionProgram::ActReturn::Site_Has::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	Tribe_Descr const & tribe = site.tribe();
	fw.Text("<span class=\"keyword\">");
	fw.Text(_("site"));
	fw.Text("</span> <span class=\"keyword\">");
	fw.Text(_("has"));
	fw.Text("</span>");
	for
		(struct {
		 	std::set<Ware_Index>::const_iterator       current;
		 	std::set<Ware_Index>::const_iterator const end;
		 } i = {group.first.begin(), group.first.end()};;)
	{
		Item_Ware_Descr const & ware_type = *tribe.get_ware_descr(*i.current);
		std::string const & ware_type_name     = ware_type.    name();
		std::string const & ware_type_descname = ware_type.descname();
		fw.Text("<a href=\"../");
		fw.Text(ware_type_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(ware_type_descname);
		fw.Text("\"><img src=\"../");
		fw.Text(ware_type_name);
		fw.Text("/menu.png\" alt=\"");
		fw.Text(ware_type_descname);
		fw.Text("\"/></a>");
		if (++i.current == i.end)
			break;
		fw.Unsigned8(',');
	}
	if (1 < group.second) {
		char buffer[32];
		sprintf(buffer, ":%u", group.second);
		fw.Text(buffer);
	}
}

void ProductionProgram::ActReturn::Workers_Need_Experience::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const &) const
{
	fw.Text("<a href=\"#workers\" title=\"");
	fw.Text(_("workers of this site"));
	fw.Text("\"><span class=\"keyword\">");
	fw.Text(_("workers"));
	fw.Text(_("</span></a> <span class=\"keyword\">"));
	fw.Text(_("need"));
	fw.Text("</span> <span class=\"keyword\">");
	fw.Text(_("experience"));
	fw.Text("</span>");
}


void ProductionProgram::ActCall::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const &) const
{
	std::string const & program_descname = m_program->descname();
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#call\" title=\"");
	fw.Text(_("Documentation for program command call"));
	fw.Text("\">");
	fw.Text(_("call"));
	fw.Text("</a> <a href=\"#program_");
	fw.Text(m_program->name());
	fw.Text("\" title=\"");
	char buffer[64];
	snprintf
		(buffer, sizeof(buffer),
		 _("site's program %s"), program_descname.c_str());
	fw.Text(buffer);
	fw.Text("\">");
	fw.Text(program_descname);
	fw.Text("</a>");
	if (m_failure_handling_method != Ignore) {
		fw.Text
			(" <span class=\"keyword\">on</span> "
			 "<span class=\"keyword\">failure</span> <span class=\"keyword\">");
		fw.Text
			(m_failure_handling_method == Fail ? "fail</span>" : "repeat</span>");
	}
}


void ProductionProgram::ActWorker::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#worker\" title=\"");
	fw.Text(_("Documentation for program command worker"));
	fw.Text("\">");
	fw.Text(_("worker"));
	fw.Text("</a> <a href=\"../");
	Worker_Descr const & worker_descr =
		*site.tribe().get_worker_descr(site.working_positions().at(0).first);
	fw.Text(worker_descr.name());
	fw.Text("/index_" + i18n::get_locale() + ".xhtml#program_");
	fw.Text(m_program);
	fw.Text("\" title=\"");
	char buffer[64];
	snprintf
		(buffer, sizeof(buffer),
		 _("%s's program %s"),
		 worker_descr.descname().c_str(), m_program.c_str());
	fw.Text(buffer);
	fw.Text("\">");
	fw.Text(m_program);
	fw.Text("</a>");
}


void ProductionProgram::ActSleep::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const &) const
{
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#sleep\" title=\"");
	fw.Text(_("Documentation for program command sleep"));
	fw.Text("\">");
	fw.Text(_("sleep"));
	fw.Text("</a>");
	if (m_duration) {
		char buffer[32];
		snprintf
			(buffer, sizeof(buffer),
			 _(" %u.%03u s"), m_duration / 1000, m_duration % 1000);
		fw.Text(buffer);
	}
}


void ProductionProgram::ActAnimate::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const &) const
{
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#animate\" title=\"");
	fw.Text(_("Documentation for program command animate"));
	fw.Text("\">");
	fw.Text(_("animate"));
	fw.Text("</a>");
	if (m_duration) {
		char buffer[32];
		snprintf
			(buffer, sizeof(buffer),
			 _(" %u.%03u s"), m_duration / 1000, m_duration % 1000);
		fw.Text(buffer);
	}
}


void ProductionProgram::ActConsume::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	Tribe_Descr const & tribe = site.tribe();
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#consume\" title=\"");
	fw.Text(_("Documentation for program command consume"));
	fw.Text("\">");
	fw.Text(_("consume"));
	fw.Text("</a> ");
	for
		(struct {
		 	Groups::const_iterator       current;
		 	Groups::const_iterator const end;
		 } j = {groups().begin(), groups().end()};;)
	{
		for
			(struct {
			 	std::set<Ware_Index>::const_iterator       current;
			 	std::set<Ware_Index>::const_iterator const end;
			 } i = {j.current->first.begin(), j.current->first.end()};;)
		{
			Item_Ware_Descr const & ware = *tribe.get_ware_descr(*i.current);
			std::string const & ware_name     = ware.    name();
			std::string const & ware_descname = ware.descname();
			fw.Text("<a href=\"../");
			fw.Text(ware_name);
			fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
			fw.Text(ware_descname);
			fw.Text("\"><img src=\"../");
			fw.Text(ware_name);
			fw.Text("/menu.png\" alt=\"");
			fw.Text(ware_descname);
			fw.Text("\"/></a>");
			if (++i.current == i.end)
				break;
			fw.Unsigned8(',');
		}
		if (1 < j.current->second) {
			char buffer[32];
			sprintf(buffer, ":%u", j.current->second);
			fw.Text(buffer);
		}
		if (++j.current == j.end)
			break;
		fw.Unsigned8(' ');
	}
}


void ProductionProgram::ActProduce::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	Tribe_Descr const & tribe = site.tribe();
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#produce\" title=\"");
	fw.Text(_("Documentation for program command produce"));
	fw.Text("\">");
	fw.Text(_("produce"));
	fw.Text("</a> ");
	for
		(struct {
		 	Items::const_iterator       current;
		 	Items::const_iterator const end;
		 } i = {items().begin(), items().end()};;)
	{
		Item_Ware_Descr const & ware = *tribe.get_ware_descr(i.current->first);
		std::string const & ware_name     = ware.    name();
		std::string const & ware_descname = ware.descname();
		fw.Text("<a href=\"../");
		fw.Text(ware_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(ware_descname);
		fw.Text("\"><img src=\"../");
		fw.Text(ware_name);
		fw.Text("/menu.png\" alt=\"");
		fw.Text(ware_descname);
		fw.Text("\"/></a>");
		if (1 < i.current->second) {
			char buffer[32];
			sprintf(buffer, ":%u", i.current->second);
			fw.Text(buffer);
		}
		if (++i.current == i.end)
			break;
		fw.Unsigned8(' ');
	}
}


void ProductionProgram::ActMine::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const & site) const
{
	World          const & world             = site.tribe().world();
	Resource_Descr const & resource          = *world.get_resource(m_resource);
	std::string    const & world_basedir     = world.basedir();
	std::string    const & resource_name     = resource.    name();
	std::string    const & resource_descname = resource.descname();
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#mine\" title=\"");
	fw.Text(_("Documentation for program command mine"));
	fw.Text("\">");
	fw.Text(_("mine"));
	fw.Text("</a> <a href=\"../../../");
	fw.Text(world_basedir);
	fw.Text("index_" + i18n::get_locale() + ".xhtml#resource_");
	fw.Text(resource_name);
	fw.Text("\" title=\"");
	fw.Text(resource_descname);
	fw.Text("\"><img src=\"../../../");
	fw.Text(world_basedir);
	fw.Text("/resources/");
	fw.Text(resource_name);
	fw.Text("_1f.png\" alt=\"");
	fw.Text(resource_descname);
	char buffer[32];
	sprintf(buffer, "\"/></a> %u %u %u", m_distance, m_max, m_chance);
	fw.Text(buffer);
}


void ProductionProgram::ActCheck_Soldier::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const &) const
{
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#check_soldier\" title=\"");
	fw.Text(_("Documentation for program command check_soldier"));
	fw.Text("\">");
	fw.Text(_("check soldier"));
	fw.Text("</a> ");
	assert
		(attribute == atrHP      or attribute == atrAttack or
		 attribute == atrDefense or attribute == atrEvade);
	fw.Text
		(attribute == atrHP      ? _("hp")      :
		 attribute == atrAttack  ? _("attack")  :
		 attribute == atrDefense ? _("defense") :
		 _("evade"));
	char buffer[32];
	sprintf(buffer, " %u", level);
	fw.Text(buffer);
}


void ProductionProgram::ActTrain::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const &) const
{
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#train\" title=\"");
	fw.Text(_("Documentation for program command train"));
	fw.Text("\">");
	fw.Text(_("train"));
	fw.Text("</a> ");
	assert
		(attribute == atrHP      or attribute == atrAttack or
		 attribute == atrDefense or attribute == atrEvade);
	fw.Text
		(attribute == atrHP      ? _("hp")      :
		 attribute == atrAttack  ? _("attack")  :
		 attribute == atrDefense ? _("defense") :
		 _("evade"));
	char buffer[32];
	sprintf(buffer, " %u %u", level, target_level);
	fw.Text(buffer);
}


void ProductionProgram::ActPlayFX::writeHTML
	(::FileWrite & fw, ProductionSite_Descr const &) const
{
	fw.Text("<a href=\"../../../doc/");
	fw.Text(_("productionsite_program_reference.xhtml"));
	fw.Text("#playFX\" title=\"");
	fw.Text(_("Documentation for program command playFX"));
	fw.Text("\">");
	fw.Text(_("playFX"));
	fw.Text("</a> ");
	fw.Text(name);
	char buffer[32];
	sprintf(buffer, " %u", priority);
	fw.Text(buffer);
}


void Tribe_Descr::writeHTMLWares(std::string const & directory) {
	assert(directory.size());
	assert(*directory.rbegin() == '/');
	ORDERED(Ware_Index);

	//  Write an index_<locale>.xhtml in each ware type's directory and add the
	//  ware type to ordered for the table of contents.
	for (Ware_Index i = m_wares.get_nitems(); Ware_Index::First() < i;) {
		Item_Ware_Descr const & ware_descr = *get_ware_descr(--i);
		::FileWrite fw;
		ware_descr.writeHTML(fw);
		writeCrossReferences(fw, m_ware_references[i.value()]);
		fw.Text(HTML_FILE_END);
		RealFSImpl fs(directory + ware_descr.name());
		fw.Write(fs, ("index_" + i18n::get_locale() + ".xhtml").c_str());
		ordered.insert
			(std::pair<std::string const *, Ware_Index>
			 	(&ware_descr.descname(), i));
	}

	//  Write the table of contents to ware_types.xhtml.
	::FileWrite fw;
	fw.Text
		(HTML_FILE_BEGIN
		 HTML_SCRIPT_SORTTABLE
		 "<title>");
	fw.Text(_("Ware types"));
	fw.Text
		("</title>\n"
		 "</head>\n"
		 "<body>\n"
		 "<h1>");
	fw.Text(_("Ware types"));
	fw.Text
		("</h1>\n"
		 "<table class=\"sortable\">\n"
		 "<thead><tr><th class=\"sorttable_nosort\">");
	fw.Text(_("Icon"));
	fw.Text("</th><th>");
	fw.Text(_("Name"));
	fw.Text
		("</th></tr></thead>\n"
		 "<tbody>\n");
	container_iterate_const(Ordered, ordered, i) {
		Item_Ware_Descr const & ware_descr = *get_ware_descr(i.current->second);
		std::string const & ware_name     = ware_descr.    name();
		std::string const & ware_descname = ware_descr.descname();
		fw.Text("<tr><td><a href=\"");
		fw.Text(ware_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(ware_descname);
		fw.Text("\"><img src=\"");
		fw.Text(ware_name);
		fw.Text("/menu.png\" alt=\"\"/></a></td><td><a href=\"");
		fw.Text(ware_name);
		fw.Text("/index_" + i18n::get_locale() + ".xhtml\" title=\"");
		fw.Text(ware_descname);
		fw.Text("\">");
		fw.Text(ware_descname);
		fw.Text("</a></td></tr>\n");
	}
	fw.Text
		("</tbody>\n"
		 "</table>\n"
		 HTML_FILE_END);
	RealFSImpl wares_toc_fs(directory);
	fw.Write(wares_toc_fs, _("ware_types.xhtml"));
}


void Item_Ware_Descr::writeHTML(::FileWrite & fw) const {
	fw.Text
		(HTML_FILE_BEGIN
		 "<title>");
	fw.Text(descname());
	fw.Text
		("</title>\n"
		 "</head>\n"
		 "<body>\n"
		 "<h1>");
	fw.Text(descname());
	fw.Text
		("</h1>\n"
		 "<p>");
	fw.Text(helptext());
	fw.Text("</p>\n");
}

}

#endif
