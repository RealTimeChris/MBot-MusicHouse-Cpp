// Test.hpp - Header for the "test" command.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {


	std::vector<std::string> redNumbers{ ":red_square:32", ":red_square:19", ":red_square:21", ":red_square:25", ":red_square:34", ":red_square:27", ":red_square:36",
		":red_square:30", ":red_square:23", ":red_square:5", ":red_square:16", ":red_square:1", ":red_square:14", ":red_square:9", ":red_square:18", ":red_square:7",
		":red_square:12", ":red_square:3" };
	std::vector<std::string> blackNumbers{ ":black_large_square:15", ":black_large_square:4", ":black_large_square:2", ":black_large_square:17", ":black_large_square:6",
		":black_large_square:13", ":black_large_square:11", ":black_large_square:8", ":black_large_square:10", ":black_large_square:24", ":black_large_square:33",
		":black_large_square:20", ":black_large_square:31", ":black_large_square:22", ":black_large_square:29", ":black_large_square:28", ":black_large_square:35",
		":black_large_square:26" };


	class Test : public BaseFunction {
	  public:
		Test() {
			this->commandName = "test";
			this->helpDescription = "Testing purposes!";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /test!\n------");
			msgEmbed.setTitle("__**Test Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<Test>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				GuildMember guildMember =
					GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = argsNew.eventData.getAuthorId(), .guildId = argsNew.eventData.getGuildId() }).get();
				Guild guild = Guilds::getCachedGuildAsync({ .guildId = argsNew.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };
				VoiceStateData voiceStateData{};
				Channel channel = Channels::getCachedChannelAsync({ .channelId = argsNew.eventData.getChannelId() }).get();
				bool areTheyACommander = doWeHaveAdminPermissions(argsNew, argsNew.eventData, discordGuild, channel, guildMember);

				if (!areTheyACommander) {
					return;
				}

				if (guild.voiceStates.contains(guildMember.id)) {
					voiceStateData = guild.voiceStates.at(guildMember.id);
					std::unique_ptr<DiscordCoreAPI::EmbedData> newEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					newEmbed->setAuthor(argsNew.eventData.getUserName(), argsNew.eventData.getAvatarUrl());
					newEmbed->setDescription("------\n__**Enjoy!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Joining Now:**__");
					newEmbed->setColor(discordGuild.data.borderColor);
					RespondToInputEventData dataPackage(argsNew.eventData);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
				} else {
					std::unique_ptr<DiscordCoreAPI::EmbedData> newEmbed{ std::make_unique<DiscordCoreAPI::EmbedData>() };
					newEmbed->setAuthor(argsNew.eventData.getUserName(), argsNew.eventData.getAvatarUrl());
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(discordGuild.data.borderColor);
					RespondToInputEventData dataPackage(argsNew.eventData);
					dataPackage.setResponseType(InputEventResponseType::Follow_Up_Message);
					dataPackage.addMessageEmbed(*newEmbed);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					newerEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(argsNew.eventData).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}
				UpdateVoiceStateData theData{};
				theData.channelId = guild.voiceStates[guildMember.id].channelId;
				theData.guildId = guild.id;
				for (uint32_t x = 0; x < 100; x += 1) {
					theData.channelId = guild.voiceStates[guildMember.id].channelId;
					argsNew.discordCoreClient->getBotUser().updateVoiceStatus(theData);
					std::this_thread::sleep_for(250ms);
					theData.channelId = 0;
					argsNew.discordCoreClient->getBotUser().updateVoiceStatus(theData);
					std::this_thread::sleep_for(250ms);
				}


				return;
			} catch (...) {
				reportException("Test::execute()");
			}
		}
		~Test(){};
	};
}
