#ifndef extendible_hash_table_h
#define extendible_hash_table_h

#include <iostream>
#include <functional>
#include <queue>
#include <vector>
#include <boost/thread.hpp>
#include "abstract_hash_table.h"

#define CACHE_LINE_SIZE 64
typedef boost::shared_mutex shared_mutex;
typedef std::uint32_t hash_value_t;

namespace multicore_hash {
	template<typename key_t, typename value_t>
	class extendible_hash_table : public abstract_hash_table<key_t, value_t> {
	private:
		abstract_hash<hash_value_t> *hash;
			
		const std::uint8_t bucket_entries = 4; //(CACHE_LINE_SIZE - sizeof(std::uint8_t))/sizeof(hash_entry);
		struct alignas(CACHE_LINE_SIZE) hash_bucket {
			std::uint8_t  local_depth;
			std::uint8_t  entry_count;
			const std::uint32_t origin_index;

			key_t *keys;
			value_t *values;
			shared_mutex local_mutex;

			hash_bucket(std::uint8_t _local_depth, std::uint8_t _bucket_entries, std::uint32_t _origin_index) : local_depth(_local_depth), origin_index(_origin_index) {
				keys = new key_t[_bucket_entries];
				values = new value_t[_bucket_entries];
				entry_count = 0;
			}

			~hash_bucket() {
				delete[] keys;
				delete[] values;
			}
			void clear(){
				for (std::uint8_t e = 0; e < entry_count; e++) {
					keys[e] = "";
					values[e] = "";
				}
				entry_count = 0;
			}
			void insert_next(key_t new_key, value_t new_value) {
				keys  [entry_count] = new_key;
				values[entry_count] = new_value;
				entry_count++;
			}

			void move_last_to(std::uint32_t i) {
				keys  [i] = keys[entry_count-1];
				values[i] = values[entry_count-1];
				keys  [entry_count-1] = "";
				values[entry_count-1] = "";
				entry_count--;

			}
		};

		std::uint8_t global_depth = 2;
		std::uint64_t entry_count = 0;
		hash_bucket **directory;

		shared_mutex global_mutex;

		std::uint32_t createBitMask(std::uint32_t b)
		{
			std::uint32_t r = 0;
			for (std::uint32_t i=0; i<=b; i++)
				r |= 1 << i;
			return r;
		}

		void create_bucket_and_insert(hash_bucket *bucket, std::vector<hash_bucket*> &buckets_to_insert, std::uint32_t bucket_number, key_t key, value_t value) {
			// Redistribute entries from bucket
			bucket->local_depth++;
			std::string text = "Bucket: " + std::to_string(bucket_number) + ' ' + std::to_string((int)bucket->local_depth);
			std::cout << text << std::endl;
			// std::cout << "LD: " << (int)bucket->local_depth << std::endl;
			// std::cout << "BN: " << (int)bucket_number << std::endl;
			bucket->entry_count = 0;
			std::uint32_t image_number = bucket_number + (1<<(bucket->local_depth-1));
			// std::cout << "IN: " << (int)image_number << std::endl;
			hash_bucket *image_bucket = new hash_bucket(bucket->local_depth, bucket_entries, image_number);
			for (uint8_t i = 0; i < bucket_entries; i++) {
				if (!((hash->get_hash(bucket->keys[i]) >> (bucket->local_depth-1)) & 1)) { // Original bucket
					bucket->insert_next(bucket->keys[i], bucket->values[i]);
				}
				else { // Image bucket
					image_bucket->insert_next(bucket->keys[i], bucket->values[i]);
				}
			}
			// std::cout << "BEC: " << (int)bucket->entry_count << std::endl;
			// std::cout << "IEC: " << (int)image_bucket->entry_count << std::endl;

			// Insert new value
			hash_value_t new_hash_value = hash->get_hash(key);
			// std::cout << "NVH: " << (int)new_hash_value << std::endl;

			if ((!((new_hash_value >> (bucket->local_depth-1)) & 1)) && bucket->entry_count < bucket_entries) { // Insert new in original bucket
				// std::cout << "BInsert: " << std::endl;
				bucket->insert_next(key, value);
				// std::cout << "BEC: " << (int)bucket->entry_count << std::endl;
			} else if (((new_hash_value >> (bucket->local_depth-1)) & 1) && image_bucket->entry_count < bucket_entries) { // Insert new in image bucket
				// std::cout << "IInsert: " << std::endl;
				image_bucket->insert_next(key, value);
			}
			buckets_to_insert.push_back(image_bucket);
		}

