#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand starboard_admin_def(dpp::cluster& bot) {
		return dpp::slashcommand("starboard", "manage the starboard", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			.add_option(dpp::command_option(dpp::co_sub_command, "add", "add a channel for starboard")
							.add_option(dpp::command_option(dpp::co_channel, "channel", "channel", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "remove", "remove a channel")
							.add_option(dpp::command_option(dpp::co_channel, "channel", "channel to remove", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "list", "list channels"));
	}

	void handle_starboard_admin(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto sub = event.command.get_command_interaction().options[0].name;
		dpp::snowflake g_id = event.command.guild_id;

		if (sub == "add") {
			dpp::snowflake ch_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
			db::sb_add_channel(g_id, ch_id);
			event.reply(dpp::message("added <#" + std::to_string(ch_id) + ">.").set_flags(dpp::m_ephemeral));

		} else if (sub == "remove") {
			dpp::snowflake ch_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
			db::sb_remove_channel(g_id, ch_id);
			event.reply(dpp::message("removed <#" + std::to_string(ch_id) + ">.").set_flags(dpp::m_ephemeral));

		} else if (sub == "list") {
			auto channels = db::sb_get_channels(g_id);
			if (channels.empty()) {
				event.reply(dpp::message("no channels yet.").set_flags(dpp::m_ephemeral));
				return;
			}
			std::string out = "**starboardable channels:**\n";
			for (auto ch : channels)
				out += "<#" + std::to_string(ch) + ">\n";
			event.reply(
				dpp::message(out).set_flags(dpp::m_ephemeral).set_allowed_mentions(false, false, false, false, {}, {}));
		}
	}

	dpp::slashcommand starboard_ctx_def(dpp::cluster& bot) {
		dpp::slashcommand cmd("starboard", "", bot.me.id);
		cmd.type = dpp::ctxm_message;
		return cmd;
	}

	void handle_starboard_ctx(const dpp::message_context_menu_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake ch_id = event.command.channel_id;

		if (!db::sb_is_allowed(g_id, ch_id)) {
			event.reply(
				dpp::message("this channel isn't set up for starboard nominations.").set_flags(dpp::m_ephemeral));
			return;
		}

		std::string sb_ch_str = db::get_setting_str(g_id, "sb_channel", "0");
		dpp::snowflake sb_ch = 0;
		try {
			sb_ch = std::stoull(sb_ch_str);
		} catch (...) {}
		if (sb_ch == 0) {
			event.reply(dpp::message("no starboard channel configured. Admins: use `/settings sb_channel`.")
							.set_flags(dpp::m_ephemeral));
			return;
		}

		int pos_cost = db::get_setting_int(g_id, "sb_pos_cost", 1000);
		int neg_cost = db::get_setting_int(g_id, "sb_neg_cost", 1000);
		const dpp::message& target = event.get_message();

		std::string pos_id = "sb_pos_" + std::to_string(target.id) + "_" + std::to_string(ch_id);
		std::string neg_id = "sb_neg_" + std::to_string(target.id) + "_" + std::to_string(ch_id);

		dpp::component pos_btn;
		pos_btn.set_type(dpp::cot_button)
			.set_id(pos_id)
			.set_label("positive (" + std::to_string(pos_cost) + " aura)")
			.set_style(dpp::cos_success);

		dpp::component neg_btn;
		neg_btn.set_type(dpp::cot_button)
			.set_id(neg_id)
			.set_label("negative (" + std::to_string(neg_cost) + " aura)")
			.set_style(dpp::cos_danger);

		dpp::component row;
		row.set_type(dpp::cot_action_row);
		row.add_component(pos_btn);
		row.add_component(neg_btn);

		dpp::message reply;
		reply.set_flags(dpp::m_ephemeral);
		reply.set_content("post this message to the starboard?");
		reply.add_component(row);
		event.reply(reply);
	}

	void handle_starboard_button(const dpp::button_click_t& event, dpp::cluster& bot) {
		const std::string& id = event.custom_id;
		bool is_positive = (id.rfind("sb_pos_", 0) == 0);

		std::string payload = id.substr(7);
		size_t sep = payload.find('_');
		if (sep == std::string::npos) return;

		dpp::snowflake src_msg_id, src_ch_id;
		try {
			src_msg_id = std::stoull(payload.substr(0, sep));
			src_ch_id = std::stoull(payload.substr(sep + 1));
		} catch (...) { return; }

		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = event.command.get_issuing_user().id;

		dpp::snowflake sb_ch = 0;
		try {
			sb_ch = std::stoull(db::get_setting_str(g_id, "sb_channel", "0"));
		} catch (...) {}
		if (sb_ch == 0) {
			event.reply(dpp::message("starboard channel not configured.").set_flags(dpp::m_ephemeral));
			return;
		}

		int cost = db::get_setting_int(g_id, is_positive ? "sb_pos_cost" : "sb_neg_cost", 100);
		int aura = db::get_aura(g_id, u_id);
		if (aura < cost) {
			event.reply(dpp::message("not enough aura (you have **" + std::to_string(aura) + "**, need **" +
									 std::to_string(cost) + "**).")
							.set_flags(dpp::m_ephemeral));
			return;
		}

		event.reply(dpp::message("posting…").set_flags(dpp::m_ephemeral));
		db::rmv_aura(g_id, u_id, cost);

		bot.message_get(
			src_msg_id, src_ch_id,
			[&bot, g_id, u_id, sb_ch, src_ch_id, is_positive, cost](const dpp::confirmation_callback_t& cb) {
				if (cb.is_error()) return;
				const dpp::message& orig = std::get<dpp::message>(cb.value);

				dpp::embed emb;
				emb.set_color(is_positive ? 0x52CB6C : 0xCF3048);
				emb.add_field("Posted by", "<@" + std::to_string(orig.author.id) + ">", true);
				emb.add_field("Channel", "<#" + std::to_string(src_ch_id) + ">", true);
				emb.add_field("Paid by", "<@" + std::to_string(u_id) + ">  (" + std::to_string(cost) + " aura)", true);

				if (!orig.content.empty()) {
					std::string content = orig.content;
					if (content.size() > 1024) content = content.substr(0, 1021) + "…";
					emb.set_description(content);
				}

				for (auto& att : orig.attachments) {
					if (att.filename.size() >= 4) {
						std::string ext = att.filename.substr(att.filename.size() - 4);
						for (auto& c : ext)
							c = (char)tolower((unsigned char)c);
						if (ext == ".png" || ext == ".jpg" || ext == "jpeg" || ext == ".gif" || ext == "webp") {
							emb.set_image(att.url);
							break;
						}
					}
				}

				std::string jump = "https://discord.com/channels/" + std::to_string(g_id) + "/" +
								   std::to_string(src_ch_id) + "/" + std::to_string(orig.id);
				emb.set_footer("jump to original : " + jump, "");

				bot.message_create(dpp::message(sb_ch, emb));
			});
	}

} // namespace commands
