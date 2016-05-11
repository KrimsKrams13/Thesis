#ifndef extendible_hash_table_h
#define extendible_hash_table_h

#include <iostream>
#include <functional>
#include <queue>
#include <vector>
#include <boost/thread.hpp>
#include "abstract_hash_table.h"
#include "macros.h"
#include "push_ops.h"


typedef std::uint32_t hash_value_t;

namespace multicore_hash {
	template<std::uint8_t initial_global_depth>
	class extendible_hash_table : public abstract_hash_table {
	private:
		abstract_hash<hash_value_t> *hash;
			
		static const std::uint8_t bucket_entries = 4; //(CACHE_LINE_SIZE - sizeof(std::uint8_t))/sizeof(hash_entry);
		struct alignas(CACHE_LINE_SIZE) hash_bucket {
			std::uint8_t  local_depth;
			std::uint8_t  entry_count;
			const std::uint32_t original_index;

			std::string *keys;
			std::string *values;
			boost::shared_mutex local_mutex;

			hash_bucket(std::uint8_t _local_depth, std::uint8_t _bucket_entries, const std::uint32_t _original_index) : local_depth(_local_depth), original_index(_original_index){
				keys = new std::string[_bucket_entries];
				values = new std::string[_bucket_entries];
				entry_count = 0;
			}
			hash_bucket(const hash_bucket &other) : local_depth(other.local_depth), original_index(other.original_index){
				keys = new std::string[bucket_entries];
				values = new std::string[bucket_entries];
				std::copy(&other.keys[0], &other.keys[bucket_entries], &keys[0]);
				std::copy(&other.values[0], &other.values[bucket_entries], &values[0]);
				entry_count = other.entry_count;
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
			void set_equal_to_copy(const hash_bucket &other){
				local_depth  = other.local_depth;
				entry_count  = other.entry_count;
				std::copy(&other.keys[0], &other.keys[bucket_entries], &keys[0]);
				std::copy(&other.values[0], &other.values[bucket_entries], &values[0]);
			}
			void insert_next(std::string new_key, std::string new_value) {
				keys  [entry_count] = new_key;
				values[entry_count] = new_value;
				entry_count++;
			}

			void move_last_to(std::uint32_t i) {
				entry_count--;
				keys  [i] = keys[entry_count];
				values[i] = values[entry_count];
				keys  [entry_count] = "";
				values[entry_count] = "";

			}
		};

		std::uint8_t global_depth = initial_global_depth;
		hash_bucket **directory;

		boost::shared_mutex global_mutex;

		std::uint32_t createBitMask(std::uint32_t b)
		{
			std::uint32_t r = 0;
			for (std::uint32_t i=0; i<=b; i++)
				r |= 1 << i;
			return r;
		}

		void create_bucket_and_insert(hash_bucket *bucket, std::vector<hash_bucket*> &buckets_to_insert, std::uint32_t bucket_number, std::string key, std::string value) {
			// Redistribute entries from bucket
			// print_extendible_hash_bucket(bucket, bucket_number, false);
			bucket->local_depth++;
			bucket->entry_count = 0;
			std::uint32_t image_number = bucket_number + (1<<(bucket->local_depth-1));
			hash_bucket *image_bucket = new hash_bucket(bucket->local_depth, bucket_entries, image_number);
			for (uint8_t i = 0; i < bucket_entries; i++) {
				if (!((hash->get_hash(bucket->keys[i]) >> (bucket->local_depth-1)) & 1)) { // Original bucket
					bucket->insert_next(bucket->keys[i], bucket->values[i]);
				}
				else { // Image bucket
					image_bucket->insert_next(bucket->keys[i], bucket->values[i]);
				}
			}

			// Insert new value
			hash_value_t new_hash_value = hash->get_hash(key);
			if ((!((new_hash_value >> (bucket->local_depth-1)) & 1)) && bucket->entry_count < bucket_entries) { // Insert new in original bucket
				bucket->insert_next(key, value);
			} else if (((new_hash_value >> (bucket->local_depth-1)) & 1) && image_bucket->entry_count < bucket_entries) { // Insert new in image bucket
				image_bucket->insert_next(key, value);
			}
			buckets_to_insert.push_back(image_bucket);
		}

