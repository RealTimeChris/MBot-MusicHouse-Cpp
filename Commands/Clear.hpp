// Clear.hpp - Header for the "clear" command.
// Aug 19, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Clear : public BaseFunction {
	  public:
		Clear() {
			this->commandName	  = "clear";
			this->helpDescription = "Clears the current song queue.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /clear! And that's it!\n------");
			msgEmbed.setTitle("__**Clear Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Clear>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelCacheData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };

				DiscordGuild discordGuild{ managerAgent, guild };

				bool checkIfAllowedInChannel = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!checkIfAllowedInChannel) {
					return;
				}

				GuildMemberCacheData guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}

				discordGuild.data.playlist = Playlist{};
				discordGuild.writeDataToDB(managerAgent);

				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
				msgEmbed.setDescription("\n------\n__**You have cleared the song queue!**__\n------");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Queue Cleared:**__");
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "Clear::execute()" << error.what() << std::endl;
			}
		}
		~Clear(){};
	};

}// namespace DiscordCoreAPI
