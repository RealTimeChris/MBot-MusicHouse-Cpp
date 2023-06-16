// Stop.hpp - Header for the "stop" command.
// Aug 19, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Stop : public BaseFunction {
	  public:
		Stop() {
			this->commandName	  = "stop";
			this->helpDescription = "Stops the current song from playing.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /stop!\n------");
			msgEmbed.setTitle("__**Stop Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Stop>();
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

				InputEventData newEvent = argsNew.getInputEventData();

				uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				uint64_t previousPlayedTime{ 0 };
				if (Play::timeOfLastPlay.contains(argsNew.getInteractionData().guildId.operator const uint64_t&())) {
					previousPlayedTime = Play::timeOfLastPlay.at(argsNew.getInteractionData().guildId.operator const uint64_t&());
				}

				if (currentTime - previousPlayedTime < 5000) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but please wait a total of 5 seconds after playing, before attempting to stop!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Timing Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				Snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != 0) {
					currentVoiceChannelId = guildMember.getVoiceStateData().channelId;
				} else {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Connection Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				VoiceConnection& voiceConnection = guild.connectToVoice(guildMember.user.id);
				discordGuild.getDataFromDB(managerAgent);
				if (!voiceConnection.areWeConnected()) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Connection Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (guildMember.getVoiceStateData().channelId == 0 || guildMember.getVoiceStateData().channelId != voiceConnection.getChannelId()) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Stopping Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (!guild.areWeConnected() || !DiscordCoreClient::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					jsonifier::string msgString = "------\n**There's no music playing to be stopped!**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Stopping Issue:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
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
				DiscordCoreClient::getSongAPI(guild.id).stop();
				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
				msgEmbed.setDescription("\n------\n__**Songs Remaining In Queue:**__ " + jsonifier::toString(discordGuild.data.playlist.songQueue.size()) + "\n------");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Stopping Playback:**__");
				RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
				dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage02.addMessageEmbed(msgEmbed);
				InputEvents::respondToInputEventAsync(dataPackage02).get();
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "Stop::execute()" << error.what() << std::endl;
			}
		}
		~Stop(){};
	};

}// namespace DiscordCoreAPI
