// DatabaseEntities.hpp - Database stuff.
// May 24, 2021
// Chris M.
// https://github.com/RealTimeChris

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

#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <bsoncxx/json.hpp>

#include <discordcoreapi/Index.hpp>

namespace DiscordCoreAPI {

	template<typename ValueType>
	concept SignedIntegerT = std::signed_integral<ValueType>;

	template<typename ValueType>
	concept UnsignedIntegerT = std::unsigned_integral<ValueType>;

	template<typename ValueType>
	concept FloatT = std::floating_point<ValueType>;

	template<typename ValueType, typename ValueType02>
	concept ArrayT = std::same_as<ValueType, jsonifier::vector<ValueType02>>;
	
	using bsoncxx::builder::basic::kvp;

	class MongoEntity;

	class MongoEntity {
	  public:
		MongoEntity() noexcept = default;
		//MongoEntity& operator[](const jsonifier::string& keyNew) {
			//if (!mapOfValues.contains(keyNew)) {
			//mapOfValues.emplace(std::make_pair(keyNew, MongoEntity{ keyNew }));
			//}
			//return mapOfValues[keyNew];
		//}

		inline MongoEntity(const jsonifier::string& keyNew) {
			key = keyNew;
		}

		template<SignedIntegerT ValueType> MongoEntity& operator=(ValueType&& value) {
			document.append(kvp(key.data(), bsoncxx::types::b_int64(value)));
			return *this;
		}

		template<typename ValueType01, ArrayT<ValueType01> ValueType> MongoEntity(ValueType&& value) {  
			document.append(kvp(key.data(), bsoncxx::types::b_array(value)));
			return *this;
		}

		bsoncxx::v_noabi::builder::basic::document convertToDocument(MongoEntity&newEntity) {
			for (auto& [keyNew, value]: mapOfValues) {
				//bsoncxx::builder::concatenate(document.view(), value.document.view());
			}
		}

		std::map<jsonifier::string, MongoEntity> mapOfValues{};
		bsoncxx::builder::basic::document document{};
		jsonifier::string key{};
	};

	struct DiscordUserData {
		jsonifier::vector<uint64_t> botCommanders{};
		jsonifier::string userName{ "" };
		Snowflake userId{ 0 };
	};

	struct DiscordGuildData {
		jsonifier::vector<Snowflake> musicChannelIds{};
		jsonifier::string borderColor{ "FEFEFE" };
		unsigned int memberCount{ 0 };
		jsonifier::string guildName{ "" };
		Snowflake djRoleId{ 0 };
		Snowflake guildId{ 0 };
		Playlist playlist{};
	};

	struct DiscordGuildMemberData {
		jsonifier::string guildMemberMention{ "" };
		jsonifier::string displayName{ "" };
		Snowflake guildMemberId{ 0 };
		Snowflake globalId{ "" };
		jsonifier::string userName{ "" };
		Snowflake guildId{ 0 };
	};

	enum class DatabaseWorkloadType {
		Discord_User_Write = 0,
		Discord_User_Read = 1,
		Discord_Guild_Write = 2,
		Discord_Guild_Read = 3,
		Discord_Guild_Member_Write = 4,
		Discord_Guild_Member_Read = 5
	};

	struct DatabaseWorkload {
		DiscordGuildMemberData guildMemberData{};
		DatabaseWorkloadType workloadType{};
		DiscordGuildData guildData{};
		DiscordUserData userData{};
	};

	struct DatabaseReturnValue {
		DiscordGuildMemberData discordGuildMember{};
		DiscordGuildData discordGuild{};
		DiscordUserData discordUser{};
	};

