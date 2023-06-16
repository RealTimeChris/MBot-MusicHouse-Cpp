// PlayRN.hpp - Header for the "play right now" command.
// Sep 9, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class PlayRN : public BaseFunction {
	  public:
		PlayRN() {
			this->commandName	  = "playrn";
			this->helpDescription = "Add a song to the front of the queue and play it if nothing is playing.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /playrn SONGNAME OR Url.\n------");
			msgEmbed.setTitle("__**PlayRN Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<PlayRN>();
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
				uint64_t previousPlayedTime{ 0 };
				if (Play::timeOfLastPlay.contains(newEvent.getInteractionData().guildId.operator const uint64_t&())) {
					previousPlayedTime = Play::timeOfLastPlay.at(newEvent.getInteractionData().guildId.operator const uint64_t&());
				}

				if (currentTime - previousPlayedTime < 5000) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(newEvent.getUserData().userName, newEvent.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but please wait a total of 5 seconds in between plays!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Timing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage{ newEvent };
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(newEvent, 20000);
					return;
				}

				previousPlayedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				Play::timeOfLastPlay.insert_or_assign(newEvent.getInteractionData().guildId.operator const uint64_t&(), previousPlayedTime);

				RespondToInputEventData dataPackage{ newEvent };
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
				Snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != 0) {
					currentVoiceChannelId = guildMember.getVoiceStateData().channelId;
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
					newEmbed->setAuthor(newEvent.getUserData().userName, newEvent.getUserData().getUserImageUrl(UserImageTypes::Avatar));
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
					newEmbed->setAuthor(newEvent.getUserData().userName, newEvent.getUserData().getUserImageUrl(UserImageTypes::Avatar));
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

				if (argsNew.getCommandArguments().values.size() == 0 && discordGuild.data.playlist.songQueue.size() == 0) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(newEvent.getUserData().userName, newEvent.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there's nothing to play!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackageNew).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				} else if (argsNew.getCommandArguments().values.size() == 0 && DiscordCoreClient::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(newEvent.getUserData().userName, newEvent.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there's already something playing!**__\n------");
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
					for (auto& value: searchResultsNew) {
						searchResults.emplace_back(value);
					}
				}
				jsonifier::vector<EmbedData> embedsFromSearch{};
				int32_t x{ 0 };
				for (Song& value : searchResults) {
					x += 1;
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(newEvent.getUserData().userName, newEvent.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("__**Title:**__ [" + value.songTitle + "](" + value.viewUrl + ")" + "\n__**Description:**__ " + value.description +
											 "\n__**Duration:**__ " + value.duration);
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**" + jsonifier::toString(x) + " of " + jsonifier::toString(searchResults.size()) + "**__");
					newEmbed->setImage(value.thumbnailUrl);
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					embedsFromSearch.emplace_back(*newEmbed);
				}

				if (embedsFromSearch.size() > 0) {
					discordGuild.getDataFromDB(managerAgent);
					Song song			 = searchResults.at(0);
					song.addedByUserId	 = guildMember.user.id;
					song.addedByUserName = guildMember.getUserData().userName;

					jsonifier::vector<Song> songVector{};

					if (discordGuild.data.playlist.songQueue.size() > 0) {
						if (discordGuild.data.playlist.currentSong.songId != "") {
							songVector.resize(discordGuild.data.playlist.songQueue.size() + 2);
							songVector.at(1) = discordGuild.data.playlist.currentSong;
							songVector.at(0) = song;
							for (uint8_t y = 0; y < discordGuild.data.playlist.songQueue.size(); y += 1) {
								songVector.at(static_cast<uint64_t>(y) + 2) = discordGuild.data.playlist.songQueue.at(static_cast<uint64_t>(y));
							}

						} else {
							songVector.resize(discordGuild.data.playlist.songQueue.size() + 1);
							songVector.at(0) = song;

							for (uint8_t y = 0; y < discordGuild.data.playlist.songQueue.size(); y += 1) {
								songVector[static_cast<uint64_t>(y) + 1] = discordGuild.data.playlist.songQueue[static_cast<uint64_t>(y)];
							}
						}
					} else {
						if (discordGuild.data.playlist.currentSong.songId != "") {
							songVector.resize(2);
							songVector.at(1) = discordGuild.data.playlist.currentSong;
							songVector.at(0) = song;
						} else {
							songVector.resize(1);
							songVector.at(0) = song;
						}
					}

					discordGuild.data.playlist.songQueue   = songVector;
					discordGuild.data.playlist.currentSong = Song();
					discordGuild.writeDataToDB(managerAgent);
				}
				auto channelId = argsNew.getInputEventData().getChannelData().id;
				auto theTask   = [=](SongCompletionEventData eventData) mutable -> CoRoutine<void, false> {
					  auto argsNewer = std::move(argsNew);
					  co_await NewThreadAwaitable<void, false>();
					  UserCacheData userNew = Users::getCachedUser({ eventData.guildMember.user.id });
					  std::this_thread::sleep_for(150ms);
					  discordGuild.getDataFromDB(managerAgent);
					  if (discordGuild.data.playlist.songQueue.size()) {
						  UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
						  if (!eventData.wasItAFail) {
							  discordGuild.getDataFromDB(managerAgent);
							  discordGuild.data.playlist.sendNextSong();
							  DiscordCoreClient::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong, guildMember);
							  discordGuild.writeDataToDB(managerAgent);
							  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(UserImageTypes::Avatar));
							  newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl +
													   ")" + "\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " +
													   discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" +
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
							  DiscordCoreClient::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong, guildMember);
							  discordGuild.writeDataToDB(managerAgent);
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

							  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(UserImageTypes::Avatar));
							  newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl +
													   ")" + "\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " +
													   discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" +
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
							  CreateMessageData dataPackage03(channelId);
							  dataPackage03.addMessageEmbed(*newEmbed);
							  Messages::createMessageAsync(dataPackage03).get();
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
				DiscordCoreClient::getSongAPI(guild.id).onSongCompletion(theTask);
				if (discordGuild.data.playlist.songQueue.size()) {
					DiscordCoreClient::getSongAPI(guild.id).stop();
					discordGuild.getDataFromDB(managerAgent);
					discordGuild.data.playlist.sendNextSong();
					DiscordCoreClient::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong, guildMember);
					discordGuild.writeDataToDB(managerAgent);
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl + ")" +
											 "\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " +
											 discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" + discordGuild.data.playlist.currentSong.addedByUserId +
											 ">");
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
					RespondToInputEventData dataPackageNew{ newEvent };
					dataPackageNew.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackageNew.addMessageEmbed(*newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackageNew).get();
				}
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "PlayRN::execute()" << error.what() << std::endl;
			}
		};
		~PlayRN(){};
	};

};// namespace DiscordCoreAPI
