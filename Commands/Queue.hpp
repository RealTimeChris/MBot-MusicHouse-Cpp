// Queue.hpp - Header for the "Queue" command.
// Sep 1, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"
#include <regex>

namespace DiscordCoreAPI {

	jsonifier::vector<EmbedData> updateMessageEmbeds(jsonifier::vector<Song>& playlist, DiscordGuild& discordGuild, InputEventData& originalEvent, UserCacheData& theUser,
		int32_t currentPageIndex) {
		jsonifier::vector<jsonifier::vector<EmbedFieldData>> msgEmbedFields{};
		msgEmbedFields.emplace_back(jsonifier::vector<EmbedFieldData>());
		int32_t msgEmbedFieldsPage{ 0 };
		for (int32_t y = 0; y < playlist.size(); y += 1) {
			if (y % 25 == 0 && y > 0) {
				msgEmbedFieldsPage += 1;
				msgEmbedFields.emplace_back(jsonifier::vector<EmbedFieldData>());
			}
			EmbedFieldData msgEmbedField{};
			msgEmbedField.Inline = false;
			msgEmbedField.value	 = "__**Title:**__ [" + playlist.at(y).songTitle + "](" + playlist.at(y).viewUrl + ")\n__**Added By:**__ <@!" +
				jsonifier::toString(playlist.at(y).addedByUserId.operator const uint64_t&()) + ">";

			msgEmbedField.name = "__**" + jsonifier::toString(y + 1) + " of " + jsonifier::toString(playlist.size()) + "**__";
			msgEmbedFields.at(msgEmbedFieldsPage).emplace_back(msgEmbedField);
		}
		msgEmbedFieldsPage = 0;
		jsonifier::vector<EmbedData> newMsgEmbeds{};
		for (int32_t y = 0; y < msgEmbedFields.size(); y += 1) {
			UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
			newEmbed->setAuthor(theUser.userName, theUser.getUserImageUrl(UserImageTypes::Avatar));
			newEmbed->setColor(jsonifier::string{ discordGuild.data.borderColor });
			newEmbed->setTimeStamp(getTimeAndDate());
			newEmbed->setTitle("__**Playlist, Page " + jsonifier::toString(y + 1) + " of " + jsonifier::toString(msgEmbedFields.size()) + "**__");
			newEmbed->setFooter("React with ✅ to edit the contents of the current page. React with ❌ to exit!");
			newEmbed->setDescription("__**React with ✅ to edit the contents of the current page. React with ❌ to exit!**__").fields = msgEmbedFields.at(y);
			newMsgEmbeds.emplace_back(*newEmbed);
		}
		RespondToInputEventData dataPackage(originalEvent);
		dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
		dataPackage.addMessageEmbed(newMsgEmbeds.at(currentPageIndex));
		dataPackage.addContent("");
		dataPackage.addButton(false, "check", "Edit", ButtonStyle::Success, "✅");
		dataPackage.addButton(false, "back", "Back", ButtonStyle::Success, "◀️");
		dataPackage.addButton(false, "next", "Next", ButtonStyle::Success, "▶️");
		dataPackage.addButton(false, "exit", "Exit", ButtonStyle::Success, "❌");
		InputEvents::respondToInputEventAsync(dataPackage).get();
		return newMsgEmbeds;
	}

	class TheQueue : public BaseFunction {
	  public:
		TheQueue() {
			this->commandName	  = "queue";
			this->helpDescription = "View and edit the song queue.";
			UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
			newEmbed->setDescription("------\nSimply enter /queue, and follow the instructions!\n------");
			newEmbed->setTitle("__**Queue Usage:**__");
			newEmbed->setTimeStamp(getTimeAndDate());
			newEmbed->setColor("FeFeFe");
			this->helpEmbed = *newEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<TheQueue>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				UniquePtr<ChannelCacheData> channel{ makeUnique<ChannelCacheData>(argsNew.getChannelData()) };

				UniquePtr<GuildCacheData> guild{ makeUnique<GuildCacheData>(argsNew.getInteractionData().guildId) };

				UniquePtr<DiscordGuild> discordGuild(makeUnique<DiscordGuild>(DiscordCoreAPI::managerAgent, *guild));

				bool checkIfAllowedInChannel = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), *discordGuild);