	public:
		extendible_hash_table(abstract_hash<hash_value_t> *_hash) : hash(_hash) {
			directory = (hash_bucket**) malloc(directory_size()*sizeof(hash_bucket*));
			for (std::uint8_t b = 0; b < directory_size(); b++) {
				hash_bucket *bucket = new hash_bucket(global_depth, bucket_entries, b);
				directory[b] = bucket;
			}
		}
		~extendible_hash_table() {
			boost::unique_lock<shared_mutex> global_exclusive_lock(global_mutex);
			// global_lock->lock();
			for (std::uint32_t b = 0; b < directory_size(); b++){
				if (directory[b]) {
					// Deallocate the memory
					if (directory[b]->origin_index == b)
						delete directory[b];
					
					// void actual pointer
					directory[b] = NULL;
				}
			}
			free(directory);
			global_exclusive_lock.unlock();
		}

		void print_extendible_hash_table(bool exclusive) {
			for (std::uint8_t j = 0; j < bucket_entries; j++)
				std::cout << "_______";
			std::cout << "_______" << "_______" << "_______" << "_______" << std::endl;
			for (std::uint32_t i = 0; i < directory_size(); i++)
				print_extendible_hash_bucket(directory[i], i, exclusive);
			for (std::uint8_t j = 0; j < bucket_entries; j++)
				std::cout << "_______";
			std::cout << "_______" << "_______" << "_______" << "_______" << std::endl;
		}
		void print_extendible_hash_bucket(hash_bucket* bucket, std::uint32_t i, bool exclusive) {
			std::cout << ((bucket->origin_index == i) ? "\033[1m " : "\033[0m "); 
			std::cout << (i < 10 ? "   " : (i < 100 ? "  " : (i < 1000 ? " " : ""))) << (int) i << " | ";
			std::cout << "\033[0m";
			std::cout << ((bucket->origin_index == i) ? "\033[1;31m" : "\033[0;31m"); 
			std::cout << (bucket->origin_index < 10 ? "   " : (bucket->origin_index < 100 ? "  " : (bucket->origin_index < 1000 ? " " : ""))) << (int) bucket->origin_index;
			std::cout << "\033[0m";
			std::cout << " | ";
			std::cout << ((bucket->origin_index == i) ? "\033[1;32m" : "\033[0;32m"); 
			std::cout << (bucket->local_depth < 10 ? "   " : (bucket->local_depth < 100 ? "  " : (bucket->local_depth < 1000 ? " " : ""))) << (int) bucket->local_depth;
			std::cout << "\033[0m";
			std::cout << " | ";
			std::cout << ((bucket->origin_index == i) ? "\033[1;33m" : "\033[0;33m"); 
			std::cout << (bucket->entry_count < 10 ? "   " : (bucket->entry_count < 100 ? "  " : (bucket->entry_count < 1000 ? " " : ""))) << (int) bucket->entry_count;
			std::cout << ((bucket->origin_index == i) ? "\033[1;34m" : "\033[0m"); 
			std::cout << " | ";
			for (std::uint8_t j = 0; j < bucket_entries; j++)
			{
				if (!exclusive || bucket->origin_index == i){
					std::uint32_t value = atoi(bucket->keys[j].c_str());
					if (j >= bucket->entry_count)
						std::cout << "     | ";
					else
						std::cout << ((value < 10 ? "   " : (value < 100 ? "  " : (value < 1000 ? " " : ""))) + bucket->keys[j]) << " | ";
				}
				else 
					std::cout << "     | ";
			}
			std::cout << std::endl;
			std::cout << "\033[0m";
		}

