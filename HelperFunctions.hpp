// HelperFunctions.hpp - Header for some helper functions.
// May 28, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "DatabaseEntities.hpp"

namespace DiscordCoreAPI {

	bool checkIfAllowedPlayingInChannel(InputEventData eventData, DiscordGuild discordGuild) {
		bool isItFound = true;
		jsonifier::string msgString{};
		EmbedData msgEmbed{};
		auto channelData = eventData.getChannelData();
		if (discordGuild.data.musicChannelIds.size() > 0) {
			isItFound			  = false;
			
			for (auto& value: discordGuild.data.musicChannelIds) {
				if (eventData.getChannelData().id == value && channelData.nsfw) {
					isItFound = true;
					break;
				} else if (eventData.getChannelData().id == value && !channelData.getFlagValue(ChannelFlags::NSFW)) {
					msgString = "------\n**Sorry, but please do that in an NSFW channel.**\n";
					break;
				} else {
					msgString = "------\n**Sorry, but please do that in one of the following channels:**\n------\n";
					msgString += "<#" + value + ">\n";
				}
			}
		}
		if (!channelData.nsfw) {
			msgString = "------\n**Sorry, but please do that in an NSFW channel.**\n";
			isItFound = false;
		}
		msgString += "------";
		if (isItFound == false) {
			msgEmbed.setAuthor(eventData.getUserData().userName, eventData.getUserData().getUserImageUrl(UserImageTypes::Avatar));
			msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
			msgEmbed.setDescription(msgString);
			msgEmbed.setTitle("__**Permissions Issue:**__");
			RespondToInputEventData replyMessageData{ eventData };
			replyMessageData.addMessageEmbed(msgEmbed);

			if (eventData.responseType != InputEventResponseType::Unset) {
				InputEvents::deleteInputEventResponseAsync(eventData).get();
				replyMessageData.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
			} else {
				replyMessageData.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
			}

			InputEvents::respondToInputEventAsync(replyMessageData).get();
		}
		return isItFound;
	}

	bool checkIfWeHaveControl(InputEventData eventData, DiscordGuild guildData, GuildMemberCacheData guildMember) {
		if (guildData.data.djRoleId == 0) {
			return true;
		}
		bool doWeHaveControl = false;
		DiscordGuildMember guildMemberData(managerAgent, guildMember);

		auto myRoles =
			Roles::getGuildMemberRolesAsync({ .guildMember = guildMember, .guildId = Snowflake{ static_cast<uint64_t>(guildData.data.guildId) } })
				.get();

		for (auto& value: myRoles) {
			if (value.id == Snowflake{ static_cast<uint64_t>(guildData.data.djRoleId) }) {
				doWeHaveControl = true;
			}
		}

		if (!doWeHaveControl) {
			jsonifier::string msgString = "------\n**Sorry, but you lack the permissions to do that!**\n------";
			EmbedData msgEmbed{};
			msgEmbed.setAuthor(guildMember.getUserData().userName, guildMember.getGuildMemberImageUrl(GuildMemberImageTypes::Avatar));
			msgEmbed.setDescription(msgString);
			msgEmbed.setColor(jsonifier::string{ guildData.data.borderColor });
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("Permissions Issue");
			RespondToInputEventData dataPackage{ eventData };
			dataPackage.addMessageEmbed(msgEmbed);
			dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
			InputEvents::respondToInputEventAsync(dataPackage).get();
		}
		return doWeHaveControl;
	}

	bool checkForBotCommanderStatus(GuildMemberCacheData guildMember, DiscordUser& discordUser) {
		bool areWeACommander;
		for (auto& value: discordUser.data.botCommanders) {
			if (guildMember.user.id == value) {
				areWeACommander = true;
				return areWeACommander;
				break;
			}
		}
		return false;
	}

	bool doWeHaveAdminPermissions(BaseFunctionArguments& argsNew, InputEventData& eventData, DiscordGuild& discordGuild, ChannelCacheData& channel,
		GuildMemberCacheData& guildMember, bool displayResponse = true) {
		RespondToInputEventData dataPackage{ eventData };
		dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
		InputEvents::respondToInputEventAsync(dataPackage).get();
		bool doWeHaveAdmin = Permissions{ guildMember.permissions }.checkForPermission(guildMember, channel, Permission::Administrator);
		if (doWeHaveAdmin) {
			return true;
		}
		DiscordUser discordUser(managerAgent, jsonifier::string{ DiscordCoreClient::getInstance()->getBotUser().userName }, DiscordCoreClient::getInstance()->getBotUser().id);
		bool areWeACommander = checkForBotCommanderStatus(guildMember, discordUser);

		if (areWeACommander) {
			return true;
		}

		if (displayResponse) {
			jsonifier::string msgString = "------\n**Sorry, but you don't have the permissions required for that!**\n------";
			EmbedData msgEmbed{};
			msgEmbed.setAuthor(eventData.getUserData().userName, eventData.getUserData().getUserImageUrl(UserImageTypes::Avatar));
			msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
			msgEmbed.setDescription(msgString);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("__**Permissions Issue:**__");
			RespondToInputEventData dataPackageNew{ eventData };
			dataPackageNew.addMessageEmbed(msgEmbed);
			dataPackageNew.setResponseType(InputEventResponseType::Edit_Interaction_Response);
			eventData = InputEvents::respondToInputEventAsync(dataPackageNew).get();
		}
		return false;
	}
}
