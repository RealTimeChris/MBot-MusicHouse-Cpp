// disconnect.hpp - header for the "disconnect" std::function.
// aug 25, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class disconnect : public base_function {
	  public:
		disconnect() {
			this->commandName	  = "disconnect";
			this->helpDescription = "Disconnect the bot from voice chat.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /disconnect!\n------");
			msgEmbed.setTitle("__**Disconnect Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<disconnect>();
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
				if (guild.areWeConnected()) {
					embed_data newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed.setDescription("------\n__**I'm disconnecting from the voice channel!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Disconnected:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					input_events::deleteInputEventResponseAsync(newEvent, 20000);
					discordGuild.writeDataToDB();
				} else {
					embed_data newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed.setDescription("------\n__**I was already disconnected!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Disconnected:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				jsonifier::vector<song> newVector{};
				if (discordGuild.data.playlist.currentSong.songId != "") {
					newVector.emplace_back(discordGuild.data.playlist.currentSong);
				}
				discordGuild.data.playlist.currentSong = song{};
				for (auto& value : discordGuild.data.playlist.songQueue) {
					newVector.emplace_back(value);
				}
				discordGuild.data.playlist.songQueue = newVector;
				discordGuild.writeDataToDB();
				guild.disconnect();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "disconnect::execute()" << error.what() << std::endl;
			}
		}
		~disconnect(){};
	};

}// namespace discord_core_api
