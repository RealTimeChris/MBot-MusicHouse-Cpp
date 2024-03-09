// test.hpp - header for the "test" command.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class test : public base_function {
	  public:
		test() {
			this->commandName	  = "test";
			this->helpDescription = "Testing purposes!";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /test!\n------");
			msgEmbed.setTitle("__**Test Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<test>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				jsonifier::vector<song> searchResults{};
				guild_data guild{ argsNew.getInteractionData().guildId };
				guild_member_data guildMember{ argsNew.getGuildMemberData() };
				for (uint64_t x = 0; x < 100; ++x) {
					voice_connection& voiceConnection = guild.connectToVoice(guildMember.user.id, guildMember.getVoiceStateData().channelId);
					guild.disconnect();
				}				

			} catch (const std::runtime_error& error) {
				std::cout << "test::execute()" << error.what() << std::endl;
			}
		}
		~test(){};
	};
}// namespace discord_core_api
