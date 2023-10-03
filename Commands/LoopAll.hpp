// loop_all.hpp - header for the "loop song" command.
// aug 19, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "../HelperFunctions.hpp"

namespace discord_core_api {
	class loop_all : public base_function {
	  public:
		loop_all() {
			this->commandName	  = "loopall";
			this->helpDescription = "enables or disables looping of the current queue.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /loopall!\n------");
			msgEmbed.setTitle("__**Loop-All usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<loop_all>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_cache_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ managerAgent, guild };

				bool areWeAllowed = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!areWeAllowed) {
					return;
				}

				guild_member_cache_data guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}

				if (discordGuild.data.playlist.isLoopAllEnabled) {
					discordGuild.data.playlist.isLoopAllEnabled = false;
				} else {
					discordGuild.data.playlist.isLoopSongEnabled = false;
					discordGuild.data.playlist.isLoopAllEnabled	 = true;
				}
				embed_data msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				if (discordGuild.data.playlist.isLoopAllEnabled) {
					msgEmbed.setDescription("\n------\n__**looping-all has been enabled!**__\n------\n");
				} else {
					msgEmbed.setDescription("\n------\n__**looping-all has been disabled!**__\n------\n");
				}
				discordGuild.writeDataToDB(managerAgent);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**looping-all change:**__");
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "loop_all::execute()" << error.what() << std::endl;
			}
		}
		~loop_all(){};
	};

}// namespace discord_core_api
