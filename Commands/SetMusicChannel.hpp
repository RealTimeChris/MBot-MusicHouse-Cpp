// set_music_channel.hpp - header for the "set music channel" command.
// jun 30, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class set_music_channel : public base_function {
	  public:
		set_music_channel() {
			this->commandName	  = "setmusicchannel";
			this->helpDescription = "Sets the channels from which you can issue music commands!";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /setmusichannel add in order to add the current channel.\nAlternatively enter /setmusicchannel "
									"remove to remove the current channel.\nAlso, enter "
									"/setmusicchannel view or purge to view or purge the currently enabled channels.\n------");
			msgEmbed.setTitle("__**Set Music Channel Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<set_music_channel>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ guild };
				guild_member_data guildMember{ argsNew.getGuildMemberData() };
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(inputEventData, channel, guildMember);

				if (doWeHaveAdminPermission == false) {
					return;
				}

				if (argsNew.getSubCommandName() == "add") {
					snowflake channelID = channel.id;
					for (uint64_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						if (channelID == discordGuild.data.musicChannelIds.at(x)) {
							jsonifier::string msgString = "------\n**That channel is already on the list of enabled channels!**\n------";
							embed_data messageEmbed;
							messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
							messageEmbed.setColor("fefefe");
							messageEmbed.setTimeStamp(getTimeAndDate());
							messageEmbed.setDescription(msgString);
							messageEmbed.setTitle("__**Already Listed:**__");
							respond_to_input_event_data dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(messageEmbed);
							auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
							input_events::deleteInputEventResponseAsync(newEvent, 20000);
							return;
						}
					}

					discordGuild.data.musicChannelIds.emplace_back(channelID);
					discordGuild.writeDataToDB();
					embed_data messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					messageEmbed.setColor("fefefe");
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription("------\n**You've succesfully added <#" + channelID + "> to your list of accepted music channels!**\n------");
					messageEmbed.setTitle("__**Music Channel Added:**__");
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
					for (uint64_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						if (channelID == discordGuild.data.musicChannelIds.at(x)) {
							isItPresent = true;
							discordGuild.data.musicChannelIds.erase(discordGuild.data.musicChannelIds.begin() + x);
							discordGuild.writeDataToDB();
							msgString += "------\n**You've succesfully removed the channel <#" + channelID + "> from the list of enabled music channels!**\n------";
						}
					}

					if (isItPresent == false) {
						jsonifier::string msgString2 = "------\n**That channel is not present on the list of enabled music channels!**\n------";
						embed_data messageEmbed;
						messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
						messageEmbed.setColor("fefefe");
						messageEmbed.setTimeStamp(getTimeAndDate());
						messageEmbed.setDescription(msgString2);
						messageEmbed.setTitle("__**Missing From List:**__");
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
					messageEmbed.setTitle("__**Music Channel Removed:**__");
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

						for (uint64_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
							snowflake currentID = discordGuild.data.musicChannelIds.at(x);

							msgString += "__**Channel #" + jsonifier::toString(x) + "**__<#" + currentID + "> \n";
						}

						msgString += "------\n__**The music commands will now work in any channel!**__";

						discordGuild.data.musicChannelIds = jsonifier::vector<snowflake>();
						discordGuild.writeDataToDB();
					} else {
						msgString += "------\n**Sorry, but there are no channels to remove!**\n------";
					}

					embed_data messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					messageEmbed.setColor("fefefe");
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**Music Channels Removed:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (argsNew.getSubCommandName() == "view") {
					jsonifier::string msgString = "__You have the following channels enabled for music commands, on this server:__\n------\n";

					for (uint64_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						snowflake currentID = discordGuild.data.musicChannelIds.at(x);

						msgString += "__**Channel #" + jsonifier::toString(x) + ":**__ <#" + currentID + "> \n";
					}

					msgString += "------\n";

					embed_data messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					messageEmbed.setColor("fefefe");
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**Music Channels Enabled:**__");
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