		bool get(const key_t& key, value_t& value) override {
			hash_value_t hash_value = hash->get_hash(key);
			
			// Take global lock shared;
			boost::shared_lock<shared_mutex> global_shared_lock(global_mutex);
			std::uint32_t bucket_number = hash_value & (directory_size()-1);

			// Take local lock shared;
			boost::shared_lock<shared_mutex> local_shared_lock(directory[bucket_number]->local_mutex);
			for (uint8_t i = 0; i < bucket_entries; i++) {
				if (directory[bucket_number]->keys[i] == key) {
					value = directory[bucket_number]->values[i];
					local_shared_lock.unlock();
					global_shared_lock.unlock();
					return true;
				}
			}
			local_shared_lock.unlock();
			global_shared_lock.unlock();
			return false;
		}

		void insert(const key_t& key, const value_t& new_value) override {
			// std::cout << "KEY: " << key << std::endl;

			boost::shared_lock<shared_mutex> global_shared_lock(global_mutex);
			std::cout << key << std::endl;
			// Search for free slot
			hash_value_t hash_value = hash->get_hash(key);

			std::uint32_t bucket_number = directory[hash_value & createBitMask(global_depth-1)]->origin_index;

			boost::unique_lock<shared_mutex> local_exclusive_lock(directory[bucket_number]->local_mutex);
			if (directory[bucket_number]->entry_count < bucket_entries) {
				directory[bucket_number]->insert_next(key, new_value);
				local_exclusive_lock.unlock();
				global_shared_lock.unlock();
				return;
			}

			std::vector<hash_bucket*> buckets_to_insert;

			//      Not made                      Original bucket overflow                    Image bucket overflow
			while (buckets_to_insert.empty() || !(buckets_to_insert.back()->entry_count) || !(directory[bucket_number]->entry_count)) {
				if (buckets_to_insert.empty() || !(buckets_to_insert.back()->entry_count)) {
					create_bucket_and_insert(directory[bucket_number], buckets_to_insert, bucket_number, key, new_value);
				} else {
					create_bucket_and_insert(buckets_to_insert.back(), buckets_to_insert, bucket_number, key, new_value);
				}
			}

			// Check if no split happened at all
			if (buckets_to_insert.empty()) {
				local_exclusive_lock.unlock();
				global_shared_lock.unlock();
				return;
			}
			boost::upgrade_lock<shared_mutex> global_upgrade_lock(global_mutex);
			// Check if directory doesn't need to be doubled
			if (buckets_to_insert.back()->local_depth <= global_depth) {
				auto image_bucket = buckets_to_insert.back();
				auto tmp_index = image_bucket->origin_index;
				while (tmp_index < directory_size()) {
					directory[tmp_index] = image_bucket;
					tmp_index += (1<<image_bucket->local_depth);
				}
				// print_extendible_hash_table(false);
				local_exclusive_lock.unlock();
				global_shared_lock.unlock();
				return;
			}	
			std::string text = "SPLITTING: " + std::to_string((int)bucket_number);
			std::cout << text << std::endl;

			std::uint8_t splits = buckets_to_insert.back()->local_depth - global_depth;
			std::cout << "GD: " << (int)global_depth << std::endl;
			std::cout << "LD: " << (int)buckets_to_insert.back()->local_depth << std::endl;
			std::cout << "SS: " << (int)splits << std::endl;
			std::uint32_t old_size = (1<<global_depth);
			std::uint32_t new_size = 1 << (global_depth + splits);
			hash_bucket** tmp_directory = (hash_bucket**) realloc(directory, new_size*sizeof(hash_bucket*));
			std::cout << "Realloc Done: " << new_size << std::endl;
			
			if (!tmp_directory) {
				std::cout << "REALLOC ERROR" << std::endl;
			}		
			for (typename std::vector<hash_bucket*>::iterator it = buckets_to_insert.begin(); it != buckets_to_insert.end(); ++it) {
				std::copy(&tmp_directory[0], &tmp_directory[old_size], &tmp_directory[old_size]);
				old_size <<= 1;
				hash_bucket* image_bucket = *it;
				tmp_directory[image_bucket->origin_index] = image_bucket;
			}
			directory = tmp_directory;
			global_depth += splits;
			// print_extendible_hash_table(false);
			// print_extendible_hash_table(false);

			// // std::cout << "Rehash " << key << ", ec: " << (int)directory[bucket_number]->entry_count << std::endl;
			// // Split bucket, and rehash entries.
			// 	// Increase local depth
			// directory[bucket_number]->local_depth++;				
			// // std::cout << "Key: " << key << ", bucket_number " << bucket_number << ", local_depth " << (int)directory[bucket_number]->local_depth << std::endl; 

			// 	// Create mask to get specific bits
			// std::uint32_t mask = createBitMask(directory[bucket_number]->local_depth-1);
			
			// 	// Copy all entries
			// key_t tmp_keys[bucket_entries];
			// value_t tmp_values[bucket_entries];
			// std::copy(&directory[bucket_number]->keys[0], &directory[bucket_number]->keys[bucket_entries], &tmp_keys[0]);
			// std::copy(&directory[bucket_number]->values[0], &directory[bucket_number]->values[bucket_entries], &tmp_values[0]);

			// 	// Clear the bucket:
			// directory[bucket_number]->clear();

			// 	// Rehash entries in bucket.
			// for (uint8_t i = 0; i < bucket_entries; i++) {
			// 	std::uint32_t image_number = mask & hash->get_hash(tmp_keys[i]);
			// 	// std::cout << "Image Number: " << image_number << " entry: " << keys[i] << ' ' << std::endl;
			// 	if (directory[image_number]->origin_index != image_number)
			// 	{
			// 		hash_bucket *image = new hash_bucket(directory[bucket_number]->local_depth, bucket_entries, image_number);
			// 		directory[image_number] = image;
			// 		// std::cout << "Key: " << key << ", image_number " << image_number << ", local_depth " << (int)directory[image_number]->local_depth << std::endl; 
			// 		std::uint32_t tmp_image_number = image_number;
			// 		while (tmp_image_number < directory_size()) {
			// 			// std::cout << "Tmp Image Number: " << tmp_image_number << std::endl;
			// 			directory[tmp_image_number] = image;
			// 			tmp_image_number += 1<<(directory[bucket_number]->local_depth);
			// 		}
			// 	}
			// 	directory[image_number]->insert_next(tmp_keys[i], tmp_values[i]);
			// }		

			// if (directory[mask & hash_value]->entry_count >= bucket_entries){
			// 	local_exclusive_lock.unlock();
			// 	global_upgrade_lock.unlock();
			// 	return insert(key, new_value); // Do another iteration, increasing the size again
			// }
			// directory[mask & hash_value]->insert_next(key, new_value);

			local_exclusive_lock.unlock();
			global_upgrade_lock.unlock();
		}
		// Returns previous value, if found, -1 otherwise
		void update(const key_t& key, const value_t& new_value) override {
			hash_value_t hash_value = hash->get_hash(key);

			boost::shared_lock<shared_mutex> global_shared_lock(global_mutex);
			std::uint32_t bucket_number = hash_value & (directory_size()-1);
			
			boost::unique_lock<shared_mutex> local_exclusive_lock(directory[bucket_number]->local_mutex);
			for (uint8_t i = 0; i < bucket_entries; i++) {
				if (directory[bucket_number]->keys[i] == key) {
					directory[bucket_number]->values[i] = new_value;
					break;
				}
			}
			local_exclusive_lock.unlock();
			global_shared_lock.unlock();
		}

