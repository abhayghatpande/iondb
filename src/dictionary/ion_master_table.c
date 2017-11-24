/******************************************************************************/
/**
@file		ion_master_table.c
@author		Eric Huang, Graeme Douglas, Scott Fazackerley, Wade Penson,
			Dana Klamut
@brief		Master table handling.
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

#include "ion_master_table.h"

FILE				*ion_master_table_file		= NULL;
ion_dictionary_id_t ion_master_table_next_id	= 1;

ion_err_t
ion_master_table_write(
	ion_dictionary_config_info_t	*config,
	long							where
) {
	long old_pos = ftell(ion_master_table_file);

	if (ION_MASTER_TABLE_CALCULATE_POS == where) {
		where = (int) (config->id * ION_MASTER_TABLE_RECORD_SIZE(config));
	}

	if (ION_MASTER_TABLE_CALCULATE_POS > where) {
		if (0 != fseek(ion_master_table_file, 0, SEEK_END)) {
			return err_file_bad_seek;
		}
	}
	else if (0 != fseek(ion_master_table_file, where, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fwrite(&(config->id), sizeof(config->id), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->use_type), sizeof(config->use_type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->type), sizeof(config->type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->key_size), sizeof(config->key_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->value_size), sizeof(config->value_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->dictionary_size), sizeof(config->dictionary_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->dictionary_type), sizeof(config->dictionary_type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->dictionary_status), sizeof(config->dictionary_status), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (0 != fseek(ion_master_table_file, old_pos, SEEK_SET)) {
		return err_file_bad_seek;
	}

	return err_ok;
}

/**
@brief		Read a record to the master table.
@details	Automatically, this call will reposition the file position
			back to where it was once the call is complete.
@param[out]	config
				A pointer to a previously allocated config object to write to.
@param[in]	where
				An integral value representing where to read from in the file.
				This file offset is byte-aligned, not record aligned, in general.

				One special flag can be passed in here:
					- @c ION_MASTER_TABLE_CALCULATE_POS
						Calculate the position based on the passed-in config id.
@returns	An error code describing the result of the call.
*/
ion_err_t
ion_master_table_read(
	ion_dictionary_config_info_t	*config,
	long							where
) {
	long old_pos = ftell(ion_master_table_file);

	if (ION_MASTER_TABLE_CALCULATE_POS == where) {
		where = (int) (config->id * ION_MASTER_TABLE_RECORD_SIZE(config));
	}

	if (0 != fseek(ion_master_table_file, where, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fread(&(config->id), sizeof(config->id), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (1 != fread(&(config->use_type), sizeof(config->use_type), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (1 != fread(&(config->type), sizeof(config->type), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (1 != fread(&(config->key_size), sizeof(config->key_size), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (1 != fread(&(config->value_size), sizeof(config->value_size), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (1 != fread(&(config->dictionary_size), sizeof(config->dictionary_size), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (1 != fread(&(config->dictionary_type), sizeof(config->dictionary_type), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (1 != fread(&(config->dictionary_status), sizeof(config->dictionary_status), 1, ion_master_table_file)) {
		return err_file_read_error;
	}

	if (0 != fseek(ion_master_table_file, old_pos, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (0 == config->id) {
		return err_item_not_found;
	}

	return err_ok;
}

/* Returns the next dictionary ID, then increments. */
ion_err_t
ion_master_table_get_next_id(
	ion_dictionary_id_t *id
) {
	ion_err_t error								= err_ok;

	/* Flush master row. This writes the next ID to be used, so add 1. */
	ion_dictionary_config_info_t master_config	= { .id = ion_master_table_next_id + 1 };

	error = ion_master_table_write(&master_config, 0);

	if (err_ok != error) {
		return error;
	}

	*id = ion_master_table_next_id++;

	return err_ok;
}

ion_err_t
ion_init_master_table(
	void
) {
	ion_err_t error = err_ok;

	/* If it's already open, then we don't do anything. */
	if (NULL != ion_master_table_file) {
		return err_ok;
	}

	ion_master_table_file = fopen(ION_MASTER_TABLE_FILENAME, "r+b");

	/* File may not exist. */
	if (NULL == ion_master_table_file) {
		ion_master_table_file = fopen(ION_MASTER_TABLE_FILENAME, "w+b");

		if (NULL == ion_master_table_file) {
			return err_file_open_error;
		}

		/* Clean fresh file was opened. */
		ion_master_table_next_id = 1;

		/* Write master row. */
		ion_dictionary_config_info_t master_config = { .id = ion_master_table_next_id };

		if (err_ok != (error = ion_master_table_write(&master_config, 0))) {
			return error;
		}
	}
	else {
		/* Here we read an existing file. */

		/* Find existing ID count. */
		ion_dictionary_config_info_t master_config;

		if (ion_master_table_read(&master_config, 0)) {
			return err_file_read_error;
		}

		ion_master_table_next_id = master_config.id;
	}

	return err_ok;
}

ion_err_t
ion_close_master_table(
	void
) {
	if (NULL != ion_master_table_file) {
		if (0 != fclose(ion_master_table_file)) {
			return err_file_close_error;
		}
	}

	ion_master_table_file = NULL;

	return err_ok;
}

ion_err_t
ion_close_all_master_table(
	void
) {
	ion_err_t						err;
	ion_dictionary_handler_t		handler;
	ion_dictionary_t				dict;
	ion_dictionary_config_info_t	config;

	ion_dictionary_id_t id = ion_master_table_next_id;

	if (NULL != ion_master_table_file) {
		id--;

		while (id > 0) {
			err = ion_lookup_in_master_table(id, &config);

			/* Dictionary corresponding to ID has been found and dictionary is open. */
			if ((err_ok == err) && (ion_dictionary_status_closed != config.dictionary_status)) {
				err = ion_switch_handler(config.dictionary_type, &handler);

				if (err_ok != err) {
					return err;
				}

				dict.handler	= &handler;

				/* Initialize dictionary instance. */
				err				= dictionary_open(&handler, &dict, &config);

				if (err_ok != err) {
					return err;
				}

				/* Close the dictionary instance. */
				err = dictionary_close(&dict);

				if (err_ok != err) {
					return err;
				}
			}

			id--;
		}

		if (0 != fclose(ion_master_table_file)) {
			return err_file_close_error;
		}
	}

	ion_master_table_file = NULL;

	return err_ok;
}

ion_err_t
ion_delete_master_table(
	void
) {
	if (0 != fremove(ION_MASTER_TABLE_FILENAME)) {
		return err_file_delete_error;
	}

	/* Reset master table ID as master table has been deleted. */
	ion_master_table_next_id = 1;

	return err_ok;
}

/**
@brief		Adds the given dictionary to the master table.
@param		dictionary
				A pointer to the dictionary object to add to the master table.
@param		dictionary_size
				The implementation specific size parameter used when
				creating the dictionary. This parameter must be passed
				to this function by @ref ion_master_table_create_dictionary,
				since not all implementations track the dictionary size.
*/
ion_err_t
ion_add_to_master_table(
	ion_dictionary_t		*dictionary,
	ion_dictionary_size_t	dictionary_size
) {
	ion_dictionary_config_info_t config = {
		.id = dictionary->instance->id, .use_type = 0, .type = dictionary->instance->key_type, .key_size = dictionary->instance->record.key_size, .value_size = dictionary->instance->record.value_size, .dictionary_size = dictionary_size, .dictionary_type = dictionary->instance->type, .dictionary_status = dictionary->status
	};

	return ion_master_table_write(&config, ION_MASTER_TABLE_WRITE_FROM_END);
}

ion_err_t
ion_master_table_create_dictionary(
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size
) {
	ion_err_t			err;
	ion_dictionary_id_t id;

	err = ion_master_table_get_next_id(&id);

	if (err_ok != err) {
		return err;
	}

	err = dictionary_create(handler, dictionary, id, key_type, key_size, value_size, dictionary_size);

	if (err_ok != err) {
		return err;
	}

	err = ion_add_to_master_table(dictionary, dictionary_size);

	return err;
}

ion_err_t
ion_lookup_in_master_table(
	ion_dictionary_id_t				id,
	ion_dictionary_config_info_t	*config
) {
	ion_err_t error = err_ok;

	config->id	= id;
	error		= ion_master_table_read(config, ION_MASTER_TABLE_CALCULATE_POS);

	if (err_ok != error) {
		return error;
	}

	return err_ok;
}

ion_err_t
ion_find_by_use_master_table(
	ion_dictionary_config_info_t	*config,
	ion_dict_use_t					use_type,
	char							whence
) {
	ion_dictionary_id_t				id;
	ion_dictionary_config_info_t	tconfig;
	ion_err_t						error;

	tconfig.id	= 0;

	id			= 1;

	if (ION_MASTER_TABLE_FIND_LAST == whence) {
		id = ion_master_table_next_id - 1;
	}

	/* Loop through all items. */
	for (; id < ion_master_table_next_id && id > 0; id += whence) {
		error = ion_lookup_in_master_table(id, &tconfig);

		if (err_item_not_found == error) {
			continue;
		}

		if (err_ok != error) {
			return error;
		}

		/* If this config has the right type, set the output pointer. */
		if (tconfig.use_type == use_type) {
			*config = tconfig;

			return err_ok;
		}
	}

	return err_item_not_found;
}

ion_err_t
ion_delete_from_master_table(
	ion_dictionary_id_t id
) {
	ion_dictionary_config_info_t	blank	= { 0 };
	long							where	= (id * ION_MASTER_TABLE_RECORD_SIZE(&blank));

	return ion_master_table_write(&blank, where);
}

ion_dictionary_type_t
ion_get_dictionary_type(
	ion_dictionary_id_t id
) {
	ion_err_t						err;
	ion_dictionary_config_info_t	config;

	err = ion_lookup_in_master_table(id, &config);

	if (err_ok != err) {
		return dictionary_type_error_t;
	}

	return config.dictionary_type;
}

ion_err_t
ion_open_dictionary(
	ion_dictionary_handler_t	*handler,		/* Passed in empty, to be set. */
	ion_dictionary_t			*dictionary,	/* Passed in empty, to be set. */
	ion_dictionary_id_t			id
) {
	ion_err_t						err;
	ion_dictionary_config_info_t	config;

	err = ion_lookup_in_master_table(id, &config);

	/* Lookup for id failed. */
	if (err_ok != err) {
		return err_uninitialized;
	}

	ion_switch_handler(config.dictionary_type, handler);

	err = dictionary_open(handler, dictionary, &config);

	return err;
}

ion_err_t
ion_close_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t err;

	err = dictionary_close(dictionary);
	return err;
}

ion_err_t
ion_delete_dictionary(
	ion_dictionary_t	*dictionary,
	ion_dictionary_id_t id
) {
	ion_err_t				err;
	ion_dictionary_type_t	type;

	if (ion_dictionary_status_closed != dictionary->status) {
		id	= dictionary->instance->id;
		err = dictionary_delete_dictionary(dictionary);

		if (err_ok != err) {
			return err_dictionary_destruction_error;
		}

		err = ion_delete_from_master_table(id);
	}
	else {
		type = ion_get_dictionary_type(id);

		if (dictionary_type_error_t == type) {
			return err_dictionary_destruction_error;
		}

		ion_dictionary_handler_t handler;

		ion_switch_handler(type, &handler);

		err = dictionary_destroy_dictionary(&handler, id);

		if (err_ok != err) {
			return err;
		}

		err = ion_delete_from_master_table(id);
	}

	return err;
}

ion_err_t
ion_switch_handler(
	ion_dictionary_type_t		type,
	ion_dictionary_handler_t	*handler
) {
	switch (type) {
		case dictionary_type_bpp_tree_t: {
			bpptree_init(handler);
			break;
		}

		case dictionary_type_flat_file_t: {
			ffdict_init(handler);
			break;
		}

		case dictionary_type_open_address_file_hash_t: {
			oafdict_init(handler);
			break;
		}

		case dictionary_type_open_address_hash_t: {
			oadict_init(handler);
			break;
		}

		case dictionary_type_skip_list_t: {
			sldict_init(handler);
			break;
		}

		case dictionary_type_linear_hash_t: {
			linear_hash_dict_init(handler);
			break;
		}

		case dictionary_type_error_t: {
			return err_uninitialized;
		}
	}

	return err_ok;
}
