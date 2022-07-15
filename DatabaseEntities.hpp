// DatabaseEntities.hpp - Database stuff.
// May 24, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>

#include <discordcoreapi/Index.hpp>

namespace DiscordCoreAPI {

	struct DiscordUserData {
		std::vector<uint64_t> botCommanders{};
		std::string userName{ "" };
		uint64_t userId{ 0 };
	};

	struct DiscordGuildData {
		std::vector<uint64_t> musicChannelIds{};
		std::string borderColor{ "FEFEFE" };
		unsigned int memberCount{ 0 };
		std::string guildName{ "" };
		uint64_t djRoleId{ 0 };
		uint64_t guildId{ 0 };
		Playlist playlist{};
	};

	struct DiscordGuildMemberData {
		std::string guildMemberMention{ "" };
		std::string displayName{ "" };
		uint64_t guildMemberId{ 0 };
		std::string globalId{ "" };
		std::string userName{ "" };
		uint64_t guildId{ 0 };
	};

	enum class DatabaseWorkloadType {
		DISCORD_USER_WRITE = 0,
		DISCORD_USER_READ = 1,
		DISCORD_GUILD_WRITE = 2,
		DISCORD_GUILD_READ = 3,
		DISCORD_GUILD_MEMBER_WRITE = 4,
		DISCORD_GUILD_MEMBER_READ = 5
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
		static void initialize(uint64_t botUserIdNew) {
			DatabaseManagerAgent::botUserId = botUserIdNew;
			auto newClient = DatabaseManagerAgent::getClient();
			mongocxx::database newDataBase = (*newClient)[std::to_string(DatabaseManagerAgent::botUserId)];
			mongocxx::collection newCollection = newDataBase[std::to_string(DatabaseManagerAgent::botUserId)];
		}

		static mongocxx::pool::entry getClient() {
			std::lock_guard<std::mutex> workloadLock{ DatabaseManagerAgent::workloadMutex02 };
			return DatabaseManagerAgent::thePool.acquire();
		}

