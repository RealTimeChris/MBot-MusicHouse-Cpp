// set_music_channel.hpp - header for the "set music channel" command.
// jun 30, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "../HelperFunctions.hpp"

namespace discord_core_api {

	class set_music_channel : public base_function {
	  public:
		set_music_channel() {
			this->commandName	  = "setmusicchannel";
			this->helpDescription = "sets the channels from which you can issue music commands!";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /setmusichannel add in order to add the current channel.\nAlternatively enter /setmusicchannel "
									"remove to remove the current channel.\nAlso, enter "
									"/setmusicchannel view or purge to view or purge the currently enabled channels.\n------");
			msgEmbed.setTitle("__**set music channel usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<set_music_channel>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_cache_data channel{ argsNew.getChannelData() };

				guild_cache_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ managerAgent, guild };
				guild_member_cache_data guildMember{ argsNew.getGuildMemberData() };
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember);

				if (doWeHaveAdminPermission == false) {
					return;
				}

				if (argsNew.getSubCommandName() == "add") {
					snowflake channelID = channel.id;
					for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						if (channelID == discordGuild.data.musicChannelIds.at(x)) {
							jsonifier::string msgString = "------\n**that channel is already on the list of enabled channels!**\n------";
							embed_data messageEmbed;
							messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
							messageEmbed.setColor("fefefe");
							messageEmbed.setTimeStamp(getTimeAndDate());
							messageEmbed.setDescription(msgString);
							messageEmbed.setTitle("__**already listed:**__");
							respond_to_input_event_data dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(messageEmbed);
							auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
							input_events::deleteInputEventResponseAsync(newEvent, 20000);
							return;
						}
					}

					discordGuild.data.musicChannelIds.emplace_back(channelID);
					discordGuild.writeDataToDB(managerAgent);
					embed_data messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					messageEmbed.setColor("fefefe");
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription("------\n**you've succesfully added <#" + channelID + "> to your list of accepted music channels!**\n------");
					messageEmbed.setTitle("__**music channel added:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (argsNew.getSubCommandName() == "remove") {
					snowflake channelID;
					channelID = channel.id;

					jsonifier::string msgString;
					bool isItPresent = false;
					for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						if (channelID == discordGuild.data.musicChannelIds.at(x)) {
							isItPresent = true;
							discordGuild.data.musicChannelIds.erase(discordGuild.data.musicChannelIds.begin() + x);
							discordGuild.writeDataToDB(managerAgent);
							msgString += "------\n**you've succesfully removed the channel <#" + channelID + "> from the list of enabled music channels!**\n------";
						}
					}

					if (isItPresent == false) {
						jsonifier::string msgString2 = "------\n**that channel is not present on the list of enabled music channels!**\n------";
						embed_data messageEmbed;
						messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
						messageEmbed.setColor("fefefe");
						messageEmbed.setTimeStamp(getTimeAndDate());
						messageEmbed.setDescription(msgString2);
						messageEmbed.setTitle("__**missing from list:**__");
						respond_to_input_event_data dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(messageEmbed);
						auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
						input_events::deleteInputEventResponseAsync(newEvent, 20000);
						return;
					}

					embed_data messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					messageEmbed.setColor("fefefe");
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**music channel removed:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (argsNew.getSubCommandName() == "purge") {
					jsonifier::string msgString;

					if (discordGuild.data.musicChannelIds.size() > 0) {
						msgString = "__You've removed the following channels from your list of enabled music channels:__\n------\n";

						for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
							snowflake currentID = discordGuild.data.musicChannelIds.at(x);

							msgString += "__**channel #" + jsonifier::toString(x) + "**__<#" + currentID + "> \n";
						}

						msgString += "------\n__**the music commands will now work in any channel!**__";

						discordGuild.data.musicChannelIds = jsonifier::vector<snowflake>();
						discordGuild.writeDataToDB(managerAgent);
					} else {
						msgString += "------\n**Sorry, but there are no channels to remove!**\n------";
					}

					embed_data messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					messageEmbed.setColor("fefefe");
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**music channels removed:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (argsNew.getSubCommandName() == "view") {
					jsonifier::string msgString = "__You have the following channels enabled for music commands, on this server:__\n------\n";

					for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						snowflake currentID = discordGuild.data.musicChannelIds.at(x);

						msgString += "__**channel #" + jsonifier::toString(x) + ":**__ <#" + currentID + "> \n";
					}

					msgString += "------\n";

					embed_data messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					messageEmbed.setColor("fefefe");
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**music channels enabled:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "set_music_channel::execute()" << error.what() << std::endl;
			}
		}
		~set_music_channel(){};
	};
}// namespace discord_core_api
