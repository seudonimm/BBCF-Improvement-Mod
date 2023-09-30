#pragma once
#include "JonbDBReader.h"

//constexpr auto OFFSET_FROM_FPAC = 0x60;
constexpr auto FPAC_JONBIN_OFFSET_FROM_BBCF_P1 = 0x88E700; 
constexpr auto FPAC_JONBIN_OFFSET_FROM_BBCF_P2 = 0x88E760;
//constexpr auto PREINIT_OFFSET_FROM_BBCF_P1 = 0xDB6B2C;
//constexpr auto PREINIT_OFFSET_FROM_BBCF_P2 = 0xDB6B5C;

std::map<std::string, JonbDBEntry> JonbDBReader::parse_all_jonbins(char* bbcf_base_addr, int player_num) {
	auto fpac_offset = 0;
	std::map<std::string, JonbDBEntry> jonbin_map{};// = new std::map<std::string, JonbDBEntry>();
	if (player_num == 1) {
		fpac_offset = FPAC_JONBIN_OFFSET_FROM_BBCF_P1;
	}
	else {
		fpac_offset = FPAC_JONBIN_OFFSET_FROM_BBCF_P2;
	}

	JonbDBIndexHeader* jonb_index_header = *((JonbDBIndexHeader**)(bbcf_base_addr + fpac_offset));
	char* first_full_entry = (char*)jonb_index_header + jonb_index_header->offset_to_first_full_entry;
	//the index header has size 32, move 32 to the first JonbDBIndexEntry
	JonbDBIndexEntry* curr_addr = (JonbDBIndexEntry *)((char*)jonb_index_header + 32);
	auto iter = 0;
	bool get_alternative_offset = false; /*currently there are some characters like arakune who 
										 use the mystery_val instead of offset_from_first_full_entry for their offset, 
										 idk why that happens but this is to check for it and adjust accordingly*/
	while ((char*)curr_addr < first_full_entry) {
		//find the actual position from the index
		JonbDBEntry* entry_pos = (JonbDBEntry * )(first_full_entry + curr_addr->offset_from_first_full_entry);
		if (entry_pos->JONB[0] != 'J') {// if the value in JONB[0] isn't the literal 'J', need to use the mystery_val
			get_alternative_offset = true;
		}
		if (get_alternative_offset) {
			entry_pos = (JonbDBEntry*)(first_full_entry + curr_addr->mystery_val);
		}
		
		std::string jonbin_name = entry_pos->sprite_name; //for ex ar000_02.bmp
		if (!jonbin_name.empty() && jonbin_name.size() > 4) {
			jonbin_name.pop_back();//removes .bmp to for the map keys
			jonbin_name.pop_back();
			jonbin_name.pop_back();
			jonbin_name.pop_back();
		}
		jonbin_map[jonbin_name] = *entry_pos;


		//move to the next JonbDBIndexEntry
		curr_addr++;
	}
	return jonbin_map;
}