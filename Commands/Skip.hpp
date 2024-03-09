// skip.hpp - header for the "skip" command.
// aug 18, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {
	class skip : public base_function {
	  public:
		static std::unordered_map<uint64_t, uint64_t> timeOfLastSkip;

		skip() {
			this->commandName	  = "skip";
			this->helpDescription = "Skips to the next song in the queue.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /skip.\n------");
			msgEmbed.setTitle("__**Skip Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<skip>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ guild };

				bool areWeAllowed = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!areWeAllowed) {
					return;
				}

				guild_member_data guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}
				input_event_data newEvent = argsNew.getInputEventData();

				uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				uint64_t previousSkippedTime{ 0 };
				if (skip::timeOfLastSkip.contains(argsNew.getInteractionData().guildId.operator const uint64_t&())) {
					previousSkippedTime = skip::timeOfLastSkip.at(argsNew.getInteractionData().guildId.operator const uint64_t&());
				}

				if (currentTime - previousSkippedTime < 5000) {
					embed_data newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but please wait a total of 5 seconds in between skips!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Timing Issue:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				previousSkippedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				skip::timeOfLastSkip.insert_or_assign(argsNew.getInteractionData().guildId.operator const uint64_t&(), previousSkippedTime);
				snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != size_t{ 0 }) {
					currentVoiceChannelId = guildMember.getVoiceStateData().channelId;
				} else {
					embed_data newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Connection Issue:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				voice_connection& voiceConnection = guild.connectToVoice(guildMember.user.id);
				if (!voiceConnection.areWeConnected()) {
					embed_data newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Connection Issue:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (guildMember.getVoiceStateData().channelId == 0 || guildMember.getVoiceStateData().channelId != voiceConnection.getChannelId()) {
					embed_data newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Skipping Issue:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage02{ argsNew.getInputEventData() };
					dataPackage02.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(newEmbed);
					input_events::respondToInputEventAsync(dataPackage02).get();

					return;
				}

				if (!guild.areWeConnected() || !discord_core_client::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					jsonifier::string msgString = "------\n**There's no music playing to be skipped!**\n------";
					embed_data msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					msgEmbed.setColor("fefefe");
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Skipping Issue:**__");
					respond_to_input_event_data dataPackage02{ argsNew.getInputEventData() };
					dataPackage02.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed);
					input_events::respondToInputEventAsync(dataPackage02).get();

					return;
				}

				if (!discordGuild.data.playlist.songQueue.size()) {
					jsonifier::string msgString = "------\n**There's no more songs for us to skip to!**\n------";
					embed_data msgEmbed02;
					msgEmbed02.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					msgEmbed02.setColor("fefefe");
					msgEmbed02.setTimeStamp(getTimeAndDate());
					msgEmbed02.setDescription(msgString);
					msgEmbed02.setTitle("__**Song Queue Issue:**__");
					respond_to_input_event_data dataPackage02{ argsNew.getInputEventData() };
					dataPackage02.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage02.addMessageEmbed(msgEmbed02);
					input_events::respondToInputEventAsync(dataPackage02).get();

					return;
				} else {
					if (discord_core_client::getSongAPI(guild.id).areWeCurrentlyPlaying() && discordGuild.data.playlist.songQueue.size()) {
						jsonifier::string msgString = "------\n**We're skipping to the next song!**\n------";
						embed_data msgEmbed02{};
						msgEmbed02.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar))
							.setColor("fefefe")
							.setTimeStamp(getTimeAndDate())
							.setDescription(msgString)
							.setTitle("__**Song Skip:**__");
						respond_to_input_event_data dataPackage02{ argsNew.getInputEventData() };
						dataPackage02.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
						dataPackage02.addMessageEmbed(msgEmbed02);
						discordGuild.data.playlist.currentSong = discordGuild.data.playlist.songQueue.at(0);
						discordGuild.writeDataToDB();
						auto newEvent02 = input_events::respondToInputEventAsync(dataPackage02).get();
						discord_core_client::getSongAPI(guild.id).skip();
					} else if (!discordGuild.data.playlist.songQueue.size()) {
						embed_data newEmbed{};
						newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
						newEmbed.setDescription("------\n__**Sorry, but there's nothing left to play here!**__\n------");
						newEmbed.setTimeStamp(getTimeAndDate());
						newEmbed.setTitle("__**Now Playing:**__");
						newEmbed.setColor("fefefe");
						if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("✅ Loop-All, ✅ Loop-Song");
						} else if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("❌ Loop-All, ✅ Loop-Song");
						} else if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("✅ Loop-All, ❌ Loop-Song");
						} else if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							newEmbed.setFooter("❌ Loop-All, ❌ Loop-Song");
						}
						respond_to_input_event_data dataPackage02{ argsNew.getInputEventData() };
						dataPackage02.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
						dataPackage02.addMessageEmbed(newEmbed);
						input_events::respondToInputEventAsync(dataPackage02).get();
						return;
					} else {
						jsonifier::string msgString = "------\n**There's no music playing to be skipped!**\n------";
						embed_data msgEmbed02;
						msgEmbed02.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
						msgEmbed02.setColor("fefefe");
						msgEmbed02.setTimeStamp(getTimeAndDate());
						msgEmbed02.setDescription(msgString);
						msgEmbed02.setTitle("__**Skipping Issue:**__");
						respond_to_input_event_data dataPackage02{ argsNew.getInputEventData() };
						dataPackage02.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
						dataPackage02.addMessageEmbed(msgEmbed02);
						input_events::respondToInputEventAsync(dataPackage02).get();
						return;
					}
				}
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "skip::execute()" << error.what() << std::endl;
			}
		}
		~skip(){};
	};
	std::unordered_map<uint64_t, uint64_t> skip::timeOfLastSkip{};
}// namespace discord_core_api
