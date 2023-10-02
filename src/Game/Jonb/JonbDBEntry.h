#pragma once
#include <vector>
#include <string>
#include <ctype.h>
enum BoxEntry_
{
	BoxEntryType_Hurtbox,
	BoxEntryType_Hitbox
};

class BoxEntry
{
public:
	BoxEntry_ type;
	float offsetX;
	float offsetY;
	float width;
	float height;
};

class JonbDBEntry {
public:
	char JONB[6]; //just JONB  literal string + 2 bytes
	char sprite_name[16]; //for ex ar000_02.bmp
	char pad_0[22];
	uint8_t pad_2;
	uint8_t hurtbox_count;//uint8_t
	uint8_t pad_3;
	uint8_t hitbox_count;//uint8_t
	char pad_1[0xa3];
	//BoxEntry* all_boxes; //I should implement later, leave it for now for it is not necessary, can check hitbox/hurtbox count for current purpose
};
class JonbDBIndexHeader {
public:
	char FPAC[4]; //just FPAC  literal string
	uint32_t offset_to_first_full_entry;
	char pad_0[24];
};
class JonbDBIndexEntry {
public:
	char jonbin_name[20]; //for ex ae030_08ex00.jonbin
	char pad_1[12];
	uint32_t offset_from_first_full_entry2; //some characters use the offset from this, others from the one after, idk what causes it to happen
	uint32_t offset_from_first_full_entry1; //see comment above
	char pad_2[8];
};
	