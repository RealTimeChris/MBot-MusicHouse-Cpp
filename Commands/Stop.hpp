// stop.hpp - header for the "stop" command.
// aug 19, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class stop : public base_function {
	  public:
		stop() {
			this->commandName	  = "stop";
			this->helpDescription = "Stops the current song from playing.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /stop!\n------");
			msgEmbed.setTitle("__**Stop Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<stop>();
		}
		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ guild };

				bool checkIfAllowedInChannel = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild);

				if (!checkIfAllowedInChannel) {
					return;
				}

				guild_member_data guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}

				input_event_data newEvent = argsNew.getInputEventData();

				int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				int64_t previousPlayedTime{ 0 };
				if (play::timeOfLastPlay.contains(argsNew.getInteractionData().guildId.operator const uint64_t&())) {
					previousPlayedTime = play::timeOfLastPlay.at(argsNew.getInteractionData().guildId.operator const uint64_t&());
				}

				if (currentTime - previousPlayedTime < 5000) {
					embed_data newEmbed{};
					newEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed.setDescription("------\n__**Sorry, but please wait a total of 5 seconds after playing, before attempting to stop!**__\n------");
					newEmbed.setTimeStamp(getTimeAndDate());
					newEmbed.setTitle("__**Timing Issue:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != 0) {
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
				discordGuild.getDataFromDB();
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
					newEmbed.setTitle("__**Stopping Issue:**__");
					newEmbed.setColor("fefefe");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				if (!guild.areWeConnected() || !discord_core_client::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					jsonifier::string msgString = "------\n**there's no music playing to be stopped!**\n------";
					embed_data msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					msgEmbed.setColor("fefefe");
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Stopping Issue:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				jsonifier::vector<song> newVector{};
				if (discordGuild.data.playlist.currentSong.songId != "") {
					newVector.emplace_back(discordGuild.data.playlist.currentSong);
				}
				discordGuild.data.playlist.currentSong = song{};
				for (auto& value : discordGuild.data.playlist.songQueue) {
					newVector.emplace_back(value);
				}
				discordGuild.data.playlist.songQueue = newVector;
				discordGuild.writeDataToDB();
				discord_core_client::getSongAPI(guild.id).stop();
				embed_data msgEmbed{};
				msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				msgEmbed.setDescription("\n------\n__**Songs remaining in queue:**__ " + jsonifier::toString(discordGuild.data.playlist.songQueue.size()) + "\n------");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Stopping Playback:**__");
				respond_to_input_event_data dataPackage02{ argsNew.getInputEventData() };
				dataPackage02.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
				dataPackage02.addMessageEmbed(msgEmbed);
				input_events::respondToInputEventAsync(dataPackage02).get();
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "stop::execute()" << error.what() << std::endl;
			}
		}
		~stop(){};
	};

}// namespace discord_core_api
