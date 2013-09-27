/*
 * Copyright (C) 2008-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "chat.h"

#include "logic/player.h"

using namespace Widelands;

std::string ChatMessage::toOldRichText() const
{
	std::string message = "<p font-color=#33ff33 font-size=9>";

	// Escape richtext characters
	// The goal of this code is two-fold:
	//  1. Assuming an honest game host, we want to prevent the ability of
	//     clients to use richtext.
	//  2. Assuming a malicious host or meta server, we want to reduce the
	//     likelihood that a bug in the richtext renderer can be exploited,
	//     by restricting the set of allowed richtext commands.
	//     Most notably, images are not allowed in richtext at all.
	//
	// Note that we do want host and meta server to send some richtext code,
	// as the ability to send formatted commands is nice for the usability
	// of meta server and dedicated servers, so we're treading a bit of a
	// fine line here.
	std::string sanitized;
	for (std::string::size_type pos = 0; pos < msg.size(); ++pos) {
		if (msg[pos] == '<') {
			if (playern < 0) {
				static const std::string good1 = "</p><p";
				static const std::string good2 = "<br>";
				if (!msg.compare(pos, good1.size(), good1)) {
					std::string::size_type nextclose = msg.find('>', pos + good1.size());
					if
						(nextclose != std::string::npos &&
						(nextclose == pos + good1.size() || msg[pos + good1.size()] == ' '))
					{
						sanitized += good1;
						pos += good1.size() - 1;
						continue;
					}
				} else if (!msg.compare(pos, good2.size(), good2)) {
					sanitized += good2;
					pos += good2.size() - 1;
					continue;
				}
			}

			sanitized += "&lt;";
		} else {
			sanitized += msg[pos];
		}
	}

	// time calculation
	char ts[13];
	strftime(ts, sizeof(ts), "[%H:%M] </p>", localtime(&time));
	message += ts;

	message += "<p font-size=14 font-face=DejaVuSerif font-color=#";
	message += color();

	if (recipient.size() && sender.size()) {
		// Personal message handling
		if (sanitized.compare(0, 3, "/me")) {
			message += " font-decoration=underline>";
			message += sender;
			message += " @ ";
			message += recipient;
			message += ":</p><p font-size=14 font-face=DejaVuSerif> ";
			message += sanitized;
		} else {
			message += ">@";
			message += recipient;
			message += " >> </p><p font-size=14";
			message += " font-face=DejaVuSerif font-color=#";
			message += color();
			message += " font-style=italic> ";
			message += sender;
			message += sanitized.substr(3);
		}
	} else {
		// Normal messages handling
		if (not sanitized.compare(0, 3, "/me")) {
			message += " font-style=italic>-> ";
			if (sender.size())
				message += sender;
			else
				message += "***";
			message += sanitized.substr(3);
		} else if (sender.size()) {
			message += " font-decoration=underline>";
			message += sender;
			message += ":</p><p font-size=14 font-face=DejaVuSerif> ";
			message += sanitized;
		} else {
			message += " font-weight=bold>*** ";
			message += sanitized;
		}
	}

	// return the formated message
	return message + "<br></p>";
}

std::string ChatMessage::toPrintable() const
{
	std::string message = "<p><font color=33ff33 size=9>";

	// Escape richtext characters
	// The goal of this code is two-fold:
	//  1. Assuming an honest game host, we want to prevent the ability of
	//     clients to use richtext.
	//  2. Assuming a malicious host or meta server, we want to reduce the
	//     likelihood that a bug in the richtext renderer can be exploited,
	//     by restricting the set of allowed richtext commands.
	//     Most notably, images are not allowed in richtext at all.
	//
	// Note that we do want host and meta server to send some richtext code,
	// as the ability to send formatted commands is nice for the usability
	// of meta server and dedicated servers, so we're treading a bit of a
	// fine line here.
	std::string sanitized;
	for (std::string::size_type pos = 0; pos < msg.size(); ++pos) {
		if (msg[pos] == '<') {
			if (playern < 0) {
				static const std::string good1 = "</p><p";
				static const std::string good2 = "<br>";
				if (!msg.compare(pos, good1.size(), good1)) {
					std::string::size_type nextclose = msg.find('>', pos + good1.size());
					if
						(nextclose != std::string::npos &&
						(nextclose == pos + good1.size() || msg[pos + good1.size()] == ' '))
					{
						sanitized += good1;
						pos += good1.size() - 1;
						continue;
					}
				} else if (!msg.compare(pos, good2.size(), good2)) {
					sanitized += good2;
					pos += good2.size() - 1;
					continue;
				}
			}

			sanitized += "\\<";
		} else {
			sanitized += msg[pos];
		}
	}

	// time calculation
	char ts[13];
	strftime(ts, sizeof(ts), "[%H:%M] ", localtime(&time));
	message += ts;

	message += "</font><font size=14 face=DejaVuSerif color=";
	message += color();

	if (recipient.size() && sender.size()) {
		// Personal message handling
		if (sanitized.compare(0, 3, "/me")) {
			message += " bold=1>";
			message += sender;
			message += " @ ";
			message += recipient;
			message += ":</font><font size=14 face=DejaVuSerif shadow=1 color=eeeeee> ";
			message += sanitized;
		} else {
			message += ">@";
			message += recipient;
			message += " \\> </font><font size=14";
			message += " face=DejaVuSerif color=";
			message += color();
			message += " italic=1 shadow=1> ";
			message += sender;
			message += sanitized.substr(3);
		}
	} else {
		// Normal messages handling
		if (not sanitized.compare(0, 3, "/me")) {
			message += " italic=1>-\\> ";
			if (sender.size())
				message += sender;
			else
				message += "***";
			message += sanitized.substr(3);
		} else if (sender.size()) {
			message += " bold=1>";
			message += sender;
			message += ":</font><font size=14 face=DejaVuSerif shadow=1 color=eeeeee> ";
			message += sanitized;
		} else {
			message += " bold=1>*** ";
			message += sanitized;
		}
	}

	// return the formated message
	return message + "</font><br></p>";
}



std::string ChatMessage::toPlainString() const
{
	return sender + ": " + msg;
}

std::string ChatMessage::color() const
{
	if ((playern >= 0) && playern < MAX_PLAYERS) {
		const RGBColor & clr = Player::Colors[playern];
		char buf[sizeof("ffffff")];
		snprintf(buf, sizeof(buf), "%.2x%.2x%.2x", clr.r, clr.g, clr.b);
		return buf;
	}
	return "999999";
}
