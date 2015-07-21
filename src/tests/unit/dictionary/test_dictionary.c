/*
 * dictionary.c
 *
 *  Created on: Jul 15, 2014
 *      Author: workstation
 */

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "./../../CuTest.h"
#include "./../../../dictionary/dicttypes.h"
#include "./../../../dictionary/dictionary.h"
#include "./../../../dictionary/ion_master_table.h"
#include "./../../../dictionary/skiplist/slhandler.h"

void
test_dictionary_compare_numerics(
	CuTest		*tc
)
{

	ion_key_t 		key_one;
	ion_key_t 		key_two;

	key_one 		= (ion_key_t)&(int){1};
	key_two 		= (ion_key_t)&(int){1};

	CuAssertTrue(tc, IS_EQUAL ==
			dictionary_compare_signed_value(key_one, key_two, sizeof(int)));

	key_one 		= (ion_key_t)&(int){1};
	key_two 		= (ion_key_t)&(int){2};

	CuAssertTrue(tc, ZERO >
				dictionary_compare_signed_value(key_one, key_two,sizeof(int)));

	key_one 		= (ion_key_t)&(int){2};
	key_two 		= (ion_key_t)&(int){0};

	CuAssertTrue(tc, ZERO <
					dictionary_compare_signed_value(key_one, key_two,sizeof(int)));
	int i;
	for (i = 1; i< 10; i++)
	{
		CuAssertTrue(tc, ZERO <
						dictionary_compare_signed_value((ion_key_t)&i, key_two ,sizeof(int)));
	}


	//case for unsigned signed char
	{
		unsigned char * key_one;
		unsigned char * key_two;

		key_one 		= &(unsigned char){UCHAR_MAX};
		key_two 		= &(unsigned char){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_unsigned_value(key_one, key_two, sizeof(unsigned char)));

	}

	{
		unsigned short * key_one;
		unsigned short * key_two;

		key_one 		= &(unsigned short){USHRT_MAX};
		key_two 		= &(unsigned short){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_unsigned_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(unsigned short)));

	}

	{
			unsigned int * key_one;
			unsigned int * key_two;

			key_one 		= &(unsigned int){UINT_MAX};
			key_two 		= &(unsigned int){0};

			CuAssertTrue(tc, ZERO <
								dictionary_compare_unsigned_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(unsigned int)));

	}

	{
			unsigned long * key_one;
			unsigned long * key_two;

			key_one 		= &(unsigned long){ULONG_MAX};
			key_two 		= &(unsigned long){0};

			CuAssertTrue(tc, ZERO <
								dictionary_compare_unsigned_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(unsigned long)));

	}

	{
		long * key_one;
		long * key_two;

		key_one 		= &(long){LONG_MAX};
		key_two 		= &(long){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(long)));

	}
	{
		char * key_one;
		char * key_two;

		key_one 		= &(char){CHAR_MAX};
		key_two 		= &(char){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(char)));

		}

		{
			char * key_one;
			char * key_two;

			key_one 		= &(char){0};
			key_two 		= &(char){-1};

			CuAssertTrue(tc, ZERO <
								dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(char)));

			}


		{
			short * key_one;
			short * key_two;

			short i,j;

			key_one 		= &i;
			key_two 		= &j;

			for (i = SHRT_MIN/10; i < SHRT_MAX/10; i++)
			{
				for (j = SHRT_MIN/10; j < SHRT_MAX/10; j++)
				{
					if (i < j)
					{
						CuAssertTrue(tc, ZERO >
								dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(short)));
					}else if (i == j)
					{
						CuAssertTrue(tc, ZERO ==
								dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(short)));
					}else
					{
						CuAssertTrue(tc, ZERO <
											dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(short)));
					}
				}
			}
			}
}

void
test_dictionary_master_table(
	CuTest		*tc
)
{
	err_t err;
	/* Cleanup, just in case */
	remove(ION_MASTER_TABLE_FILENAME);

	/* Test init */
	err = ion_init_master_table();

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, NULL != ion_master_table_file);
	CuAssertTrue(tc, 1 == ion_master_table_next_id);
	/*************/

	/* Test create */
	dictionary_handler_t 	handler;
	dictionary_t 			dictionary;
	sldict_init(&handler);
	err = ion_master_table_create_dictionary(&handler, &dictionary, key_type_numeric_signed, 4, 10, 20);

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, 2 == ion_master_table_next_id);
	/***************/

	/* Test close */
	err = ion_close_master_table();

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, NULL == ion_master_table_file);
	/**************/

	/* Test re-open */
	err = ion_init_master_table();

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, NULL != ion_master_table_file);
	CuAssertTrue(tc, 2 == ion_master_table_next_id);
	/****************/

	/* Test lookup 1st dictionary */
	ion_dictionary_config_info_t config;
	err = ion_lookup_in_master_table(1, &config);

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, 1 == config.id);
	CuAssertTrue(tc, key_type_numeric_signed == config.type);
	CuAssertTrue(tc, 4 == config.key_size);
	CuAssertTrue(tc, 10 == config.value_size);
	CuAssertTrue(tc, 20 == config.dictionary_size);
	/******************************/

	/* Test create 2nd dictionary */
	dictionary_handler_t 	handler2;
	dictionary_t 			dictionary2;
	sldict_init(&handler2);
	err = ion_master_table_create_dictionary(&handler2, &dictionary2, key_type_numeric_signed, 2, 7, 14);

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, 3 == ion_master_table_next_id);
	/******************************/

	/* Test 2nd lookup */
	err = ion_lookup_in_master_table(2, &config);

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, 2 == config.id);
	CuAssertTrue(tc, key_type_numeric_signed == config.type);
	CuAssertTrue(tc, 2 == config.key_size);
	CuAssertTrue(tc, 7 == config.value_size);
	CuAssertTrue(tc, 14 == config.dictionary_size);
	/*******************/

	/* Test delete */
	err = ion_delete_from_master_table(&dictionary);

	CuAssertTrue(tc, err_ok == err);
	/***************/
	
	/* Test lookup on non-existent row */
	err = ion_lookup_in_master_table(1, &config);

	CuAssertTrue(tc, err_item_not_found == err);
	/***********************************/
	
	/* Test close */
	err = ion_close_master_table();

	CuAssertTrue(tc, err_ok == err);
	CuAssertTrue(tc, NULL == ion_master_table_file);
	/**************/
}

CuSuite*
dictionary_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_dictionary_compare_numerics);
	SUITE_ADD_TEST(suite, test_dictionary_master_table);

	return suite;
}

void
runalltests_dictionary()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= dictionary_getsuite();

	CuSuiteRun(suite);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}