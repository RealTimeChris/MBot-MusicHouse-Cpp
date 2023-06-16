// StreamAudio.hpp - Header for the "disconnect" std::function.
// Aug 25, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class StreamAudio : public BaseFunction {
	  public:
		StreamAudio() {
			this->commandName	  = "streamaudio";
			this->helpDescription = "Stream Audio from one bot to another.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /streamaudio!\n------");
			msgEmbed.setTitle("__**StreamAudio Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<StreamAudio>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelCacheData channel{ argsNew.getChannelData() };


				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };

				GuildMemberCacheData guildMember{ argsNew.getGuildMemberData() };

				Snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != 0) {
					currentVoiceChannelId = guildMember.getVoiceStateData().channelId;
				} else {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getGuildMemberImageUrl(GuildMemberImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				StreamInfo theInfo{};
				theInfo.address = argsNew.getCommandArguments().values["connectionip"].value;
				theInfo.port	= 51072;
				theInfo.type	= StreamType::Server;
				if (argsNew.getCommandArguments().values["botaudio"].value == "true") {
					theInfo.streamBotAudio = true;
				} else {
					theInfo.streamBotAudio = false;
				}
				VoiceConnection& voiceConnection = guild.connectToVoice(guildMember.user.id, 0, false, false, theInfo);
				if (!voiceConnection.areWeConnected()) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getGuildMemberImageUrl(GuildMemberImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Connection Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (guildMember.getVoiceStateData().channelId == 0 || guildMember.getVoiceStateData().channelId != voiceConnection.getChannelId()) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getGuildMemberImageUrl(GuildMemberImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				std::this_thread::sleep_for(1000ms);
				DiscordCoreClient::getSongAPI(guild.id).play();

				UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
				newEmbed->setAuthor(guildMember.getUserData().userName, guildMember.getGuildMemberImageUrl(GuildMemberImageTypes::Avatar));
				newEmbed->setDescription("------\n__**Congratulations - you've been connected to an audio channel to stream audio!**__\n------");
				newEmbed->setTimeStamp(getTimeAndDate());
				newEmbed->setTitle("__**Streaming Audio:**__");
				newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
				RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
				dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
				dataPackage02.addMessageEmbed(*newEmbed);
				InputEvents::respondToInputEventAsync(dataPackage02).get();
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "StreamAudio::execute()" << error.what() << std::endl;
			}
		}
		~StreamAudio(){};
	};

}// namespace DiscordCoreAPI
