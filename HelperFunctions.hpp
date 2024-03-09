// HelperFunctions.hpp - header for some helper functions.
// may 28, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "DatabaseEntities.hpp"

namespace discord_core_api {

	bool checkIfAllowedPlayingInChannel(input_event_data eventData, discord_guild discordGuild) {
		bool isItFound = true;
		if (discordGuild.data.musicChannelIds.size() > 0) {
			isItFound = false;
			jsonifier::string msgString = "------\n**Sorry, but please do that in one of the following channels:**\n------\n";
			embed_data msgEmbed{};
			for (auto& value: discordGuild.data.musicChannelIds) {
				if (eventData.getChannelData().id == value) {
					isItFound = true;
					break;
				} else {
					msgString += "<#" + value + ">\n";
				}
			}
			msgString += "------";
			if (isItFound == false) {
				msgEmbed.setAuthor(eventData.getUserData().userName, eventData.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.setColor("fefefe");
				msgEmbed.setDescription(msgString);
				msgEmbed.setTitle("__**Permissions Issue:**__");
				respond_to_input_event_data replyMessageData{ eventData };
				replyMessageData.addMessageEmbed(msgEmbed);

				if (eventData.responseType != input_event_response_type::Unset) {
					input_events::deleteInputEventResponseAsync(eventData).get();
					replyMessageData.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
				} else {
					replyMessageData.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
				}

				input_events::respondToInputEventAsync(replyMessageData).get();
			}
		}
		return isItFound;
	}

	bool checkIfWeHaveControl(input_event_data eventData, discord_guild guildData, guild_member_data guildMember) {
		if (guildData.data.djRoleId == 0) {
			return true;
		}
		bool doWeHaveControl = false;
		discord_guild_member guildMemberData{ guildMember };

		auto myRoles =
			roles::getGuildMemberRolesAsync({ .guildMember = guildMember, .guildId = snowflake{ static_cast<uint64_t>(guildData.data.guildId) } })
				.get();

		for (auto& value: myRoles) {
			if (value.id == snowflake{ static_cast<uint64_t>(guildData.data.djRoleId) }) {
				doWeHaveControl = true;
			}
		}

		if (!doWeHaveControl) {
			jsonifier::string msgString = "------\n**Sorry, but you lack the permissions to do that!**\n------";
			embed_data msgEmbed{};
			msgEmbed.setAuthor(guildMember.getUserData().userName, guildMember.getGuildMemberImageUrl(guild_member_image_types::Avatar));
			msgEmbed.setDescription(msgString);
			msgEmbed.setColor(guildData.data.borderColor);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("Permissions Issue");
			respond_to_input_event_data dataPackage{ eventData };
			dataPackage.addMessageEmbed(msgEmbed);
			dataPackage.setResponseType(input_event_response_type::Ephemeral_Interaction_Response);
			input_events::respondToInputEventAsync(dataPackage).get();
		}
		return doWeHaveControl;
	}

	bool checkForBotCommanderStatus(guild_member_data guildMember, discord_user& discordUser) {
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

	bool doWeHaveAdminPermissions(input_event_data& eventData, channel_data& channel, guild_member_data& guildMember, bool displayResponse = true) {
		respond_to_input_event_data dataPackage{ eventData };
		dataPackage.setResponseType(input_event_response_type::Ephemeral_Deferred_Response);
		input_events::respondToInputEventAsync(dataPackage).get();
		bool doWeHaveAdmin = permissions{ guildMember.permissions }.checkForPermission(guildMember, channel, permission::administrator);
		if (doWeHaveAdmin) {
			return true;
		}
		discord_user discordUser(discord_core_client::getInstance()->getBotUser().userName,  discord_core_client::getInstance()->getBotUser().id);
		bool areWeACommander = checkForBotCommanderStatus(guildMember, discordUser);

		if (areWeACommander) {
			return true;
		}

		if (displayResponse) {
			jsonifier::string msgString = "------\n**Sorry, but you don't have the permissions required for that!**\n------";
			embed_data msgEmbed{};
			msgEmbed.setAuthor(eventData.getUserData().userName,  eventData.getUserData().getUserImageUrl(user_image_types::Avatar));
			msgEmbed.setColor("fefefe");
			msgEmbed.setDescription(msgString);
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setTitle("__**Permissions Issue:**__");
			respond_to_input_event_data dataPackageNew{ eventData };
			dataPackageNew.addMessageEmbed(msgEmbed);
			dataPackageNew.setResponseType(input_event_response_type::Edit_Interaction_Response);
			eventData = input_events::respondToInputEventAsync(dataPackageNew).get();
		}
		return false;
	}
}
