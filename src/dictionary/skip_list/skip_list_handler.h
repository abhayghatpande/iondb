/******************************************************************************/
/**
@file		skip_list_handler.h
@author		Eric Huang
@brief		Handler liaison between dictionary API and skiplist implementation
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/
#if !defined(SKIP_LIST_HANDLER_H_)
#define SKIP_LIST_HANDLER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "skip_list_types.h"
#include "skip_list.h"

/**
@brief	  Registers a skiplist handler to a dictionary instance.

@details	Binds each unique skiplist function to the generic dictionary
			interface. Only needs to be called once when the skiplist is
			initialized.

@param	  handler
				An instance of a dictionary handler that is to be bound.
				It is assumed @p handler is initialized by the user.
*/
void
sldict_init(
	ion_dictionary_handler_t *handler
);

/**
@brief	  Inserts a @p key and @p value pair into the dictionary.

@param	  dictionary
				The dictionary instance to insert the value into.
@param	  key
				The key to use.
@param	  value
				The value to use.
@return	 Status of insertion.
*/
ion_status_t
sldict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Creates an instance of a dictionary.

@details	Creates an instance of a dictionary given a @p key_size and
			@p value_size, in bytes as well as the @p dictionary_size, which
			is the maximum number of levels in the skiplist. By nature of the
			structure, the maximum number of elements is bounded only by memory
			use.

@param		id
@param		key_type
@param	  key_size
				Size of the key in bytes.
@param	  value_size
				Size of the value in bytes.
@param		dictionary_size
@param		compare
@param	  handler
				Handler to be bound to the dictionary instance being created.
				Assumption is that the handler has been initialized prior.
@param	  dictionary
				Pointer in which the created dictionary instance is to be
				stored. Assumption is that it has been properly allocated by
				the user.
@return	 Status of creation.
*/
ion_err_t
sldict_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
);

/**
@brief	  Deletes the @p key and associated value from the given dictionary
			instance.

@param	  dictionary
				The instance of the dictionary to delete from.
@param	  key
				The key to be deleted.
@return	 Status of deletion.
*/
ion_status_t
sldict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
);

/**
@brief	  Deletes an instance of a dictionary and its associated data.

@param	  dictionary
				The instance of the dictionary to be deleted.
@return	 Status of dictionary deletion.
*/
ion_err_t
sldict_delete_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief	  Deletes an instance of a closed dictionary.
@param	  id
				The identifier identifying the dictionary to destroy.
@return	 Status of dictionary deletion.
*/
ion_err_t
sldict_destroy_dictionary(
	ion_dictionary_id_t id
);

/**
@brief	  Updates the value stored at a given key.

@details	Updates the value for a given @p key. If the key doesn't exist,
			the key value pair will be added as if it was an insert.

@param	  dictionary
				The instance of the dictionary to be updated.
@param	  key
				The key that is to be updated.
@param	  value
				The new value to be used.
@return Status of update.
*/
ion_status_t
sldict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

#if defined(__cplusplus)
}
#endif

#endif /* SKIP_LIST_HANDLER_H_ */
