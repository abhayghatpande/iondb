/******************************************************************************/
/**
@file		test_iinq_rewrite.c
@author		Kai Neubauer
@brief		iinq tests.
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

#include "test_iinq_rewrite.h"
#include "../../../util/sort/sort.h"
#include "../../../iinq/iinq.h"

#define IINQ_PAGE_SIZE    512

DEFINE_SCHEMA(test1, {
	double col1;
	char col2[40];
	int col3;
	char col4[10];
});

typedef void (*iinq_test_results_func_t)(
		planck_unit_test_t *,
		ion_iinq_result_t *,
		uint32_t,
		uint32_t
);

typedef struct {
	uint32_t total;
	uint32_t count;
	planck_unit_test_t *tc;
	iinq_test_results_func_t func;
} iinq_test_query_state_t;


IINQ_NEW_PROCESSOR_FUNC(check_results) {
	iinq_test_query_state_t *query_state = state;

	query_state->func(query_state->tc, result, query_state->count, query_state->total);
	query_state->count++;
}

void
iinq_rewrite_create_test1(
		planck_unit_test_t *tc
) {
	ion_err_t error;

	ion_key_type_t key_type;
	ion_key_size_t key_size;
	ion_value_size_t value_size;

	key_type = key_type_numeric_signed;
	key_size = sizeof(uint32_t);
	value_size = SCHEMA_SIZE(test1);

	error = CREATE_DICTIONARY(test1, key_type, key_size, value_size);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
}

void
iinq_rewrite_test_select_all_from_check_results(
		planck_unit_test_t *tc,
		ion_iinq_result_t *result,
		uint32_t count,
		uint32_t total
) {
	UNUSED(count);
	UNUSED(total);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, SCHEMA_SIZE(test1) + sizeof(uint32_t),
									 result->num_bytes); // Size of tuple + key

	ion_key_t key = result->processed;
	struct iinq_test1_schema *tuple = (struct iinq_test1_schema *) (result->processed + sizeof(int));

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(key, uint32_t));
	PLANCK_UNIT_ASSERT_TRUE(tc, 2.5 == tuple->col1);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Hello", tuple->col2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, tuple->col3);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Goodbye", tuple->col4); // col4
}

void
iinq_rewrite_insert_value_test1(
		planck_unit_test_t *tc,
		ion_key_t key,
		ion_value_t value
) {
	ion_status_t status;
	status = INSERT(test1, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
}

void
iinq_rewrite_test_insert_multiple_same_values_test1(
		planck_unit_test_t *tc
) {
	ion_key_t key = IONIZE(1, uint32_t);
	DECLARE_SCHEMA_VAR(test1, test_val);

	test_val.col1 = 2.5;
	strcpy(test_val.col2, "Hello");
	test_val.col3 = 1;
	strcpy(test_val.col4, "Goodbye");

	iinq_rewrite_insert_value_test1(tc, key, &test_val);
	iinq_rewrite_insert_value_test1(tc, key, &test_val);
	iinq_rewrite_insert_value_test1(tc, key, &test_val);
}

void
iinq_rewrite_test_insert_multiple_different_values_test1(
		planck_unit_test_t *tc
) {
	ion_key_t key = 1;
	DECLARE_SCHEMA_VAR(test1, test_val);

	test_val.col1 = 2.5;
	strcpy(test_val.col2, "Hello");
	test_val.col3 = 1;
	strcpy(test_val.col4, "Goodbye");

	iinq_rewrite_insert_value_test1(tc, key, &test_val);
	iinq_rewrite_insert_value_test1(tc, key, &test_val);

	test_val.col1 = 1.0;
	strcpy(test_val.col2, "Goodbye");
	test_val.col3 = 3;
	strcpy(test_val.col4, "Hello");

	iinq_rewrite_insert_value_test1(tc, key, &test_val);
}


void
iinq_rewrite_test_select_all_from_test1(
		planck_unit_test_t *tc
) {
	ion_iinq_query_processor_t processor;
	iinq_test_query_state_t state;

	iinq_rewrite_create_test1(tc);
	iinq_rewrite_test_insert_multiple_same_values_test1(tc);

	state.func = iinq_rewrite_test_select_all_from_check_results;
	state.tc = tc;
	state.count = 0;

	processor = IINQ_QUERY_PROCESSOR(check_results, &state);

	do {
		ion_err_t error;
		ion_iinq_result_t result;
		int jmp_r;
		jmp_buf selectbuf;
		result.raw_record_size = 0;
		result.num_bytes = 0;
		int read_page_remaining = IINQ_PAGE_SIZE;
		int write_page_remaining = IINQ_PAGE_SIZE;
		FILE *input_file;
		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */
		ion_iinq_cleanup_t *first;
		ion_iinq_cleanup_t *last;
		ion_iinq_cleanup_t *ref_cursor;
		ion_iinq_cleanup_t *last_cursor;
		first = NULL;
		last = NULL;
		ref_cursor = NULL;
		last_cursor = NULL;
		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */
		ion_iinq_source_t test1;
		test1.cleanup.next = NULL;
		test1.cleanup.last = last;
		test1.cleanup.reference = &test1;
		if (NULL == first) { first = &test1.cleanup; }
		if (NULL != last) { last->next = &test1.cleanup; }
		last = &test1.cleanup;
		test1.cleanup.next = NULL;
		test1.dictionary.handler = &test1.handler;
		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));
		if (err_ok != error) { break; }
		result.raw_record_size += test1.dictionary.instance->record.key_size;
		result.raw_record_size += test1.dictionary.instance->record.value_size;
		result.num_bytes += test1.dictionary.instance->record.key_size;
		result.num_bytes += test1.dictionary.instance->record.value_size;
		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);
		if (err_ok != error) { break; }
		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.data = alloca(result.raw_record_size);
		result.processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.key = result.processed;
		result.processed += test1.dictionary.instance->record.key_size;
		test1.value = result.processed;
		result.processed += test1.dictionary.instance->record.value_size;
		test1.ion_record.key = test1.key;
		test1.ion_record.value = test1.value;
		struct iinq_test1_schema *test1_tuple;
		test1_tuple = test1.value;
		ref_cursor = first;
		while (ref_cursor != last) {
			if (NULL == ref_cursor || (cs_cursor_active !=
									   (ref_cursor->reference->cursor_status = ref_cursor->reference->cursor->next(
											   ref_cursor->reference->cursor,
											   &ref_cursor->reference->ion_record)) && cs_cursor_initialized !=
																					   ref_cursor->reference->cursor_status)) { break; }
			ref_cursor = ref_cursor->next;
		}
		ref_cursor = last;
		goto SKIP_COMPUTE_SELECT;
		COMPUTE_SELECT:;
		memcpy(result.processed, result.data, result.num_bytes);
		goto DONE_COMPUTE_SELECT;
		SKIP_COMPUTE_SELECT:;
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) { break; }
				last_cursor = ref_cursor;
				while (NULL != ref_cursor && (cs_cursor_active !=
											  (ref_cursor->reference->cursor_status = ref_cursor->reference->cursor->next(
													  ref_cursor->reference->cursor,
													  &ref_cursor->reference->ion_record)) &&
											  cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);
					if ((cs_cursor_active !=
						 (ref_cursor->reference->cursor_status = ref_cursor->reference->cursor->next(
								 ref_cursor->reference->cursor, &ref_cursor->reference->ion_record)) &&
						 cs_cursor_initialized != ref_cursor->reference->cursor_status)) { goto IINQ_QUERY_CLEANUP; }
					ref_cursor = ref_cursor->last;
				}
				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {
					ref_cursor = last;
				}
				/* end of _FROM_ADVANCE_CURSORS */
				result.processed = alloca(result.num_bytes);
				goto COMPUTE_SELECT;
				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}
			IINQ_QUERY_CLEANUP:
			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);
				first = first->next;
			}
			if (err_ok != error) { goto IINQ_QUERY_END; }
		} while (0);
		IINQ_QUERY_END:;
	} while (0);


	DROP(test1);
}

