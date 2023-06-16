// Pause.hpp - Header for the "pause" command.
// Aug 19, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Pause : public BaseFunction {
	  public:
		Pause() {
			this->commandName	  = "pause";
			this->helpDescription = "Pauses the currently playing music.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /pause!\n------");
			msgEmbed.setTitle("__**Pause Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Pause>();
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

				RespondToInputEventData dataPackage00(argsNew.getInputEventData());
				dataPackage00.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				InputEvents::respondToInputEventAsync(dataPackage00).get();
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
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				VoiceConnection& voiceConnection = guild.connectToVoice(guildMember.user.id);
				discordGuild.getDataFromDB(managerAgent);
				if (!voiceConnection.areWeConnected() || !guild.areWeConnected()) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Connection Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (guildMember.getVoiceStateData().channelId == 0 || guildMember.getVoiceStateData().channelId != voiceConnection.getChannelId()) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Pausing Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (!DiscordCoreClient::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but I need to be either playing or paused for this command to be possible!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Pausing Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				DiscordCoreClient::getSongAPI(guild.id).pauseToggle();

				EmbedData msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
				msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
				msgEmbed.setDescription("\n------\n__**Songs Remaining In Queue:**__ " + jsonifier::toString(discordGuild.data.playlist.songQueue.size()) + "\n------");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Paused Playback:**__");
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				dataPackage.addMessageEmbed(msgEmbed);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "Pause::execute()" << error.what() << std::endl;
			}
		}
		~Pause(){};
	};

}// namespace DiscordCoreAPI