		// Returns deleted value, if found, -1 otherwise
		void remove(const key_t& key) override {
			hash_value_t hash_value = hash->get_hash(key);
			boost::shared_lock<shared_mutex> global_shared_lock(global_mutex);
			std::uint32_t bucket_number = hash_value & (directory_size()-1);
			boost::unique_lock<shared_mutex> local_exclusive_lock(directory[bucket_number]->local_mutex);
			for (uint8_t i = 0; i < directory[bucket_number]->entry_count; i++) {
				if (directory[bucket_number]->keys[i] == key) { // Entry to be updated found
					directory[bucket_number]->move_last_to(i);
					break;
				}
			}
			local_exclusive_lock.unlock();
			global_shared_lock.unlock();
		}

		void range_scan(const key_t& start_key, const key_t* end_key, abstract_push_op& apo) override{
			typedef std::tuple<key_t, value_t> hash_entry;

			auto cmp = [](hash_entry a, hash_entry b) { return std::get<0>(a) > std::get<0>(b);};
			std::priority_queue<hash_entry, std::vector<hash_entry>, decltype(cmp)> pri_queue(cmp);

			// FULL SCAN
			boost::shared_lock<shared_mutex> global_shared_lock(global_mutex);
			for (std::uint32_t i = 0; i < directory_size(); i++) {			
				for (std::uint8_t j = 0; j < directory[i]->entry_count; j++) {
					if (directory[i]->keys[j] >= start_key && directory[i]->keys[j] <= *end_key)
						pri_queue.push(std::make_tuple(directory[i]->keys[j], directory[i]->values[j]));
				}
			}
			// Apply push op
			while(!pri_queue.empty()) {
				hash_entry current = pri_queue.top();
				key_t key     = std::get<0>(current);
				value_t value = std::get<1>(current);
				const char* keyp = key.c_str();
				if (!apo.invoke(keyp, key.length(), value)) {
					global_shared_lock.unlock();
					return;
				}
				pri_queue.pop();
			}
			global_shared_lock.unlock();
		}

