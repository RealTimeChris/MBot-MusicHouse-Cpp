// PlayQ.hpp - Header for the "play right now" command.
// Sep 9, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class PlayQ : public BaseFunction {
	  public:
		static std::unordered_map<uint64_t, uint64_t> timeOfLastPlay;

		PlayQ() {
			this->commandName	  = "playq";
			this->helpDescription = "Plays a specific song from the current queue.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /playq SONGNUMBER.\n------");
			msgEmbed.setTitle("__**PlayQ Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<PlayQ>();
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
				uint64_t currentTime		= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				uint64_t previousPlayedTime{ 0 };
				if (PlayQ::timeOfLastPlay.contains(argsNew.getInteractionData().guildId.operator const uint64_t&())) {
					previousPlayedTime = PlayQ::timeOfLastPlay.at(argsNew.getInteractionData().guildId.operator const uint64_t&());
				}

				if (currentTime - previousPlayedTime < 5000) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but please wait a total of 5 seconds in between plays!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Timing Issue:**__");
					newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
					RespondToInputEventData dataPackage{ newEvent };
					dataPackage.addMessageEmbed(*newEmbed);
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					auto newerEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					InputEvents::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}

				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();

				previousPlayedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				PlayQ::timeOfLastPlay.insert_or_assign(argsNew.getInteractionData().guildId.operator const uint64_t&(), previousPlayedTime);

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

				if (!discordGuild.data.playlist.songQueue.size()) {
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
				}

				int32_t trackNumber{};
				if (argsNew.getCommandArguments().values["tracknumber"].value != "") {
					trackNumber = std::stoull(argsNew.getCommandArguments().values["tracknumber"].value.operator jsonifier::string().data()) - 1;
				}

				if (trackNumber >= discordGuild.data.playlist.songQueue.size()) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(newEvent.getUserData().userName, newEvent.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but that number is out of the range of the current track list!**__\n------");
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
				Playlist currentPlaylist = discordGuild.data.playlist;
				Song currentSong		 = discordGuild.data.playlist.currentSong;
				Song currentNew			 = currentPlaylist.songQueue.at(trackNumber);
				currentPlaylist.songQueue.erase(currentPlaylist.songQueue.begin() + trackNumber);
				jsonifier::vector<Song> newVector{};
				Playlist newPlaylist{};
				newVector.emplace_back(currentNew);
				newVector.emplace_back(currentSong);
				for (auto& value : currentPlaylist.songQueue) {
					newVector.emplace_back(value);
				}
				discordGuild.data.playlist.songQueue = newVector;
				discordGuild.writeDataToDB(managerAgent);
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
				std::cout << "PlayQ::execute()" << error.what() << std::endl;
			}
		};
		~PlayQ(){};
	};
	std::unordered_map<uint64_t, uint64_t> PlayQ::timeOfLastPlay{};

}// namespace DiscordCoreAPI