		std::uint8_t calc_new_local_depth(hash_bucket *bucket, const hash_value_t new_hash_value, std::uint32_t bucket_number) {
			// Calculating hash values
			hash_value_t hash_values[bucket_entries+1];
			for (std::uint8_t e = 0; e < bucket_entries; e++) {
				hash_values[e] = hash->get_hash(bucket->keys[e]);
			}
			hash_values[bucket_entries] = new_hash_value;

			// Checking common digits
			std::uint8_t new_local_depth = bucket->local_depth;
			std::uint8_t common_digit_nums;
			do {
				new_local_depth++;
				common_digit_nums = 0;
				for (std::uint8_t e = 0; e < bucket_entries+1; e++) {
					common_digit_nums += (hash_values[e] >> (new_local_depth-1)) & 1;
				}
			}
			while (common_digit_nums == 0 || common_digit_nums == bucket_entries+1);

			// Debugging section
			// std::cout << "nld: " << (int) new_local_depth << std::endl;
			if (new_local_depth >= 19) {
				std::cout << bucket_number << std::endl;
				// std::cout << "WHAT: " << std::endl;
				for (std::uint8_t e = 0; e < bucket_entries+1; e++) {
					std::bitset<32> x(hash_values[e]);
					std::cout << hash_values[e] << "\t" << x << std::endl;
				}
				// std::cout << "old_local_depth: " << (int)bucket->local_depth << std::endl;
				// std::cout << "new_local_depth: " << (int)new_local_depth <<  std::endl;
				// print_extendible_hash_bucket(bucket, bucket->original_index, false);
				new_local_depth = bucket->local_depth;
				common_digit_nums = 0;
				do {
					new_local_depth++;
					std::cout << "new_local_depth: " << (int)new_local_depth << std::endl;
					common_digit_nums = 0;
					for (std::uint8_t e = 0; e < bucket_entries+1; e++) {
						std::cout << (int)((hash_values[e] >> (new_local_depth-1)) & 1) << std::endl;
						common_digit_nums += (hash_values[e] >> (new_local_depth-1)) & 1;
					}
				}
				while (common_digit_nums == 0 || common_digit_nums == bucket_entries+1);
				print_extendible_hash_table(false);
				// print_extendible_hash_table(true);
			}
			return new_local_depth;
		}
		void insert_internal_shared(const std::string& key, const std::string& new_value) {

			boost::shared_lock<boost::shared_mutex> global_shared_lock(global_mutex);

			hash_value_t hash_value = hash->get_hash(key);
			std::uint32_t bucket_number = directory[hash_value & createBitMask(global_depth-1)]->original_index;

			boost::unique_lock<boost::shared_mutex> local_exclusive_lock(directory[bucket_number]->local_mutex);

			if (directory[bucket_number]->entry_count < bucket_entries) {
				directory[bucket_number]->insert_next(key, new_value);
				local_exclusive_lock.unlock();
				global_shared_lock.unlock();
				return;
			} 
			local_exclusive_lock.unlock();
			global_shared_lock.unlock();
			return insert_internal_exclusive(key, new_value);
		}
		void insert_internal_exclusive(const std::string& key, const std::string& new_value) {
			boost::unique_lock<boost::shared_mutex> global_exclusive_lock(global_mutex);

			// Search for free slot
			hash_value_t hash_value = hash->get_hash(key);
			std::uint32_t bucket_number = directory[hash_value & createBitMask(global_depth-1)]->original_index;

			std::vector<hash_bucket*> buckets_to_insert;

			if (directory[bucket_number]->entry_count < bucket_entries) {
				directory[bucket_number]->insert_next(key, new_value);
				global_exclusive_lock.unlock();
				return;
			} 

			//      Not made                      Original bucket overflow                    Image bucket overflow
			while (buckets_to_insert.empty() || !(buckets_to_insert.back()->entry_count) || !(directory[bucket_number]->entry_count)) {
				if (buckets_to_insert.empty() || !(buckets_to_insert.back()->entry_count)) {
					create_bucket_and_insert(directory[bucket_number], buckets_to_insert, bucket_number, key, new_value);
				} else {
					create_bucket_and_insert(buckets_to_insert.back(), buckets_to_insert, bucket_number, key, new_value);
				}
			}
			if (buckets_to_insert.back()->local_depth <= global_depth) {
				for (typename std::vector<hash_bucket*>::iterator it = buckets_to_insert.begin(); it != buckets_to_insert.end(); ++it) {
					hash_bucket*  image_bucket   = *it;
					std::uint32_t ptr_index 	 = image_bucket->original_index;
					while (ptr_index < directory_size()) { // Update all other pointers with this prefix.
						directory[ptr_index] = image_bucket;
						ptr_index += (1<<image_bucket->local_depth);
					}
				}
				global_exclusive_lock.unlock();
				return;
			}

			std::uint8_t splits = buckets_to_insert.back()->local_depth - global_depth;
			// std::cout << "BUCKET amount: " << buckets_to_insert.size() << std::endl;
			// std::cout << "SS: " << (int)splits << std::endl;

			std::uint32_t old_size = 1 << global_depth;
			// std::cout << "OS: " << old_size << std::endl;
			std::uint32_t new_size = 1 << (global_depth + splits);
			// std::cout << "NS: " << new_size << std::endl;

			// std::cout << "A1" << std::endl;
			directory = (hash_bucket**) realloc(directory, new_size*sizeof(hash_bucket*));
			// std::cout << "A" << std::endl;
			if (!directory) {
				std::cout << "REALLOC ERROR" << std::endl;
			}		
			// std::cout << "B" << std::endl;
			for (typename std::vector<hash_bucket*>::iterator it = buckets_to_insert.begin(); it != buckets_to_insert.end(); ++it) {
				// std::cout << "c" << std::endl;
				// std::cout << "OS NOW: " << old_size << std::endl;
				std::copy(&directory[0], &directory[old_size], &directory[old_size]);
				// std::cout << "d" << std::endl;
				old_size <<= 1;
				// std::cout << "e" << std::endl;
				hash_bucket* image_bucket   = *it;
				// std::cout << "f" << std::endl;
				directory[image_bucket->original_index] = image_bucket;
				// std::cout << "g" << std::endl;
			}
			// std::cout << "h" << std::endl;
			global_depth += splits;
			global_exclusive_lock.unlock();
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
			boost::unique_lock<boost::shared_mutex> global_exclusive_lock(global_mutex);
			for (std::uint32_t b = 0; b < directory_size(); b++){
				if (directory[b]) {
					// Deallocate the memory
					if (directory[b]->local_depth == global_depth) { // Delete if only one pointer is pointing.
						delete directory[b];
					} else { 
						std::uint32_t mask = (createBitMask(global_depth-directory[b]->local_depth-1)<<(directory[b]->local_depth));
						if ((mask & b) == mask) { // Delete if is last pointer to bucket
							delete directory[b];
						}
					}
					
					// void actual pointer
					directory[b] = NULL;
				}
			}
			free(directory);
			global_exclusive_lock.unlock();
		}

