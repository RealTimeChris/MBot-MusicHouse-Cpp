// play_rn.hpp - header for the "play right now" command.
// sep 9, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class play_rn : public base_function {
	  public:
		play_rn() {
			this->commandName	  = "playrn";
			this->helpDescription = "add a song to the front of the queue and play it if nothing is playing.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /playrn songname or url.\n------");
			msgEmbed.setTitle("__**play_rn Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<play_rn>();
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

				int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				int64_t previousPlayedTime{ 0 };
				if (play::timeOfLastPlay.contains(newEvent.getInteractionData().guildId.operator const uint64_t&())) {
					previousPlayedTime = static_cast<int64_t>(play::timeOfLastPlay.at(newEvent.getInteractionData().guildId.operator const uint64_t&()));
				}

				if (currentTime - previousPlayedTime < 5000) {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(newEvent.getUserData().userName,  newEvent.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but please wait a total of 5 seconds in between plays!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Timing Issue:**__");
					newEmbed->setColor("fefefe");
					respond_to_input_event_data dataPackage{ newEvent };
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					input_events::deleteInputEventResponseAsync(newEvent, 20000);
					return;
				}

				previousPlayedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				play::timeOfLastPlay.insert_or_assign(newEvent.getInteractionData().guildId.operator const uint64_t&(), previousPlayedTime);

				respond_to_input_event_data dataPackage{ newEvent };
				dataPackage.setResponseType(input_event_response_type::Ephemeral_Deferred_Response);
				newEvent = input_events::respondToInputEventAsync(dataPackage).get();
				snowflake currentVoiceChannelId{};
				if (guildMember.getVoiceStateData().channelId != 0) {
					currentVoiceChannelId = guildMember.getVoiceStateData().channelId;
				} else {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor("fefefe");
					respond_to_input_event_data dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					auto newerEvent = input_events::respondToInputEventAsync(dataPackageNew).get();
					input_events::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}
				voice_connection& voiceConnection = guild.connectToVoice(guildMember.user.id);

				if (!voiceConnection.areWeConnected()) {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(newEvent.getUserData().userName,  newEvent.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there is no voice connection that is currently held by me!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Connection Issue:**__");
					newEmbed->setColor("fefefe");
					respond_to_input_event_data dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					auto newerEvent = input_events::respondToInputEventAsync(dataPackageNew).get();
					input_events::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}

				if (guildMember.getVoiceStateData().channelId == 0 || guildMember.getVoiceStateData().channelId != voiceConnection.getChannelId()) {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(newEvent.getUserData().userName,  newEvent.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but you need to be in a correct voice channel to issue those commands!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor("fefefe");
					respond_to_input_event_data dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					auto newerEvent = input_events::respondToInputEventAsync(dataPackageNew).get();
					input_events::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}

				if (argsNew.getCommandArguments().values.size() == 0 && discordGuild.data.playlist.songQueue.size() == 0) {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(newEvent.getUserData().userName,  newEvent.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there's nothing to play!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor("fefefe");
					respond_to_input_event_data dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					auto newerEvent = input_events::respondToInputEventAsync(dataPackageNew).get();
					input_events::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				} else if (argsNew.getCommandArguments().values.size() == 0 && discord_core_client::getSongAPI(guild.id).areWeCurrentlyPlaying()) {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(newEvent.getUserData().userName,  newEvent.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("------\n__**Sorry, but there's already something playing!**__\n------");
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Playing Issue:**__");
					newEmbed->setColor("fefefe");
					respond_to_input_event_data dataPackageNew{ newEvent };
					dataPackageNew.addMessageEmbed(*newEmbed);
					dataPackageNew.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					auto newerEvent = input_events::respondToInputEventAsync(dataPackageNew).get();
					input_events::deleteInputEventResponseAsync(newerEvent, 20000);
					return;
				}


				jsonifier::vector<song> searchResults{};
				if (argsNew.getCommandArguments().values.size() > 0) {
					auto searchResultsNew =
						discord_core_client::getSongAPI(guild.id).searchForSong(argsNew.getCommandArguments().values["songname"].operator jsonifier::string(), 1);
					for (auto& value: searchResultsNew) {
						searchResults.emplace_back(value);
					}
				}
				jsonifier::vector<embed_data> embedsFromSearch{};
				uint64_t x{ 0 };
				for (song& value : searchResults) {
					x += 1;
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(newEvent.getUserData().userName,  newEvent.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("__**Title:**__ [" + value.songTitle + "](" + value.viewUrl + ")" + "\n__**Description:**__ " + value.description +
											 "\n__**Duration:**__ " + value.duration);
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**" + jsonifier::toString(x) + " of " + jsonifier::toString(searchResults.size()) + "**__");
					newEmbed->setImage(value.thumbnailUrl);
					newEmbed->setColor("fefefe");
					embedsFromSearch.emplace_back(*newEmbed);
				}

				if (embedsFromSearch.size() > 0) {
					discordGuild.getDataFromDB();
					song songNew			 = searchResults.at(0);
					songNew.addedByUserId = guildMember.user.id;

					jsonifier::vector<song> songVector{};

					if (discordGuild.data.playlist.songQueue.size() > 0) {
						if (discordGuild.data.playlist.currentSong.songId != "") {
							songVector.resize(discordGuild.data.playlist.songQueue.size() + 2);
							songVector.at(1) = discordGuild.data.playlist.currentSong;
							songVector.at(0) = songNew;
							for (uint8_t y = 0; y < discordGuild.data.playlist.songQueue.size(); y += 1) {
								songVector.at(static_cast<uint64_t>(y) + 2) = discordGuild.data.playlist.songQueue.at(static_cast<uint64_t>(y));
							}

						} else {
							songVector.resize(discordGuild.data.playlist.songQueue.size() + 1);
							songVector.at(0) = songNew;

							for (uint8_t y = 0; y < discordGuild.data.playlist.songQueue.size(); y += 1) {
								songVector[static_cast<uint64_t>(y) + 1] = discordGuild.data.playlist.songQueue[static_cast<uint64_t>(y)];
							}
						}
					} else {
						if (discordGuild.data.playlist.currentSong.songId != "") {
							songVector.resize(2);
							songVector.at(1) = discordGuild.data.playlist.currentSong;
							songVector.at(0) = songNew;
						} else {
							songVector.resize(1);
							songVector.at(0) = songNew;
						}
					}

					discordGuild.data.playlist.songQueue   = songVector;
					discordGuild.data.playlist.currentSong = song();
					discordGuild.writeDataToDB();
				}
				auto channelId = argsNew.getInputEventData().getChannelData().id;
				auto theTask   = [=](song_completion_event_data eventData) mutable -> co_routine<void, false> {
					  auto argsNewer = std::move(argsNew);
					  co_await newThreadAwaitable<void, false>();
					  user_data userNew = users::getCachedUser({ eventData.guildMemberId });
					  std::this_thread::sleep_for(150ms);
					  discordGuild.getDataFromDB();
					  if (discordGuild.data.playlist.songQueue.size()) {
						  unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
						  if (!eventData.wasItAFail) {
							  discordGuild.getDataFromDB();
							  discordGuild.data.playlist.sendNextSong();
							  discord_core_client::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong);
							  discordGuild.writeDataToDB();
							  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(user_image_types::Avatar));
							  newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl +
													   ")" + "\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " +
													   discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" +
													   discordGuild.data.playlist.currentSong.addedByUserId + ">");
							  newEmbed->setImage(discordGuild.data.playlist.currentSong.thumbnailUrl);
							  newEmbed->setTimeStamp(getTimeAndDate());
							  newEmbed->setTitle("__**Now Playing:**__");
							  newEmbed->setColor("fefefe");
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
							  create_message_data dataPackage02(channelId);
							  dataPackage02.addMessageEmbed(*newEmbed);
							  messages::createMessageAsync(dataPackage02).get();
						  } else {
							  discordGuild.getDataFromDB();
							  discordGuild.data.playlist.sendNextSong();
							  discord_core_client::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong);
							  discordGuild.writeDataToDB();
							  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(user_image_types::Avatar));
							  newEmbed->setDescription("__**it appears as though there was an error when trying to play the previous track!**__");
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
							  create_message_data dataPackage02(channelId);
							  dataPackage02.addMessageEmbed(*newEmbed);
							  messages::createMessageAsync(dataPackage02).get();

							  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(user_image_types::Avatar));
							  newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl +
													   ")" + "\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " +
													   discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" +
													   discordGuild.data.playlist.currentSong.addedByUserId + ">");
							  newEmbed->setImage(discordGuild.data.playlist.currentSong.thumbnailUrl);
							  newEmbed->setTimeStamp(getTimeAndDate());
							  newEmbed->setTitle("__**Now Playing:**__");
							  newEmbed->setColor("fefefe");
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
							  create_message_data dataPackage03(channelId);
							  dataPackage03.addMessageEmbed(*newEmbed);
							  messages::createMessageAsync(dataPackage03).get();
						  }
						  discordGuild.writeDataToDB();
					  } else {
						  unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
						  newEmbed->setAuthor(userNew.userName, userNew.getUserImageUrl(user_image_types::Avatar));
						  newEmbed->setDescription("------\n__**Sorry, but there's nothing left to play here!**__\n------");
						  newEmbed->setTimeStamp(getTimeAndDate());
						  newEmbed->setTitle("__**Now Playing:**__");
						  newEmbed->setColor("fefefe");
						  if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("✅ Loop-All, ✅ Loop-Song");
						  } else if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("❌ Loop-All, ✅ Loop-Song");
						  } else if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("✅ Loop-All, ❌ Loop-Song");
						  } else if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
							  newEmbed->setFooter("❌ Loop-All, ❌ Loop-Song");
						  }
						  create_message_data dataPackage02(channelId);
						  dataPackage02.addMessageEmbed(*newEmbed);
						  messages::createMessageAsync(dataPackage02).get();
						  discord_core_client::getSongAPI(guild.id).stop();
					  }
					  co_return;
				};
				discord_core_client::getSongAPI(guild.id).onSongCompletion(theTask);
				if (discordGuild.data.playlist.songQueue.size()) {
					discord_core_client::getSongAPI(guild.id).stop();
					discordGuild.getDataFromDB();
					discordGuild.data.playlist.sendNextSong();
					discord_core_client::getSongAPI(guild.id).play(discordGuild.data.playlist.currentSong);
					discordGuild.writeDataToDB();
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setDescription("__**Title:**__ [" + discordGuild.data.playlist.currentSong.songTitle + "](" + discordGuild.data.playlist.currentSong.viewUrl + ")" +
											 "\n__**Description:**__ " + discordGuild.data.playlist.currentSong.description + "\n__**Duration:**__ " +
											 discordGuild.data.playlist.currentSong.duration + "\n__**Added By:**__ <@!" + discordGuild.data.playlist.currentSong.addedByUserId +
											 ">");
					newEmbed->setImage(discordGuild.data.playlist.currentSong.thumbnailUrl);
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Now Playing:**__");
					newEmbed->setColor("fefefe");
					if (discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("✅ Loop-All, ✅ Loop-Song");
					} else if (!discordGuild.data.playlist.isLoopAllEnabled && discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("❌ Loop-All, ✅ Loop-Song");
					} else if (discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("✅ Loop-All, ❌ Loop-Song");
					} else if (!discordGuild.data.playlist.isLoopAllEnabled && !discordGuild.data.playlist.isLoopSongEnabled) {
						newEmbed->setFooter("❌ Loop-All, ❌ Loop-Song");
					}
					respond_to_input_event_data dataPackageNew{ newEvent };
					dataPackageNew.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackageNew.addMessageEmbed(*newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackageNew).get();
				}
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "play_rn::execute()" << error.what() << std::endl;
			}
		};
		~play_rn(){};
	};

};// namespace discord_core_api
