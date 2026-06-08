// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <iostream>
#include <string>

#include "axuctl.h"

int cmd_bg(int argc, char* argv[]) {
	if (argc < 5) {
		usage();
		return 1;
	}

	std::string action = argv[2];
	std::string gid = argv[3];
	std::string uid = argv[4];
	std::string bg_key = "bg_override_" + uid;
	std::string art_key = "bg_artist_" + uid;
	std::string inv_key = "bg_invert_" + uid;

	if (action == "set") {
		if (argc < 8) {
			usage();
			return 1;
		}
		std::string invert = (std::string(argv[7]) == "0") ? "false" : "true";
		setting_set(gid.c_str(), bg_key.c_str(), argv[5]);
		setting_set(gid.c_str(), art_key.c_str(), argv[6]);
		setting_set(gid.c_str(), inv_key.c_str(), invert.c_str());
		std::cout << "set bg for " << uid << " in guild " << gid << "\n"
				  << "bg: " << argv[5] << "\n"
				  << "artist: " << argv[6] << "\n"
				  << "invert: " << invert << "\n";
		return 0;

	} else if (action == "clear") {
		setting_set(gid.c_str(), bg_key.c_str(), "");
		setting_set(gid.c_str(), art_key.c_str(), "");
		setting_set(gid.c_str(), inv_key.c_str(), "false");
		std::cout << "cleared bg for " << uid << " in guild " << gid << "\n";
		return 0;

	} else if (action == "info") {
		std::string bg = setting_get(gid.c_str(), bg_key.c_str());
		std::string art = setting_get(gid.c_str(), art_key.c_str());
		std::string inv = setting_get(gid.c_str(), inv_key.c_str());
		if (bg.empty())
			std::cout << "no custom bg for " << uid << " in guild " << gid << "\n";
		else
			std::cout << "bg: " << bg << "\n"
					  << "artist: " << art << "\n"
					  << "invert: " << inv << "\n";
		return 0;
	}

	usage();
	return 1;
}
