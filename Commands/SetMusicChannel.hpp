// SetMusicChannel.hpp - Header for the "set music channel" command.
// Jun 30, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class SetMusicChannel : public BaseFunction {
	  public:
		SetMusicChannel() {
			this->commandName	  = "setmusicchannel";
			this->helpDescription = "Sets the channels from which you can issue music commands!";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /setmusichannel add in order to add the current channel.\nAlternatively enter /setmusicchannel "
									"remove to remove the current channel.\nAlso, enter "
									"/setmusicchannel view or purge to view or purge the currently enabled channels.\n------");
			msgEmbed.setTitle("__**Set Music Channel Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<SetMusicChannel>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelCacheData channel{ argsNew.getChannelData() };

				GuildCacheData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };
				GuildMemberCacheData guildMember{ argsNew.getGuildMemberData() };
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember);

				if (doWeHaveAdminPermission == false) {
					return;
				}

				if (argsNew.getSubCommandName() == "add") {
					Snowflake channelID = channel.id;
					for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						if (channelID == discordGuild.data.musicChannelIds.at(x)) {
							jsonifier::string msgString = "------\n**That channel is already on the list of enabled channels!**\n------";
							EmbedData messageEmbed;
							messageEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
							messageEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
							messageEmbed.setTimeStamp(getTimeAndDate());
							messageEmbed.setDescription(msgString);
							messageEmbed.setTitle("__**Already Listed:**__");
							RespondToInputEventData dataPackage(argsNew.getInputEventData());
							dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
							dataPackage.addMessageEmbed(messageEmbed);
							auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
							InputEvents::deleteInputEventResponseAsync(newEvent, 20000);
							return;
						}
					}

					discordGuild.data.musicChannelIds.emplace_back(channelID);
					discordGuild.writeDataToDB(managerAgent);
					EmbedData messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					messageEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription("------\n**You've succesfully added <#" + channelID + "> to your list of accepted music channels!**\n------");
					messageEmbed.setTitle("__**Music Channel Added:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (argsNew.getSubCommandName() == "remove") {
					Snowflake channelID;
					channelID = channel.id;

					jsonifier::string msgString;
					bool isItPresent = false;
					for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						if (channelID == discordGuild.data.musicChannelIds.at(x)) {
							isItPresent = true;
							discordGuild.data.musicChannelIds.erase(discordGuild.data.musicChannelIds.begin() + x);
							discordGuild.writeDataToDB(managerAgent);
							msgString += "------\n**You've succesfully removed the channel <#" + channelID + "> from the list of enabled music channels!**\n------";
						}
					}

					if (isItPresent == false) {
						jsonifier::string msgString2 = "------\n**That channel is not present on the list of enabled music channels!**\n------";
						EmbedData messageEmbed;
						messageEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
						messageEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
						messageEmbed.setTimeStamp(getTimeAndDate());
						messageEmbed.setDescription(msgString2);
						messageEmbed.setTitle("__**Missing from List:**__");
						RespondToInputEventData dataPackage(argsNew.getInputEventData());
						dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
						dataPackage.addMessageEmbed(messageEmbed);
						auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
						InputEvents::deleteInputEventResponseAsync(newEvent, 20000);
						return;
					}

					EmbedData messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					messageEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**Music Channel Removed:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (argsNew.getSubCommandName() == "purge") {
					jsonifier::string msgString;

					if (discordGuild.data.musicChannelIds.size() > 0) {
						msgString = "__You've removed the following channels from your list of enabled music channels:__\n------\n";

						for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
							Snowflake currentID = discordGuild.data.musicChannelIds.at(x);

							msgString += "__**Channel #" + jsonifier::toString(x) + "**__<#" + currentID + "> \n";
						}

						msgString += "------\n__**The music commands will now work in ANY CHANNEL!**__";

						discordGuild.data.musicChannelIds = jsonifier::vector<Snowflake>();
						discordGuild.writeDataToDB(managerAgent);
					} else {
						msgString += "------\n**Sorry, but there are no channels to remove!**\n------";
					}

					EmbedData messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					messageEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**Music Channels Removed:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
				if (argsNew.getSubCommandName() == "view") {
					jsonifier::string msgString = "__You have the following channels enabled for music commands, on this server:__\n------\n";

					for (int32_t x = 0; x < discordGuild.data.musicChannelIds.size(); x += 1) {
						Snowflake currentID = discordGuild.data.musicChannelIds.at(x);

						msgString += "__**Channel #" + jsonifier::toString(x) + ":**__ <#" + currentID + "> \n";
					}

					msgString += "------\n";

					EmbedData messageEmbed;
					messageEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					messageEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					messageEmbed.setTimeStamp(getTimeAndDate());
					messageEmbed.setDescription(msgString);
					messageEmbed.setTitle("__**Music Channels Enabled:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(messageEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "SetMusicChannel::execute()" << error.what() << std::endl;
			}
		}
		~SetMusicChannel(){};
	};
}// namespace DiscordCoreAPI