void
iinq_rewrite_test_select_all_from_test1_where_equality_filter(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_test1(tc);

	DECLARE_SCHEMA_VAR(test1, test_val);
	ion_key_t key = IONIZE(1, uint32_t);

	test_val.col1 = 31.02;
	strcpy(test_val.col2, "Goodbye");
	test_val.col3 = 3;
	strcpy(test_val.col4, "Hello");

	iinq_rewrite_insert_value_test1(tc, key, &test_val);

	key = IONIZE(2, uint32_t);
	strcpy(test_val.col2, "United Kingdom");

	iinq_rewrite_insert_value_test1(tc, key, &test_val);

	DROP(test1);
}

void
iinq_rewrite_test_select_all_from_test1_where_equality_filter_check_results(
		planck_unit_test_t *tc,
		ion_iinq_result_t *result,
		uint32_t count,
		uint32_t total
) {
	UNUSED(count);
	UNUSED(total);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, SCHEMA_SIZE(test1) + sizeof(uint32_t), result->num_bytes);
	ion_key_t key = result->processed;

	struct iinq_test1_schema *tuple = (struct iinq_test1_schema *) (result->processed + sizeof(uint32_t));

	switch (NEUTRALIZE(key, uint32_t)) {
		case 1:
			PLANCK_UNIT_ASSERT_TRUE(tc, tuple->col1 == 31.02);
			break;
		case 2:
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "United Kingdom", tuple->col2);
			break;
		default:
			PLANCK_UNIT_SET_FAIL(tc);
	}
}

void
iinq_rewrite_test_select_all_from_test1_where_comparison(
		planck_unit_test_t *tc
) {
	UNUSED(tc);
}

void
iinq_rewrite_test_select_all_from_test1_where_orderby(
		planck_unit_test_t *tc
) {
	UNUSED(tc);
}

planck_unit_suite_t *
iinq_rewrite_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1_where_equality_filter);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1_where_comparison);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1_where_orderby);

	return suite;
}

void
run_all_tests_iinq_rewrite(
) {
	planck_unit_suite_t *suite = iinq_rewrite_get_suite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);

	fremove(ION_MASTER_TABLE_FILENAME);
}