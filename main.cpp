#include <dpp/dpp.h>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <cstdlib>
#include <random>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include <algorithm>

using json = nlohmann::json;
const std::string STATS = "stats.json";

void save_stats(const json& j) {
	std::ofstream o(STATS);
	o << j << std::endl;
}

json load_stats() {
	std::ifstream i(STATS);
	if (!i.is_open()) return json::object();
	json j;
	i >> j;
	return j;
}

const dpp::snowflake non_english_ch = 1482901206018822296;
const std::vector<dpp::snowflake> ALLOWED_CHANNELS = { 
	non_english_ch,
	1469591365645111478,
	1480102566871433317,
	1480688308811071732,
	1482523765308002314,
	1482653036861067396,
	1482634709526904975,
	1482470065415786628,
	1469597924596912188,
	1482470983813300465,
	1469593150858465394,
	1170145267224428635
};

const std::vector<std::string> RELIABLE_PROVIDERS = {
	"tenor.com", "giphy.com", "youtube.com", "youtu.be", "klipy.com",	"twitter.com", "x.com", "instagram.com", "tiktok.com"
};

const std::vector<std::string> AURA_LOSSES = {
	"**HOLY AURA LOSS 💔**",
	"aura loss 💔",
	"💔💀 aura loss 😱",
	"Aura loss 💔",
	"damn you got less aura than axuaxi after that 💔",
	"holy aura loss 💔"
};

const std::vector<std::string> SPANISH_LOSS = {
	"adios aura 💔",
	"au rev'aura 💔",
	"AURA ⬇️  ",
	"你的aura减少。",	
	"Aura berkurang 💔"
};

std::vector<dpp::snowflake> SPECIALS = {
	802736184546689044,
};

std::string get_random_resp(const std::vector<std::string>& pool) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, pool.size() - 1);
	return pool[dis(gen)];
}

