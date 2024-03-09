// loop_song.hpp - header for the "loop song" command.
// aug 19, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class loop_song : public base_function {
	  public:
		loop_song() {
			this->commandName	  = "loop_song";
			this->helpDescription = "Enables or disables looping of the current song.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /loopsong!\n------");
			msgEmbed.setTitle("__**Loop-Song Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<loop_song>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ guild };

				bool areWeAllowed = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!areWeAllowed) {
					return;
				}

				guild_member_data guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}

				if (discordGuild.data.playlist.isLoopSongEnabled) {
					discordGuild.data.playlist.isLoopSongEnabled = false;
				} else {
					discordGuild.data.playlist.isLoopAllEnabled	 = false;
					discordGuild.data.playlist.isLoopSongEnabled = true;
				}
				embed_data msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				if (discordGuild.data.playlist.isLoopSongEnabled) {
					msgEmbed.setDescription("\n------\n__**Looping-Song has been enabled!**__\n------\n");
				} else {
					msgEmbed.setDescription("\n------\n__**Looping-Song has been disabled!**__\n------\n");
				}
				discordGuild.writeDataToDB();
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Looping-Song Change:**__");
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "loop_song::execute()" << error.what() << std::endl;
			}
		}
		~loop_song(){};
	};

}// namespace discord_core_api
