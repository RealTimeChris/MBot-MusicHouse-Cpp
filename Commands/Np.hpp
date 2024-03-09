// np.hpp - header for the "Now Playing" command.
// sep 4, 2021
// chris m.
// https://gibhub.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class np : public base_function {
	  public:
		np() {
			this->commandName	  = "np";
			this->helpDescription = "Displays the currently playing song.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /np!\n------");
			msgEmbed.setTitle("__**Now Playing Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<np>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ guild };

				bool checkIfAllowedInChannel = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!checkIfAllowedInChannel) {
					return;
				}

				guild_member_data guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}
				discordGuild.getDataFromDB();
				embed_data newEmbed{};
				newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				newEmbed.setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl + ")" +
					"\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " + discordGuild.data.playlist.currentSong.duration +
					"\n__**Added By:**__ <@!" + discordGuild.data.playlist.currentSong.addedByUserId + ">");
				newEmbed.setImage(discordGuild.data.playlist.currentSong.thumbnailUrl);
				newEmbed.setTimeStamp(getTimeAndDate());
				newEmbed.setTitle("__**Now Playing:**__");
				newEmbed.setColor("fefefe");
				if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
					newEmbed.setFooter("✅ Loop-All, ✅ Loop-Song");
				}
				if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
					newEmbed.setFooter("❌ Loop-All, ✅ Loop-Song");
				}
				if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
					newEmbed.setFooter("✅ Loop-All, ❌ Loop-Song");
				}
				if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
					newEmbed.setFooter("❌ Loop-All, ❌ Loop-Song");
				}
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Interaction_Response);
				dataPackage.addMessageEmbed(newEmbed);
				auto newEvent02 = input_events::respondToInputEventAsync(dataPackage).get();


				return;
			} catch (const std::runtime_error& error) {
				std::cout << "np:execute()" << error.what() << std::endl;
			}
		}
		~np(){};
	};

}// namespace discord_core_api