		void reverse_range_scan(const key_t& start_key, const key_t* end_key, abstract_push_op& apo) override{
			typedef std::tuple<key_t, value_t> hash_entry;

			auto cmp = [](hash_entry a, hash_entry b) { return std::get<0>(a) < std::get<0>(b);};
			std::priority_queue<hash_entry, std::vector<hash_entry>, decltype(cmp)> pri_queue(cmp);

			// FULL SCAN
			boost::shared_lock<shared_mutex> global_shared_lock(global_mutex);
			for (std::uint32_t i = 0; i < directory_size(); i++) {			
				for (std::uint8_t j = 0; j < directory[i]->entry_count; j++) {
					if (directory[i]->keys[j] >= start_key && directory[i]->keys[j] <= *end_key)
						pri_queue.push(std::make_tuple(directory[i]->keys[j], directory[i]->values[j]));
				}
			}
			// Apply push op
			while(!pri_queue.empty()) {
				hash_entry current = pri_queue.top();
				key_t key     = std::get<0>(current);
				value_t value = std::get<1>(current);
				const char* keyp = key.c_str();
				if (!apo.invoke(keyp, key.length(), value)) {
					global_shared_lock.unlock();
					return;
				}
				pri_queue.pop();
			}
			global_shared_lock.unlock();
		}

		std::uint8_t get_global_depth() {
			return global_depth;
		}
		std::uint8_t get_bucket_entries() {
			return bucket_entries;
		}
		hash_bucket **get_directory() {
			return directory;
		}

		size_t directory_size() {
			return (1<<global_depth);
		}
		size_t size() {
			boost::shared_lock<shared_mutex> global_shared_lock(global_mutex);
			size_t total_entry_count = 0;
			for (std::uint32_t i = 0; i < directory_size(); i++) {
				if (directory[i]->origin_index == i) {
					total_entry_count += directory[i]->entry_count;
				}
			}
			global_shared_lock.unlock();
			return total_entry_count;
		}
	};
}

#endif