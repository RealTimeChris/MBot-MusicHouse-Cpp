// database_entities.hpp - database stuff.
// may 24, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#include <regex>

#ifdef BSONCXX_POLY_USE_BOOST
	#undef BSONCXX_POLY_USE_BOOST
#endif

#ifndef BSONCXX_POLY_USE_STD_EXPERIMENTAL
	#define BSONCXX_POLY_USE_STD_EXPERIMENTAL
#endif

#ifndef _GLIBCXX17_INLINE
	#define _GLIBCXX17_INLINE inline
#endif

#include <Mongocxx/Instance.hpp>
#include <Mongocxx/Database.hpp>
#include <Mongocxx/Client.hpp>
#include <Mongocxx/Pool.hpp>

#include <Discordcoreapi/Index.hpp>

namespace discord_core_api {

	struct discord_user_data {
		jsonifier::vector<uint64_t> botCommanders{};
		jsonifier::string userName{ "" };
		snowflake userId{ 0 };
	};

	struct discord_guild_data {
		jsonifier::vector<snowflake> musicChannelIds{};
		jsonifier::string borderColor{ "fefefe" };
		unsigned int memberCount{ 0 };
		jsonifier::string guildName{ "" };
		snowflake djRoleId{ 0 };
		snowflake guildId{ 0 };
		playlist playlist{};
	};

	struct discord_guild_member_data {
		jsonifier::string guildMemberMention{ "" };
		jsonifier::string displayName{ "" };
		snowflake guildMemberId{ 0 };
		jsonifier::string globalId{ "" };
		jsonifier::string userName{ "" };
		snowflake guildId{ 0 };
	};

	enum class database_workload_type {
		Discord_User_Write		   = 0,
		Discord_User_Read		   = 1,
		Discord_Guild_Write		   = 2,
		Discord_Guild_Read		   = 3,
		Discord_Guild_Member_Write = 4,
		Discord_Guild_Member_Read  = 5
	};

	struct database_workload {
		discord_guild_member_data guildMemberData{};
		database_workload_type workloadType{};
		discord_guild_data guildData{};
		discord_user_data userData{};
	};

	struct database_return_value {
		discord_guild_member_data discordGuildMember{};
		discord_guild_data discordGuild{};
		discord_user_data discordUser{};
	};

	class database_manager_agent {
	  public:
		inline static void initialize(snowflake botUserIdNew) {
			database_manager_agent::botUserId	   = botUserIdNew;
			auto newClient					   = database_manager_agent::getClient();
			mongocxx::database newDataBase	   = (*newClient)[database_manager_agent::botUserId.operator jsonifier::string().data()];
			mongocxx::collection newCollection = newDataBase[database_manager_agent::botUserId.operator jsonifier::string().data()];
		}

		inline static mongocxx::pool::entry getClient() {
			return database_manager_agent::thePool.acquire();
		}

