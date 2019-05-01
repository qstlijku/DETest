#include "DARE.h"

#include "FileHandler.h"
#include "IBinaryArchive.h"
#include "DB.h"

void DARE::addSoundResource(uint32_t res) {
	char buffer[80];
	snprintf(buffer, sizeof(buffer), "soundbinary/%08x.spk", res);

	SDL_RWops *fp = FH::openFile(buffer);
	spkFile &spk = spks[res];
	spk.open(CBinaryArchiveReader(fp));
	SDL_RWclose(fp);
}

void DARE::addAtomicObject(uint32_t res) {
	char buffer[80];
	snprintf(buffer, sizeof(buffer), "soundbinary/%08x.sbao", res);

	SDL_RWops *fp = FH::openFile(buffer);
	if (fp) {
		sbaoFile &sbao = atomicObjects[res];
		sbao.open(CBinaryArchiveReader(fp), SDL_RWsize(fp));
		SDL_RWclose(fp);
	}
}

void DARE::reset() {
	atomicObjects.clear();
	spks.clear();
}

sbaoFile& DARE::loadAtomicObject(uint32_t res) {
	//Search Through SPKs
	for (auto& it : spks) {
		for (auto& sbao : it.second.objs) {
			if (sbao.Id == res)
				return sbao;
		}
	}

	if (atomicObjects.count(res) == 0)
		addAtomicObject(res);

	SDL_assert_release(atomicObjects.count(res));
	return atomicObjects[res];
}

bool DARE::isAtomicObjectLoaded(uint32_t res) {
	return atomicObjects.count(res);
}

void DARE::addAODependency(uint32_t res) {
	/*uint32_t spkRes = DB::instance().getSpkFromSBAO(res);
	if (spkRes != -1 && spks.count(spkRes) == 0) {
		if (spkRes == 0x2fffffff) return;
		addSoundResource(spkRes);
	}*/
}

void DARE::addSndDataDependency(uint32_t res) {
	/*uint32_t spkRes = DB::instance().getSpkFromSBAO(res);
	if (spkRes != -1 && spks.count(spkRes) == 0)
		addSoundResource(spkRes);
	else if (spkRes == -1) {
		try {
			loadAtomicObject(res);
		} catch(...) {}
	}*/
}

DARE & DARE::instance() {
	static DARE dare;
	return dare;
}
