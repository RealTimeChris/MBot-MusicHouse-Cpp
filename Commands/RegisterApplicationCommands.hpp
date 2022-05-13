// RegisterApplicationCommands.hpp - Registers the slash commands of this bot.
// May 27, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class RegisterApplicationCommands : public BaseFunction {
	  public:
		RegisterApplicationCommands() {
			this->commandName = "registerapplicationcommands";
			this->helpDescription = "Register some application commands.";
			EmbedData msgEmbed;
			msgEmbed.setDescription("------\nSimply enter /RegisterApplicationCommandsn------");
			msgEmbed.setTitle("__**Register Slash Commands Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<RegisterApplicationCommands>();
		}

		virtual void execute(BaseFunctionArguments& newArgs) {
			try {
				InputEvents::deleteInputEventResponseAsync(newArgs.eventData);
				RespondToInputEventData dataPackage(newArgs.eventData);
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
				Guild guild = Guilds::getCachedGuildAsync({ .guildId = newArgs.eventData.getGuildId() }).get();
				DiscordGuild discordGuild{ guild };

				CreateGlobalApplicationCommandData createBotInfoCommandData{};
				createBotInfoCommandData.dmPermission = true;
				createBotInfoCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createBotInfoCommandData.type = ApplicationCommandType::Chat_Input;
				createBotInfoCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createBotInfoCommandData.description = "Displays info about the current bot.";
				createBotInfoCommandData.name = "botinfo";
				ApplicationCommands::createGlobalApplicationCommandAsync(createBotInfoCommandData);

				CreateGlobalApplicationCommandData createDisplayGuildsDataCommandData{};
				createDisplayGuildsDataCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createDisplayGuildsDataCommandData.dmPermission = true;
				createDisplayGuildsDataCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createDisplayGuildsDataCommandData.type = ApplicationCommandType::Chat_Input;
				createDisplayGuildsDataCommandData.description = "View the list of servers that this bot is in.";
				createDisplayGuildsDataCommandData.name = "displayguildsdata";
				ApplicationCommands::createGlobalApplicationCommandAsync(createDisplayGuildsDataCommandData);

				CreateGlobalApplicationCommandData playCommandData{};
				playCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				playCommandData.dmPermission = false;
				playCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				playCommandData.type = ApplicationCommandType::Chat_Input;
				playCommandData.description = "Search for and play a song.";
				playCommandData.name = "play";
				ApplicationCommandOptionData playCommandDataOptionOne;
				playCommandDataOptionOne.name = "songname";
				playCommandDataOptionOne.type = ApplicationCommandOptionType::String;
				playCommandDataOptionOne.description = "The name of the song that you would like to search.";
				playCommandDataOptionOne.required = false;
				playCommandData.options.push_back(playCommandDataOptionOne);
				ApplicationCommands::createGlobalApplicationCommandAsync(playCommandData);

				CreateGlobalApplicationCommandData RegisterApplicationCommandsCommandData{};
				RegisterApplicationCommandsCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				RegisterApplicationCommandsCommandData.dmPermission = false;
				RegisterApplicationCommandsCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				RegisterApplicationCommandsCommandData.type = ApplicationCommandType::Chat_Input;
				RegisterApplicationCommandsCommandData.description = "Register the programmatically designated slash commands.";
				RegisterApplicationCommandsCommandData.name = "registerapplicationcommands";
				ApplicationCommands::createGlobalApplicationCommandAsync(RegisterApplicationCommandsCommandData);

				CreateGlobalApplicationCommandData createSetGameChannelCommandData{};
				createSetGameChannelCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createSetGameChannelCommandData.dmPermission = false;
				createSetGameChannelCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createSetGameChannelCommandData.type = ApplicationCommandType::Chat_Input;
				createSetGameChannelCommandData.description = "Sets the channels which you can issue music commands in.";
				createSetGameChannelCommandData.name = "setmusicchannel";
				ApplicationCommandOptionData createSetGameChannelOptionOne;
				createSetGameChannelOptionOne.type = ApplicationCommandOptionType::Sub_Command;
				createSetGameChannelOptionOne.name = "view";
				createSetGameChannelOptionOne.description = "View the currently enabled channels.";
				createSetGameChannelCommandData.options.push_back(createSetGameChannelOptionOne);
				ApplicationCommandOptionData createSetGameChannelOptionTwo;
				createSetGameChannelOptionTwo.type = ApplicationCommandOptionType::Sub_Command;
				createSetGameChannelOptionTwo.name = "add";
				createSetGameChannelOptionTwo.description = "Add a channel to the list of enabled channels.";
				createSetGameChannelCommandData.options.push_back(createSetGameChannelOptionTwo);
				ApplicationCommandOptionData createSetGameChannelOptionThree;
				createSetGameChannelOptionThree.type = ApplicationCommandOptionType::Sub_Command;
				createSetGameChannelOptionThree.name = "remove";
				createSetGameChannelOptionThree.description = "Remove a channel from the list of enabled channels.";
				createSetGameChannelCommandData.options.push_back(createSetGameChannelOptionThree);
				ApplicationCommandOptionData createSetGameChannelOptionFour;
				createSetGameChannelOptionFour.type = ApplicationCommandOptionType::Sub_Command;
				createSetGameChannelOptionFour.name = "purge";
				createSetGameChannelOptionFour.description = "Purges the list of channels for the server.";
				createSetGameChannelCommandData.options.push_back(createSetGameChannelOptionFour);
				ApplicationCommands::createGlobalApplicationCommandAsync(createSetGameChannelCommandData);

				CreateGlobalApplicationCommandData createSetBorderColorCommandData{};
				createSetBorderColorCommandData.dmPermission = false;
				createSetBorderColorCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createSetBorderColorCommandData.type = ApplicationCommandType::Chat_Input;
				createSetBorderColorCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createSetBorderColorCommandData.description = "Set the default color of borders.";
				createSetBorderColorCommandData.name = "setbordercolor";
				createSetBorderColorCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createSetBorderColorOptionTwo;
				createSetBorderColorOptionTwo.type = ApplicationCommandOptionType::String;
				createSetBorderColorOptionTwo.name = "hexcolorvalue";
				createSetBorderColorOptionTwo.required = true;
				createSetBorderColorOptionTwo.description = "The hex-color-value to set the borders to.";
				createSetBorderColorCommandData.options.push_back(createSetBorderColorOptionTwo);
				ApplicationCommands::createGlobalApplicationCommandAsync(createSetBorderColorCommandData);

				CreateGlobalApplicationCommandData createSkipData{};
				createSkipData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createSkipData.dmPermission = false;
				createSkipData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createSkipData.type = ApplicationCommandType::Chat_Input;
				createSkipData.name = "skip";
				createSkipData.description = "Skip to the next song in the queue.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createSkipData);

				CreateGlobalApplicationCommandData createStopData{};
				createStopData.dmPermission = false;
				createStopData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createStopData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createStopData.type = ApplicationCommandType::Chat_Input;
				createStopData.name = "stop";
				createStopData.description = "Stops the currently playing music.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createStopData);

				CreateGlobalApplicationCommandData createLoopSongData{};
				createLoopSongData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createLoopSongData.dmPermission = false;
				createLoopSongData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createLoopSongData.type = ApplicationCommandType::Chat_Input;
				createLoopSongData.name = "loopsong";
				createLoopSongData.description = "Set the current song to be looped over.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createLoopSongData);

				CreateGlobalApplicationCommandData createLoopAllData{};
				createLoopAllData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createLoopAllData.dmPermission = false;
				createLoopAllData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createLoopAllData.type = ApplicationCommandType::Chat_Input;
				createLoopAllData.name = "loopall";
				createLoopAllData.description = "Set the current playlist to be looped over.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createLoopAllData);

				CreateGlobalApplicationCommandData createSeekCommandData{};
				createSeekCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createSeekCommandData.dmPermission = false;
				createSeekCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createSeekCommandData.description = "Seek into a song.";
				createSeekCommandData.name = "seek";
				createSeekCommandData.type = ApplicationCommandType::Chat_Input;
				ApplicationCommandOptionData createSeekOptionOne;
				createSeekOptionOne.type = ApplicationCommandOptionType::String;
				createSeekOptionOne.name = "hours";
				createSeekOptionOne.description = "The number of hours into the song to seek.";
				createSeekOptionOne.required = false;
				ApplicationCommandOptionData createSeekOptionTwo;
				createSeekOptionTwo.type = ApplicationCommandOptionType::String;
				createSeekOptionTwo.name = "minutes";
				createSeekOptionTwo.description = "The number of minutes into the song to seek.";
				createSeekOptionTwo.required = false;
				ApplicationCommandOptionData createSeekOptionThree;
				createSeekOptionThree.type = ApplicationCommandOptionType::String;
				createSeekOptionThree.name = "seconds";
				createSeekOptionThree.description = "The number of seconds into the song to seek.";
				createSeekOptionThree.required = false;
				createSeekCommandData.options.push_back(createSeekOptionOne);
				createSeekCommandData.options.push_back(createSeekOptionTwo);
				createSeekCommandData.options.push_back(createSeekOptionThree);
				ApplicationCommands::createGlobalApplicationCommandAsync(createSeekCommandData);

				CreateGlobalApplicationCommandData createClearData{};
				createClearData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createClearData.dmPermission = false;
				createClearData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createClearData.type = ApplicationCommandType::Chat_Input;
				createClearData.name = "clear";
				createClearData.description = "Clears the current song queue.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createClearData);

				CreateGlobalApplicationCommandData createEditQueueData{};
				createEditQueueData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createEditQueueData.dmPermission = false;
				createEditQueueData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createEditQueueData.type = ApplicationCommandType::Chat_Input;
				createEditQueueData.name = "editqueue";
				createEditQueueData.description = "Edit the current server's queue of songs.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createEditQueueData);

				CreateGlobalApplicationCommandData createDisconnectData{};
				createDisconnectData.dmPermission = false;
				createDisconnectData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createDisconnectData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createDisconnectData.type = ApplicationCommandType::Chat_Input;
				createDisconnectData.name = "disconnect";
				createDisconnectData.description = "Disconnect from the current voice channel.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createDisconnectData);

				CreateGlobalApplicationCommandData createTestData{};
				createTestData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createTestData.dmPermission = true;
				createTestData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createTestData.type = ApplicationCommandType::Chat_Input;
				createTestData.name = "test";
				createTestData.description = "Test command.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createTestData);

				CreateGlobalApplicationCommandData createPauseData{};
				createPauseData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createPauseData.dmPermission = false;
				createPauseData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createPauseData.type = ApplicationCommandType::Chat_Input;
				createPauseData.name = "pause";
				createPauseData.description = "Pause the music.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createPauseData);

				CreateGlobalApplicationCommandData createQueueData{};
				createQueueData.dmPermission = false;
				createQueueData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createQueueData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createQueueData.type = ApplicationCommandType::Chat_Input;
				createQueueData.name = "queue";
				createQueueData.description = "Look at the queue and possibly edit it.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createQueueData);

				CreateGlobalApplicationCommandData createHelpData{};
				createHelpData.dmPermission = true;
				createHelpData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createHelpData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createHelpData.type = ApplicationCommandType::Chat_Input;
				createHelpData.name = "help";
				createHelpData.description = "A help command for this bot.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createHelpData);

				CreateGlobalApplicationCommandData createNpData{};
				createNpData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				createNpData.dmPermission = false;
				createNpData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				createNpData.type = ApplicationCommandType::Chat_Input;
				createNpData.name = "np";
				createNpData.description = "Displays the currently playing song.";
				ApplicationCommands::createGlobalApplicationCommandAsync(createNpData);

				CreateGlobalApplicationCommandData playRNCommandData{};
				playRNCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				playRNCommandData.dmPermission = false;
				playRNCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				playRNCommandData.type = ApplicationCommandType::Chat_Input;
				playRNCommandData.description = "Plays a song immediately.";
				playRNCommandData.name = "playrn";
				ApplicationCommandOptionData playRNCommandDataOptionOne;
				playRNCommandDataOptionOne.name = "songname";
				playRNCommandDataOptionOne.type = ApplicationCommandOptionType::String;
				playRNCommandDataOptionOne.description = "The name of the song that you would like to search.";
				playRNCommandDataOptionOne.required = false;
				playRNCommandData.options.push_back(playRNCommandDataOptionOne);
				ApplicationCommands::createGlobalApplicationCommandAsync(playRNCommandData);

				CreateGlobalApplicationCommandData playQCommandData{};
				playQCommandData.defaultMemberPermissions = std::to_string(static_cast<int64_t>(Permission::Use_Application_Commands));
				playQCommandData.dmPermission = false;
				playQCommandData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				playQCommandData.type = ApplicationCommandType::Chat_Input;
				playQCommandData.description = "Plays a song from the queue immediately.";
				playQCommandData.name = "playq";
				ApplicationCommandOptionData playQCommandDataOptionOne;
				playQCommandDataOptionOne.name = "tracknumber";
				playQCommandDataOptionOne.type = ApplicationCommandOptionType::Integer;
				playQCommandDataOptionOne.minValue = 1;
				playQCommandDataOptionOne.maxValue = 10000;
				playQCommandDataOptionOne.description = "The number of the track in the current queue.";
				playQCommandDataOptionOne.required = true;
				playQCommandData.options.push_back(playQCommandDataOptionOne);
				ApplicationCommands::createGlobalApplicationCommandAsync(playQCommandData);

				CreateGlobalApplicationCommandData userInfoData{};
				userInfoData.dmPermission = false;
				userInfoData.applicationId = newArgs.discordCoreClient->getBotUser().id;
				userInfoData.type = ApplicationCommandType::Message;
				userInfoData.name = "User Info";
				ApplicationCommands::createGlobalApplicationCommandAsync(userInfoData);

				EmbedData msgEmbed{};
				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.setColor("FeFeFe");
				msgEmbed.setDescription("------\nNicely done, you've registered some commands!\n------");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**Register Application Commands Complete:**__");
				RespondToInputEventData responseData(newEvent);
				responseData.setResponseType(InputEventResponseType::Edit_Interaction_Response);
				responseData.addMessageEmbed(msgEmbed);
				auto event = InputEvents::respondToInputEventAsync(responseData).get();
				return;
			} catch (...) {
				reportException("RegisterApplicationCommands::execute()");
			}
		}
		virtual ~RegisterApplicationCommands(){};
	};
}
