#include "NomadDB.h"

#include "IBinaryArchive.h"

void NomadDBRef::read(IBinaryArchive& fp) {
	fp.serialize(libID);
}
