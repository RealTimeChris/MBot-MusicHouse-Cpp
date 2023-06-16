// Np.hpp - Header for the "now playing" command.
// Sep 4, 2021
// Chris M.
// https://gibhub.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Np : public BaseFunction {
	  public:
		Np() {
			this->commandName	  = "np";
			this->helpDescription = "Displays the currently playing song.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /np!\n------");
			msgEmbed.setTitle("__**Now Playing Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Np>();
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
				discordGuild.getDataFromDB(managerAgent);
				EmbedData newEmbed{};
				newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				newEmbed.setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl + ")" +
										"\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " +
										discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" + discordGuild.data.playlist.currentSong.addedByUserId + "> (" +
										discordGuild.data.playlist.currentSong.addedByUserName + ")");
				newEmbed.setImage(discordGuild.data.playlist.currentSong.thumbnailUrl);
				newEmbed.setTimeStamp(getTimeAndDate());
				newEmbed.setTitle("__**Now Playing:**__");
				newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
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
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
				dataPackage.addMessageEmbed(newEmbed);
				auto newEvent02 = InputEvents::respondToInputEventAsync(dataPackage).get();


				return;
			} catch (const std::runtime_error& error) {
				std::cout << "NP:execute()" << error.what() << std::endl;
			}
		}
		~Np(){};
	};

}// namespace DiscordCoreAPI