		static DatabaseReturnValue submitWorkloadAndGetResults(DatabaseWorkload workload) {
			std::lock_guard<std::mutex> workloadLock{ DatabaseManagerAgent::workloadMutex01 };
			DatabaseReturnValue newData{};
			mongocxx::pool::entry thePtr = DatabaseManagerAgent::getClient();
			try {
				auto newDataBase = (*thePtr)[std::to_string(DatabaseManagerAgent::botUserId)];
				auto newCollection = newDataBase[std::to_string(DatabaseManagerAgent::botUserId)];
				switch (workload.workloadType) {
					case (DatabaseWorkloadType::DISCORD_USER_WRITE): {
						auto doc = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.userData.userId)));
						auto resultNew = newCollection.find_one(document.view());
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (resultNewer.get_ptr() == NULL) {
							auto doc02 = DatabaseManagerAgent::convertUserDataToDBDoc(workload.userData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_USER_READ): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.userData.userId)));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew.get_ptr() != NULL) {
							DiscordUserData userData = DatabaseManagerAgent::parseUserData(*resultNew.get_ptr());
							newData.discordUser = userData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_WRITE): {
						auto doc = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.guildData.guildId)));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (resultNewer.get_ptr() == NULL) {
							auto doc02 = DatabaseManagerAgent::convertGuildDataToDBDoc(workload.guildData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_READ): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::types::b_int64(workload.guildData.guildId)));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew.get_ptr() != NULL) {
							DiscordGuildData guildData = DatabaseManagerAgent::parseGuildData(*resultNew.get_ptr());
							newData.discordGuild = guildData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_MEMBER_WRITE): {
						auto doc = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId.c_str()));
						auto resultNewer = newCollection.find_one_and_replace(document.view(), std::move(doc.extract()),
							mongocxx::v_noabi::options::find_one_and_replace{}.return_document(mongocxx::v_noabi::options::return_document::k_after));
						if (resultNewer.get_ptr() == NULL) {
							auto doc02 = DatabaseManagerAgent::convertGuildMemberDataToDBDoc(workload.guildMemberData);
							newCollection.insert_one(std::move(doc02.extract()));
							return newData;
						}
						break;
					}
					case (DatabaseWorkloadType::DISCORD_GUILD_MEMBER_READ): {
						bsoncxx::builder::basic::document document{};
						document.append(bsoncxx::builder::basic::kvp("_id", workload.guildMemberData.globalId.c_str()));
						auto resultNew = newCollection.find_one(document.view());
						if (resultNew.get_ptr() != NULL) {
							DiscordGuildMemberData guildMemberData = DatabaseManagerAgent::parseGuildMemberData(*resultNew.get_ptr());
							newData.discordGuildMember = guildMemberData;
							return newData;
						} else {
							return newData;
						}
						break;
					}
				}
				return newData;
			} catch (...) {
				reportException("DatabaseManagerAgent::run() Error: ");
				return newData;
			}
		}

	  protected:
		static mongocxx::instance instance;
		static std::mutex workloadMutex01;
		static std::mutex workloadMutex02;
		static mongocxx::pool thePool;
		static uint64_t botUserId;

		static bsoncxx::builder::basic::document convertUserDataToDBDoc(DiscordUserData discordUserData) {
			bsoncxx::builder::basic::document buildDoc;
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(discordUserData.userId)));
				buildDoc.append(kvp("userId", bsoncxx::types::b_int64(discordUserData.userId)));
				buildDoc.append(kvp("userName", discordUserData.userName.c_str()));
				buildDoc.append(kvp("botCommanders", [discordUserData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordUserData.botCommanders) {
						subArray.append(bsoncxx::types::b_int64(value));
					}
				}));
				return buildDoc;
			} catch (...) {
				reportException("DatabaseManagerAgent::convertUserDataToDBDoc()");
				return buildDoc;
			}
		}

		static DiscordUserData parseUserData(bsoncxx::document::value docValue) {
			DiscordUserData userData{};
			try {
				userData.userName = docValue.view()["userName"].get_utf8().value.to_string();
				userData.userId = docValue.view()["userId"].get_int64().value;
				auto botCommandersArray = docValue.view()["botCommanders"].get_array();
				std::vector<uint64_t> newVector;
				for (const auto& value: botCommandersArray.value) {
					newVector.push_back(value.get_int64().value);
				}
				userData.botCommanders = newVector;
				return userData;
			} catch (...) {
				reportException("DatabaseManagerAgent::parseUserData()");
				return userData;
			}
		}

		static bsoncxx::builder::basic::document convertGuildDataToDBDoc(DiscordGuildData discordGuildData) {
			bsoncxx::builder::basic::document buildDoc{};
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("_id", bsoncxx::types::b_int64(discordGuildData.guildId)));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(discordGuildData.guildId)));
				buildDoc.append(kvp("guildName", discordGuildData.guildName.c_str()));
				buildDoc.append(kvp("memberCount", bsoncxx::types::b_int32(discordGuildData.memberCount)));
				buildDoc.append(kvp("borderColor", discordGuildData.borderColor.c_str()));
				buildDoc.append(kvp("djRoleId", bsoncxx::types::b_int64(discordGuildData.djRoleId)));
				buildDoc.append(kvp("musicChannelIds", [discordGuildData](bsoncxx::builder::basic::sub_array subArray) {
					for (auto& value: discordGuildData.musicChannelIds) {
						subArray.append(bsoncxx::types::b_int64(value));
					}
				}));
				buildDoc.append(kvp("playlist", [discordGuildData](bsoncxx::builder::basic::sub_document subDocument01) {
					subDocument01.append(kvp("isLoopAllEnabled", bsoncxx::types::b_bool(discordGuildData.playlist.isLoopAllEnabled)));
					subDocument01.append(kvp("isLoopSongEnabled", bsoncxx::types::b_bool(discordGuildData.playlist.isLoopSongEnabled)));
					subDocument01.append(kvp("currentSong", [&](bsoncxx::builder::basic::sub_document subDocument02) {
						subDocument02.append(kvp("downloadUrls", [discordGuildData](bsoncxx::builder::basic::sub_array subArray01) {
							for (auto& value: discordGuildData.playlist.currentSong.finalDownloadUrls) {
								subArray01.append([&](bsoncxx::builder::basic::sub_document subDocument03) {
									subDocument03.append(kvp("contentSize", bsoncxx::types::b_int32(value.contentSize)));
									subDocument03.append(kvp("urlPath", value.urlPath.c_str()));
								});
							};
						}));

						subDocument02.append(kvp("addedByUserId", bsoncxx::types::b_int64(discordGuildData.playlist.currentSong.addedByUserId)),
							kvp("addedByUserName", discordGuildData.playlist.currentSong.addedByUserName.c_str()),
							kvp("contentLength", bsoncxx::types::b_int32(discordGuildData.playlist.currentSong.contentLength)),
							kvp("description", discordGuildData.playlist.currentSong.description.c_str()),
							kvp("secondDownloadUrl", discordGuildData.playlist.currentSong.secondDownloadUrl.c_str()),
							kvp("duration", discordGuildData.playlist.currentSong.duration.c_str()), kvp("songTitle", discordGuildData.playlist.currentSong.songTitle.c_str()),
							kvp("firstDownloadUrl", discordGuildData.playlist.currentSong.firstDownloadUrl.c_str()),
							kvp("thumbnailUrl", discordGuildData.playlist.currentSong.thumbnailUrl.c_str()),
							kvp("type", bsoncxx::types::b_int32(( int )discordGuildData.playlist.currentSong.type)),
							kvp("songId", discordGuildData.playlist.currentSong.songId.c_str()), kvp("viewUrl", discordGuildData.playlist.currentSong.viewUrl.c_str()));
					}));

					subDocument01.append(kvp("songList", [discordGuildData](bsoncxx::builder::basic::sub_array subArray01) {
						for (auto& value: discordGuildData.playlist.songQueue) {
							subArray01.append([&](bsoncxx::builder::basic::sub_document subDocument02) {
								subDocument02.append(kvp("downloadUrls", [discordGuildData](bsoncxx::builder::basic::sub_array subArray02) {
									for (auto& value02: discordGuildData.playlist.currentSong.finalDownloadUrls) {
										subArray02.append([&](bsoncxx::builder::basic::sub_document subDocument03) {
											subDocument03.append(kvp("contentSize", bsoncxx::types::b_int32(value02.contentSize)));
											subDocument03.append(kvp("urlPath", value02.urlPath.c_str()));
										});
									};
								}));
								subDocument02.append(kvp("addedByUserId", bsoncxx::types::b_int64(value.addedByUserId)), kvp("addedByUserName", value.addedByUserName.c_str()),
									kvp("contentLength", bsoncxx::types::b_int32(value.contentLength)), kvp("description", value.description.c_str()),
									kvp("duration", value.duration.c_str()), kvp("thumbnailUrl", value.thumbnailUrl.c_str()), kvp("songId", value.songId.c_str()),
									kvp("type", bsoncxx::types::b_int32(( int )value.type)), kvp("viewUrl", value.viewUrl.c_str()));
								subDocument02.append(kvp("songTitle", value.songTitle.c_str()), kvp("firstDownloadUrl", value.firstDownloadUrl.c_str()),
									kvp("secondDownloadUrl", value.secondDownloadUrl.c_str()));
							});
						}
					}));
				}));
				return buildDoc;
			} catch (...) {
				reportException("DatabaseManagerAgent::convertGuildDataToDBDoc()");
				return buildDoc;
			}
		};

		static DiscordGuildData parseGuildData(bsoncxx::document::value docValue) {
			DiscordGuildData guildData{};
			try {
				auto currentSongValue = docValue.view()["playlist"].get_document().value["currentSong"].get_document();
				guildData.playlist.currentSong.addedByUserId = currentSongValue.value["addedByUserId"].get_int64().value;
				guildData.playlist.currentSong.addedByUserName = currentSongValue.value["addedByUserName"].get_utf8().value.to_string();
				guildData.playlist.currentSong.firstDownloadUrl = currentSongValue.value["firstDownloadUrl"].get_utf8().value.to_string();
				guildData.playlist.currentSong.secondDownloadUrl = currentSongValue.value["secondDownloadUrl"].get_utf8().value.to_string();
				guildData.playlist.currentSong.contentLength = currentSongValue.value["contentLength"].get_int32().value;
				guildData.playlist.currentSong.description = currentSongValue.value["description"].get_utf8().value.to_string();
				guildData.playlist.currentSong.thumbnailUrl = currentSongValue.value["thumbnailUrl"].get_utf8().value.to_string();
				guildData.playlist.currentSong.duration = currentSongValue.value["duration"].get_utf8().value.to_string();
				guildData.playlist.currentSong.songId = currentSongValue.value["songId"].get_utf8().value.to_string();
				guildData.playlist.currentSong.songTitle = currentSongValue.value["songTitle"].get_utf8().value.to_string();
				guildData.playlist.currentSong.type = static_cast<SongType>(currentSongValue.value["type"].get_int32().value);
				guildData.playlist.currentSong.viewUrl = currentSongValue.value["viewUrl"].get_utf8().value.to_string();
				guildData.playlist.isLoopSongEnabled = docValue.view()["playlist"].get_document().value["isLoopSongEnabled"].get_bool().value;
				guildData.playlist.isLoopAllEnabled = docValue.view()["playlist"].get_document().value["isLoopAllEnabled"].get_bool().value;

				for (auto& value02: currentSongValue.value["downloadUrls"].get_array().value) {
					DownloadUrl downloadUrl;
					downloadUrl.contentSize = value02["contentSize"].get_int32().value;
					downloadUrl.urlPath = value02["urlPath"].get_utf8().value.to_string();
					guildData.playlist.currentSong.finalDownloadUrls.push_back(downloadUrl);
				}
				for (auto& value: docValue.view()["playlist"].get_document().view()["songList"].get_array().value) {
					Song newSong{};
					for (auto& value02: value["downloadUrls"].get_array().value) {
						DownloadUrl downloadUrl;
						downloadUrl.contentSize = value02["contentSize"].get_int32().value;
						downloadUrl.urlPath = value02["urlPath"].get_utf8().value.to_string();
						newSong.finalDownloadUrls.push_back(downloadUrl);
					}
					newSong.addedByUserName = value["addedByUserName"].get_utf8().value.to_string();
					newSong.description = value["description"].get_utf8().value.to_string();
					newSong.addedByUserId = value["addedByUserId"].get_int64().value;
					newSong.duration = value["duration"].get_utf8().value.to_string();
					newSong.thumbnailUrl = value["thumbnailUrl"].get_utf8().value.to_string();
					newSong.contentLength = value["contentLength"].get_int32().value;
					newSong.type = ( SongType )value["type"].get_int32().value;
					newSong.songId = value["songId"].get_utf8().value.to_string();
					newSong.firstDownloadUrl = value["firstDownloadUrl"].get_utf8().value.to_string();
					newSong.secondDownloadUrl = value["secondDownloadUrl"].get_utf8().value.to_string();
					newSong.songTitle = value["songTitle"].get_utf8().value.to_string();
					newSong.viewUrl = value["viewUrl"].get_utf8().value.to_string();
					guildData.playlist.songQueue.push_back(newSong);
				}
				guildData.djRoleId = docValue.view()["djRoleId"].get_int64().value;
				guildData.borderColor = docValue.view()["borderColor"].get_utf8().value.to_string();
				guildData.guildName = docValue.view()["guildName"].get_utf8().value.to_string();
				guildData.guildId = docValue.view()["guildId"].get_int64().value;
				for (auto& value: docValue.view()["musicChannelIds"].get_array().value) {
					guildData.musicChannelIds.push_back(value.get_int64().value);
				}
				guildData.memberCount = docValue.view()["memberCount"].get_int32().value;
				return guildData;
			} catch (...) {
				reportException("DatabaseManagerAgent::parseGuildData()");
				return guildData;
			}
		};

		static bsoncxx::builder::basic::document convertGuildMemberDataToDBDoc(DiscordGuildMemberData discordGuildMemberData) {
			bsoncxx::builder::basic::document buildDoc{};
			try {
				using bsoncxx::builder::basic::kvp;
				buildDoc.append(kvp("guildMemberMention", discordGuildMemberData.guildMemberMention.c_str()));
				buildDoc.append(kvp("_id", discordGuildMemberData.globalId.c_str()));
				buildDoc.append(kvp("guildId", bsoncxx::types::b_int64(discordGuildMemberData.guildId)));
				buildDoc.append(kvp("guildMemberId", bsoncxx::types::b_int64(discordGuildMemberData.guildMemberId)));
				buildDoc.append(kvp("globalId", discordGuildMemberData.globalId.c_str()));
				buildDoc.append(kvp("userName", discordGuildMemberData.userName.c_str()));
				buildDoc.append(kvp("displayName", discordGuildMemberData.displayName.c_str()));
				return buildDoc;
			} catch (...) {
				reportException("DatabaseManagerAgent::convertGuildDataToDBDoc()");
				return buildDoc;
			}
		};

		static DiscordGuildMemberData parseGuildMemberData(bsoncxx::document::value docValue) {
			DiscordGuildMemberData guildMemberData{};
			try {
				guildMemberData.guildMemberMention = docValue.view()["guildMemberMention"].get_utf8().value.to_string();
				guildMemberData.guildId = docValue.view()["guildId"].get_int64().value;
				guildMemberData.displayName = docValue.view()["displayName"].get_utf8().value.to_string();
				guildMemberData.globalId = docValue.view()["globalId"].get_utf8().value.to_string();
				guildMemberData.guildMemberId = docValue.view()["guildMemberId"].get_int64().value;
				guildMemberData.userName = docValue.view()["userName"].get_utf8().value.to_string();
				return guildMemberData;
			} catch (...) {
				reportException("DatabaseManagerAgent::parseGuildData()");
				return guildMemberData;
			}
		};
	};

	class DiscordUser {
	  public:
		static int32_t guildCount;

		DiscordUserData data{};

		DiscordUser(std::string userNameNew, uint64_t userIdNew) {
			this->data.userId = userIdNew;
			this->data.userName = userNameNew;
			this->getDataFromDB();
			this->data.userId = userIdNew;
			this->data.userName = userNameNew;
			this->writeDataToDB();
		}

		void writeDataToDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_USER_WRITE;
			workload.userData = this->data;
			DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
		}

		void getDataFromDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_USER_READ;
			workload.userData = this->data;
			auto result = DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
			if (result.discordUser.userId != 0) {
				this->data = result.discordUser;
			}
		}
	};

	class DiscordGuild {
	  public:
		DiscordGuildData data{};

		DiscordGuild(GuildData guildData) {
			this->data.guildId = guildData.id;
			this->data.guildName = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->getDataFromDB();
			this->data.guildId = guildData.id;
			this->data.guildName = guildData.name;
			this->data.memberCount = guildData.memberCount;
			this->writeDataToDB();
		}

		void writeDataToDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_WRITE;
			workload.guildData = this->data;
			DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
		}

		void getDataFromDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_READ;
			workload.guildData = this->data;
			auto result = DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
			if (result.discordGuild.guildId != 0) {
				this->data = result.discordGuild;
			}
		}
	};

	class DiscordGuildMember {
	  public:
		DiscordGuildMemberData data{};

		DiscordGuildMember(GuildMemberData guildMemberData) {
			this->data.guildMemberId = guildMemberData.id;
			this->data.guildId = guildMemberData.guildId;
			this->data.globalId = std::to_string(this->data.guildId) + " + " + std::to_string(this->data.guildMemberId);
			this->getDataFromDB();
			if (guildMemberData.nick == "") {
				this->data.displayName = guildMemberData.userName;
				this->data.guildMemberMention = "<@" + std::to_string(this->data.guildMemberId) + ">";
			} else {
				this->data.displayName = guildMemberData.nick;
				this->data.guildMemberMention = "<@!" + std::to_string(this->data.guildMemberId) + ">";
			}
			this->data.userName = guildMemberData.userName;
			this->writeDataToDB();
		}

		void writeDataToDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_MEMBER_WRITE;
			workload.guildMemberData = this->data;
			DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
		}

		void getDataFromDB() {
			DatabaseWorkload workload{};
			workload.workloadType = DatabaseWorkloadType::DISCORD_GUILD_MEMBER_READ;
			workload.guildMemberData = this->data;
			auto result = DatabaseManagerAgent::submitWorkloadAndGetResults(workload);
			if (result.discordGuildMember.globalId != "") {
				this->data = result.discordGuildMember;
			}
		}
	};

	void savePlaylist(DiscordGuild& guild) {
		Playlist playlist = getSongAPIMap().at(guild.data.guildId)->playlist;
		getSongAPIMap().at(guild.data.guildId)->playlist = playlist;
		guild.data.playlist = playlist;
		guild.writeDataToDB();
	}

	Playlist loadPlaylist(DiscordGuild& guild) {
		guild.getDataFromDB();
		if (getSongAPIMap().contains(guild.data.guildId)) {
			getSongAPIMap().at(guild.data.guildId)->playlist = guild.data.playlist;
			return guild.data.playlist;
		} else {
			return Playlist();
		}
	}

	mongocxx::instance DatabaseManagerAgent::instance{};
	mongocxx::pool DatabaseManagerAgent::thePool{ mongocxx::uri{} };
	std::mutex DatabaseManagerAgent::workloadMutex01{};
	std::mutex DatabaseManagerAgent::workloadMutex02{};
	uint64_t DatabaseManagerAgent::botUserId{ 0 };
	int32_t DiscordUser::guildCount{ 0 };

}
