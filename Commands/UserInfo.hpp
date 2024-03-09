// user_info.hpp - header for the "user info" command.
// aug 13, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class user_info : public base_function {
	  public:
		user_info() {
			this->commandName	  = "userinfo";
			this->helpDescription = "displays some info about a chosen user.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /userinfo, or /userinfo @usermention, to display the info of another user.\n------");
			msgEmbed.setTitle("__**user_data info Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<user_info>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				guild_data guild{ argsNew.getInteractionData().guildId };

				discord_guild discordGuild{ guild };
				snowflake userId{ argsNew.getCommandArguments().values["user"].operator size_t() };
				guild_member_data guildMember = guild_members::getCachedGuildMember({ .guildMemberId = userId, .guildId = guild.id });
				user_data theUser			 = users::getCachedUser({ .userId = argsNew.getUserData().id });
				jsonifier::vector<embed_field_data> fields;
				embed_field_data field{};
				field.Inline = true;
				field.value	 = guildMember.getUserData().userName  + "#" + theUser.discriminator;
				field.name	 = "__User tag: __";
				fields.emplace_back(field);
				embed_field_data field1{};
				field1.Inline = true;
				field1.value  = guildMember.getUserData().userName;
				field1.name	  = "__User name:__";
				fields.emplace_back(field1);
				if (guildMember.nick == "") {
					embed_field_data field2{};
					field2.Inline = true;
					field2.value  = guildMember.getUserData().userName;
					field2.name	  = "__Display name:__";
					fields.emplace_back(field2);
				} else {
					embed_field_data field2{};
					field2.Inline = true;
					field2.value  = guildMember.nick;
					field2.name	  = "__Display name:__";
					fields.emplace_back(field2);
				}

				embed_field_data field3{};
				field3.Inline = true;
				field3.value  = guildMember.user.id.operator jsonifier::string();
				field3.name	  = "__User id:__";
				fields.emplace_back(field3);
				embed_field_data field5{};
				field5.Inline = true;
				field5.value  = guildMember.user.id.operator jsonifier::string();
				field5.name	  = "__Created at:__";
				fields.emplace_back(field5);
				permissions permsString							= permissions{ permissions::getCurrentChannelPermissions(guildMember, channel) };
				jsonifier::vector<jsonifier::string> permissionsArray = permsString.displayPermissions();
				jsonifier::string msgString;
				for (uint64_t x = 0; x < permissionsArray.size(); x += 1) {
					msgString += permissionsArray.at(x);
					if (x < permissionsArray.size() - 1) {
						msgString += ", ";
					}
				}
				embed_field_data field6{};
				field6.Inline = false;
				field6.value  = "";
				field6.name	  = "__Roles:__";

				for (uint64_t x = 0; x < guildMember.roles.size(); x += 1) {
					field6.value += "<@&" + guildMember.roles.at(x) + ">";
					if (x < guildMember.roles.size() - 1) {
						field6.value += ", ";
					}
				}
				fields.emplace_back(field6);
				embed_field_data field7{};
				field7.Inline = false;
				field7.value  = msgString;
				field7.name	  = "__Permissions:__";
				fields.emplace_back(field7);

				embed_data msgEmbed;
				msgEmbed.setColor("fefefe");
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**user_data info:**__");
				msgEmbed.setImage(guildMember.getGuildMemberImageUrl(guild_member_image_types::Avatar) + "?size=4096");
				msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				msgEmbed.fields = fields;
				respond_to_input_event_data dataPackage02(argsNew.getInputEventData());
				dataPackage02.addMessageEmbed(msgEmbed);
				dataPackage02.setResponseType(input_event_response_type::Interaction_Response);
				auto eventNew = input_events::respondToInputEventAsync(dataPackage02).get();

				return;
			} catch (const std::runtime_error& error) {
				std::cout << "user_info::execute()" << error.what() << std::endl;
			}
		}
		~user_info(){};
	};

}// namespace discord_core_api
