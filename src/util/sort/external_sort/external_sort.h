/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		External sorting algorithms for values stored in files.
@details	Assumes unspanned pages (A whole resides in page boundaries. It is not
			spanned across multiple pages).
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#if !defined(ION_FILE_SORT_H_)
#define ION_FILE_SORT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "../sort.h"
#include "../../../key_value/kv_system.h"
#include "external_sort_types.h"
#include "flash_min_sort.h"

ion_err_t
ion_external_sort_init(
	ion_external_sort_t				*es,
	FILE							*file,
	ion_sort_comparator_context_t	context,
	ion_sort_comparator_t			compare_function,
	ion_key_size_t					key_size,
	ion_value_size_t				value_size,
	ion_page_size_t					page_size,
	ion_file_sort_algorithm_e		sort_algorithm
);

ion_err_t
ion_external_sort_init_cursor(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor,
	void						*buffer,
	ion_buffer_size_t			buffer_size
);

ion_err_t
ion_external_sort_dump_all(
	ion_external_sort_t *es,
	FILE				*output_file,
	void				*buffer,
	ion_buffer_size_t	buffer_size
);

#if defined(__cplusplus)
}
#endif

#endif /* ION_FILE_SORT_H_ */
