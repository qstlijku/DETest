#include "DB.h"

#include <sqlite_modern_cpp.h>
#include "Hash.h"
#include "SDL.h"
#include "FileHandler.h"
#include <filesystem>

std::string base = SDL_GetBasePath();

DB::DB() {
	dbPath = base + "Disrupt1.db";
	//dbPath = ":memory:";

	db = new sqlite::database(dbPath);

	uint32_t version;
	*db << "PRAGMA user_version;" >> version;
	if (version != getVersion())
		reinit();
}

DB::~DB() {
	delete db;
}

std::unique_ptr<DB::File> DB::getFileByHash(uint32_t hash) {
	try {
		std::unique_ptr<DB::File> ptr = std::make_unique<DB::File>();
		ptr->hash = hash;
		*db << "select path, type from files where hash=?;" << hash >> tie(ptr->path, ptr->type);
		return ptr;
	}
	catch (...) { }
	return NULL;
}

std::unique_ptr<DB::File> DB::getFileByPath(const char* path) {
	try {
		std::unique_ptr<DB::File> ptr = std::make_unique<DB::File>();
		ptr->path = path;
		*db << "select hash, type from files where path=?;" << path >> tie(ptr->hash, ptr->type);
		return ptr;
	}
	catch (...) {}
	return NULL;
}

std::string DB::getStrFromCRC(uint32_t hash) {
	//Yes, I know we have a DB for this, but it's just too slow otherwise
	static std::unordered_map<uint32_t, std::string> cache;
	auto it = cache.find(hash);
	if (it != cache.end())
		return it->second;

	try {
		std::string str;
		*db << "select str from hashes where hash=?;" << hash >> str;
		cache[hash] = str;
		return str;
	}
	catch (...) {
		char buffer[12];
		snprintf(buffer, sizeof(buffer), "_%08x", hash);
		cache[hash] = buffer;
		return std::string(buffer);
	}
}

std::string DB::getStrFromDobbs(uint32_t hash) {
	//Yes, I know we have a DB for this, but it's just too slow otherwise
	static std::unordered_map<uint32_t, std::string> cache;
	auto it = cache.find(hash);
	if (it != cache.end())
		return it->second;

	try {
		std::string str;
		*db << "select str from hashes where dobbs=?;" << hash >> str;
		cache[hash] = str;
		return str;
	}
	catch (...) {
		char buffer[12];
		snprintf(buffer, sizeof(buffer), "_%08x", hash);
		cache[hash] = buffer;
		return std::string(buffer);
	}
}

uint32_t DB::getSpkFromSBAO(uint32_t resID) {
	try {
		uint32_t spk;
		*db << "select spk from dare where sbao=? LIMIT 1;" << resID >> spk;
		return spk;
	}
	catch (...) { }
	return -1;
}

uint64_t DB::getVersion() {
	return 201;
}

void DB::reinit() {
	//Delete tables
	*db << "DROP TABLE IF EXISTS files;";
	*db << "DROP TABLE IF EXISTS hashes;";
	*db << "DROP TABLE IF EXISTS dare;";

	*db << "PRAGMA TEMP_STORE = MEMORY;";
	*db << "PRAGMA JOURNAL_MODE = MEMORY;";
	*db << "PRAGMA LOCKING_MODE = EXCLUSIVE;";
	*db << "PRAGMA SYNCHRONOUS = OFF;";

	//FNV hash table
	*db <<
		"create table if not exists files ("
		"   hash integer not null,"
		"   hash64 integer not null,"
		"   path text primary key not null,"
		"   type text not null"
		");";

	//CRC and dobbs Hash Table
	*db <<
		"create table if not exists hashes ("
		"   hash integer not null,"
		"   dobbs integer not null,"
		"   str text primary key not null,"
		"   type text not null"
		");";

	//Dare Table
	*db <<
		"create table if not exists dare ("
		"   spk integer not null,"
		"   sbao integer not null"
		");";

	*db << "begin;";
	char buffer[500];

	//Fill with Known files
	handleFNVFile((base + "res/Watch Dogs.filelist").c_str());

	//Fill with known FNV
	handleFNVFile((base + "res/arches.txt").c_str());

	handleFNVFile((base + "res/archeBrute.txt").c_str());

	handleCRCFile((base + "res/classNames.txt").c_str(), "ClassNames");
	handleCRCFile((base + "res/exeStrings.txt").c_str(), "etc");
	handleCRCFile((base + "res/strings.txt").c_str(), "etc");
	handleCRCFile((base + "res/materialNames.txt").c_str(), "Material");

	FILE *fp = fopen((base + "res/dare.txt").c_str(), "r");
	auto psa = *db << "insert into dare (spk,sbao) values (?,?);";
	while (fgets(buffer, sizeof(buffer), fp)) {
		buffer[strlen(buffer) - 1] = '\0';
		uint32_t spk, sbao;
		sscanf(buffer, "%u,%u", &spk, &sbao);
		psa << spk << sbao;
		psa++;
	}
	fclose(fp);
	psa.used(true);

	*db << "commit;";

	*db << ("PRAGMA user_version = " + std::to_string(getVersion()) + ";");
}

DB & DB::instance() {
	static DB db;
	return db;
}

void DB::handleCRCFile(const char *file, const char* type) {
	FILE *fp = fopen(file, "r");

	char line[512];
	auto ps = *db << "insert into hashes (hash,dobbs,str,type) values (?,?,?,?);";
	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = '\0';

		uint32_t hash = Hash::crc32buf((const char*)line, strlen(line));
		uint32_t dobbs = Hash::gearDobbsHash((const unsigned char*)line, strlen(line));
		try {
			ps << hash << dobbs << line << type;
			ps++;
		}
		catch (...) {
			//SDL_Log("Duplicate key %08x %s", hash, line);
		}
	}
	ps.used(true);

	fclose(fp);
}

void DB::handleFNVFile(const char* file) {
	FILE* fp = fopen(file, "r");

	char line[512];
	auto ps = *db << "insert into files (hash,hash64,path,type) values (?,?,?,?);";
	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = '\0';
		uint32_t hash = Hash::getFilenameHash(line);
		uint64_t hash64 = Hash::getFilenameHash64(line);
		try {
			ps << hash << hash64 << line << FH::getTypeFromExtension(line);
			ps++;
		}
		catch (...) {
			//SDL_Log("Duplicate key %08x %s", hash, line);
		}
	}
	ps.used(true);

	fclose(fp);
}