				if (!checkIfAllowedInChannel) {
					return;
				}

				GuildMemberCacheData guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), *discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}

				InputEventData newEvent{ argsNew.getInputEventData() };
				discordGuild->getDataFromDB(managerAgent);
				if (discordGuild->data.playlist.songQueue.size() == 0) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					newEmbed->setColor(jsonifier::string{ discordGuild->data.borderColor });
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Empty Playlist:**__");
					newEmbed->setDescription("------\n__**Sorry, but there is nothing here to display!**__\n------");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				int32_t currentPageIndex{ 0 };

				jsonifier::vector<jsonifier::vector<EmbedFieldData>> msgEmbedFields{};
				msgEmbedFields.emplace_back(jsonifier::vector<EmbedFieldData>());
				int32_t msgEmbedFieldsPage{ 0 };
				for (int32_t y = 0; y < discordGuild->data.playlist.songQueue.size(); y += 1) {
					if (y % 25 == 0 && y > 0) {
						if (y > 0) {
							msgEmbedFieldsPage += 1;
						}
						msgEmbedFields.emplace_back(jsonifier::vector<EmbedFieldData>());
					}
					EmbedFieldData msgEmbedField{};
					msgEmbedField.Inline = false;
					msgEmbedField.value	 = "__**Title:**__ [" + discordGuild->data.playlist.songQueue.at(y).songTitle + "](" + discordGuild->data.playlist.songQueue.at(y).viewUrl +
						")\n__**Added By:**__ <@!" + jsonifier::toString(discordGuild->data.playlist.songQueue.at(y).addedByUserId.operator const uint64_t&()) + ">";
					msgEmbedField.name = "__**" + jsonifier::toString(y + 1) + " of " + jsonifier::toString(discordGuild->data.playlist.songQueue.size()) + "**__";
					msgEmbedFields.at(msgEmbedFieldsPage).emplace_back(msgEmbedField);
				}
				jsonifier::vector<EmbedData> msgEmbeds{};
				msgEmbedFieldsPage = 0;
				for (int32_t y = 0; y < msgEmbedFields.size(); y += 1) {
					UniquePtr<EmbedData> newEmbed{ makeUnique<EmbedData>() };
					newEmbed->setColor(jsonifier::string{ discordGuild->data.borderColor })
						.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar))
						.setTimeStamp(getTimeAndDate())
						.setTitle("__**Playlist, Page " + jsonifier::toString(y + 1) + " of " + jsonifier::toString(msgEmbedFields.size()) + "**__")
						.setFooter("React with ✅ to edit the contents of the current page. React with ❌ to exit!")
						.setDescription("__**React with ✅ to edit the contents of the current page. React with ❌ to exit!**__")
						.fields = msgEmbedFields.at(y);
					msgEmbeds.emplace_back(*newEmbed);
				}
				RespondToInputEventData dataPackage0(newEvent);
				dataPackage0.setResponseType(InputEventResponseType::Interaction_Response);
				dataPackage0.addMessageEmbed(msgEmbeds.at(currentPageIndex));
				dataPackage0.addContent("");
				dataPackage0.addButton(false, "check", "Edit", ButtonStyle::Success, "✅");
				dataPackage0.addButton(false, "back", "Back", ButtonStyle::Success, "◀️");
				dataPackage0.addButton(false, "next", "Next", ButtonStyle::Success, "▶️");
				dataPackage0.addButton(false, "exit", "Exit", ButtonStyle::Success, "❌");
				newEvent = InputEvents::respondToInputEventAsync(dataPackage0).get();
				for (int32_t y = 0; y < 1; y) {
					bool doWeQuit{ false };
					UniquePtr<ButtonCollector> button{ makeUnique<ButtonCollector>(newEvent) };
					auto createResponseData = makeUnique<CreateInteractionResponseData>();
					auto embedData			= makeUnique<EmbedData>();
					embedData->setColor("FEFEFE");
					embedData->setTitle("__**Permissions Issue:**__");
					embedData->setTimeStamp(getTimeAndDate());
					embedData->setDescription("Sorry, but that button can only be pressed by <@" + argsNew.getUserData().id + ">!");
					createResponseData->addMessageEmbed(*embedData);
					createResponseData->setResponseType(InteractionCallbackType::Channel_Message_With_Source);
					createResponseData->setFlags(64);
					auto buttonCollectedData = button->collectButtonData(false, 120000, 1, *createResponseData, argsNew.getUserData().id).get();
					newEvent				 = *buttonCollectedData.at(0).interactionData;
					Snowflake userID		 = argsNew.getUserData().id;
					if (buttonCollectedData.size() == 0 || buttonCollectedData.at(0).buttonId == "exit" || buttonCollectedData.at(0).buttonId == "empty" || doWeQuit) {
						RespondToInputEventData dataPackage02(*buttonCollectedData.at(0).interactionData);
						dataPackage02.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage02.addMessageEmbed(msgEmbeds.at(currentPageIndex));
						dataPackage02.addContent("");
						newEvent = InputEvents::respondToInputEventAsync(dataPackage02).get();
						break;
					} else if (buttonCollectedData.at(0).buttonId == "next" && (currentPageIndex == (msgEmbeds.size() - 1))) {
						currentPageIndex = 0;
						UniquePtr<RespondToInputEventData> dataPackage02{ makeUnique<RespondToInputEventData>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(currentPageIndex));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", ButtonStyle::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", ButtonStyle::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", ButtonStyle::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", ButtonStyle::Success, "❌");
						newEvent = InputEvents::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "next" && (currentPageIndex < msgEmbeds.size())) {
						currentPageIndex += 1;
						UniquePtr<RespondToInputEventData> dataPackage02{ makeUnique<RespondToInputEventData>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(currentPageIndex));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", ButtonStyle::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", ButtonStyle::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", ButtonStyle::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", ButtonStyle::Success, "❌");
						newEvent = InputEvents::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "back" && (currentPageIndex > 0)) {
						currentPageIndex -= 1;
						UniquePtr<RespondToInputEventData> dataPackage02{ makeUnique<RespondToInputEventData>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(currentPageIndex));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", ButtonStyle::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", ButtonStyle::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", ButtonStyle::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", ButtonStyle::Success, "❌");
						newEvent = InputEvents::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "back" && (currentPageIndex == 0)) {
						currentPageIndex = (int32_t)msgEmbeds.size() - 1;
						UniquePtr<RespondToInputEventData> dataPackage02{ makeUnique<RespondToInputEventData>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(currentPageIndex));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", ButtonStyle::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", ButtonStyle::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", ButtonStyle::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", ButtonStyle::Success, "❌");
						newEvent = InputEvents::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "check") {
						msgEmbeds.at(currentPageIndex)
							.setDescription("__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> "
											"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle "
											"the playlist.\nType 'exit' to exit.__\n");
						msgEmbeds.at(currentPageIndex)
							.setFooter("Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> "
									   "<destinationTrackNumber>' to swap tracks.\nType "
									   "'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
						newEvent = *buttonCollectedData.at(0).interactionData;
						UniquePtr<RespondToInputEventData> dataPackage03{ makeUnique<RespondToInputEventData>(*buttonCollectedData.at(0).interactionData) };
						dataPackage03->setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage03->addMessageEmbed(msgEmbeds.at(currentPageIndex));
						dataPackage03->addContent("");
						newEvent = InputEvents::respondToInputEventAsync(*dataPackage03).get();
						while (!doWeQuit) {
							UniquePtr<RespondToInputEventData> dataPackage02{ makeUnique<RespondToInputEventData>(newEvent) };
							dataPackage02->setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage02->addMessageEmbed(msgEmbeds.at(currentPageIndex));
							dataPackage02->addContent("");
							newEvent = InputEvents::respondToInputEventAsync(*dataPackage02).get();

							auto messageFilter = [=](MessageData message) -> bool {
								if (userID == message.author.id) {
									return true;
								} else {
									return false;
								}
							};
							UserCacheData theUser = argsNew.getUserData();
							UniquePtr<ObjectCollector<MessageData>> messageCollector{ makeUnique<ObjectCollector<MessageData>>() };
							auto returnedMessages = messageCollector->collectObjects(1, 120000, messageFilter).get();
							if (returnedMessages.objects.size() == 0) {
								auto inputEventData = argsNew.getInputEventData();
								msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex);
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, *discordGuild, newEvent, theUser, currentPageIndex);
								doWeQuit  = true;
								break;
							}
							jsonifier::vector<jsonifier::string> args2;
							jsonifier::string newString = convertToLowerCase(returnedMessages.objects.at(0).content);
							std::regex wordRegex("[a-z]{1,12}");
							std::smatch wordRegexMatch;
							jsonifier::string newerString = newString.data();
							std::string newestString	  = newerString.operator std::basic_string<char>();
							regex_search(newestString, wordRegexMatch, wordRegex);
							args2.emplace_back(wordRegexMatch.str());
							std::regex_iterator<const char*>::regex_type rx("\\d{1,4}");
							std::regex_iterator<const char*> next(newString.data(), newString.data() + strlen(newString.data()), rx);
							std::regex_iterator<const char*> end;

							for (; next != end; ++next) {
								args2.emplace_back(next->str());
							}

							std::regex digitRegex("\\d{1,3}");
							if (args2.size() == 0 || convertToLowerCase(args2.at(0)) == "exit") {
								DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
								Messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex);
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, *discordGuild, newEvent, theUser, currentPageIndex);
								doWeQuit  = true;
								discordGuild->writeDataToDB(managerAgent);
								break;
							} else if (convertToLowerCase(args2.at(0)) != "remove" && convertToLowerCase(args2.at(0)) != "swap" && convertToLowerCase(args2.at(0)) != "exit" &&
									   convertToLowerCase(args2.at(0)) != "shuffle") {
								msgEmbeds.at(currentPageIndex)
									.setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
													"<sourceTrackNumber> "
													"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType exit "
													"to exit.__\n");
								msgEmbeds.at(currentPageIndex)
									.setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType "
											   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
											   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
								DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
								Messages::deleteMessageAsync(dataPackage);
								RespondToInputEventData dataPackage04(newEvent);
								dataPackage04.setResponseType(InputEventResponseType::Edit_Interaction_Response);
								dataPackage04.addMessageEmbed(msgEmbeds.at(currentPageIndex));
								dataPackage04.addContent("");
								newEvent = InputEvents::respondToInputEventAsync(dataPackage04).get();
								continue;
							} else if (convertToLowerCase(args2.at(0)) == "remove") {
								if (args2.size() < 2 || !regex_search(args2.at(1).data(), digitRegex)) {
									msgEmbeds.at(currentPageIndex)
										.setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(currentPageIndex)
										.setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
									Messages::deleteMessageAsync(dataPackage);
									RespondToInputEventData dataPackage04(newEvent);
									dataPackage04.setResponseType(InputEventResponseType::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(currentPageIndex));
									dataPackage04.addContent("");
									newEvent = InputEvents::respondToInputEventAsync(dataPackage04).get();
									continue;
								}
								if ((std::stoll(args2.at(1).data()) - 1) < 0 || ( size_t )(std::stoll(args2.at(1).data()) - 1) >= discordGuild->data.playlist.songQueue.size() ||
									args2.size() < 1) {
									msgEmbeds.at(currentPageIndex)
										.setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(currentPageIndex)
										.setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
									Messages::deleteMessageAsync(dataPackage);
									RespondToInputEventData dataPackage04(newEvent);
									dataPackage04.setResponseType(InputEventResponseType::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(currentPageIndex));
									dataPackage04.addContent("");
									newEvent = InputEvents::respondToInputEventAsync(dataPackage04).get();
									continue;
								}
								int32_t removeIndex = ( int32_t )std::stoll(args2.at(1).data()) - 1;

								discordGuild->data.playlist.songQueue.erase(discordGuild->data.playlist.songQueue.begin() + removeIndex);
								DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
								Messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex);
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, *discordGuild, newEvent, theUser, currentPageIndex);
								doWeQuit  = true;
								discordGuild->writeDataToDB(managerAgent);
								break;
							} else if (convertToLowerCase(args2.at(0)) == "swap") {
								if (args2.size() < 3 || !regex_search(args2.at(1).data(), digitRegex) || !regex_search(args2.at(2).data(), digitRegex)) {
									msgEmbeds.at(currentPageIndex)
										.setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(currentPageIndex)
										.setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
									Messages::deleteMessageAsync(dataPackage);
									RespondToInputEventData dataPackage04(newEvent);
									dataPackage04.setResponseType(InputEventResponseType::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(currentPageIndex));
									dataPackage04.addContent("");
									newEvent = InputEvents::respondToInputEventAsync(dataPackage04).get();
									continue;
								}
								if (args2.size() < 2 ||
									((std::stoll(args2.at(1).data()) - 1) < 0 || ( size_t )(std::stoll(args2.at(1).data()) - 1) >= discordGuild->data.playlist.songQueue.size() ||
										(std::stoll(args2.at(2).data()) - 1) < 0 ||
										( size_t )(std::stoll(args2.at(2).data()) - 1) >= discordGuild->data.playlist.songQueue.size() ||
										args2.size() < 2)) {
									msgEmbeds.at(currentPageIndex)
										.setDescription("__**PLEASE ENTER A PROPER INPUT!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(currentPageIndex)
										.setFooter("PLEASE ENTER A PROPER INPUT!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
									Messages::deleteMessageAsync(dataPackage);
									RespondToInputEventData dataPackage04(newEvent);
									dataPackage04.setResponseType(InputEventResponseType::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(currentPageIndex));
									dataPackage04.addContent("");
									newEvent = InputEvents::respondToInputEventAsync(dataPackage04).get();
									continue;
								}

								int32_t sourceIndex		 = ( int32_t )std::stoll(args2.at(1).data()) - 1;
								int32_t destinationIndex = ( int32_t )std::stoll(args2.at(2).data()) - 1;
								discordGuild->data.playlist.modifyQueue(sourceIndex, destinationIndex);
								DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
								Messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex);
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, *discordGuild, newEvent, theUser, currentPageIndex);
								doWeQuit  = true;
								discordGuild->writeDataToDB(managerAgent);
								break;
							} else if (convertToLowerCase(args2.at(0)) == "shuffle") {
								auto oldSongArray = discordGuild->data.playlist;
								jsonifier::vector<Song> newVector{};
								while (oldSongArray.songQueue.size() > 0) {
									std::mt19937_64 randomEngine{ static_cast<uint64_t>(
										std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) };
									int32_t randomIndex = static_cast<int32_t>(
										(static_cast<float>(randomEngine()) / static_cast<float>(randomEngine.max()) * static_cast<float>(oldSongArray.songQueue.size())));
									newVector.emplace_back(oldSongArray.songQueue.at(randomIndex));
									oldSongArray.songQueue.erase(oldSongArray.songQueue.begin() + randomIndex);
								}
								oldSongArray.songQueue		= newVector;
								discordGuild->data.playlist = oldSongArray;
								DeleteMessageData dataPackage{ returnedMessages.objects.at(0) };
								Messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + currentPageIndex);
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, *discordGuild, newEvent, theUser, currentPageIndex);
								doWeQuit  = true;
								discordGuild->writeDataToDB(managerAgent);
								break;
							}
						}
						if (doWeQuit) {
							discordGuild->writeDataToDB(managerAgent);
							continue;
						}
					};
					return;
				}
			} catch (const std::runtime_error& error) {
				std::cout << "Queue::execute()" << error.what() << std::endl;
			};
		};
		~TheQueue(){};
	};

};// namespace DiscordCoreAPI