		void print_extendible_hash_table(bool exclusive) {
			std::cout << (int)global_depth << std::endl;
			for (std::uint8_t j = 0; j < bucket_entries; j++)
				std::cout << "_______";
			std::cout << "_______" << "_______" << "_______" << "_______" << std::endl;
			for (std::uint32_t i = 0; i < directory_size(); i++) {
				print_extendible_hash_bucket(directory[i], i, exclusive);
			}
			for (std::uint8_t j = 0; j < bucket_entries; j++)
				std::cout << "_______";
			std::cout << "_______" << "_______" << "_______" << "_______" << std::endl;
			std::cout << "DONE:" << std::endl;
		}
		void print_extendible_hash_bucket(hash_bucket* bucket, std::uint32_t i, bool exclusive) {
			std::string text;
			text += ((bucket->original_index == i) ? "\033[1m " : "\033[0m "); 
			text += (i < 10 ? "   " : (i < 100 ? "  " : (i < 1000 ? " " : ""))) + std::to_string((int)i) + " | ";
			text += "\033[0m";
			text += ((bucket->original_index == i) ? "\033[1;31m" : "\033[0;31m"); 
			text += (bucket->original_index < 10 ? "   " : (bucket->original_index < 100 ? "  " : (bucket->original_index < 1000 ? " " : ""))) + std::to_string((int)bucket->original_index);
			text += "\033[0m";
			text += " | ";
			text += ((bucket->original_index == i) ? "\033[1;32m" : "\033[0;32m"); 
			text += (bucket->local_depth < 10 ? "   " : (bucket->local_depth < 100 ? "  " : (bucket->local_depth < 1000 ? " " : ""))) + std::to_string((int)bucket->local_depth);
			text += "\033[0m";
			text += " | ";
			text += ((bucket->original_index == i) ? "\033[1;33m" : "\033[0;33m"); 
			text += (bucket->entry_count < 10 ? "   " : (bucket->entry_count < 100 ? "  " : (bucket->entry_count < 1000 ? " " : ""))) + std::to_string((int)bucket->entry_count);
			text += ((bucket->original_index == i) ? "\033[1;34m" : "\033[0m"); 
			text += " | ";
			for (std::uint8_t j = 0; j < bucket_entries; j++)
			{
				if (!exclusive || bucket->original_index == i){
					std::uint32_t value = atoi(bucket->keys[j].c_str());
					if (j >= bucket->entry_count)
						text += "     | ";
					else
						text += ((value < 10 ? "   " : (value < 100 ? "  " : (value < 1000 ? " " : ""))) + bucket->keys[j]) + " | ";
				}
				else 
					text += "     | ";
			}
			text += "\033[0m";
			std::cout << text << std::endl;
		}

