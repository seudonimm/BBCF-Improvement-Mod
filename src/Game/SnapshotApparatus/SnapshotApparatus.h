#pragma once
#include "Game/GhidraDefs.h"
class Snapshot {
	char padding[0xa10000];
};
static Snapshot snapshot_replay_pre_allocated[10];

class SnapshotApparatus {


public:
	unsigned int snapshot_count;
	GGPOSessionCallbacks*  callbacks_ptr;
	SnapshotApparatus();

bool save_snapshot(Snapshot** pbuf);
bool load_snapshot(Snapshot* buf);
};