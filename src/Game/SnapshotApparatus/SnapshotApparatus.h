#pragma once
#include "Game/GhidraDefs.h"
#include "Core/Interfaces.h"
class Snapshot {
	char padding[0xa10000];
};
static Snapshot snapshot_replay_pre_allocated[1]; //keeping this with only one element for now while its not used for any implementation to save space



class SnapshotApparatus {


public:
	unsigned int snapshot_count;
	CharData* p1_ptr;//p1 CharData*
	CharData* p2_ptr; //p2 CharData*
	//p1 and p2 ptrs are used for now to determine when I need to remake the snapshot
	GGPOSessionCallbacks*  callbacks_ptr;
	SnapshotApparatus();

	
bool save_snapshot(Snapshot** pbuf);
bool load_snapshot(Snapshot* buf);
bool check_if_valid(CharData* p1, CharData* p2);
};