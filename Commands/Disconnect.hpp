// Disconnect.hpp - Header for the "disconnect" std::function.
// Aug 25, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Disconnect : public BaseFunction {
	  public:
		Disconnect() {
			this->commandName	  = "disconnect";
			this->helpDescription = "Disconnect the bot from voice chat.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /disconnect!\n------");
			msgEmbed.setTitle("__**Disconnect Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Disconnect>();
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
				if (guild.areWeConnected()) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**I'm disconnecting from the voice channel!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Disconnected:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(newEvent, 20000);
					discordGuild.writeDataToDB(managerAgent);
				} else {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**I was already disconnected!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Disconnected:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				jsonifier::vector<Song> newVector{};
				if (discordGuild.data.playlist.currentSong.songId != "") {
					newVector.emplace_back(discordGuild.data.playlist.currentSong);
				}
				discordGuild.data.playlist.currentSong = Song{};
				for (auto& value : discordGuild.data.playlist.songQueue) {
					newVector.emplace_back(value);
				}
				discordGuild.data.playlist.songQueue = newVector;
				discordGuild.writeDataToDB(managerAgent);
				guild.disconnect();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "Disconnect::execute()" << error.what() << std::endl;
			}
		}
		~Disconnect(){};
	};

}// namespace DiscordCoreAPI
