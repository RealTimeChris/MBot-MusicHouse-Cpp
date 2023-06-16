// Play.hpp - Header for the "play" command.
// Jun 30, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class Play : public BaseFunction {
	  public:
		static std::unordered_map<uint64_t, uint64_t> timeOfLastPlay;

		Play() {
			this->commandName	  = "play";
			this->helpDescription = "Add a song to the queue and play it if nothing is playing.";
			EmbedData newEmbed;
			newEmbed.setDescription("------\nSimply enter /play SONGNAME OR Url.\n------");
			newEmbed.setTitle("__**Play Usage:**__");
			newEmbed.setTimeStamp(getTimeAndDate());
			newEmbed.setColor("FeFeFe");
			this->helpEmbed = newEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<Play>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelCacheData channel{ argsNew.getChannelData() };
				GuildData guild{ Guilds::getCachedGuild({ argsNew.getInteractionData().guildId }) };
				DiscordGuild discordGuild{ managerAgent, guild };
				bool areWeAllowed{ checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), discordGuild) };
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
				uint64_t previousPlayedTime{ 0 };
				if (Play::timeOfLastPlay.contains(argsNew.getInteractionData().guildId.operator const uint64_t&())) {
					previousPlayedTime = Play::timeOfLastPlay.at(argsNew.getInteractionData().guildId.operator const uint64_t&());
				}

				if (currentTime - previousPlayedTime < 5000) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but please wait a total of 5 seconds in between plays!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Timing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(newEvent).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}

				if (argsNew.getCommandArguments().values.size() == 0 && DiscordCoreClient::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there's already something playing!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage{ newEvent };
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}

				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();

				previousPlayedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				Play::timeOfLastPlay.insert_or_assign(argsNew.getInteractionData().guildId.operator const uint64_t&(), previousPlayedTime);
				Snowflake voiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != 0) {
					voiceChannelId = guildMember.getVoiceStateData().channelId;
				} else {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackageNew).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}
				VoiceConnection& voiceConnection = guild.connectToVoice(guildMember.user.id);
				if (!voiceConnection.areWeConnected()) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Connection Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackageNew).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}
				if (guildMember.getVoiceStateData().channelId == 0 || guildMember.getVoiceStateData().channelId != voiceConnection.getChannelId()) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackageNew).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}
				jsonifier::vector<Song> searchResults{};
				if (argsNew.getCommandArguments().values.size() > 0) {
					auto searchResultsNew = DiscordCoreClient::getSongAPI(guild.id).searchForSong(argsNew.getCommandArguments().values["songname"].value);
					if (searchResultsNew.size() > 0) {
						searchResults.emplace_back(searchResultsNew.at(0));
					}
				}

				if (searchResults.size() <= 0 && argsNew.getCommandArguments().values.size() > 0) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**No songs could be found as a result of your search!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Search Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackageNew).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}

				int32_t currentPageIndex = 0;
				MoveThroughMessagePagesData returnData{};
				if (searchResults.size() > 0) {

					DiscordGuild discordGuild{ managerAgent, GuildData{ argsNew.getInteractionData().guildId } };
					discordGuild.getDataFromDB(managerAgent);
					size_t songSize	   = discordGuild.data.playlist.songQueue.size();
					UserCacheData user = Users::getCachedUser({ .userId = guildMember.user.id });
					searchResults.at(0).addedByUserName = user.userName;
					searchResults.at(0).addedByUserId	= guildMember.user.id;
					discordGuild.data.playlist.songQueue.emplace_back(searchResults[0]);
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					jsonifier::string descriptionString{};
					discordGuild.writeDataToDB(managerAgent);
					descriptionString = "------\n__**Added the following songs to the queue:\n";
					descriptionString += "[" + searchResults.at(0).songTitle + "](" + searchResults.at(0).viewUrl + ")\n" +
						"Position: " + jsonifier::toString(discordGuild.data.playlist.songQueue.size()) + "\n";
					descriptionString += "**__\n------";
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription(descriptionString);
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing New Songs:**__");
					newEmbed->setColor("fefefe");
					RespondToInputEventData dataPackage{ newEvent };
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					returnData.currentPageIndex -= 1;
				}
				auto channelId = argsNew.getInputEventData().getChannelData().id;
				auto theTask   = [=](SongCompletionEventData eventData) mutable -> CoRoutine<void, false> {
					  auto argsNewer = std::move(argsNew);
					  co_await NewThreadAwaitable<void, false>();
					  UserCacheData userNew = Users::getCachedUser({ eventData.guildMember.user.id });
					  std::this_thread::sleep_for(150ms);
					  discordGuild.getDataFromDB(managerAgent);
					  if (discordGuild.data.playlist.areThereAnySongs()) {
						  UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
						  if (!eventData.wasItAFail) {
							  discordGuild.getDataFromDB(managerAgent);
							  discordGuild.data.playlist.sendNextSong();
							  DiscordCoreClient::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong, guildMember);
							  discordGuild.writeDataToDB(managerAgent);
							  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(UserImageTypes::Avatar));
							  newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" +
								  discordGuild.data.playlist.currentSong.viewUrl + ")" + "\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description +
								  "\n__**Duration:**__ " + discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" +
								  discordGuild.data.playlist.currentSong.addedByUserId + ">");
							  newEmbed->setImage(discordGuild.data.playlist.currentSong.thumbnailUrl);
							  newEmbed->setTimeStamp(getTimeAndDate());
							  newEmbed->setTitle("__**Now Playing:**__");
							  newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
							  if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("✅ Loop-All, ✅ Loop-Song");
							  }
							  if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("❌ Loop-All, ✅ Loop-Song");
							  }
							  if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("✅ Loop-All, ❌ Loop-Song");
							  }
							  if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("❌ Loop-All, ❌ Loop-Song");
							  }
							  CreateMessageData dataPackage02(channelId);
							  dataPackage02.addMessageEmbed(*newEmbed);
							  Messages::createMessageAsync(dataPackage02).get();
						  } else {
							  GuildMemberCacheData guildMemberNew{ eventData.guildMember };
							  discordGuild.getDataFromDB(managerAgent);
							  discordGuild.data.playlist.sendNextSong();
							  discordGuild.writeDataToDB(managerAgent);
							  DiscordCoreClient::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong, guildMember);
							  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(UserImageTypes::Avatar));
							  newEmbed->setDescription("__**It appears as though there was an error when trying to play the previous track!**__");
							  newEmbed->setTimeStamp(getTimeAndDate());
							  newEmbed->setTitle("__**Playing Error:**__");
							  newEmbed->setColor("fe0000");
							  if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("✅ Loop-All, ✅ Loop-Song");
							  }
							  if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("❌ Loop-All, ✅ Loop-Song");
							  }
							  if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("✅ Loop-All, ❌ Loop-Song");
							  }
							  if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
								  newEmbed->setFooter("❌ Loop-All, ❌ Loop-Song");
							  }
							  CreateMessageData dataPackage02(channelId);
							  dataPackage02.addMessageEmbed(*newEmbed);
							  Messages::createMessageAsync(dataPackage02).get();
						  }
						  discordGuild.writeDataToDB(managerAgent);
					  } else {
						  UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
						  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(UserImageTypes::Avatar));
						  newEmbed->setDescription("------\n__**Sorry, but there's nothing left to play here!**__\n------");
						  newEmbed->setTimeStamp(getTimeAndDate());
						  newEmbed->setTitle("__**Now Playing:**__");
						  newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
						  if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("✅ Loop-All, ✅ Loop-Song");
						  } else if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("❌ Loop-All, ✅ Loop-Song");
						  } else if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("✅ Loop-All, ❌ Loop-Song");
						  } else if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("❌ Loop-All, ❌ Loop-Song");
						  }
						  CreateMessageData dataPackage02(channelId);
						  dataPackage02.addMessageEmbed(*newEmbed);
						  Messages::createMessageAsync(dataPackage02).get();
						  DiscordCoreClient::getSongAPI(guild.id).stop();
					  }
					  co_return;
				};
				if (DiscordCoreClient::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					std::cout << "WERE CURRENTLY PLAYING!" << std::endl;
					return;
				} else {
					DiscordCoreClient::getSongAPI(guild.id).onSongCompletion(theTask);
				}
				discordGuild.getDataFromDB(managerAgent);
				if (discordGuild.data.playlist.areThereAnySongs()) {
					discordGuild.getDataFromDB(managerAgent);
					discordGuild.data.playlist.sendNextSong();
					discordGuild.writeDataToDB(managerAgent);
					DiscordCoreClient::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong, guildMember);
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl + ")" +
						"\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " + discordGuild.data.playlist.currentSong.duration +
						"\n__**Added By:**__ <@!" + discordGuild.data.playlist.currentSong.addedByUserId + ">");
					newEmbed->setImage(discordGuild.data.playlist.currentSong.thumbnailUrl);
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Now Playing:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("✅ Loop-All, ✅ Loop-Song");
					} else if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("❌ Loop-All, ✅ Loop-Song");
					} else if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("✅ Loop-All, ❌ Loop-Song");
					} else if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("❌ Loop-All, ❌ Loop-Song");
					}
					RespondToInputEventData dataPackage02(newEvent);
					dataPackage02.setResponseType(InputEventResponseType::Follow_Up_Message);
					dataPackage02.addMessageEmbed(*newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage02).get();

				} else {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there's nothing to play!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackageNew{ newEvent };
					dataPackageNew.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					dataPackageNew.addMessageEmbed(*newEmbed);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackageNew).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
				}
			} catch (const std::runtime_error& error) {
				std::cout << "Play::execute()" << error.what() << std::endl;
			}
		};
		~Play(){};
	};
	std::unordered_map<uint64_t, uint64_t> Play::timeOfLastPlay;

};// namespace DiscordCoreAPI