		inline static database_return_value submitWorkloadAndGetResults(const database_workload& workload) {
			std::lock_guard<std::mutex> workloadLock{ database_manager_agent::workloadMutex01 };
			while (database_manager_agent::botUserId == 0) {
				std::this_thread::sleep_for(1ms);
			}
			database_return_value newData{};
			mongocxx::pool::entry thePtr = database_manager_agent::getClient();
			try {
				auto newDataBase   = (*thePtr)[database_manager_agent::botUserId.operator jsonifier::string().data()];
				auto newCollection = newDataBase[database_manager_agent::botUserId.operator jsonifier::string().data()];
				switch (workload.workloadType) {
					case (database_workload_type::Discord_User_Write): {
						auto doc = database_manager_agent::convertUserDataToDBDoc(workload.userData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.userData.userId.operator const uint64_t&()))));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = database_manager_agent::convertUserDataToDBDoc(workload.userData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_User_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.userData.userId.operator const uint64_t&()))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							discord_user_data userData = database_manager_agent::parseUserData(*resultNew);
							newData.discordUser		 = userData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Write): {
						auto doc = database_manager_agent::convertGuildDataToDBDoc(workload.guildData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.guildData.guildId.operator const uint64_t&()))));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = database_manager_agent::convertGuildDataToDBDoc(workload.guildData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(workload.guildData.guildId.operator const uint64_t&()))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							discord_guild_data guildData = database_manager_agent::parseGuildData(*resultNew);
							newData.discordGuild	   = guildData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Member_Write): {
						auto doc = database_manager_agent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = database_manager_agent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (database_workload_type::Discord_Guild_Member_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							discord_guild_member_data guildMemberData = database_manager_agent::parseGuildMemberData(*resultNew);
							newData.discordGuildMember			   = guildMemberData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
				}
				return newData;
			} catch (const std::runtime_error& error) {
				std::cout << "database_manager_agent::run()" << error.what() << std::endl;
				return newData;
			}
		}

	  protected:
		inline static mongocxx::instance instance{};
		inline static std::mutex workloadMutex01{};
		inline static mongocxx::pool thePool{};
		inline static snowflake botUserId{};

		template<jsonifier::concepts::string_t return_type, typename value_type_to_search>
		static return_type getValueIfNotNull(value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				return return_type{ valueToSearch[valueToFind].get_string().value };
			} else {
				return {};
			}
		}

		template<jsonifier_internal::snowflake_t return_type, typename value_type_to_search>
		static return_type getValueIfNotNull(value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				return return_type{ static_cast<uint64_t>(valueToSearch[valueToFind].get_int64().value) };
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::enum_t return_type, typename value_type_to_search>
		static return_type getValueIfNotNull(value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				return static_cast<return_type>(valueToSearch[valueToFind].get_int64().value);
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::unsigned_t return_type, typename value_type_to_search>
		static return_type getValueIfNotNull(value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				return return_type{ valueToSearch[valueToFind].get_int64().value };
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::signed_t return_type, typename value_type_to_search>
		static return_type getValueIfNotNull(value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				return return_type{ valueToSearch[valueToFind].get_int64().value };
			} else {
				return {};
			}
		}

		template<jsonifier::concepts::bool_t return_type, typename value_type_to_search>
		static return_type getValueIfNotNull(value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				return return_type{ valueToSearch[valueToFind].get_bool().value };
			} else {
				return {};
			}
		}

		inline static bsoncxx::builder::basic::document convertUserDataToDBDoc(discord_user_data discordUserData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(discordUserData.userId.operator const uint64_t&()))));
				buildDoc.append(kvp("userId", bsoncxx::types::b_int64(static_cast<int64_t>(discordUserData.userId.operator const uint64_t&()))));
				buildDoc.append(kvp("userName", discordUserData.userName));
				buildDoc.append(kvp("botCommanders", [discordUserData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordUserData.botCommanders) {
						subArray.append(bsoncxx::types::b_int64(static_cast<int64_t>(value)));
					}
				}));
				return buildDoc;
			} catch (const std::runtime_error& error) {
				std::cout << "database_manager_agent::convertUserDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		}

		inline static discord_user_data parseUserData(bsoncxx::document::value docValue) {
			discord_user_data userData{};
			try {
				userData.userId			= getValueIfNotNull<snowflake>(docValue.view(), "_id");
				userData.userName		= getValueIfNotNull<jsonifier::string>(docValue.view(), "userName");
				userData.userId			= getValueIfNotNull<snowflake>(docValue.view(), "userId");
				auto botCommandersArray = docValue.view()["botCommanders"].get_array();
				jsonifier::vector<uint64_t> newVector;
				for (const auto& value: botCommandersArray.value) {
					newVector.emplace_back(value.get_int64().value);
				}
				userData.botCommanders = newVector;
				return userData;
			} catch (const std::runtime_error& error) {
				std::cout << "database_manager_agent::parseUserData()" << error.what() << std::endl;
				return userData;
			}
		}

		inline static bsoncxx::builder::basic::document convertGuildDataToDBDoc(discord_guild_data discordGuildData) {
			bsoncxx::builder::basic::document buildDoc{};
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildName", discordGuildData.guildName));
				buildDoc.append(kvp("memberCount", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.memberCount))));
				buildDoc.append(kvp("borderColor", discordGuildData.borderColor));
				buildDoc.append(kvp("djRoleId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.djRoleId.operator const uint64_t&()))));
				buildDoc.append(kvp("musicChannelIds", [&](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.musicChannelIds) {
						subArray.append(bsoncxx::types::b_int64(static_cast<int64_t>(value.operator const uint64_t&())));
					}
				}));
				buildDoc.append(kvp("playlist", [&](bsoncxx::builder::basic::sub_document subDocument01) {
					subDocument01.append(kvp("isLoopAllEnabled", bsoncxx::types::b_bool(discordGuildData.playlist.isLoopAllEnabled)));
					subDocument01.append(kvp("isLoopSongEnabled", bsoncxx::types::b_bool(discordGuildData.playlist.isLoopSongEnabled)));
					subDocument01.append(kvp("currentSong", [&](bsoncxx::builder::basic::sub_document subDocument02) {
						subDocument02.append(kvp("downloadUrls", [&](bsoncxx::builder::basic::sub_array subArray01) {
							for (auto& value: discordGuildData.playlist.currentSong.finalDownloadUrls) {
								subArray01.append([&](bsoncxx::builder::basic::sub_document subDocument03) {
									subDocument03.append(kvp("contentSize", bsoncxx::types::b_int64(static_cast<int64_t>(value.contentSize))));
									subDocument03.append(kvp("urlPath", jsonifier::string{ value.urlPath }));
								});
							};
						}));

						subDocument02.append(kvp("addedByUserId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.playlist.currentSong.addedByUserId.operator const uint64_t&()))),
							kvp("contentLength", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.playlist.currentSong.contentLength))),
							kvp("Description", jsonifier::string{ discordGuildData.playlist.currentSong.description}),
							kvp("secondDownloadUrl", jsonifier::string{ discordGuildData.playlist.currentSong.secondDownloadUrl }),
							kvp("Duration", jsonifier::string{ discordGuildData.playlist.currentSong.duration }),
							kvp("songTitle", jsonifier::string{ discordGuildData.playlist.currentSong.songTitle }),
							kvp("firstDownloadUrl", jsonifier::string{ discordGuildData.playlist.currentSong.firstDownloadUrl }),
							kvp("thumbnailUrl", jsonifier::string{ discordGuildData.playlist.currentSong.thumbnailUrl }),
							kvp("type", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.playlist.currentSong.type))),
							kvp("songId", jsonifier::string{ discordGuildData.playlist.currentSong.songId }),
							kvp("viewUrl", jsonifier::string{ discordGuildData.playlist.currentSong.viewUrl }));
					}));

					subDocument01.append(kvp("songList", [&](bsoncxx::builder::basic::sub_array subArray01) {
						for (auto& value: discordGuildData.playlist.songQueue) {
							subArray01.append([&](bsoncxx::builder::basic::sub_document subDocument02) {
								subDocument02.append(kvp("downloadUrls", [&](bsoncxx::builder::basic::sub_array subArray02) {
									for (auto& value02: discordGuildData.playlist.currentSong.finalDownloadUrls) {
										subArray02.append([&](bsoncxx::builder::basic::sub_document subDocument03) {
											subDocument03.append(kvp("contentSize", bsoncxx::types::b_int64(static_cast<int64_t>(value02.contentSize))));
											subDocument03.append(kvp("urlPath", jsonifier::string{ value02.urlPath }));
										});
									};
								}));
								subDocument02.append(kvp("addedByUserId", bsoncxx::types::b_int64(static_cast<int64_t>(value.addedByUserId.operator const uint64_t&()))),
									kvp("contentLength", bsoncxx::types::b_int64(static_cast<int64_t>(value.contentLength))),
									kvp("Description", jsonifier::string{ value.description}), kvp("Duration", jsonifier::string{ value.duration }),
									kvp("thumbnailUrl", value.thumbnailUrl), kvp("songId", jsonifier::string{ value.songId }),
									kvp("type", bsoncxx::types::b_int64(static_cast<int64_t>(value.type))), kvp("viewUrl", value.viewUrl));
								subDocument02.append(kvp("songTitle", jsonifier::string{ value.songTitle }), kvp("firstDownloadUrl", jsonifier::string{ value.firstDownloadUrl }),
									kvp("secondDownloadUrl", jsonifier::string{ value.secondDownloadUrl }));
							});
						}
					}));
				}));
				return buildDoc;
			} catch (const std::runtime_error& error) {
				std::cout << "database_manager_agent::convertGuildDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		};

		inline static discord_guild_data parseGuildData(bsoncxx::document::value docValue) {
			discord_guild_data guildData{};
			try {
				auto currentSongValue = docValue.view()["playlist"].get_document().value["currentSong"].get_document();
				guildData.playlist.currentSong.addedByUserId	 = getValueIfNotNull<snowflake>(currentSongValue.value, "addedByUserId");
				guildData.playlist.currentSong.firstDownloadUrl	 = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "firstDownloadUrl");
				guildData.playlist.currentSong.secondDownloadUrl = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "secondDownloadUrl");
				guildData.playlist.currentSong.contentLength	 = getValueIfNotNull<int64_t>(currentSongValue.value, "contentLength");
				guildData.playlist.currentSong.description		 = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "Description");
				guildData.playlist.currentSong.thumbnailUrl		 = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "thumbnailUrl");
				guildData.playlist.currentSong.duration			 = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "Duration");
				guildData.playlist.currentSong.songId			 = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "songId");
				guildData.playlist.currentSong.songTitle		 = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "songTitle");
				guildData.playlist.currentSong.type				 = getValueIfNotNull<song_type>(currentSongValue.value, "type");
				guildData.playlist.currentSong.viewUrl			 = getValueIfNotNull<jsonifier::string>(currentSongValue.value, "viewUrl");
				guildData.playlist.isLoopSongEnabled			 = getValueIfNotNull<bool>(docValue.view()["playlist"].get_document().value, "isLoopSongEnabled");
				guildData.playlist.isLoopAllEnabled				 = getValueIfNotNull<bool>(docValue.view()["playlist"].get_document().value, "isLoopAllEnabled");
				for (auto& value02: currentSongValue.value["downloadUrls"].get_array().value) {
					download_url downloadUrl;
					downloadUrl.contentSize = getValueIfNotNull<int64_t>(value02, "contentSize");
					downloadUrl.urlPath		= getValueIfNotNull<jsonifier::string>(value02, "urlPath");
					guildData.playlist.currentSong.finalDownloadUrls.emplace_back(downloadUrl);
				}
				for (auto& value: docValue.view()["playlist"].get_document().view()["songList"].get_array().value) {
					song newSong{};
					for (auto& value02: value["downloadUrls"].get_array().value) {
						download_url downloadUrl;
						downloadUrl.contentSize = getValueIfNotNull<int64_t>(value02, "contentSize");
						downloadUrl.urlPath		= getValueIfNotNull<jsonifier::string>(value02, "urlPath");
						newSong.finalDownloadUrls.emplace_back(downloadUrl);
					}
					newSong.description	  = getValueIfNotNull<jsonifier::string>(value, "Description");
					newSong.addedByUserId = getValueIfNotNull<snowflake>(value, "addedByUserId");
					newSong.duration	  = getValueIfNotNull<jsonifier::string>(value, "Duration");
					newSong.thumbnailUrl  = getValueIfNotNull<jsonifier::string>(value, "thumbnailUrl");
					newSong.contentLength = getValueIfNotNull<int64_t>(value, "contentLength");
					int64_t songType{};
					songType				  = getValueIfNotNull<int64_t>(value, "type");
					newSong.type			  = static_cast<song_type>(songType);
					newSong.songId			  = getValueIfNotNull<jsonifier::string>(value, "songId");
					newSong.firstDownloadUrl  = getValueIfNotNull<jsonifier::string>(value, "firstDownloadUrl");
					newSong.secondDownloadUrl = getValueIfNotNull<jsonifier::string>(value, "secondDownloadUrl");
					newSong.songTitle		  = getValueIfNotNull<jsonifier::string>(value, "songTitle");
					newSong.viewUrl			  = getValueIfNotNull<jsonifier::string>(value, "viewUrl");
					guildData.playlist.songQueue.emplace_back(newSong);
				}
				guildData.djRoleId	  = getValueIfNotNull<snowflake>(docValue.view(), "djRoleId");
				guildData.borderColor = getValueIfNotNull<jsonifier::string>(docValue.view(), "borderColor");
				guildData.guildName	  = getValueIfNotNull<jsonifier::string>(docValue.view(), "guildName");
				guildData.guildId	  = getValueIfNotNull<snowflake>(docValue.view(), "guildId");
				for (auto& value: docValue.view()["musicChannelIds"].get_array().value) {
					guildData.musicChannelIds.emplace_back(value.get_int64().value);
				}
				guildData.memberCount = getValueIfNotNull<int64_t>(docValue.view(), "memberCount");
				return guildData;
			} catch (const std::runtime_error& error) {
				std::cout << "database_manager_agent::parseGuildData()" << error.what() << std::endl;
				return guildData;
			}
		};


		inline static bsoncxx::builder::basic::document convertGuildMemberDataToDBDoc(discord_guild_member_data discordGuildMemberData) {
			bsoncxx::builder::basic::document buildDoc{};
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("guildMemberMention", jsonifier::string{ discordGuildMemberData.guildMemberMention }));
				buildDoc.append(kvp("_id", jsonifier::string{ discordGuildMemberData.globalId }));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildMemberData.guildId.operator const uint64_t&()))));
				buildDoc.append(kvp("guildMemberId", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildMemberData.guildMemberId.operator const uint64_t&()))));
				buildDoc.append(kvp("globalId", jsonifier::string{ discordGuildMemberData.globalId }));
				buildDoc.append(kvp("userName", jsonifier::string{ discordGuildMemberData.userName }));
				buildDoc.append(kvp("displayName", jsonifier::string{ discordGuildMemberData.displayName }));
				return buildDoc;
			} catch (const std::runtime_error& error) {
				std::cout << "database_manager_agent::convertGuildDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		};

		inline static discord_guild_member_data parseGuildMemberData(bsoncxx::document::value docValue) {
			discord_guild_member_data guildMemberData{};
			try {
				guildMemberData.guildMemberMention = getValueIfNotNull<jsonifier::string>(docValue.view(), "guildMemberMention");
				guildMemberData.guildId			   = getValueIfNotNull<snowflake>(docValue.view(), "guildId");
				guildMemberData.displayName		   = getValueIfNotNull<jsonifier::string>(docValue.view(), "displayName");
				guildMemberData.globalId		   = getValueIfNotNull<jsonifier::string>(docValue.view(), "globalId");
				guildMemberData.guildMemberId	   = getValueIfNotNull<jsonifier::string>(docValue.view(), "guildMemberId");
				guildMemberData.userName		   = getValueIfNotNull<jsonifier::string>(docValue.view(), "userName");
				return guildMemberData;
			} catch (const std::runtime_error& error) {
				std::cout << "database_manager_agent::parseGuildData()" << error.what() << std::endl;
				return guildMemberData;
			}
		};
	};

	class discord_user {
	  public:
		static int64_t guildCount;

		discord_user_data data{};

		inline discord_user(jsonifier::string userNameNew, snowflake userIdNew) {
			this->data.userId	= userIdNew;
			this->data.userName = userNameNew;
			this->getDataFromDB();
			this->data.userId	= userIdNew;
			this->data.userName = userNameNew;
		}

		inline void writeDataToDB() {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_User_Write;
			workload.userData	  = this->data;
			database_manager_agent::submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB() {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_User_Read;
			workload.userData	  = this->data;
			auto result			  = database_manager_agent::submitWorkloadAndGetResults(workload);
			if (result.discordUser.userId != 0) {
				this->data = result.discordUser;
			}
		}
	};

	class discord_guild {
	  public:
		inline discord_guild() noexcept = default;
		discord_guild_data data{};

		inline discord_guild(guild_data guildData) {
			this->data.guildId	   = guildData.id;
			this->data.guildName   = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->getDataFromDB();
			this->data.guildId	   = guildData.id;
			this->data.guildName   = guildData.name;
			this->data.memberCount = guildData.memberCount;
		}

		inline void writeDataToDB() {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_Guild_Write;
			workload.guildData	  = this->data;
			database_manager_agent::submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB() {
			database_workload workload{};
			workload.workloadType = database_workload_type::Discord_Guild_Read;
			workload.guildData	  = this->data;
			auto result			  = database_manager_agent::submitWorkloadAndGetResults(workload);
			if (result.discordGuild.guildId != 0) {
				this->data = result.discordGuild;
			}
		}
	};

	class discord_guild_member {
	  public:
		discord_guild_member_data data{};

		inline discord_guild_member() noexcept = default;

		inline discord_guild_member(guild_member_data guildMemberData) {
			this->data.guildMemberId = guildMemberData.user.id;
			this->data.guildId		 = guildMemberData.guildId.operator const uint64_t&();
			this->data.globalId		 = this->data.guildId + " + " + this->data.guildMemberId;
			this->getDataFromDB();
			if (guildMemberData.nick == "") {
				this->data.displayName		  = guildMemberData.getUserData().userName;
				this->data.guildMemberMention = "<@" + this->data.guildMemberId + ">";
			} else {
				this->data.displayName		  = guildMemberData.nick;
				this->data.guildMemberMention = "<@!" + this->data.guildMemberId + ">";
			}
			this->data.userName = guildMemberData.getUserData().userName;
		}

		inline void writeDataToDB() {
			database_workload workload{};
			workload.workloadType	 = database_workload_type::Discord_Guild_Member_Write;
			workload.guildMemberData = this->data;
			database_manager_agent::submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB() {
			database_workload workload{};
			workload.workloadType	 = database_workload_type::Discord_Guild_Member_Read;
			workload.guildMemberData = this->data;
			auto result				 = database_manager_agent::submitWorkloadAndGetResults(workload);
			if (result.discordGuildMember.globalId != "") {
				this->data = result.discordGuildMember;
			}
		}
	};


	discord_core_api::database_manager_agent managerAgent{};
}
