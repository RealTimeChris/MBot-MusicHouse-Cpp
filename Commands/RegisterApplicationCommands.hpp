// RegisterApplicationCommands.hpp - Registers the slash commands of this bot.
// May 27, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class RegisterApplicationCommands {
	  public:
		RegisterApplicationCommands() {
			createStreamAudioData.dmPermission			   = false;
			createStreamAudioData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createStreamAudioData.type					   = ApplicationCommandType::Chat_Input;
			createStreamAudioData.name					   = "streamaudio";
			createStreamAudioData.description			   = "Connects to a voice channel to stream audio.";
			ApplicationCommandOptionData streamAudioOptionOne{};
			streamAudioOptionOne.type		 = ApplicationCommandOptionType::String;
			streamAudioOptionOne.description = "Which ip to connect to.";
			streamAudioOptionOne.name		 = "connectionip";
			streamAudioOptionOne.required	 = true;
			createStreamAudioData.options.emplace_back(streamAudioOptionOne);
			ApplicationCommandOptionData streamAudioOptionTwo{};
			streamAudioOptionTwo.type		 = ApplicationCommandOptionType::Boolean;
			streamAudioOptionTwo.description = "Whether or not to enable retransmission of other bot's audio.";
			streamAudioOptionTwo.name		 = "botaudio";
			streamAudioOptionTwo.required	 = true;
			createStreamAudioData.options.emplace_back(streamAudioOptionTwo);

			createBotInfoCommandData.dmPermission			  = true;
			createBotInfoCommandData.type					  = ApplicationCommandType::Chat_Input;
			createBotInfoCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createBotInfoCommandData.description			  = "Displays info about the current bot.";
			createBotInfoCommandData.name					  = "botinfo";

			createDisplayGuildsDataCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createDisplayGuildsDataCommandData.dmPermission				= true;
			createDisplayGuildsDataCommandData.type						= ApplicationCommandType::Chat_Input;
			createDisplayGuildsDataCommandData.description				= "View the list of servers that this bot is in.";
			createDisplayGuildsDataCommandData.name						= "displayguildsdata";

			createSeekCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createSeekCommandData.dmPermission			   = false;
			createSeekCommandData.type					   = ApplicationCommandType::Chat_Input;
			createSeekCommandData.description			   = "Seek into a currently playing song.";
			createSeekCommandData.name					   = "seek";
			ApplicationCommandOptionData seekCommandDataOptionOne;
			seekCommandDataOptionOne.name		 = "minutes";
			seekCommandDataOptionOne.type		 = ApplicationCommandOptionType::String;
			seekCommandDataOptionOne.description = "The number of minutes into the song to seek.";
			seekCommandDataOptionOne.required	 = true;
			createSeekCommandData.options.emplace_back(seekCommandDataOptionOne);
			ApplicationCommandOptionData seekCommandDataOptionTwo;
			seekCommandDataOptionTwo.name		 = "seconds";
			seekCommandDataOptionTwo.type		 = ApplicationCommandOptionType::String;
			seekCommandDataOptionTwo.description = "The number of seconds into the song to seek.";
			seekCommandDataOptionTwo.required	 = true;
			createSeekCommandData.options.emplace_back(seekCommandDataOptionTwo);

			createPlayCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createPlayCommandData.dmPermission			   = false;
			createPlayCommandData.type					   = ApplicationCommandType::Chat_Input;
			createPlayCommandData.description			   = "Search for and play a song.";
			createPlayCommandData.name					   = "play";
			ApplicationCommandOptionData playCommandDataOptionOne;
			playCommandDataOptionOne.name		 = "songname";
			playCommandDataOptionOne.type		 = ApplicationCommandOptionType::String;
			playCommandDataOptionOne.description = "The name of the song that you would like to search.";
			playCommandDataOptionOne.required	 = false;
			createPlayCommandData.options.emplace_back(playCommandDataOptionOne);

			RegisterApplicationCommandsCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			RegisterApplicationCommandsCommandData.dmPermission				= false;
			RegisterApplicationCommandsCommandData.type						= ApplicationCommandType::Chat_Input;
			RegisterApplicationCommandsCommandData.description				= "Register the programmatically designated slash commands.";
			RegisterApplicationCommandsCommandData.name						= "registerapplicationcommands";

			setMusicChannelCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			setMusicChannelCommandData.dmPermission				= false;
			setMusicChannelCommandData.type						= ApplicationCommandType::Chat_Input;
			setMusicChannelCommandData.description				= "Sets the channels which you can issue music commands in.";
			setMusicChannelCommandData.name						= "setmusicchannel";
			ApplicationCommandOptionData createSetGameChannelOptionOne;
			createSetGameChannelOptionOne.type		  = ApplicationCommandOptionType::Sub_Command;
			createSetGameChannelOptionOne.name		  = "view";
			createSetGameChannelOptionOne.description = "View the currently enabled channels.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionOne);
			ApplicationCommandOptionData createSetGameChannelOptionTwo;
			createSetGameChannelOptionTwo.type		  = ApplicationCommandOptionType::Sub_Command;
			createSetGameChannelOptionTwo.name		  = "add";
			createSetGameChannelOptionTwo.description = "Add a channel to the list of enabled channels.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionTwo);
			ApplicationCommandOptionData createSetGameChannelOptionThree;
			createSetGameChannelOptionThree.type		= ApplicationCommandOptionType::Sub_Command;
			createSetGameChannelOptionThree.name		= "remove";
			createSetGameChannelOptionThree.description = "Remove a channel from the list of enabled channels.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionThree);
			ApplicationCommandOptionData createSetGameChannelOptionFour;
			createSetGameChannelOptionFour.type		   = ApplicationCommandOptionType::Sub_Command;
			createSetGameChannelOptionFour.name		   = "purge";
			createSetGameChannelOptionFour.description = "Purges the list of channels for the server.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionFour);

			createSetBorderColorCommandData.dmPermission			 = false;
			createSetBorderColorCommandData.type					 = ApplicationCommandType::Chat_Input;
			createSetBorderColorCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createSetBorderColorCommandData.description				 = "Set the default color of borders.";
			createSetBorderColorCommandData.name					 = "setbordercolor";
			createSetBorderColorCommandData.type					 = ApplicationCommandType::Chat_Input;
			ApplicationCommandOptionData createSetBorderColorOptionTwo;
			createSetBorderColorOptionTwo.type		  = ApplicationCommandOptionType::String;
			createSetBorderColorOptionTwo.name		  = "hexcolorvalue";
			createSetBorderColorOptionTwo.required	  = true;
			createSetBorderColorOptionTwo.description = "The hex-color-value to set the borders to.";
			createSetBorderColorCommandData.options.emplace_back(createSetBorderColorOptionTwo);

			createSetCommandPrefixCommandData.dmPermission			 = false;
			createSetCommandPrefixCommandData.type					 = ApplicationCommandType::Chat_Input;
			createSetCommandPrefixCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createSetCommandPrefixCommandData.description				 = "Set the default color of borders.";
			createSetCommandPrefixCommandData.name					 = "setcommandprefix";
			createSetCommandPrefixCommandData.type					 = ApplicationCommandType::Chat_Input;
			ApplicationCommandOptionData createSetCommandPrefixOptionTwo;
			createSetCommandPrefixOptionTwo.type		  = ApplicationCommandOptionType::String;
			createSetCommandPrefixOptionTwo.name		  = "hexcolorvalue";
			createSetCommandPrefixOptionTwo.required	  = true;
			createSetCommandPrefixOptionTwo.description = "The hex-color-value to set the borders to.";
			createSetCommandPrefixCommandData.options.emplace_back(createSetCommandPrefixOptionTwo);

			createSkipData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createSkipData.dmPermission				= false;
			createSkipData.type						= ApplicationCommandType::Chat_Input;
			createSkipData.name						= "skip";
			createSkipData.description				= "Skip to the next song in the queue.";

			createStopData.dmPermission				= false;
			createStopData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createStopData.type						= ApplicationCommandType::Chat_Input;
			createStopData.name						= "stop";
			createStopData.description				= "Stops the currently playing music.";

			createLoopSongData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createLoopSongData.dmPermission				= false;
			createLoopSongData.type						= ApplicationCommandType::Chat_Input;
			createLoopSongData.name						= "loopsong";
			createLoopSongData.description				= "Set the current song to be looped over.";

			createLoopAllData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createLoopAllData.dmPermission			   = false;
			createLoopAllData.type					   = ApplicationCommandType::Chat_Input;
			createLoopAllData.name					   = "loopall";
			createLoopAllData.description			   = "Set the current discordGuild.data.playlist to be looped over.";

			createSeekCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createSeekCommandData.dmPermission			   = false;
			createSeekCommandData.description			   = "Seek into a song.";
			createSeekCommandData.name					   = "seek";
			createSeekCommandData.type					   = ApplicationCommandType::Chat_Input;
			ApplicationCommandOptionData createSeekOptionOne;
			createSeekOptionOne.type		= ApplicationCommandOptionType::String;
			createSeekOptionOne.name		= "hours";
			createSeekOptionOne.description = "The number of hours into the song to seek.";
			createSeekOptionOne.required	= false;
			ApplicationCommandOptionData createSeekOptionTwo;
			createSeekOptionTwo.type		= ApplicationCommandOptionType::String;
			createSeekOptionTwo.name		= "minutes";
			createSeekOptionTwo.description = "The number of minutes into the song to seek.";
			createSeekOptionTwo.required	= false;
			ApplicationCommandOptionData createSeekOptionThree;
			createSeekOptionThree.type		  = ApplicationCommandOptionType::String;
			createSeekOptionThree.name		  = "seconds";
			createSeekOptionThree.description = "The number of seconds into the song to seek.";
			createSeekOptionThree.required	  = false;
			createSeekCommandData.options.emplace_back(createSeekOptionOne);
			createSeekCommandData.options.emplace_back(createSeekOptionTwo);
			createSeekCommandData.options.emplace_back(createSeekOptionThree);

			createClearData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createClearData.dmPermission			 = false;
			createClearData.type					 = ApplicationCommandType::Chat_Input;
			createClearData.name					 = "clear";
			createClearData.description				 = "Clears the current song queue.";

			createDisconnectData.dmPermission			  = false;
			createDisconnectData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createDisconnectData.type					  = ApplicationCommandType::Chat_Input;
			createDisconnectData.name					  = "disconnect";
			createDisconnectData.description			  = "Disconnect from the current voice channel.";

			createTestData.dmPermission				= true;
			createTestData.type						= ApplicationCommandType::Chat_Input;
			createTestData.name						= "test";
			createTestData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createTestData.description				= "Test command.";
			ApplicationCommandOptionData createTestCommandOptionOne{};
			createTestCommandOptionOne.type		   = ApplicationCommandOptionType::String;
			createTestCommandOptionOne.name		   = "test";
			createTestCommandOptionOne.required	   = true;
			createTestCommandOptionOne.description = "The test to be entered.";
			createTestData.options.emplace_back(createTestCommandOptionOne);

			createPauseData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createPauseData.dmPermission			 = false;
			createPauseData.type					 = ApplicationCommandType::Chat_Input;
			createPauseData.name					 = "pause";
			createPauseData.description				 = "Pause the music.";

			createQueueData.dmPermission			 = false;
			createQueueData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createQueueData.type					 = ApplicationCommandType::Chat_Input;
			createQueueData.name					 = "queue";
			createQueueData.description				 = "Look at the queue and possibly edit it.";

			createHelpData.dmPermission				= true;
			createHelpData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createHelpData.type						= ApplicationCommandType::Chat_Input;
			createHelpData.name						= "help";
			createHelpData.description				= "A help command for this bot.";

			createNpData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createNpData.dmPermission			  = false;
			createNpData.type					  = ApplicationCommandType::Chat_Input;
			createNpData.name					  = "np";
			createNpData.description			  = "Displays the currently playing song.";

			createPlayRNCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createPlayRNCommandData.dmPermission			 = false;
			createPlayRNCommandData.type					 = ApplicationCommandType::Chat_Input;
			createPlayRNCommandData.description				 = "Plays a song immediately.";
			createPlayRNCommandData.name					 = "playrn";
			ApplicationCommandOptionData playRNCommandDataOptionOne;
			playRNCommandDataOptionOne.name		   = "songname";
			playRNCommandDataOptionOne.type		   = ApplicationCommandOptionType::String;
			playRNCommandDataOptionOne.description = "The name of the song that you would like to search.";
			playRNCommandDataOptionOne.required	   = false;
			createPlayRNCommandData.options.emplace_back(playRNCommandDataOptionOne);

			createPlayQCommandData.defaultMemberPermissions = Permission::Use_Application_Commands;
			createPlayQCommandData.dmPermission				= false;
			createPlayQCommandData.type						= ApplicationCommandType::Chat_Input;
			createPlayQCommandData.description				= "Plays a song from the queue immediately.";
			createPlayQCommandData.name						= "playq";
			ApplicationCommandOptionData playQCommandDataOptionOne;
			playQCommandDataOptionOne.name		  = "tracknumber";
			playQCommandDataOptionOne.type		  = ApplicationCommandOptionType::Integer;
			playQCommandDataOptionOne.minValue	  = 1;
			playQCommandDataOptionOne.maxValue	  = 10000;
			playQCommandDataOptionOne.description = "The number of the track in the current queue.";
			playQCommandDataOptionOne.required	  = true;
			createPlayQCommandData.options.emplace_back(playQCommandDataOptionOne);

			createUserInfoData.dmPermission = false;
			createUserInfoData.type			= ApplicationCommandType::Message;
			createUserInfoData.name			= "UserCacheData Info";
		}
		CreateGlobalApplicationCommandData createBotInfoCommandData{};
		CreateGlobalApplicationCommandData createDisplayGuildsDataCommandData{};
		CreateGlobalApplicationCommandData createSeekCommandData{};
		CreateGlobalApplicationCommandData createPlayCommandData{};
		CreateGlobalApplicationCommandData RegisterApplicationCommandsCommandData{};
		CreateGlobalApplicationCommandData setMusicChannelCommandData{};
		CreateGlobalApplicationCommandData createSetBorderColorCommandData{};
		CreateGlobalApplicationCommandData createSetCommandPrefixCommandData{};
		CreateGlobalApplicationCommandData createStreamAudioData{};
		CreateGlobalApplicationCommandData createSkipData{};
		CreateGlobalApplicationCommandData createStopData{};
		CreateGlobalApplicationCommandData createLoopSongData{};
		CreateGlobalApplicationCommandData createLoopAllData{};
		CreateGlobalApplicationCommandData createClearData{};
		CreateGlobalApplicationCommandData createDisconnectData{};
		CreateGlobalApplicationCommandData createTestData{};
		CreateGlobalApplicationCommandData createPauseData{};
		CreateGlobalApplicationCommandData createQueueData{};
		CreateGlobalApplicationCommandData createHelpData{};
		CreateGlobalApplicationCommandData createNpData{};
		CreateGlobalApplicationCommandData createPlayRNCommandData{};
		CreateGlobalApplicationCommandData createPlayQCommandData{};
		CreateGlobalApplicationCommandData createUserInfoData{};

		~RegisterApplicationCommands(){};
	};
}// namespace DiscordCoreAPI
