
#ifndef SMPI_H_
#define SMPI_H_


/**
 * Return Codes
 */
#define SMPI_OK  				 	 0  // No Error
#define SMPI_ERR					-1  // Generic error
#define SMPI_ERR_NO_SETUP			-2  // smpi_setup() was not called before
#define SMPI_ERR_INVALID_ARG		-3  // Invalid function argument
#define SMPI_ERR_INVALID_SIZE		-4  // Sender and receiver buffer sizes do not match
#define SMPI_ERR_INVALID_RANK		-5  // Given rank does not exist
#define SMPI_ERR_NO_FREE_RANK		-6  // Cannot assign a rank to a thread (e.g. created more threads than passed to smpi_setup())
#define SMPI_ERR_NO_RANK_ASSIGNED	-7  // Thread has no rank assigned


/**
 * Initializes the SMPI library.
 *
 * Should be the very first function that gets called.
 *
 * \param number_of_threads number of threads
 * \returns one of the return codes listed above
 */
int smpi_setup(unsigned number_of_threads);


/**
 * Initializes a thread.
 *
 * Should be the very first function that gets called inside a thread.
 *
 * \returns one of the return codes listed above
 */
int smpi_init();


/**
 * Returns the rank of the current thread
 *
 * \returns rank of the current thread when successful, otherwise one of the return codes listed above
 */
int smpi_get_rank();


/**
 * Sends a message to another thread
 *
 * The buffer size of the corresponding receive call needs to match the given buffer size!
 *
 * \param buffer buffer holding the message to be send
 * \param size buffer size in byte
 * \param destination rank of the receiving thread
 * \returns one of the return codes listed above
 */
int smpi_send(void* buffer, int size, int destination);


/**
 * Receives a message from another thread
 *
 * The buffer size of the corresponding send call needs to match the given buffer size!
 *
 * \param buffer buffer where the message should be copied into
 * \param size buffer size in byte
 * \param source buffer where the rank of the sending process will be written into (can be set to NULL)
 * \returns one of the return codes listed above
 */
int smpi_recv(void* buffer, int size, int* source);


/**
 * Finalizes a thread.
 *
 * Should be the very last function called in a thread.
 *
 * \returns one of the return codes listed above
 */
int smpi_finalize();


#endif /* SMPI_H_ */