int main() {
	const char* token_env = std::getenv("BOT_TOKEN");
	if (!token_env) return 1;

	dpp::cluster bot(token_env, dpp::i_default_intents | dpp::i_message_content);

	bot.on_ready([&bot](const dpp::ready_t& event) {
			bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, "ponderin"));	
			if (dpp::run_once<struct register_bot_commands>()) {
			dpp::slashcommand echo_cmd("echo", "echo", bot.me.id);
			echo_cmd.set_default_permissions(dpp::p_administrator);
			echo_cmd.set_dm_permission(false);
			echo_cmd.add_option(
					dpp::command_option(dpp::co_string, "content", "msg", true)
					);
			dpp::slashcommand mkspl_cmd("mkspl", "make someone special", bot.me.id);
			mkspl_cmd.set_default_permissions(dpp::p_administrator);
			mkspl_cmd.set_dm_permission(false);
			mkspl_cmd.add_option(
					dpp::command_option(dpp::co_user, "user", "who to toggle from specialness?", true)
			);
			dpp::slashcommand mkshm_cmd("mkshm", dpp::ctxm_message, bot.me.id);
			mkshm_cmd.set_default_permissions(dpp::p_administrator);
			mkshm_cmd.set_dm_permission(false);
			dpp::slashcommand ab_cmd("auraboard", "who has lost the most aura?", bot.me.id);
			ab_cmd.add_option(
					dpp::command_option(dpp::co_string, "sort", "top or bottom 10", true)
					.add_choice(dpp::command_option_choice("top", std::string("top")))
					.add_choice(dpp::command_option_choice("bottom", std::string("bottom")))
					.add_choice(dpp::command_option_choice("me", std::string("me")))
			);
			dpp::slashcommand aura_mv_cmd("movaura", "update json for user", bot.me.id);
			aura_mv_cmd.set_default_permissions(dpp::p_administrator);
			aura_mv_cmd.add_option(
				dpp::command_option(dpp::co_user, "user", "who", true)
			);
			aura_mv_cmd.add_option(
				dpp::command_option(dpp::co_string, "mode", "mode for cmd", true)
				.add_choice(dpp::command_option_choice("set", std::string("set")))
				.add_choice(dpp::command_option_choice("rmv", std::string("rmv")))
				.add_choice(dpp::command_option_choice("add", std::string("add")))
			);
			aura_mv_cmd.add_option(dpp::command_option(dpp::co_integer, "amt", "mode for cmd", true));
			bot.global_bulk_command_create({echo_cmd, mkspl_cmd, mkshm_cmd, ab_cmd, aura_mv_cmd});
			}
			});
	bot.on_message_context_menu([&bot](const dpp::message_context_menu_t& event) {
		if (event.command.get_command_name() == "mkshm") {
				dpp::message msg = event.get_message();
				std::string evil_msg = "\"" + msg.content + "\" 💔 aura loss";
				dpp::message reply(msg.channel_id, evil_msg);
				reply.set_reference(msg.id);
				bot.message_create(reply);
				event.reply(dpp::message("shamed").set_flags(dpp::m_ephemeral));
		} //TODO: change this stupid shit where i keep repeating the same calls a million times across the code and just make a function like a normal person		
	});

	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
			if (event.command.get_command_name() == "echo") {
			std::string msg = std::get<std::string>(event.get_parameter("content"));
			bot.message_create(dpp::message(event.command.channel_id, msg));
			event.reply(dpp::message("ok").set_flags(dpp::m_ephemeral));
			} else if (event.command.get_command_name() == "mkspl") {
				dpp::snowflake usr = std::get<dpp::snowflake>(event.get_parameter("user"));
				auto it = std::find(SPECIALS.begin(), SPECIALS.end(), usr);
				if (it != SPECIALS.end()) {
					SPECIALS.erase(it);
					event.reply(dpp::message("user removed").set_flags(dpp::m_ephemeral));
				} else {
					SPECIALS.push_back(usr);
					event.reply(dpp::message("user added").set_flags(dpp::m_ephemeral));
				}	
			} else if (event.command.get_command_name() == "movaura") {
				json stats = load_stats();
				std::string user_id = std::get<dpp::snowflake>(event.get_parameter("user")).str();
				int user_count = stats["users"].value(user_id, 0);
				int total_count = stats.value("total", 0);
				int64_t amt = std::get<int64_t>(event.get_parameter("amt"));
				std::string mode = std::get<std::string>(event.get_parameter("mode"));

				if (mode == "rmv") {
					stats["users"][user_id] = user_count - amt;
					stats["total"] = total_count + amt;
					event.reply(dpp::message(event.command.channel_id, "holy aura loss, " + std::to_string(amt) + " from <@" + user_id + ">."));
				} else if (mode == "add") {
					stats["users"][user_id] = user_count + amt;
					stats["total"] = total_count - amt;
					event.reply(dpp::message(event.command.channel_id, "holy aura gain, " + std::to_string(amt) + " to <@" + user_id + ">."));
				} else if (mode == "set") {
					stats["users"][user_id] = amt;
					stats["total"] = total_count + (user_count - amt);
					event.reply(dpp::message(event.command.channel_id, "holy aura, " + std::to_string(amt) + " >> <@" + user_id + ">."));
				} else {
					event.reply(dpp::message("idk what you meant by that").set_flags(dpp::m_ephemeral));
					return;
				}
				save_stats(stats);
			} else if (event.command.get_command_name() == "auraboard") {
			json stats = load_stats();
			if (stats.find("users") == stats.end() || stats["users"].empty()) {
			event.reply(dpp::message("no").set_flags(dpp::m_ephemeral));
			return;
			}
			std::vector<std::pair<std::string, int>> ab_list;
			for (auto& [user_id, count] : stats["users"].items()) {
			ab_list.push_back({user_id, count.get<int>()});
			}
			if (std::get<std::string>(event.get_parameter("sort")) == "bottom") {
			std::sort(ab_list.begin(), ab_list.end(), [](const auto& a, const auto& b) {
					return a.second < b.second;
			});
			} else if (std::get<std::string>(event.get_parameter("sort")) == "me") {
				std::string user_id = std::to_string(event.command.get_issuing_user().id);
				int user_count = stats["users"].value(user_id, 0);
				std::string user_aura = std::to_string(user_count);
				event.reply(dpp::message("You have: " + user_aura + "aura").set_flags(dpp::m_ephemeral));
				return;
			} else {
			std::sort(ab_list.begin(), ab_list.end(), [](const auto& a, const auto& b) {
					return a.second > b.second;
			});
			}
			dpp::embed embed = dpp::embed()
			.set_color(dpp::colors::white)
			.set_title(std::get<std::string>(event.get_parameter("sort")) == "bottom" ? "LEAST AURA" : "MOST AURA");
			embed.add_field("Total aura lost:", std::to_string(stats["total"].get<int>()), true);
			std::string txt = "";
			int lim = std::min((int)ab_list.size(), 10);
			for (int i = 0; i < lim; ++i) {
				txt += "<@" + ab_list[i].first + ">: " + std::to_string(ab_list[i].second) + "AURA\n";
			}
			embed.set_description(txt);
			event.reply(dpp::message(event.command.channel_id, embed));
			}
	});

	bot.on_message_create([&bot](const dpp::message_create_t& event) {
			if (event.msg.author.is_bot()) return;
			auto it = std::find(SPECIALS.begin(), SPECIALS.end(), event.msg.author.id);
			if (it != SPECIALS.end()) {
			static std::random_device rd;
			static std::mt19937 gen_e(rd());
			std::uniform_int_distribution<> dis_e(0, 100);
			if (dis_e(gen_e) > 10) {
				json stats = load_stats();
				std::string user_id = std::to_string(event.msg.author.id);
				int user_count = stats["users"].value(user_id, 0);
				stats["users"][user_id] = user_count + 10;
				int total_count = stats.value("total", 0);
				stats["total"] = total_count - 10;
				save_stats(stats);
			}
			if (dis_e(gen_e) == 1) {
			json stats = load_stats();
			std::string user_id = std::to_string(event.msg.author.id);
			int user_count = stats["users"].value(user_id, 0);
			stats["users"][user_id] = user_count - 100;
			int total_count = stats.value("total", 0);
			stats["total"] = total_count + 100;
			save_stats(stats);
			std::string evil_msg = "\"" + event.msg.content + "\" 💔 aura loss";
			dpp::message reply(event.msg.channel_id, evil_msg);
			reply.set_reference(event.msg.id);
			bot.message_create(reply);
			return;
			}
			}

			bool allowed = false;
			for (auto id : ALLOWED_CHANNELS) if (event.msg.channel_id == id) allowed = true;
			if (!allowed) return;

			std::regex url_regex(R"(https?://[^\s]+)");
			std::smatch match;
			std::string content = event.msg.content;

			if (std::regex_search(content, match, url_regex)) {
				std::string url = match.str();
				bool is_reliable = false;
				for (const auto& p : RELIABLE_PROVIDERS) if (url.find(p) != std::string::npos) is_reliable = true;
				for (const auto& attachment: event.msg.attachments) {
					if (attachment.filename.size() >= 4 && (attachment.filename.substr(attachment.filename.size() -4) == ".gif" || attachment.filename.substr(attachment.filename.size() -4) == ".png" || attachment.filename.substr(attachment.filename.size() - 4) == ".jpg")) {
						dpp::channel* chan = dpp::find_channel(event.msg.channel_id);
						if (chan) {
							auto perms = chan->get_user_permissions(&(event.msg.author));
							if (!(perms & dpp::p_embed_links)) {
								json stats = load_stats();
								std::string user_id = std::to_string(event.msg.author.id);
								int user_count = stats["users"].value(user_id, 0);
								stats["users"][user_id] = user_count - 100;
								int total_count = stats.value("total", 0);
								stats["total"] = total_count + 100;

								save_stats(stats);
								bot.channel_typing(event.msg.channel_id);

								std::string text = (event.msg.channel_id == non_english_ch)
									? get_random_resp(SPANISH_LOSS)
									: get_random_resp(AURA_LOSSES);

								dpp::message reply(event.msg.channel_id, text);
								reply.set_reference(event.msg.id);
								bot.message_create(reply);
								return;
							}
						}
					}
				}
				if (!is_reliable || content.find("<" + url + ">") != std::string::npos) return;
				dpp::channel* chan = dpp::find_channel(event.msg.channel_id);
				if (chan) {
					auto perms = chan->get_user_permissions(&(event.msg.author));
					if (!(perms & dpp::p_embed_links)) {

						bot.channel_typing(event.msg.channel_id);

						dpp::snowflake msg_id = event.msg.id;
						dpp::snowflake chan_id = event.msg.channel_id;
						std::string user_id = std::to_string(event.msg.author.id);

						bot.start_timer([&bot, msg_id, chan_id, user_id](dpp::timer t) {
								json stats = load_stats();
								int user_count = stats["users"].value(user_id, 0);
								stats["users"][user_id] = user_count - 100;
								int total_count = stats.value("total", 0);
								stats["total"] = total_count + 100;
								save_stats(stats);

								std::string text = (chan_id == non_english_ch)
								? get_random_resp(SPANISH_LOSS)
								: get_random_resp(AURA_LOSSES);

								dpp::message reply(chan_id, text);
								reply.set_reference(msg_id);

								bot.message_create(reply);

								bot.stop_timer(t);
								}, 5);

						return;
					}
				}

				dpp::snowflake msg_id = event.msg.id;
				dpp::snowflake chan_id = event.msg.channel_id;

				bot.start_timer([&bot, msg_id, chan_id, url](dpp::timer t) {
						bot.message_get(msg_id, chan_id, [&bot, chan_id, msg_id, url](const dpp::confirmation_callback_t& res) {
								if (res.is_error()) return;
								dpp::message m = std::get<dpp::message>(res.value);

								if (m.embeds.empty() && m.content.find("<" + url + ">") == std::string::npos) {
								json stats = load_stats();
								std::string user_id = std::to_string(std::get<dpp::message>(res.value).author.id);
								int user_count = stats["users"].value(user_id, 0);
								stats["users"][user_id] = user_count - 100;
								int total_count = stats.value("total", 0);
								stats["total"] = total_count + 100;

								save_stats(stats);
								std::string reply_text;
								if (chan_id == non_english_ch) {
								reply_text = get_random_resp(SPANISH_LOSS);
								} else {
								reply_text = get_random_resp(AURA_LOSSES);
								}

								dpp::message fail_reply(chan_id, reply_text);
								fail_reply.set_reference(msg_id);
								bot.message_create(fail_reply);
								}
						});
						bot.stop_timer(t);
				}, 2);
			}
	});

	bot.start(dpp::st_wait);
	return 0;
}