	class DatabaseManagerAgent {
	  public:
		inline void initialize(Snowflake botUserIdNew) {
			DatabaseManagerAgent::botUserId = botUserIdNew;
			auto newClient = DatabaseManagerAgent::getClient();
			mongocxx::database newDataBase	   = (*newClient)[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
			mongocxx::collection newCollection = newDataBase[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
		}

		inline mongocxx::pool::entry getClient() {
			return DatabaseManagerAgent::thePool.acquire();
		}

		inline DatabaseReturnValue submitWorkloadAndGetResults(DatabaseWorkload workload) {
			while (DatabaseManagerAgent::botUserId == 0) {
				std::this_thread::sleep_for(1ms);
			}
			std::lock_guard<std::mutex> workloadLock{ DatabaseManagerAgent::workloadMutex01 };
			DatabaseReturnValue newData{};
			mongocxx::pool::entry thePtr = DatabaseManagerAgent::getClient();
			try {
				auto newDataBase   = (*thePtr)[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
				auto newCollection = newDataBase[DatabaseManagerAgent::botUserId.operator jsonifier::string().data()];
				switch (workload.workloadType) {
					case (DatabaseWorkloadType::Discord_User_Write): {
						auto doc = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
						bsoncxx::builder::basic::document document{};
						document.append(
							bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(workload.userData.userId))));
						auto resultNew = newCollection.find_one(document.view());
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_User_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(
							bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(workload.userData.userId))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							DiscordUserData userData = DatabaseManagerAgent::parseUserData(*resultNew);
							newData.discordUser = userData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Write): {
						auto doc = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(workload.guildData.guildId))));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(
							bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(workload.guildData.guildId))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							DiscordGuildData guildData = DatabaseManagerAgent::parseGuildData(*resultNew);
							newData.discordGuild = guildData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Member_Write): {
						auto doc = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(workload.guildMemberData.globalId))));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (!resultNewer) {
							auto doc02 = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::Discord_Guild_Member_Read): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(workload.guildMemberData.globalId))));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew) {
							DiscordGuildMemberData guildMemberData = DatabaseManagerAgent::parseGuildMemberData(*resultNew);
							newData.discordGuildMember = guildMemberData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
				}
				return newData;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::run()" << error.what() << std::endl;
				return newData;
			}
		}

	  protected:
		mongocxx::instance instance{};
		std::mutex workloadMutex01{};
		mongocxx::pool thePool{};
		Snowflake botUserId{};

		template<typename value_type, typename value_type_to_search>
		static void getValueIfNotNull(value_type& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
		}

		template<typename value_type_to_search> static void getValueIfNotNull(jsonifier::string& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = jsonifier::string{ valueToSearch[valueToFind].get_string().value };
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(int64_t& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_int64().value;
			}
		}

		template<typename value_type_to_search> static void getValueIfNotNull(bool& value, value_type_to_search&& valueToSearch, const jsonifier::string& valueToFind) {
			if (valueToSearch[valueToFind].type() != bsoncxx::v_noabi::type::k_null) {
				value = valueToSearch[valueToFind].get_bool().value;
			}
		}

		inline static bsoncxx::builder::basic::document convertUserDataToDBDoc(DiscordUserData discordUserData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(discordUserData.userId))));
				buildDoc.append(kvp("userId", bsoncxx::types::b_int64(static_cast<uint64_t>(discordUserData.userId))));
				buildDoc.append(kvp("userName", static_cast<jsonifier::string>(discordUserData.userName)));
				buildDoc.append(kvp("botCommanders", [discordUserData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordUserData.botCommanders) {
						subArray.append(bsoncxx::types::b_int64(static_cast<uint64_t>(value)));
					}
				}));
				return buildDoc;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::convertUserDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		}

		inline static DiscordUserData parseUserData(bsoncxx::document::value docValue) {
			DiscordUserData userData{};
			try {
				getValueIfNotNull(userData.userName, docValue.view(), "userName");
				getValueIfNotNull(userData.userId, docValue.view(), "userId");
				auto botCommandersArray = docValue.view()["botCommanders"].get_array();
				jsonifier::vector<uint64_t> newVector;
				for (const auto& value: botCommandersArray.value) {
					newVector.emplace_back(value.get_int64().value);
				}
				userData.botCommanders = newVector;
				return userData;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::parseUserData()" << error.what() << std::endl;
				return userData;
			}
		}

		inline static bsoncxx::builder::basic::document convertGuildDataToDBDoc(DiscordGuildData discordGuildData) {
			bsoncxx::builder::basic::document buildDoc{};
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(discordGuildData.guildId))));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(static_cast<uint64_t>(discordGuildData.guildId))));
				buildDoc.append(kvp("guildName", discordGuildData.guildName));
				buildDoc.append(kvp("memberCount", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.memberCount))));
				buildDoc.append(kvp("borderColor", discordGuildData.borderColor));
				buildDoc.append(kvp("djRoleId", bsoncxx::types::b_int64(static_cast<uint64_t>(discordGuildData.djRoleId))));
				buildDoc.append(kvp("musicChannelIds", [&](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.musicChannelIds) {
						subArray.append(bsoncxx::types::b_int64(static_cast<uint64_t>(value)));
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
									subDocument03.append(kvp("urlPath", value.urlPath));
								});
							};
						}));
						subDocument02.append(kvp("addedByUserId", bsoncxx::types::b_int64(static_cast<uint64_t>(discordGuildData.playlist.currentSong.addedByUserId))));
						subDocument02.append(kvp("contentLength", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.playlist.currentSong.contentLength))));
						subDocument02.append(kvp("addedByUserName", bsoncxx::types::b_utf8(discordGuildData.playlist.currentSong.addedByUserName)));
						subDocument02.append(kvp("description", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.description }));
						subDocument02.append(kvp("secondDownloadUrl", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.secondDownloadUrl }));
						subDocument02.append(kvp("duration", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.duration }));
						subDocument02.append(kvp("songTitle", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.songTitle }));
						subDocument02.append(kvp("firstDownloadUrl", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.firstDownloadUrl }));
						subDocument02.append(kvp("thumbnailUrl", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.thumbnailUrl }));
						subDocument02.append(kvp("type", bsoncxx::types::b_int64(static_cast<int64_t>(discordGuildData.playlist.currentSong.type))));
						subDocument02.append(kvp("songId", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.songId }));
						subDocument02.append(kvp("viewUrl", bsoncxx::types::b_utf8{ discordGuildData.playlist.currentSong.viewUrl }));
					}));
					std::cout << "CURRENT SIZE (REAL): " << discordGuildData.playlist.songQueue.size() << std::endl;
					subDocument01.append(kvp("songList", [&](bsoncxx::builder::basic::sub_array subArray01) {
						for (auto& value: discordGuildData.playlist.songQueue) {
							subArray01.append([&](bsoncxx::builder::basic::sub_document subDocument02) {
								subDocument02.append(kvp("downloadUrls", [&](bsoncxx::builder::basic::sub_array subArray02) {
									for (auto& value02: discordGuildData.playlist.currentSong.finalDownloadUrls) {
										subArray02.append([&](bsoncxx::builder::basic::sub_document subDocument03) {
											subDocument03.append(kvp("contentSize", bsoncxx::types::b_int64(static_cast<int64_t>(value02.contentSize))));
											subDocument03.append(kvp("urlPath", value02.urlPath));
										});
									};
								}));
								subDocument02.append(kvp("addedByUserId", bsoncxx::types::b_int64(static_cast<uint64_t>(value.addedByUserId.operator const uint64_t&()))));
								subDocument02.append(kvp("addedByUserName", bsoncxx::types::b_utf8(value.addedByUserName)));
								subDocument02.append(kvp("contentLength", bsoncxx::types::b_int64(static_cast<int64_t>(value.contentLength))));
								subDocument02.append(kvp("description", value.description));
								subDocument02.append(kvp("duration", static_cast<jsonifier::string>(value.duration)));
								subDocument02.append(kvp("thumbnailUrl", value.thumbnailUrl));
								subDocument02.append(kvp("songId", static_cast<jsonifier::string>(value.songId)));
								subDocument02.append(kvp("type", bsoncxx::types::b_int64(static_cast<int64_t>(value.type))));
								subDocument02.append(kvp("viewUrl", bsoncxx::types::b_utf8{ value.viewUrl }));
								subDocument02.append(kvp("songTitle", bsoncxx::types::b_utf8{ value.songTitle }));
								subDocument02.append(kvp("firstDownloadUrl", bsoncxx::types::b_utf8{ value.firstDownloadUrl }));
								subDocument02.append(kvp("secondDownloadUrl", static_cast<jsonifier::string>(value.secondDownloadUrl)));
							});
						}
					}));
				}));
				return buildDoc;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::convertGuildDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		}

		inline static DiscordGuildData parseGuildData(bsoncxx::document::value docValue) {
			DiscordGuildData guildData{};
			try {
				auto currentSongValue = docValue.view()["playlist"].get_document().value["currentSong"].get_document();
				getValueIfNotNull(guildData.playlist.currentSong.addedByUserId, currentSongValue.value, "addedByUserId ");
				getValueIfNotNull(guildData.playlist.currentSong.firstDownloadUrl, currentSongValue.value, "firstDownloadUrl");
				getValueIfNotNull(guildData.playlist.currentSong.secondDownloadUrl, currentSongValue.value, "secondDownloadUrl");
				getValueIfNotNull(guildData.playlist.currentSong.contentLength, currentSongValue.value, "contentLength");
				getValueIfNotNull(guildData.playlist.currentSong.description, currentSongValue.value, "description");
				getValueIfNotNull(guildData.playlist.currentSong.thumbnailUrl, currentSongValue.value, "thumbnailUrl");
				getValueIfNotNull(guildData.playlist.currentSong.duration, currentSongValue.value, "duration");
				getValueIfNotNull(guildData.playlist.currentSong.songId, currentSongValue.value, "songId");
				getValueIfNotNull(guildData.playlist.currentSong.songTitle, currentSongValue.value, "songTitle");
				getValueIfNotNull(guildData.playlist.currentSong.type, currentSongValue.value, "type");
				getValueIfNotNull(guildData.playlist.currentSong.viewUrl, currentSongValue.value, "viewUrl");
				getValueIfNotNull(guildData.playlist.isLoopSongEnabled, docValue.view()["playlist"].get_document().value, "isLoopSongEnabled"); 
				getValueIfNotNull(guildData.playlist.isLoopAllEnabled, docValue.view()["playlist"].get_document().value, "isLoopAllEnabled"); 
				for (auto& value02: currentSongValue.value["downloadUrls"].get_array().value) {
					DownloadUrl downloadUrl;
					getValueIfNotNull(downloadUrl.contentSize, value02, "contentSize");
					getValueIfNotNull(downloadUrl.urlPath, value02, "urlPath");
					guildData.playlist.currentSong.finalDownloadUrls.emplace_back(downloadUrl);
				}
				std::cout << "CURRENT DATA: " << bsoncxx::to_json(docValue.view()) << std::endl;
				for (auto& value: docValue.view()["playlist"].get_document().view()["songList"].get_array().value) {
					std::cout << "CURRENT SIZE: "
							  << ( bool )docValue.view()["playlist"].get_document().view()["songList"].get_array().operator bsoncxx::v_noabi::array::view().empty() << std::endl;
					Song newSong{};
					for (auto& value02: value["downloadUrls"].get_array().value) {
						DownloadUrl downloadUrl;
						getValueIfNotNull(downloadUrl.contentSize, value02, "contentSize");
						getValueIfNotNull(downloadUrl.urlPath, value02, "urlPath");
						newSong.finalDownloadUrls.emplace_back(downloadUrl);
					}
					getValueIfNotNull(newSong.description, value, "description");
					getValueIfNotNull(newSong.addedByUserId, value, "addedByUserId");
					getValueIfNotNull(newSong.addedByUserName, value, "addedByUserName");
					getValueIfNotNull(newSong.duration, value, "duration");
					getValueIfNotNull(newSong.thumbnailUrl, value, "thumbnailUrl");
					getValueIfNotNull(newSong.contentLength, value, "contentLength");
					int64_t songType{};
					getValueIfNotNull(songType, value, "type");
					newSong.type = static_cast<SongType>(songType);
					getValueIfNotNull(newSong.songId, value, "songId");
					getValueIfNotNull(newSong.firstDownloadUrl, value, "firstDownloadUrl");
					getValueIfNotNull(newSong.secondDownloadUrl, value, "secondDownloadUrl");
					getValueIfNotNull(newSong.songTitle, value, "songTitle");
					getValueIfNotNull(newSong.viewUrl, value, "viewUrl");
					guildData.playlist.songQueue.emplace_back(newSong);
				}
				getValueIfNotNull(guildData.djRoleId, docValue.view(), "djRoleId");
				getValueIfNotNull(guildData.borderColor, docValue.view(), "borderColor");
				getValueIfNotNull(guildData.guildName, docValue.view(), "guildName");
				getValueIfNotNull(guildData.guildId, docValue.view(), "guildId");
				for (auto& value: docValue.view()["musicChannelIds"].get_array().value) {
					guildData.musicChannelIds.emplace_back(value.get_int64().value);
				}
				getValueIfNotNull(guildData.memberCount, docValue.view(), "memberCount");
				return guildData;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::parseGuildData()" << error.what() << std::endl;
				return guildData;
			}
		};

		inline static bsoncxx::builder::basic::document convertGuildMemberDataToDBDoc(DiscordGuildMemberData discordGuildMemberData) {
			bsoncxx::builder::basic::document buildDoc{};
			try {
				using bsoncxx::builder::basic::kvp;
				if (discordGuildMemberData.guildMemberMention != "") {
					buildDoc.append(kvp("guildMemberMention", static_cast<jsonifier::string>(discordGuildMemberData.guildMemberMention)));
				}
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(static_cast<uint64_t>(discordGuildMemberData.globalId))));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(discordGuildMemberData.guildId.operator const uint64_t&())));
				buildDoc.append(kvp("guildMemberId", bsoncxx::types::b_int64(discordGuildMemberData.guildMemberId.operator const uint64_t&())));
				buildDoc.append(kvp("globalId", bsoncxx::types::b_int64(static_cast<uint64_t>(discordGuildMemberData.globalId))));
				if (discordGuildMemberData.userName != "") {
					buildDoc.append(kvp("userName", static_cast<jsonifier::string>(discordGuildMemberData.userName)));
				}
				if (discordGuildMemberData.displayName != "") {
					buildDoc.append(kvp("displayName", discordGuildMemberData.displayName));
				}
				return buildDoc;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::convertGuildDataToDBDoc()" << error.what() << std::endl;
				return buildDoc;
			}
		};

		inline static DiscordGuildMemberData parseGuildMemberData(bsoncxx::document::value docValue) {
			DiscordGuildMemberData guildMemberData{};
			try {
				getValueIfNotNull(guildMemberData.guildMemberMention, docValue.view(), "guildMemberMention");
				getValueIfNotNull(guildMemberData.guildId, docValue.view(), "guildId");
				getValueIfNotNull(guildMemberData.displayName, docValue.view(), "displayName");
				getValueIfNotNull(guildMemberData.globalId, docValue.view(), "globalId");
				getValueIfNotNull(guildMemberData.guildMemberId, docValue.view(), "guildMemberId");
				getValueIfNotNull(guildMemberData.userName, docValue.view(), "userName");
				return guildMemberData;
			} catch (const std::runtime_error& error) {
				std::cout << "DatabaseManagerAgent::parseGuildData()" << error.what() << std::endl;
				return guildMemberData;
			}
		};
	};

	class DiscordUser {
	  public:
		static int64_t guildCount;

		DiscordUserData data{};

		inline DiscordUser(DatabaseManagerAgent& other, jsonifier::string userNameNew, Snowflake userIdNew) {
			this->data.userId = userIdNew;
			this->data.userName = userNameNew;
			this->getDataFromDB(other);
			this->data.userId = userIdNew;
			this->data.userName = userNameNew;
		}

		inline void writeDataToDB(DatabaseManagerAgent&other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_User_Write;
			workload.userData = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_User_Read;
			workload.userData = this->data;
			auto result = other.submitWorkloadAndGetResults(workload);
			if (result.discordUser.userId != 0) {
				this->data = result.discordUser;
			}
		}
	};

	class DiscordGuild {
	  public:
		inline DiscordGuild() noexcept = default;
		DiscordGuildData data{};

		inline DiscordGuild(DatabaseManagerAgent& other, GuildCacheData guildData) {
			this->data.guildId = guildData.id;
			this->data.guildName = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->getDataFromDB(other);
			this->data.guildId = guildData.id;
			this->data.guildName = guildData.name;
			this->data.memberCount = guildData.memberCount;
		}

		inline void writeDataToDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_Guild_Write;
			workload.guildData = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_Guild_Read;
			workload.guildData = this->data;
			auto result = other.submitWorkloadAndGetResults(workload);
			if (result.discordGuild.guildId != 0) {
				this->data = result.discordGuild;
			}
		}
	};

	class DiscordGuildMember {
	  public:
		DiscordGuildMemberData data{};

		inline DiscordGuildMember(DatabaseManagerAgent& other,GuildMemberCacheData guildMemberData) {
			this->data.guildMemberId = guildMemberData.user.id;
			this->data.guildId		 = guildMemberData.guildId.operator const uint64_t&();
			this->data.globalId		 = this->data.guildId.operator jsonifier::string() + " + " + this->data.guildMemberId;
			this->getDataFromDB(other);
			if (guildMemberData.nick == "") {
				this->data.displayName = guildMemberData.getUserData().userName;
				this->data.guildMemberMention = "<@" + this->data.guildMemberId + ">";
			} else {
				this->data.displayName = guildMemberData.nick;
				this->data.guildMemberMention = "<@!" + this->data.guildMemberId + ">";
			}
			this->data.userName = guildMemberData.getUserData().userName;
		}

		inline void writeDataToDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_Guild_Member_Write;
			workload.guildMemberData = this->data;
			other.submitWorkloadAndGetResults(workload);
		}

		inline void getDataFromDB(DatabaseManagerAgent& other) {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::Discord_Guild_Member_Read;
			workload.guildMemberData = this->data;
			auto result = other.submitWorkloadAndGetResults(workload);
			if (result.discordGuildMember.globalId != 0) {
				this->data = result.discordGuildMember;
			}
		}
	};


	DiscordCoreAPI::DatabaseManagerAgent managerAgent{};
}