		bool get(const std::string& key, std::string& value) override {
			hash_value_t hash_value = hash->get_hash(key);
			
			// Take global lock shared;
			boost::shared_lock<boost::shared_mutex> global_shared_lock(global_mutex);
			std::uint32_t bucket_number = hash_value & (directory_size()-1);

			// Take local lock shared;
			boost::shared_lock<boost::shared_mutex> local_shared_lock(directory[bucket_number]->local_mutex);
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

		void insert(const std::string& key, const std::string& new_value) override {
			insert_internal_shared(key, new_value);
		}
		
		// Returns previous value, if found, -1 otherwise
		void update(const std::string& key, const std::string& new_value) override {
			hash_value_t hash_value = hash->get_hash(key);

			boost::shared_lock<boost::shared_mutex> global_shared_lock(global_mutex);
			std::uint32_t bucket_number = hash_value & (directory_size()-1);
			
			boost::unique_lock<boost::shared_mutex> local_exclusive_lock(directory[bucket_number]->local_mutex);
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
		void remove(const std::string& key) override {
			hash_value_t hash_value = hash->get_hash(key);
			boost::shared_lock<boost::shared_mutex> global_shared_lock(global_mutex);
			std::uint32_t bucket_number = hash_value & (directory_size()-1);
			boost::unique_lock<boost::shared_mutex> local_exclusive_lock(directory[bucket_number]->local_mutex);
			for (uint8_t i = 0; i < directory[bucket_number]->entry_count; i++) {
				if (directory[bucket_number]->keys[i] == key) { // Entry to be updated found
					directory[bucket_number]->move_last_to(i);
					break;
				}
			}
			local_exclusive_lock.unlock();
			global_shared_lock.unlock();
		}

		void range_scan(const std::string& start_key, const std::string* end_key, abstract_push_op& apo) override{
			typedef std::tuple<std::string, std::string> hash_entry;

			auto cmp = [](hash_entry a, hash_entry b) { return std::get<0>(a) > std::get<0>(b);};
			std::priority_queue<hash_entry, std::vector<hash_entry>, decltype(cmp)> pri_queue(cmp);

			// FULL SCAN
			boost::shared_lock<boost::shared_mutex> global_shared_lock(global_mutex);
			for (std::uint32_t i = 0; i < directory_size(); i++) {			
				for (std::uint8_t j = 0; j < directory[i]->entry_count; j++) {
					if (directory[i]->keys[j] >= start_key && directory[i]->keys[j] <= *end_key)
						pri_queue.push(std::make_tuple(directory[i]->keys[j], directory[i]->values[j]));
				}
			}
			// Apply push op
			while(!pri_queue.empty()) {
				hash_entry current = pri_queue.top();
				std::string key     = std::get<0>(current);
				std::string value = std::get<1>(current);
				const char* keyp = key.c_str();
				if (!apo.invoke(keyp, key.length(), value)) {
					global_shared_lock.unlock();
					return;
				}
				pri_queue.pop();
			}
			global_shared_lock.unlock();
		}

		void reverse_range_scan(const std::string& start_key, const std::string* end_key, abstract_push_op& apo) override{
			typedef std::tuple<std::string, std::string> hash_entry;

			auto cmp = [](hash_entry a, hash_entry b) { return std::get<0>(a) < std::get<0>(b);};
			std::priority_queue<hash_entry, std::vector<hash_entry>, decltype(cmp)> pri_queue(cmp);

			// FULL SCAN
			boost::shared_lock<boost::shared_mutex> global_shared_lock(global_mutex);
			for (std::uint32_t i = 0; i < directory_size(); i++) {			
				for (std::uint8_t j = 0; j < directory[i]->entry_count; j++) {
					if (directory[i]->keys[j] >= start_key && directory[i]->keys[j] <= *end_key)
						pri_queue.push(std::make_tuple(directory[i]->keys[j], directory[i]->values[j]));
				}
			}
			// Apply push op
			while(!pri_queue.empty()) {
				hash_entry current = pri_queue.top();
				std::string key     = std::get<0>(current);
				std::string value = std::get<1>(current);
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
			boost::shared_lock<boost::shared_mutex> global_shared_lock(global_mutex);
			size_t total_entry_count = 0;
			for (std::uint32_t i = 0; i < directory_size(); i++) {
				if (directory[i]->original_index == i) {
					total_entry_count += directory[i]->entry_count;
				}
			}
			global_shared_lock.unlock();
			return total_entry_count;
		}
	};
}

#endif