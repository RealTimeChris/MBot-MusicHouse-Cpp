// HelperFunctions.hpp - Header for some helper functions.
// May 28, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "DatabaseEntities.hpp"

namespace DiscordCoreAPI {

	bool checkIfAllowedPlayingInChannel(InputEventData eventData, DiscordGuild discordGuild) {
		bool isItFound = true;
		if (discordGuild.data.musicChannelIds.size() > 0) {
			isItFound = false;
			std::string msgString = "------\n**Sorry, but please do that in one of the following channels:**\n------\n";
			EmbedData msgEmbed{};
			for (auto& value: discordGuild.data.musicChannelIds) {
				if (eventData.getChannelId() == value) {
					isItFound = true;
					break;
				} else {
					msgString += "<#" + std::to_string(value) + ">\n";
				}
			}
			msgString += "------";
			if (isItFound == false) {
				msgEmbed.setAuthor(eventData.getUserName(), eventData.getAvatarUrl());
				msgEmbed.setColor(discordGuild.data.borderColor);
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
		}
		return isItFound;
	}

	bool checkIfWeHaveControl(InputEventData eventData, DiscordGuild guildData, GuildMember guildMember) {
		if (guildData.data.djRoleId == 0) {
			return true;
		}
		bool doWeHaveControl = false;
		DiscordGuildMember guildMemberData(guildMember);

		auto myRoles = Roles::getGuildMemberRolesAsync({ .guildMember = guildMember, .guildId = guildData.data.guildId }).get();

		for (auto& value: myRoles) {
			if (value.id == guildData.data.djRoleId) {
				doWeHaveControl = true;
			}
		}

		if (!doWeHaveControl) {
			std::string msgString = "------\n**Sorry, but you lack the permissions to do that!**\n------";
			EmbedData msgEmbed{};
			msgEmbed.setAuthor(guildMember.userName, guildMember.userAvatar);
			msgEmbed.setDescription(msgString);
			msgEmbed.setColor(guildData.data.borderColor);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("Permissions Issue");
			RespondToInputEventData dataPackage{ eventData };
			dataPackage.addMessageEmbed(msgEmbed);
			dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
			InputEvents::respondToInputEventAsync(dataPackage).get();
		}
		return doWeHaveControl;
	}

	bool checkForBotCommanderStatus(GuildMember guildMember, DiscordUser& discordUser) {
		bool areWeACommander;
		for (auto& value: discordUser.data.botCommanders) {
			if (guildMember.id == value) {
				areWeACommander = true;
				return areWeACommander;
				break;
			}
		}
		return false;
	}

	bool doWeHaveAdminPermissions(BaseFunctionArguments newArgs, InputEventData eventData, DiscordGuild discordGuild, Channel channel, GuildMember guildMember,
		bool displayResponse = true) {
		bool doWeHaveAdmin = guildMember.permissions.checkForPermission(guildMember, channel, Permission::Administrator);

		if (doWeHaveAdmin) {
			return true;
		}
		DiscordCoreAPI::DiscordUser discordUser(newArgs.discordCoreClient->getBotUser().userName, newArgs.discordCoreClient->getBotUser().id);
		bool areWeACommander = checkForBotCommanderStatus(guildMember, discordUser);

		if (areWeACommander) {
			return true;
		}

		if (displayResponse) {
			std::string msgString = "------\n**Sorry, but you don't have the permissions required for that!**\n------";
			EmbedData msgEmbed{};
			msgEmbed.setAuthor(guildMember.userName, guildMember.userAvatar);
			msgEmbed.setColor(discordGuild.data.borderColor);
			msgEmbed.setDescription(msgString);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("__**Permissions Issue:**__");
			RespondToInputEventData dataPackage{ eventData };
			dataPackage.addMessageEmbed(msgEmbed);
			dataPackage.setResponseType(InputEventResponseType::Ephemeral_Interaction_Response);
			InputEvents::respondToInputEventAsync(dataPackage).get();
		}
		return false;
	}
}
