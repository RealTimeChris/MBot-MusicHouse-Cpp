// LoopAll.hpp - Header for the "loop song" command.
// Aug 19, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {
	class LoopAll : public BaseFunction {
	  public:
		LoopAll() {
			this->commandName	  = "loopall";
			this->helpDescription = "Enables or disables looping of the current queue.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /loopall!\n------");
			msgEmbed.setTitle("__**Loop-All Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<LoopAll>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelCacheData channel{ argsNew.getChannelData() };

				GuildData guild{ argsNew.getInteractionData().guildId };

				DiscordGuild discordGuild{ managerAgent, guild };

				bool areWeAllowed = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!areWeAllowed) {
					return;
				}

				GuildMemberCacheData guildMember{ argsNew.getGuildMemberData() };

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
				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
				if (discordGuild.data.playlist.isLoopAllEnabled) {
					msgEmbed.setDescription("\n------\n__**Looping-All has been enabled!**__\n------\n");
				} else {
					msgEmbed.setDescription("\n------\n__**Looping-All has been disabled!**__\n------\n");
				}
				discordGuild.writeDataToDB(managerAgent);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Looping-All Change:**__");
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "LoopAll::execute()" << error.what() << std::endl;
			}
		}
		~LoopAll(){};
	};

}// namespace DiscordCoreAPI
