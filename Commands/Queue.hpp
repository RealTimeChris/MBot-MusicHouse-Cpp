// queue.hpp - header for the "queue" command.
// sep 1, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"
#include <regex>

namespace discord_core_api {

	jsonifier::vector<embed_data> updateMessageEmbeds(jsonifier::vector<song>& playlist, input_event_data& originalEvent, user_data& theUser,
		uint64_t currentPageIndex) {
		jsonifier::vector<jsonifier::vector<embed_field_data>> msgEmbedFields{};
		msgEmbedFields.emplace_back(jsonifier::vector<embed_field_data>());
		uint64_t msgEmbedFieldsPage{ 0 };
		for (uint64_t y = 0; y < playlist.size(); y += 1) {
			if (y % 25 == 0 && y > 0) {
				msgEmbedFieldsPage += 1;
				msgEmbedFields.emplace_back(jsonifier::vector<embed_field_data>());
			}
			embed_field_data msgEmbedField{};
			msgEmbedField.Inline = false;
			msgEmbedField.value	 = "__**Title:**__ [" + playlist.at(y).songTitle + "](" + playlist.at(y).viewUrl + ")\n__**Added By:**__ <@!" +
				jsonifier::toString(playlist.at(y).addedByUserId.operator const uint64_t&()) + ">";

			msgEmbedField.name = "__**" + jsonifier::toString(y + 1) + " of " + jsonifier::toString(playlist.size()) + "**__";
			msgEmbedFields.at(msgEmbedFieldsPage).emplace_back(msgEmbedField);
		}
		msgEmbedFieldsPage = 0;
		jsonifier::vector<embed_data> newMsgEmbeds{};
		for (uint64_t y = 0; y < msgEmbedFields.size(); y += 1) {
			unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
			newEmbed->setAuthor(theUser.userName, theUser.getUserImageUrl(user_image_types::Avatar));
			newEmbed->setColor("fefefe");
			newEmbed->setTimeStamp(getTimeAndDate());
			newEmbed->setTitle("__**Playlist, page " + jsonifier::toString(y + 1) + " of " + jsonifier::toString(msgEmbedFields.size()) + "**__");
			newEmbed->setFooter("React with ✅ to edit the contents of the current page. React with ❌ to exit!");
			newEmbed->setDescription("__**React with ✅ to edit the contents of the current page. React with ❌ to exit!**__").fields = msgEmbedFields.at(y);
			newMsgEmbeds.emplace_back(*newEmbed);
		}
		respond_to_input_event_data dataPackage(originalEvent);
		dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
		dataPackage.addMessageEmbed(newMsgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
		dataPackage.addContent("");
		dataPackage.addButton(false, "check", "Edit", button_style::Success, "✅");
		dataPackage.addButton(false, "back", "Back", button_style::Success, "◀️");
		dataPackage.addButton(false, "next", "Next", button_style::Success, "▶️");
		dataPackage.addButton(false, "exit", "Exit", button_style::Success, "❌");
		input_events::respondToInputEventAsync(dataPackage).get();
		return newMsgEmbeds;
	}

	class the_queue : public base_function {
	  public:
		the_queue() {
			this->commandName	  = "queue";
			this->helpDescription = "View and edit the song queue.";
			unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
			newEmbed->setDescription("------\nSimply enter /queue, and follow the instructions!\n------");
			newEmbed->setTitle("__**Queue Usage:**__");
			newEmbed->setTimeStamp(getTimeAndDate());
			newEmbed->setColor("fefefe");
			this->helpEmbed = *newEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<the_queue>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				unique_ptr<channel_data> channel{ makeUnique<channel_data>(argsNew.getChannelData()) };

				unique_ptr<guild_data> guild{ makeUnique<guild_data>(argsNew.getInteractionData().guildId) };

				unique_ptr<discord_guild> discordGuild(makeUnique<discord_guild>(*guild));

				bool checkIfAllowedInChannel = checkIfAllowedPlayingInChannel(argsNew.getInputEventData(), *discordGuild);

				if (!checkIfAllowedInChannel) {
					return;
				}

				guild_member_data guildMember{ argsNew.getGuildMemberData() };

				bool doWeHaveControl = checkIfWeHaveControl(argsNew.getInputEventData(), *discordGuild, guildMember);

				if (!doWeHaveControl) {
					return;
				}

				input_event_data newEvent{ argsNew.getInputEventData() };
				discordGuild->getDataFromDB();
				if (discordGuild->data.playlist.songQueue.size() == 0) {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					newEmbed->setColor(discordGuild->data.borderColor);
					newEmbed->setTimeStamp(getTimeAndDate());
					newEmbed->setTitle("__**Empty Playlist:**__");
					newEmbed->setDescription("------\n__**Sorry, but there is nothing here to display!**__\n------");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
					dataPackage.addMessageEmbed(*newEmbed);
					newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				int64_t currentPageIndex{ 0 };

				jsonifier::vector<jsonifier::vector<embed_field_data>> msgEmbedFields{};
				msgEmbedFields.emplace_back(jsonifier::vector<embed_field_data>());
				uint64_t msgEmbedFieldsPage{ 0 };
				for (uint64_t y = 0; y < discordGuild->data.playlist.songQueue.size(); y += 1) {
					if (y % 25 == 0 && y > 0) {
						if (y > 0) {
							msgEmbedFieldsPage += 1;
						}
						msgEmbedFields.emplace_back(jsonifier::vector<embed_field_data>());
					}
					embed_field_data msgEmbedField{};
					msgEmbedField.Inline = false;
					msgEmbedField.value	 = "__**Title:**__ [" + discordGuild->data.playlist.songQueue.at(y).songTitle + "](" + discordGuild->data.playlist.songQueue.at(y).viewUrl +
						")\n__**Added By:**__ <@!" + jsonifier::toString(discordGuild->data.playlist.songQueue.at(y).addedByUserId.operator const uint64_t&()) + ">";
					msgEmbedField.name = "__**" + jsonifier::toString(y + 1) + " of " + jsonifier::toString(discordGuild->data.playlist.songQueue.size()) + "**__";
					msgEmbedFields.at(msgEmbedFieldsPage).emplace_back(msgEmbedField);
				}
				jsonifier::vector<embed_data> msgEmbeds{};
				msgEmbedFieldsPage = 0;
				for (uint64_t y = 0; y < msgEmbedFields.size(); y += 1) {
					unique_ptr<embed_data> newEmbed{ makeUnique<embed_data>() };
					newEmbed->setColor(discordGuild->data.borderColor)
						.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(user_image_types::Avatar))
						.setTimeStamp(getTimeAndDate())
						.setTitle("__**Playlist, page " + jsonifier::toString(y + 1) + " of " + jsonifier::toString(msgEmbedFields.size()) + "**__")
						.setFooter("React with ✅ to edit the contents of the current page. React with ❌ to exit!")
						.setDescription("__**React with ✅ to edit the contents of the current page. React with ❌ to exit!**__")
						.fields = msgEmbedFields.at(y);
					msgEmbeds.emplace_back(*newEmbed);
				}
				respond_to_input_event_data dataPackage0(newEvent);
				dataPackage0.setResponseType(input_event_response_type::Interaction_Response);
				dataPackage0.addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
				dataPackage0.addContent("");
				dataPackage0.addButton(false, "check", "Edit", button_style::Success, "✅");
				dataPackage0.addButton(false, "back", "Back", button_style::Success, "◀️");
				dataPackage0.addButton(false, "next", "Next", button_style::Success, "▶️");
				dataPackage0.addButton(false, "exit", "Exit", button_style::Success, "❌");
				newEvent = input_events::respondToInputEventAsync(dataPackage0).get();
				while (true) {
					bool doWeQuit{ false };
					unique_ptr<button_collector> button{ makeUnique<button_collector>(newEvent) };
					auto createResponseData = makeUnique<create_interaction_response_data>();
					auto embedData			= makeUnique<embed_data>();
					embedData->setColor("fefefe");
					embedData->setTitle("__**Permissions Issue:**__");
					embedData->setTimeStamp(getTimeAndDate());
					embedData->setDescription("Sorry, but that button can only be pressed by <@" + argsNew.getUserData().id + ">!");
					createResponseData->addMessageEmbed(*embedData);
					createResponseData->setResponseType(interaction_callback_type::Channel_Message_With_Source);
					createResponseData->setFlags(64);
					auto buttonCollectedData = button->collectButtonData(false, 120000, 1, *createResponseData, argsNew.getUserData().id).get();
					newEvent				 = *buttonCollectedData.at(0).interactionData;
					snowflake userID		 = argsNew.getUserData().id;
					if (buttonCollectedData.size() == 0 || buttonCollectedData.at(0).buttonId == "exit" || buttonCollectedData.at(0).buttonId == "empty" || doWeQuit) {
						respond_to_input_event_data dataPackage02(*buttonCollectedData.at(0).interactionData);
						dataPackage02.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage02.addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
						dataPackage02.addContent("");
						newEvent = input_events::respondToInputEventAsync(dataPackage02).get();
						break;
					} else if (buttonCollectedData.at(0).buttonId == "next" && (static_cast<uint64_t>(currentPageIndex) == (msgEmbeds.size() - 1))) {
						currentPageIndex = 0;
						unique_ptr<respond_to_input_event_data> dataPackage02{ makeUnique<respond_to_input_event_data>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", button_style::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", button_style::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", button_style::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", button_style::Success, "❌");
						newEvent = input_events::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "next" && (static_cast<uint64_t>(currentPageIndex) < msgEmbeds.size())) {
						currentPageIndex += 1;
						unique_ptr<respond_to_input_event_data> dataPackage02{ makeUnique<respond_to_input_event_data>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", button_style::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", button_style::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", button_style::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", button_style::Success, "❌");
						newEvent = input_events::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "back" && (currentPageIndex > 0)) {
						currentPageIndex -= 1;
						unique_ptr<respond_to_input_event_data> dataPackage02{ makeUnique<respond_to_input_event_data>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", button_style::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", button_style::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", button_style::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", button_style::Success, "❌");
						newEvent = input_events::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "back" && (currentPageIndex == 0)) {
						currentPageIndex = static_cast<int64_t>(msgEmbeds.size() - 1);
						unique_ptr<respond_to_input_event_data> dataPackage02{ makeUnique<respond_to_input_event_data>(*buttonCollectedData.at(0).interactionData) };
						dataPackage02->setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage02->addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
						dataPackage02->addContent("");
						dataPackage02->addButton(false, "check", "Edit", button_style::Success, "✅");
						dataPackage02->addButton(false, "back", "Back", button_style::Success, "◀️");
						dataPackage02->addButton(false, "next", "Next", button_style::Success, "▶️");
						dataPackage02->addButton(false, "exit", "Exit", button_style::Success, "❌");
						newEvent = input_events::respondToInputEventAsync(*dataPackage02).get();
						continue;
					} else if (buttonCollectedData.at(0).buttonId == "check") {
						msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
							.setDescription("__Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> "
											"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle "
											"the playlist.\nType 'exit' to exit.__\n");
						msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
							.setFooter("Type 'remove <trackNumber>' to remove a track.\nType 'swap <sourceTrackNumber> "
									   "<destinationTrackNumber>' to swap tracks.\nType "
									   "'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
						newEvent = *buttonCollectedData.at(0).interactionData;
						unique_ptr<respond_to_input_event_data> dataPackage03{ makeUnique<respond_to_input_event_data>(*buttonCollectedData.at(0).interactionData) };
						dataPackage03->setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage03->addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
						dataPackage03->addContent("");
						newEvent = input_events::respondToInputEventAsync(*dataPackage03).get();
						while (!doWeQuit) {
							unique_ptr<respond_to_input_event_data> dataPackage02{ makeUnique<respond_to_input_event_data>(newEvent) };
							dataPackage02->setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage02->addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
							dataPackage02->addContent("");
							newEvent = input_events::respondToInputEventAsync(*dataPackage02).get();

							auto messageFilter = [=](message_data message) -> bool {
								if (userID == message.author.id) {
									return true;
								} else {
									return false;
								}
							};
							user_data theUser = argsNew.getUserData();
							unique_ptr<object_collector<message_data>> messageCollector{ makeUnique<object_collector<message_data>>() };
							auto returnedMessages = messageCollector->collectObjects(1, 120000, messageFilter).get();
							if (returnedMessages.objects.size() == 0) {
								auto inputEventData = argsNew.getInputEventData();
								msgEmbeds.erase(msgEmbeds.begin() + static_cast<int64_t>(currentPageIndex));
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, newEvent, theUser, static_cast<uint64_t>(currentPageIndex));
								doWeQuit  = true;
								break;
							}
							jsonifier::vector<jsonifier::string> args2;
							jsonifier::string newString = convertToLowerCase(returnedMessages.objects.at(0).content);
							std::regex wordRegex("[a-z]{1,12}");
							std::smatch wordRegexMatch;
							jsonifier::string newerString = newString;
							std::string newestString	  = newerString.operator std::string();
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
								delete_message_data dataPackage{ returnedMessages.objects.at(0) };
								messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + static_cast<int64_t>(currentPageIndex));
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, newEvent, theUser, static_cast<uint64_t>(currentPageIndex));
								doWeQuit  = true;
								discordGuild->writeDataToDB();
								break;
							} else if (convertToLowerCase(args2.at(0)) != "remove" && convertToLowerCase(args2.at(0)) != "swap" && convertToLowerCase(args2.at(0)) != "exit" &&
								convertToLowerCase(args2.at(0)) != "shuffle") {
								msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
									.setDescription("__**Please enter a proper input!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
													"<sourceTrackNumber> "
													"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType exit "
													"to exit.__\n");
								msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
									.setFooter("Please enter a proper input!\nType 'remove <trackNumber>' to remove a track.\nType "
											   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
											   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
								delete_message_data dataPackage{ returnedMessages.objects.at(0) };
								messages::deleteMessageAsync(dataPackage);
								respond_to_input_event_data dataPackage04(newEvent);
								dataPackage04.setResponseType(input_event_response_type::Edit_Interaction_Response);
								dataPackage04.addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
								dataPackage04.addContent("");
								newEvent = input_events::respondToInputEventAsync(dataPackage04).get();
								continue;
							} else if (convertToLowerCase(args2.at(0)) == "remove") {
								if (args2.size() < 2 || !regex_search(args2.at(1).data(), digitRegex)) {
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setDescription("__**Please enter a proper input!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setFooter("Please enter a proper input!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									delete_message_data dataPackage{ returnedMessages.objects.at(0) };
									messages::deleteMessageAsync(dataPackage);
									respond_to_input_event_data dataPackage04(newEvent);
									dataPackage04.setResponseType(input_event_response_type::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
									dataPackage04.addContent("");
									newEvent = input_events::respondToInputEventAsync(dataPackage04).get();
									continue;
								}
								if ((jsonifier::strToInt64(args2.at(1)) - 1) < 0 ||
									static_cast<uint64_t>(jsonifier::strToInt64(args2.at(1)) - 1) >= discordGuild->data.playlist.songQueue.size() || args2.size() < 1) {
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setDescription("__**Please enter a proper input!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setFooter("Please enter a proper input!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									delete_message_data dataPackage{ returnedMessages.objects.at(0) };
									messages::deleteMessageAsync(dataPackage);
									respond_to_input_event_data dataPackage04(newEvent);
									dataPackage04.setResponseType(input_event_response_type::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
									dataPackage04.addContent("");
									newEvent = input_events::respondToInputEventAsync(dataPackage04).get();
									continue;
								}
								int64_t removeIndex = jsonifier::strToInt64(args2.at(1)) - 1;

								discordGuild->data.playlist.songQueue.erase(discordGuild->data.playlist.songQueue.begin() + removeIndex);
								delete_message_data dataPackage{ returnedMessages.objects.at(0) };
								messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + static_cast<int64_t>(currentPageIndex));
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, newEvent, theUser, static_cast<uint64_t>(currentPageIndex));
								doWeQuit  = true;
								discordGuild->writeDataToDB();
								break;
							} else if (convertToLowerCase(args2.at(0)) == "swap") {
								if (args2.size() < 3 || !regex_search(args2.at(1).data(), digitRegex) || !regex_search(args2.at(2).data(), digitRegex)) {
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setDescription("__**Please enter a proper input!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setFooter("Please enter a proper input!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									delete_message_data dataPackage{ returnedMessages.objects.at(0) };
									messages::deleteMessageAsync(dataPackage);
									respond_to_input_event_data dataPackage04(newEvent);
									dataPackage04.setResponseType(input_event_response_type::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
									dataPackage04.addContent("");
									newEvent = input_events::respondToInputEventAsync(dataPackage04).get();
									continue;
								}
								if (args2.size() < 2 ||
									((jsonifier::strToInt64(args2.at(1)) - 1) < 0 ||
										static_cast<uint64_t>(jsonifier::strToInt64(args2.at(1)) - 1) >= discordGuild->data.playlist.songQueue.size() ||
										(jsonifier::strToInt64(args2.at(2)) - 1) < 0 ||
										static_cast<uint64_t>(jsonifier::strToInt64(args2.at(2)) - 1) >= discordGuild->data.playlist.songQueue.size() || args2.size() < 2)) {
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setDescription("__**Please enter a proper input!**__\n__Type 'remove <trackNumber>' to remove a track.\nType 'swap "
														"<sourceTrackNumber> "
														"<destinationTrackNumber>' to swap tracks.\nType 'shuffle' to shuffle the playlist.\nType "
														"exit to exit.__\n");
									msgEmbeds.at(static_cast<uint64_t>(currentPageIndex))
										.setFooter("Please enter a proper input!\nType 'remove <trackNumber>' to remove a track.\nType "
												   "'swap <sourceTrackNumber> <destinationTrackNumber>' to swap "
												   "tracks.\nType 'shuffle' to shuffle the playlist.\nType 'exit' to exit.");
									delete_message_data dataPackage{ returnedMessages.objects.at(0) };
									messages::deleteMessageAsync(dataPackage);
									respond_to_input_event_data dataPackage04(newEvent);
									dataPackage04.setResponseType(input_event_response_type::Edit_Interaction_Response);
									dataPackage04.addMessageEmbed(msgEmbeds.at(static_cast<uint64_t>(currentPageIndex)));
									dataPackage04.addContent("");
									newEvent = input_events::respondToInputEventAsync(dataPackage04).get();
									continue;
								}

								uint64_t sourceIndex		 = ( uint64_t )jsonifier::strToInt64(args2.at(1)) - 1;
								uint64_t destinationIndex = ( uint64_t )jsonifier::strToInt64(args2.at(2)) - 1;
								discordGuild->data.playlist.modifyQueue(sourceIndex, destinationIndex);
								delete_message_data dataPackage{ returnedMessages.objects.at(0) };
								messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + static_cast<int64_t>(currentPageIndex));
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, newEvent, theUser, static_cast<uint64_t>(currentPageIndex));
								doWeQuit  = true;
								discordGuild->writeDataToDB();
								break;
							} else if (convertToLowerCase(args2.at(0)) == "shuffle") {
								auto oldSongArray = discordGuild->data.playlist;
								jsonifier::vector<song> newVector{};
								while (oldSongArray.songQueue.size() > 0) {
									std::mt19937_64 randomEngine{ static_cast<uint64_t>(
										std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) };
									int64_t randomIndex = static_cast<int64_t>(
										(static_cast<float>(randomEngine()) / static_cast<float>(randomEngine.max()) * static_cast<float>(oldSongArray.songQueue.size())));
									newVector.emplace_back(oldSongArray.songQueue.at(static_cast<uint64_t>(randomIndex)));
									oldSongArray.songQueue.erase(oldSongArray.songQueue.begin() + randomIndex);
								}
								oldSongArray.songQueue		= newVector;
								discordGuild->data.playlist = oldSongArray;
								delete_message_data dataPackage{ returnedMessages.objects.at(0) };
								messages::deleteMessageAsync(dataPackage);
								msgEmbeds.erase(msgEmbeds.begin() + static_cast<int64_t>(currentPageIndex));
								msgEmbeds = updateMessageEmbeds(discordGuild->data.playlist.songQueue, newEvent, theUser, static_cast<uint64_t>(currentPageIndex));
								doWeQuit  = true;
								discordGuild->writeDataToDB();
								break;
							}
						}
						if (doWeQuit) {
							discordGuild->writeDataToDB();
							continue;
						}
					};
					return;
				}
			} catch (const std::runtime_error& error) {
				std::cout << "queue::execute()" << error.what() << std::endl;
			};
		};
		~the_queue(){};
	};

};// namespace discord_core_api
