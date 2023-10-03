// clear.hpp - header for the "clear" command.
// aug 19, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "../HelperFunctions.hpp"

namespace discord_core_api {

	class clear : public base_function {
	  public:
		clear() {
			this->commandName	  = "clear";
			this->helpDescription = "clears the current song queue.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /clear! and that's it!\n------");
			msgEmbed.setTitle("__**clear usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<clear>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_cache_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ managerAgent, guild };

				bool checkIfAllowedInChannel = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!checkIfAllowedInChannel) {
					return;
				}

				guild_member_cache_data guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}

				discordGuild.data.playlist = playlist{};
				discordGuild.writeDataToDB(managerAgent);

				embed_data msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				msgEmbed.setDescription("\n------\n__**you have cleared the song queue!**__\n------");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**queue cleared:**__");
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "clear::execute()" << error.what() << std::endl;
			}
		}
		~clear(){};
	};

}// namespace discord_core_api
