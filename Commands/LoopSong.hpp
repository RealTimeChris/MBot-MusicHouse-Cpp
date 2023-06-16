// LoopSong.hpp - Header for the "loop song" command.
// Aug 19, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {
	class LoopSong : public BaseFunction {
	  public:
		LoopSong() {
			this->commandName	  = "LoopSong";
			this->helpDescription = "Enables or disables looping of the current song.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /loopsong!\n------");
			msgEmbed.setTitle("__**Loop-Song Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<LoopSong>();
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

				if (discordGuild.data.playlist.isLoopSongEnabled) {
					discordGuild.data.playlist.isLoopSongEnabled = false;
				} else {
					discordGuild.data.playlist.isLoopAllEnabled	 = false;
					discordGuild.data.playlist.isLoopSongEnabled = true;
				}
				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
				if (discordGuild.data.playlist.isLoopSongEnabled) {
					msgEmbed.setDescription("\n------\n__**Looping-Song has been enabled!**__\n------\n");
				} else {
					msgEmbed.setDescription("\n------\n__**Looping-Song has been disabled!**__\n------\n");
				}
				discordGuild.writeDataToDB(managerAgent);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Looping-Song Change:**__");
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "LoopSong::execute()" << error.what() << std::endl;
			}
		}
		~LoopSong(){};
	};

}// namespace DiscordCoreAPI
