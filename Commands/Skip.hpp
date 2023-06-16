// Skip.hpp - Header for the "skip" command.
// Aug 18, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {
	class Skip : public BaseFunction {
	  public:
		static std::unordered_map<uint64_t, uint64_t> timeOfLastSkip;

		Skip() {
			this->commandName	  = "skip";
			this->helpDescription = "Skips to the next song in the queue.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /skip.\n------");
			msgEmbed.setTitle("__**Skip Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Skip>();
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
				InputEventData newEvent = argsNew.getInputEventData();

				uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				uint64_t previousSkippedTime{ 0 };
				if (Skip::timeOfLastSkip.contains(argsNew.getInteractionData().guildId.operator const uint64_t&())) {
					previousSkippedTime = Skip::timeOfLastSkip.at(argsNew.getInteractionData().guildId.operator const uint64_t&());
				}

				if (currentTime - previousSkippedTime < 5000) {
					EmbedData newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but please wait a total of 5 seconds in between skips!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Timing Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				previousSkippedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				Skip::timeOfLastSkip.insert_or_assign(argsNew.getInteractionData().guildId.operator const uint64_t&(), previousSkippedTime);
				Snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != size_t{ 0 }) {
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
					newEmbed.setTitle("__**Skipping Issue:**__");
					newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(newEmbed);
					InputEvents::respondToInputEventAsync(dataPackage02).get();

					return;
				}

				if (!guild.areWeConnected() || !DiscordCoreClient::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					jsonifier::string msgString = "------\n**There's no music playing to be skipped!**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Skipping Issue:**__");
					RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed);
					InputEvents::respondToInputEventAsync(dataPackage02).get();

					return;
				}

				if (!discordGuild.data.playlist.songQueue.size()) {
					jsonifier::string msgString = "------\n**There's no more songs for us to skip to!**\n------";
					EmbedData msgEmbed02;
					msgEmbed02.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed02.setColor(jsonifier::string{ discordGuild.data.borderColor });
					msgEmbed02.setTimeStamp(getTimeAndDate());
					msgEmbed02.setDescription(msgString);
					msgEmbed02.setTitle("__**Song Queue Issue:**__");
					RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed02);
					InputEvents::respondToInputEventAsync(dataPackage02).get();

					return;
				} else {
					if (DiscordCoreClient::getSongAPI(guild.id).areWeCurrentlyPlaying() && discordGuild.data.playlist.songQueue.size()) {
						jsonifier::string msgString = "------\n**We're skipping to the next song!**\n------";
						EmbedData msgEmbed02{};
						msgEmbed02.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar))
							.setColor(jsonifier::string{ discordGuild.data.borderColor })
							.setTimeStamp(getTimeAndDate())
							.setDescription(msgString)
							.setTitle("__**Song Skip:**__");
						RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
						dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage02.addMessageEmbed(msgEmbed02);
						auto newEvent02 = InputEvents::respondToInputEventAsync(dataPackage02).get();
						DiscordCoreClient::getSongAPI(guild.id).skip(guildMember);
					} else if (!discordGuild.data.playlist.songQueue.size()) {
						EmbedData newEmbed{};
						newEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						newEmbed.setDescription("------\n__**Sorry, but there's nothing left to play here!**__\n------");
						newEmbed.setTimeStamp(getTimeAndDate());
						newEmbed.setTitle("__**Now Playing:**__");
						newEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
						if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("✅ Loop-All, ✅ Loop-Song");
						} else if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("❌ Loop-All, ✅ Loop-Song");
						} else if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("✅ Loop-All, ❌ Loop-Song");
						} else if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("❌ Loop-All, ❌ Loop-Song");
						}
						RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
						dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage02.addMessageEmbed(newEmbed);
						InputEvents::respondToInputEventAsync(dataPackage02).get();
						return;
					} else {
						jsonifier::string msgString = "------\n**There's no music playing to be skipped!**\n------";
						EmbedData msgEmbed02;
						msgEmbed02.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						msgEmbed02.setColor(jsonifier::string{ discordGuild.data.borderColor });
						msgEmbed02.setTimeStamp(getTimeAndDate());
						msgEmbed02.setDescription(msgString);
						msgEmbed02.setTitle("__**Skipping Issue:**__");
						RespondToInputEventData dataPackage02{ argsNew.getInputEventData() };
						dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
						dataPackage02.addMessageEmbed(msgEmbed02);
						InputEvents::respondToInputEventAsync(dataPackage02).get();
						return;
					}
				}
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "Skip::execute()" << error.what() << std::endl;
			}
		}
		~Skip(){};
	};
	std::unordered_map<uint64_t, uint64_t> Skip::timeOfLastSkip{};
}// namespace DiscordCoreAPI
