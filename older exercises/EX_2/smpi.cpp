
#include "smpi.h"

#include <mutex>
#include <condition_variable>
#include <map>
#include <thread>
#include <cstring>
#include <cassert>


/**
 * Class representing a SMPI thread
 */
class SMPI_Thread {
public:

	// internal send operation
	int send(void* buffer, int size, int sender) {
		// Check arguments
		if (buffer == nullptr || size <= 0) {
			return SMPI_ERR_INVALID_ARG;
		}

		// Lock all other send operations out and let them wait
		// This allows us to have several send operations at once
		std::lock_guard<std::mutex> sendLock(writer_mutex);

		// Lock the mutex used to protect the write buffer and condition variables
		std::unique_lock<std::mutex> lock(mutex);

		assert(writerBuffer == nullptr);

		// Set data needed by the receiver
		writerBuffer = buffer;
		writerBuffferSize = size;
		writerRank = sender;

		// Wake up the receives if there is already one waiting
		if (readerWaiting) {
			cond_reader.notify_one();
		}

		// Wait till receiver has copied the buffer
		while (writerBuffer) {
			cond_writer.wait(lock);
		}

		// Return result
		return writerResult;
	}

	// internal receive operation
	int recv(void* buffer, int size, int* source) {
		// Lock the mutex used to protect the write buffer and condition variables
		std::unique_lock<std::mutex> lock(mutex);

		// When there is no active send, then wait till someone sends us something
		while (!writerBuffer) {
			readerWaiting = true;
			cond_reader.wait(lock);
			readerWaiting = false;
		}

		assert(writerBuffer != nullptr);

		// Copy sender buffer
		int result = SMPI_ERR;
		if (writerBuffferSize != size) {
			result = SMPI_ERR_INVALID_SIZE;
		} else {
			std::memcpy(buffer, writerBuffer, size);
			result = SMPI_OK;
		}

		// Tell receiver the rank of the sender
		if (source) {
			*source = writerRank;
		}

		// Reset sender data
		writerBuffer = nullptr;
		writerBuffferSize = 0;
		writerRank = -1;

		// Notify sender
		writerResult = result;
		lock.unlock();
		cond_writer.notify_one();

		// Return result
		return result;
	}

private:
	std::mutex mutex;
	std::mutex writer_mutex;
	std::condition_variable cond_reader;
	std::condition_variable cond_writer;
	bool readerWaiting = false;
	void* writerBuffer = nullptr;
	int writerBuffferSize = 0;
	int writerRank = -1;
	int writerResult = 0;
};


/**
 * Class representing a SMPI instance (is a singleton)
 */
class SMPI {
friend int smpi_setup(unsigned);

public:

	// Initializes a thread
	// Assigns a rank to the calling thread
	int initThread() {
		std::lock_guard<std::mutex> lock(mutex);
		if (next_rank >= number_of_threads) {
			return SMPI_ERR_NO_FREE_RANK;
		}
		auto thread_id = std::this_thread::get_id();
		if (rankMapping.find(thread_id) == rankMapping.end()) {
			auto rank = next_rank++;
			rankMapping[thread_id] = rank;
		}
		return SMPI_OK;
	}

	// Finalizes a thread
	// Currently does noting
	int finalizeThread() {
		return SMPI_OK;
	}

	// Returns the rank of the current thread
	int getRank() {
		auto thread_id = std::this_thread::get_id();
		if (rankMapping.find(thread_id) == rankMapping.end()) {
			return SMPI_ERR_NO_RANK_ASSIGNED;
		} else {
			return rankMapping[thread_id];
		}
	}

	// send operation
	int send(void* buffer, int size, int destination) {
		if (destination < 0 || destination >= (int)number_of_threads) {
			return SMPI_ERR_INVALID_RANK;
		}
		int sender = getRank();
		return threads[destination].send(buffer, size, sender);
	}

	// receive operation
	int recv(void* buffer, int size, int* source) {
		int receiver = getRank();
		if (receiver < 0) {
			return SMPI_ERR_NO_RANK_ASSIGNED;
		}
		return threads[receiver].recv(buffer, size, source);
	}

	// Returns a pointer to the SMPI instance
	static SMPI* getInstance() {
		return instance;
	}

private:
	SMPI(unsigned number_of_threads)
			: number_of_threads(number_of_threads),
			  threads(new SMPI_Thread[number_of_threads]) {}

	std::mutex mutex;
	unsigned number_of_threads;
	SMPI_Thread* threads;
	std::map<std::thread::id, int> rankMapping;
	unsigned next_rank = 0;

	static SMPI* instance;

	static void setupInstance(unsigned number_of_threads) {
		instance = new SMPI(number_of_threads);
	}

};


SMPI* SMPI::instance = nullptr;



int smpi_setup(unsigned number_of_threads) {
	//Check input arguments
	if (number_of_threads == 0) {
		return SMPI_ERR_INVALID_ARG;
	}
	if (!SMPI::getInstance()) {
		SMPI::setupInstance(number_of_threads);
	}
	return SMPI_OK;
}


int smpi_init() {
	auto smpi = SMPI::getInstance();
	if (smpi) {
		return smpi->initThread();
	} else {
		return SMPI_ERR_NO_SETUP;
	}
}


int smpi_get_rank() {
	auto smpi = SMPI::getInstance();
	if (smpi) {
		return smpi->getRank();
	} else {
		return SMPI_ERR_NO_SETUP;
	}
}


int smpi_send(void* buffer, int size, int destination) {
	auto smpi = SMPI::getInstance();
	if (smpi) {
		return smpi->send(buffer, size, destination);
	} else {
		return SMPI_ERR_NO_SETUP;
	}
}


int smpi_recv(void* buffer, int size, int* source) {
	auto smpi = SMPI::getInstance();
	if (smpi) {
		return smpi->recv(buffer, size, source);
	} else {
		return SMPI_ERR_NO_SETUP;
	}
}


int smpi_finalize() {
	auto smpi = SMPI::getInstance();
	if (smpi) {
		return smpi->finalizeThread();
	} else {
		return SMPI_ERR_NO_SETUP;
	}
}


