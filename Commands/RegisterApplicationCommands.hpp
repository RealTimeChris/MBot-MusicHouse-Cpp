// register_application_commands.hpp - Registers the slash commands of this bot.
// May 27, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class register_application_commands {
	  public:
		register_application_commands() {
			createStreamAudioData.dmPermission			   = false;
			createStreamAudioData.defaultMemberPermissions = permission::Use_Application_Commands;
			createStreamAudioData.type					   = application_command_type::Chat_Input;
			createStreamAudioData.name					   = "streamaudio";
			createStreamAudioData.description			   = "Connects to a voice channel to stream audio.";
			application_command_option_data streamAudioOptionOne{};
			streamAudioOptionOne.type		 = application_command_option_type::String;
			streamAudioOptionOne.description = "Which ip to connect to.";
			streamAudioOptionOne.name		 = "connectionip";
			streamAudioOptionOne.required	 = true;
			createStreamAudioData.options.emplace_back(streamAudioOptionOne);
			application_command_option_data streamAudioOptionTwo{};
			streamAudioOptionTwo.type		 = application_command_option_type::Boolean;
			streamAudioOptionTwo.description = "Whether or not to enable retransmission of other bot's audio.";
			streamAudioOptionTwo.name		 = "botaudio";
			streamAudioOptionTwo.required	 = true;
			createStreamAudioData.options.emplace_back(streamAudioOptionTwo);

			createBotInfoCommandData.dmPermission			  = true;
			createBotInfoCommandData.type					  = application_command_type::Chat_Input;
			createBotInfoCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createBotInfoCommandData.description			  = "Displays info about the current bot.";
			createBotInfoCommandData.name					  = "botinfo";

			createDisplayGuildsDataCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createDisplayGuildsDataCommandData.dmPermission				= true;
			createDisplayGuildsDataCommandData.type						= application_command_type::Chat_Input;
			createDisplayGuildsDataCommandData.description				= "View the list of servers that this bot is in.";
			createDisplayGuildsDataCommandData.name						= "displayguildsdata";

			createSeekCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createSeekCommandData.dmPermission			   = false;
			createSeekCommandData.type					   = application_command_type::Chat_Input;
			createSeekCommandData.description			   = "Seek into a currently playing song.";
			createSeekCommandData.name					   = "seek";
			application_command_option_data seekCommandDataOptionOne;
			seekCommandDataOptionOne.name		 = "minutes";
			seekCommandDataOptionOne.type		 = application_command_option_type::String;
			seekCommandDataOptionOne.description = "The number of minutes into the song to seek.";
			seekCommandDataOptionOne.required	 = true;
			createSeekCommandData.options.emplace_back(seekCommandDataOptionOne);
			application_command_option_data seekCommandDataOptionTwo;
			seekCommandDataOptionTwo.name		 = "seconds";
			seekCommandDataOptionTwo.type		 = application_command_option_type::String;
			seekCommandDataOptionTwo.description = "The number of seconds into the song to seek.";
			seekCommandDataOptionTwo.required	 = true;
			createSeekCommandData.options.emplace_back(seekCommandDataOptionTwo);

			createPlayCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createPlayCommandData.dmPermission			   = false;
			createPlayCommandData.type					   = application_command_type::Chat_Input;
			createPlayCommandData.description			   = "Search for and play a song.";
			createPlayCommandData.name					   = "play";
			application_command_option_data playCommandDataOptionOne;
			playCommandDataOptionOne.name		 = "songname";
			playCommandDataOptionOne.type		 = application_command_option_type::String;
			playCommandDataOptionOne.description = "The name of the song that you would like to search.";
			playCommandDataOptionOne.required	 = false;
			createPlayCommandData.options.emplace_back(playCommandDataOptionOne);

			createPlaySearchCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createPlaySearchCommandData.dmPermission			   = false;
			createPlaySearchCommandData.type					   = application_command_type::Chat_Input;
			createPlaySearchCommandData.description			   = "Search for and play a song.";
			createPlaySearchCommandData.name					   = "playsearch";
			application_command_option_data playSearchCommandDataOptionOne;
			playSearchCommandDataOptionOne.name		 = "songname";
			playSearchCommandDataOptionOne.type		 = application_command_option_type::String;
			playSearchCommandDataOptionOne.description = "The name of the song that you would like to search.";
			playSearchCommandDataOptionOne.required	 = false;
			createPlaySearchCommandData.options.emplace_back(playSearchCommandDataOptionOne);

			RegisterApplicationCommandsCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			RegisterApplicationCommandsCommandData.dmPermission				= false;
			RegisterApplicationCommandsCommandData.type						= application_command_type::Chat_Input;
			RegisterApplicationCommandsCommandData.description				= "Register the programmatically designated slash commands.";
			RegisterApplicationCommandsCommandData.name						= "registerapplicationcommands";

			setMusicChannelCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			setMusicChannelCommandData.dmPermission				= false;
			setMusicChannelCommandData.type						= application_command_type::Chat_Input;
			setMusicChannelCommandData.description				= "Sets the channels which you can issue music commands in.";
			setMusicChannelCommandData.name						= "setmusicchannel";
			application_command_option_data createSetGameChannelOptionOne;
			createSetGameChannelOptionOne.type		  = application_command_option_type::Sub_Command;
			createSetGameChannelOptionOne.name		  = "view";
			createSetGameChannelOptionOne.description = "View the currently enabled channels.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionOne);
			application_command_option_data createSetGameChannelOptionTwo;
			createSetGameChannelOptionTwo.type		  = application_command_option_type::Sub_Command;
			createSetGameChannelOptionTwo.name		  = "add";
			createSetGameChannelOptionTwo.description = "Add a channel to the list of enabled channels.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionTwo);
			application_command_option_data createSetGameChannelOptionThree;
			createSetGameChannelOptionThree.type		= application_command_option_type::Sub_Command;
			createSetGameChannelOptionThree.name		= "remove";
			createSetGameChannelOptionThree.description = "Remove a channel from the list of enabled channels.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionThree);
			application_command_option_data createSetGameChannelOptionFour;
			createSetGameChannelOptionFour.type		   = application_command_option_type::Sub_Command;
			createSetGameChannelOptionFour.name		   = "purge";
			createSetGameChannelOptionFour.description = "Purges the list of channels for the server.";
			setMusicChannelCommandData.options.emplace_back(createSetGameChannelOptionFour);

			createSetBorderColorCommandData.dmPermission			 = false;
			createSetBorderColorCommandData.type					 = application_command_type::Chat_Input;
			createSetBorderColorCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createSetBorderColorCommandData.description				 = "Set the default color of borders.";
			createSetBorderColorCommandData.name					 = "setbordercolor";
			createSetBorderColorCommandData.type					 = application_command_type::Chat_Input;
			application_command_option_data createSetBorderColorOptionTwo;
			createSetBorderColorOptionTwo.type		  = application_command_option_type::String;
			createSetBorderColorOptionTwo.name		  = "hexcolorvalue";
			createSetBorderColorOptionTwo.required	  = true;
			createSetBorderColorOptionTwo.description = "The hex-color-value to set the borders to.";
			createSetBorderColorCommandData.options.emplace_back(createSetBorderColorOptionTwo);

			createSetCommandPrefixCommandData.dmPermission			 = false;
			createSetCommandPrefixCommandData.type					 = application_command_type::Chat_Input;
			createSetCommandPrefixCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createSetCommandPrefixCommandData.description				 = "Set the default color of borders.";
			createSetCommandPrefixCommandData.name					 = "setcommandprefix";
			createSetCommandPrefixCommandData.type					 = application_command_type::Chat_Input;
			application_command_option_data createSetCommandPrefixOptionTwo;
			createSetCommandPrefixOptionTwo.type		  = application_command_option_type::String;
			createSetCommandPrefixOptionTwo.name		  = "hexcolorvalue";
			createSetCommandPrefixOptionTwo.required	  = true;
			createSetCommandPrefixOptionTwo.description = "The hex-color-value to set the borders to.";
			createSetCommandPrefixCommandData.options.emplace_back(createSetCommandPrefixOptionTwo);

			createSkipData.defaultMemberPermissions = permission::Use_Application_Commands;
			createSkipData.dmPermission				= false;
			createSkipData.type						= application_command_type::Chat_Input;
			createSkipData.name						= "skip";
			createSkipData.description				= "Skip to the next song in the queue.";

			createStopData.dmPermission				= false;
			createStopData.defaultMemberPermissions = permission::Use_Application_Commands;
			createStopData.type						= application_command_type::Chat_Input;
			createStopData.name						= "stop";
			createStopData.description				= "Stops the currently playing music.";

			createLoopSongData.defaultMemberPermissions = permission::Use_Application_Commands;
			createLoopSongData.dmPermission				= false;
			createLoopSongData.type						= application_command_type::Chat_Input;
			createLoopSongData.name						= "loopsong";
			createLoopSongData.description				= "Set the current song to be looped over.";

			createLoopAllData.defaultMemberPermissions = permission::Use_Application_Commands;
			createLoopAllData.dmPermission			   = false;
			createLoopAllData.type					   = application_command_type::Chat_Input;
			createLoopAllData.name					   = "loopall";
			createLoopAllData.description			   = "Set the current discordGuild.data.playlist to be looped over.";

			createSeekCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createSeekCommandData.dmPermission			   = false;
			createSeekCommandData.description			   = "Seek into a song.";
			createSeekCommandData.name					   = "seek";
			createSeekCommandData.type					   = application_command_type::Chat_Input;
			application_command_option_data createSeekOptionOne;
			createSeekOptionOne.type		= application_command_option_type::String;
			createSeekOptionOne.name		= "hours";
			createSeekOptionOne.description = "The number of hours into the song to seek.";
			createSeekOptionOne.required	= false;
			application_command_option_data createSeekOptionTwo;
			createSeekOptionTwo.type		= application_command_option_type::String;
			createSeekOptionTwo.name		= "minutes";
			createSeekOptionTwo.description = "The number of minutes into the song to seek.";
			createSeekOptionTwo.required	= false;
			application_command_option_data createSeekOptionThree;
			createSeekOptionThree.type		  = application_command_option_type::String;
			createSeekOptionThree.name		  = "seconds";
			createSeekOptionThree.description = "The number of seconds into the song to seek.";
			createSeekOptionThree.required	  = false;
			createSeekCommandData.options.emplace_back(createSeekOptionOne);
			createSeekCommandData.options.emplace_back(createSeekOptionTwo);
			createSeekCommandData.options.emplace_back(createSeekOptionThree);

			createClearData.defaultMemberPermissions = permission::Use_Application_Commands;
			createClearData.dmPermission			 = false;
			createClearData.type					 = application_command_type::Chat_Input;
			createClearData.name					 = "clear";
			createClearData.description				 = "Clears the current song queue.";

			createDisconnectData.dmPermission			  = false;
			createDisconnectData.defaultMemberPermissions = permission::Use_Application_Commands;
			createDisconnectData.type					  = application_command_type::Chat_Input;
			createDisconnectData.name					  = "disconnect";
			createDisconnectData.description			  = "Disconnect from the current voice channel.";

			createTestData.dmPermission				= true;
			createTestData.type						= application_command_type::Chat_Input;
			createTestData.name						= "test";
			createTestData.defaultMemberPermissions = permission::Use_Application_Commands;
			createTestData.description				= "Test command.";
			application_command_option_data createTestCommandOptionOne{};
			createTestCommandOptionOne.type		   = application_command_option_type::String;
			createTestCommandOptionOne.name		   = "test";
			createTestCommandOptionOne.required	   = true;
			createTestCommandOptionOne.description = "The test to be entered.";
			createTestData.options.emplace_back(createTestCommandOptionOne);

			createPauseData.defaultMemberPermissions = permission::Use_Application_Commands;
			createPauseData.dmPermission			 = false;
			createPauseData.type					 = application_command_type::Chat_Input;
			createPauseData.name					 = "pause";
			createPauseData.description				 = "Pause the music.";

			createQueueData.dmPermission			 = false;
			createQueueData.defaultMemberPermissions = permission::Use_Application_Commands;
			createQueueData.type					 = application_command_type::Chat_Input;
			createQueueData.name					 = "queue";
			createQueueData.description				 = "Look at the queue and possibly edit it.";

			createHelpData.dmPermission				= true;
			createHelpData.defaultMemberPermissions = permission::Use_Application_Commands;
			createHelpData.type						= application_command_type::Chat_Input;
			createHelpData.name						= "help";
			createHelpData.description				= "A help command for this bot.";

			createNpData.defaultMemberPermissions = permission::Use_Application_Commands;
			createNpData.dmPermission			  = false;
			createNpData.type					  = application_command_type::Chat_Input;
			createNpData.name					  = "np";
			createNpData.description			  = "Displays the currently playing song.";

			createPlayRNCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createPlayRNCommandData.dmPermission			 = false;
			createPlayRNCommandData.type					 = application_command_type::Chat_Input;
			createPlayRNCommandData.description				 = "Plays a song immediately.";
			createPlayRNCommandData.name					 = "playrn";
			application_command_option_data playRNCommandDataOptionOne;
			playRNCommandDataOptionOne.name		   = "songname";
			playRNCommandDataOptionOne.type		   = application_command_option_type::String;
			playRNCommandDataOptionOne.description = "The name of the song that you would like to search.";
			playRNCommandDataOptionOne.required	   = false;
			createPlayRNCommandData.options.emplace_back(playRNCommandDataOptionOne);

			createPlayQCommandData.defaultMemberPermissions = permission::Use_Application_Commands;
			createPlayQCommandData.dmPermission				= false;
			createPlayQCommandData.type						= application_command_type::Chat_Input;
			createPlayQCommandData.description				= "Plays a song from the queue immediately.";
			createPlayQCommandData.name						= "playq";
			application_command_option_data playQCommandDataOptionOne;
			playQCommandDataOptionOne.name		  = "tracknumber";
			playQCommandDataOptionOne.type		  = application_command_option_type::Integer;
			playQCommandDataOptionOne.minValue	  = 1;
			playQCommandDataOptionOne.maxValue	  = 10000;
			playQCommandDataOptionOne.description = "The number of the track in the current queue.";
			playQCommandDataOptionOne.required	  = true;
			createPlayQCommandData.options.emplace_back(playQCommandDataOptionOne);

			createUserInfoData.dmPermission = false;
			createUserInfoData.type			= application_command_type::Message;
			createUserInfoData.name			= "UserCacheData Info";
		}
		create_global_application_command_data createBotInfoCommandData{};
		create_global_application_command_data createDisplayGuildsDataCommandData{};
		create_global_application_command_data createSeekCommandData{};
		create_global_application_command_data createPlayCommandData{};
		create_global_application_command_data createPlaySearchCommandData{};
		create_global_application_command_data RegisterApplicationCommandsCommandData{};
		create_global_application_command_data setMusicChannelCommandData{};
		create_global_application_command_data createSetBorderColorCommandData{};
		create_global_application_command_data createSetCommandPrefixCommandData{};
		create_global_application_command_data createStreamAudioData{};
		create_global_application_command_data createSkipData{};
		create_global_application_command_data createStopData{};
		create_global_application_command_data createLoopSongData{};
		create_global_application_command_data createLoopAllData{};
		create_global_application_command_data createClearData{};
		create_global_application_command_data createDisconnectData{};
		create_global_application_command_data createTestData{};
		create_global_application_command_data createPauseData{};
		create_global_application_command_data createQueueData{};
		create_global_application_command_data createHelpData{};
		create_global_application_command_data createNpData{};
		create_global_application_command_data createPlayRNCommandData{};
		create_global_application_command_data createPlayQCommandData{};
		create_global_application_command_data createUserInfoData{};

		~register_application_commands(){};
	};
}// namespace discord_core_api
